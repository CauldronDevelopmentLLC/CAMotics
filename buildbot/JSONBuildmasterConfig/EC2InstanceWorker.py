import time
import boto3

from twisted.internet import threads, defer
from twisted.python import log

from buildbot.worker import AbstractLatentWorker
from buildbot.interfaces import LatentWorkerFailedToSubstantiate


class EC2InstanceWorker(AbstractLatentWorker):
  def __init__(self, name, password, instance_id, aws_region = None,
               aws_access_id = None, aws_secret_key = None,
               max_pending_wait = 15, max_running_wait = 60, **kwargs):
    super().__init__(name, password, **kwargs)

    self.instance_id = instance_id
    self.instance = None
    self.max_pending_wait = max_pending_wait
    self.max_running_wait = max_running_wait

    # Create AWS session
    self.session = boto3.Session(region_name = aws_region,
                                 aws_access_key_id = aws_access_id,
                                 aws_secret_access_key = aws_secret_key)


  def _get_state(self): return self.instance.state['Name']


  def _wait_for(self, wait_states, allowed_states, secs):
    while True:
      state = self._get_state()

      if allowed_states and state not in allowed_states: break
      if wait_states and state in wait_states: break

      time.sleep(1)
      secs -= 1
      if secs <= 0: break

      self.instance.reload()


  def _start_instance(self):
    # Connect to instance
    if self.instance is None:
      ec2 = self.session.resource('ec2')
      self.instance = list(
        ec2.instances.filter(InstanceIds = [self.instance_id]))[0]

    # Start instance
    self.instance.start()
    self._wait_for(['pending', 'running'], None, self.max_pending_wait)
    self._wait_for(None, ['pending'], self.max_running_wait)

    if self._get_state() == 'running': return self.instance.id

    self.failed_to_start(self.instance.id, self._get_state())


  def start_instance(self, build):
    return threads.deferToThread(self._start_instance)


  def stop_instance(self, fast = False):
    if self.instance is not None: self.instance.stop()
    return defer.succeed(None)
