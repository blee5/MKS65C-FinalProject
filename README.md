# ganache

ganache is a simple static web server written in C for the Systems Level Programming class at Stuyvesant High School. It (attempts to) implement HTTP/1.1.

## How to use

Simply run through the command line:
`ganache`

By default, it will open port 80 (HTTP) and use the current working directory as the root directory for the site.

Command line options that are currently supported are:
```
-r [path]    set root directory of website to path
-p [port]    specify which port should be used
```
