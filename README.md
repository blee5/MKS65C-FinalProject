# ganache

ganache is a simple static web server written in C for the Systems Level Programming class at Stuyvesant High School. It is supposed to implement HTTP/1.1, with emphasis on "supposed to."

To run the server, place your files in the /www directory.

## Bugs and shortcomings
* Only supports GET requests
* Only replies with two status codes - 200 and 404
    * Any other cases will fail silenty, have something else happen instead, or probably cause the server to crash.
* Basically no error handling; any malformed requests will cause undefined behaviors
* Opening a directory without a trailing slash (such as `example.com/test`) will be considered opening a file and causes issues
* Sometimes random "Connection reset by peer" errors occur, but doesn't seem to affect usability.

## How to use

To launch the server, simply run through the command line:
`ganache`

By default, it will open port 80 (HTTP); the port can be changed with a command line argument. For example, `ganache 3003` will open port 3003.
