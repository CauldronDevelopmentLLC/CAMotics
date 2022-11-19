#!/usr/bin/env python3

from buildbot.plugins import *
from buildbot.interfaces import IRenderable
from buildbot.process.results import SUCCESS, FAILURE, SKIPPED

from JSONBuildmasterConfig.EC2InstanceWorker import EC2InstanceWorker

import collections
import json
import os
import sys
from copy import deepcopy
import shlex
import re
import mimetypes


class StaticDashboard:
  def __init__(self, root): self.root = root


  def __call__(self, env, start):
    path = '%s/%s' % (self.root, env['PATH_INFO'])

    if not os.path.exists(path):
      start('404 Not Found', [('Content-Type', 'text/plain')])
      yield b'Not found'

    else:
      mtype = mimetypes.guess_type(path)
      start('200 OK', [('Content-Type', mtype[0])])
      with open(path, 'rb') as f: yield f.read()


def update(d, u):
  for k, v in u.items():
    if isinstance(v, collections.abc.Mapping):
      r = update(d.get(k, {}), v)
      d[k] = r

    else: d[k] = u[k]

  return d


def if_os(os, t, f):
  @util.renderer
  def rend(props, os):
    return t if props.getProperty('os') == os else f

  return rend.withArgs(os)


def _render_env(props, env, translate_var_refs):
  result = {}
  is_win = props.getProperty('os') == 'win'
  pat = re.compile(r'\${([0-9a-zA-Z_]*)}')

  # Translate to OS specific environment variable reference
  def subst(m): return ('%%%s%%' if is_win else '$%s') % m.group(1)
  def render(v):
    return v.getRenderingFor(props) if isinstance(v, IRenderable) else str(v)

  for name, value in env.items():
    name = render(name)

    # Combine dicts with '=' and space
    if isinstance(value, collections.abc.Mapping):
      value = ' '.join(
        ['%s=%s' % (k, shlex.quote(render(v))) for k, v in value.items()])

    # Combine lists with OS specific PATH separator
    elif isinstance(value, list):
      value = (';' if is_win else ':').join([render(v) for v in value])

    elif isinstance(value, bool):
      if not value: continue # False means do not set the variable
      value = ''

    else: value = render(value)

    # Subsitute variable references
    if translate_var_refs:
      if value is not None: value = pat.sub(subst, value)

    result[name] = value

  return result


@util.renderer
def render_env(props, env): return _render_env(props, env, False)


@util.renderer
def render_env_script(props, env):
  env = _render_env(props, env, True)
  is_win = props.getProperty('os') == 'win'
  fmt = '@set %s=%s' if is_win else 'export %s=%s'
  eol = '\r\n' if is_win else '\n'

  return eol.join([fmt % (k, v) for k, v in env.items()]) + eol


@util.renderer
def _home_path(props, project):
  builddir = props.getProperty('builddir')
  if props.getProperty('os') == 'win': builddir = builddir.replace('\\', '/')
  return builddir + '/' + project


def home_path(project): return _home_path.withArgs(project)


def mode_matrix_build(dims, modes):
  if not len(dims): return

  for mode in dims[0]:
    mode_env = modes.get(mode, {})
    count = 0

    for name, env in mode_matrix_build(dims[1:], modes):
      count += 1
      yield '%s-%s' % (mode, name), update(deepcopy(env), mode_env)

    if not count: yield mode, mode_env


def resolve_modes(names, modes, matrices):
  for name in names:
    if len(name) and name[0] == '#':
      for name in matrices.get(name[1:], []): yield name
    else: yield name


class JSONBuildmasterConfig:
  def __init__(self, verbose = True):
    self.verbose = verbose
    self.root = {}
    self.projects = {}
    self.config = {
      'workers': [],
      'protocols': {},
      'change_source': changes.PBChangeSource(),
      'schedulers': [],
      'builders': [],
      'services': [],
      'www': {
        'port': 8080,
        'plugins': {'waterfall_view': {}, 'console_view': {}}
      },
      'protocols': {'pb': {'port': 9989}},
      'db': {'db_url': 'sqlite:///state.sqlite'}
    }


  def create_source_step(self, info, project):
    repo_type = info.get('type', 'github')

    if repo_type in ('git', 'github'):
      if 'url' in info: url = info['url']
      else:
        name     = info.get('name',     project)
        protocol = info.get('protocol', 'ssh')
        user     = info.get('user',     'git')
        domain   = info.get('domain',   'github.com')

        if protocol == 'ssh': url = '%s@%s:' % (user, domain)
        else: url = protocol + '://%s/' % domain

        url += '%s/%s.git' % (info['org'], name)

      return steps.Git(repourl = url, mode = 'incremental',
                       branch = info.get('branch', None),
                       submodules = info.get('submodules', False),
                       workdir = project, name = project + ' git')

    else: raise Exception('Unsupported repo type %s' % repo_type)


  def add_build_step(self, factory, project, step, mode, env):
    # Get step config
    config = self.projects[project].get("steps", {}).get(step, {})
    if not config.get("enable", False): return

    # Get step command
    cmd = list(config.get('command', []))
    if not cmd:
      raise Exception('Empty command for project', project, 'step', step)

    if self.verbose: print('      Adding step', step, cmd)

    # Build step
    factory.addStep(steps.Compile(command = cmd, env = env, workdir = project,
                                  name = '%s %s' % (project, step)))

    if config.get('upload', False):
      # Get file name
      factory.addStep(steps.SetPropertyFromCommand(
        command = [if_os('win', 'type', 'cat'), step + '.txt'],
        property = 'package_name', workdir = project,
        name = '%s %s get file name' % (project, step)))

      # Set properties
      factory.addStep(steps.SetProperties(properties = dict(
        project = project, mode = mode), name = 'Set props'))

      # Delete previous builds
      dst_dir = 'builds/%(prop:project)s/%(prop:workername)s/%(prop:mode)s'
      cmd = ['rm', '-rf', util.Interpolate(dst_dir)]
      factory.addStep(steps.MasterShellCommand
                      (command = cmd, name = 'Delete old'))

      # Upload file to master
      src = util.Interpolate('%(prop:package_name)s')
      dst = util.Interpolate(dst_dir + '/%(prop:package_name)s')
      factory.addStep(steps.FileUpload(
        workersrc = src, masterdest = dst, workdir = project,
        name = '%s %s upload' % (project, step)))


  def add_project(self, factory, project, mode, env, visited):
    if project in visited: return

    if not project in self.projects:
      raise Exception('Config for project %s not found' % project)

    config = self.projects[project]

    # Add dependent projects
    for dep in config.get('deps', []):
      self.add_project(factory, dep, mode, env, visited)

    if project in visited:
      raise Exception('Circular dependency on project %s' % project)

    if self.verbose: print('    Adding project', project)
    visited.add(project)

    # Get project environment (Must be after proj deps)
    proj_env = render_env.withArgs(deepcopy(env))

    # Get source
    if 'repo' in config:
      factory.addStep(self.create_source_step(config['repo'], project))

    # Add build steps
    for step in config.get("steps", {}):
      self.add_build_step(factory, project, step, mode, proj_env)

    # Set home variable
    env[project.upper().replace('-', '_') + '_HOME'] = home_path(project)


  def add_builder(self, worker_name, build, mode, builder_config, env):
    builder_name = '%s-%s-%s' % (worker_name, build, mode)
    if self.verbose: print('  Adding builder', builder_name)

    # Make builder
    factory = util.BuildFactory()

    # Save environment on worker
    factory.addStep(steps.StringDownload(
      render_env_script.withArgs(env),
      workerdest = if_os('win', 'env.bat', 'env'), workdir = '.', name = 'env'))

    # Add projects and their dependencies
    visited = set()
    for project in builder_config.get('projects', []):
      self.add_project(factory, project, mode, env, visited)

    # Add builder config
    self.config['builders'].append(util.BuilderConfig(
      name = builder_name, workernames = [worker_name], factory = factory))

    # Add scheduler
    self.config['schedulers'].append(schedulers.ForceScheduler(
      name = builder_name, builderNames = [builder_name]))


  def load_worker(self, worker_name, worker_path):
    if self.verbose: print('Loading worker', worker_name)

    # Load worker config
    worker_config = deepcopy(self.root.get('worker', {}))
    worker_config = update(worker_config, json.load(open(worker_path, 'r')))

    if not 'password' in worker_config:
      raise Exception('Worker %s missing password' % worker_name)
    passwd      = worker_config['password']
    concurrency = worker_config.get('concurrency', 2)
    props       = worker_config.get('props', {})

    # Create Worker
    Worker = worker.Worker
    kwargs = dict(max_builds = concurrency, properties = props)

    ec2 = worker_config.get('ec2', {})
    if 'instance_id' in ec2:
      kwargs.update(ec2)
      Worker = EC2InstanceWorker

    self.config['workers'].append(Worker(worker_name, passwd, **kwargs))

    worker_env     = worker_config.get('env',    {})
    worker_modes   = worker_config.get('modes',  {})
    build_defaults = worker_config.get('build',  {})
    worker_builds  = worker_config.get('builds', {})

    # Load mode matrices
    mode_matrices = {}
    for mat, dims in worker_config.get('mode_matrices', {}).items():
      modes = {name: env for name, env in mode_matrix_build(dims, worker_modes)}
      mode_matrices[mat] = modes.keys()
      update(worker_modes, modes)

    # Load builders
    for build in worker_builds:
      builder_config = update(deepcopy(build_defaults), worker_builds[build])
      modes = builder_config.get('modes', [])
      modes = resolve_modes(modes, worker_modes, mode_matrices)

      for mode in modes:
        env = update(deepcopy(worker_env), worker_modes.get(mode, {}))
        self.add_builder(worker_name, build, mode, builder_config, env)


  def load_root_configs(self, configs):
    # Load root configs
    for config_path in configs:
      if os.path.exists(config_path):
        with open(config_path, 'r') as f:
          self.root = update(self.root, json.load(f))
      else: print('Warning "%s" not found' % config_path)

    # Configure build master
    update(self.config, self.root.get('master', {}))


  def load_projects(self):
    self.projects = self.root.get('projects', {})

    # Apply project defaults
    defaults = self.root.get('project', {})

    for project in self.projects:
      self.projects[project] = \
        update(deepcopy(defaults), self.projects[project])

      if self.verbose: print('Loaded project', project)


  def load_workers(self, worker_path):
    for name in os.listdir(worker_path):
      path = '%s/%s/build.json' % (worker_path, name)
      if os.path.exists(path): self.load_worker(name, path)


  def load_dashboards(self):
    if not 'static_dashboards' in self.config: return

    dash_configs = self.config['static_dashboards']
    del self.config['static_dashboards']

    dashboards = []
    for config in dash_configs:
      config['app'] = StaticDashboard(config['path'])
      if not 'icon' in config: config['icon'] = 'cog'
      if not 'caption' in config:
        config['caption'] = '%s View' % config['name'].capitalize()

      del config['path']
      dashboards.append(config)

    self.config['www']['plugins']['wsgi_dashboards'] = dashboards


  def load(self, configs = ['build.json', 'local.json'],
           worker_path = 'workers'):
    self.load_root_configs(configs)
    self.load_projects()
    self.load_workers(worker_path)
    self.load_dashboards()

    return self.config
