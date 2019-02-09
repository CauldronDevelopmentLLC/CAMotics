# camotics-build
Configuration files for building [CAMotics][0] on multiple platforms using
[dockbot][1].

[0]: https://github.com/CauldronDevelopmentLLC/camotics
[1]: https://github.com/CauldronDevelopmentLLC/dockbot


# Release procedure
Using code signing key for Windows.

```
dockbot publish --key <key> -v

rsync -av --progress releases/alpha/* root@camotics.org:/var/www/camotics.org/http/releases/public/

github-release upload -c -v <version> -m release -u jcoffland -o CauldronDevelopmentLLC -r CAMotics $(find releases/alpha/release/camotics/ -name 'camotics*<version>*.*')
github-release upload -c -v <version> -m debug -u jcoffland -o CauldronDevelopmentLLC -r CAMotics $(find releases/alpha/debug/camotics/ -name 'camotics*<version>*.*')
```

Replace ``<key>`` and ``<version>`` above.  Then update version in
``camotics/web/config.json`` and possibly ``jade\mixins.jade``, update banner
in ``web/jade/header.jade``, regenerate Webpages and publish.
