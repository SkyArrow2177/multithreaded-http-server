This document describes all test cases used in testing of submissions for project 2. In most cases,
this consists of the command used to run the server, the cURL command used to make the request, and
example output from that cURL command. The cURL output is an _example only_. Your output need not be
textually identical to pass the test (due to variations in cURL versions, header ordering changes,
etc...). In particular, note that the server used for these examples returns a `Content-Length`
header, which is not required.

Each test will generally also specify the main criterion tested, but note that in addition to the
specified conditions, all tests perform basic validation of HTTP responses, including verification
that the body, if any, matches what was requested. Additionally, tests which require a `404` response
code verify that the response does not contain the actual file requested, if it exists.

Unless otherwise specified, visible tests are run against the web root under `www/vis`, and hidden tests
at `www/hidden`. For the sample server commands, it is presumed that `www` is copied into the
working directory of the server.

# Visible

## Input (zero marks)
These test cases are all worth zero marks.
* `./server v 3000`: Protocol of `v`: either exits immediately or falls back to IPv4.
* `./server 8 3000`: Protocol of `8`: either exits immediately or falls back to IPv4.
* `./server 4 <repeated-bind>`: Port already bound: exits immediately.
* `./server 4 bad`: Non-integer port: exits immediately.
* `./server 4 3000 /www/non-exist`: Nonexistent web root: exits immediately or returns error responses
  to all requests.
* `./server 4 3000 /www/vis/index.html`: File given as web root: exits immediately or returns error
  responses to all requests.

## Task 1

### `get_index`
#### Commands
* `./server 4 8000 www/vis`
* `curl --http1.0 -v http://127.0.0.1:8000/index.html`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /index.html HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 251
< Content-Type: text/html
<
<!DOCTYPE html>
<html>
    <head>
        <title>Test Page</title>
    </head>
    <body>
        <h1>This is a test page.</h1>
        <img src="/assets/image.jpg" alt="web" />
        <p><a href="/subdir/other.html">Link</a></p>
    </body>
</html>
* Closing connection 0
```
Status code must be `200`. File contents must match.

### `get_image`
#### Commands
* `./server 4 8000 www/vis`
* `curl --http1.0 -v http://127.0.0.1:8000/image.jpg -O --no-progress-meter`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /image.jpg HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 17868
< Content-Type: image/jpeg
<
{ [17868 bytes data]
* Closing connection 0
```
Status code must be `200`. File contents must match `image.jpg`.

### `get_missing`
#### Commands
* `./server 4 8000 www/vis`
* `curl --http1.0 -v http://127.0.0.1:8000/missing_file.html`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /missing_file.html HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 404 Not Found
< Content-Length: 0
<
* Closing connection 0
```
Status code must be `404`.

### `get_makefile`
#### Commands
* `./server 4 8000 www/vis`
* `curl --http1.0 -v http://127.0.0.1:8000/Makefile`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /Makefile HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 404 Not Found
< Content-Length: 0
<
* Closing connection 0
```
Status code must be `404`.

## Task 2

### `mime_index`
#### Commands
* `./server 4 8000 www/vis`
* `curl --http1.0 -v http://127.0.0.1:8000/index.html`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /index.html HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 251
< Content-Type: text/html
<
<!DOCTYPE html>
<html>
    <head>
        <title>Test Page</title>
    </head>
    <body>
        <h1>This is a test page.</h1>
        <img src="/assets/image.jpg" alt="web" />
        <p><a href="/subdir/other.html">Link</a></p>
    </body>
</html>
* Closing connection 0
```
`Content-Type` must be `text/html`.

### `mime_unknown`
#### Commands
* `./server 4 8000 www/vis`
* `curl --http1.0 -v http://127.0.0.1:8000/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 13
< Content-Type: application/octet-stream
<
Hello there.
* Closing connection 0
```
`Content-Type` must be `application/octet-stream`.

### `mime_image_subdir`
#### Commands
* `./server 4 8000 www/vis`
* `curl --http1.0 -v http://127.0.0.1:8000/assets/image.jpg -O --no-progress-meter`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /assets/image.jpg HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 17868
< Content-Type: image/jpeg
<
{ [17868 bytes data]
* Closing connection 0
```
`Content-Type` must be `image/jpeg`.

### `path_escape`
This test verifies that the server returns a `404` response for a path escape attempt for a file
that actually exists. Before running this test, make sure that the requested file does actually
exist. Create the directory `etc` under the server's working directory, and create a file `passwd`
inside `etc`.
#### Commands
* `./server 4 8000 www/vis`
* `curl --http1.0 -v http://127.0.0.1:8000/../../etc/passwd --path-as-is`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /../../etc/passwd HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 404 Not Found
< Content-Length: 0
<
* Closing connection 0
```
Status code must be `404`. Response body, if any, must not contain the contents of the requested
file.

## Task 3
All tests in this section are done over IPv6.

### `get_index_6`
#### Commands
* `./server 6 8000 www/vis`
* `curl --http1.0 -v http://[::1]:8000/index.html`
#### Example output
```
*   Trying ::1:8000...
* Connected to ::1 (::1) port 8000 (#0)
> GET /index.html HTTP/1.0
> Host: [::1]:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 251
< Content-Type: text/html
<
<!DOCTYPE html>
<html>
    <head>
        <title>Test Page</title>
    </head>
    <body>
        <h1>This is a test page.</h1>
        <img src="/assets/image.jpg" alt="web" />
        <p><a href="/subdir/other.html">Link</a></p>
    </body>
</html>
* Closing connection 0
```
Status code must be `200`.

### `mime_index_6`
#### Commands
* `./server 6 8000 www/vis`
* `curl --http1.0 -v http://[::1]:8000/index.html`
#### Example output
```
*   Trying ::1:8000...
* Connected to ::1 (::1) port 8000 (#0)
> GET /index.html HTTP/1.0
> Host: [::1]:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 251
< Content-Type: text/html
<
<!DOCTYPE html>
<html>
    <head>
        <title>Test Page</title>
    </head>
    <body>
        <h1>This is a test page.</h1>
        <img src="/assets/image.jpg" alt="web" />
        <p><a href="/subdir/other.html">Link</a></p>
    </body>
</html>
* Closing connection 0
```
`Content-Type` must be `text/html`.

## Task 4 (`stall_one`)
1. Opens two sockets, call one "slow" and one "fast".
2. Writes the first part of the request line (say, GET /ind) to slow.
3. Writes a full request GET /index.html HTTP/1.0\r\n\r\n to fast.
4. Reads and verifies the response from fast.
5. Sends the rest of the request to slow.
6. Reads and verifies the response from slow.

# Hidden

## Task 1

### `(ACK)`
For this test, the server is run with a web root which is a directory whose name is the ASCII `ACK`
character. You can set this up by running the following commands:
* `mkdir $(echo -e "\x6")`
* `cp www/hidden/project-2-test.html $(echo -e "\x6")/`

#### Commands
* `./server 4 8000 $(echo -e "\x6")`
* `curl --http1.0 -v http://127.0.0.1:8000/project-2-test.html`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /project-2-test.html HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 208
< Content-Type: text/html
<
<!DOCTYPE html>
<html>
    <head>
        <title>HTML used in hidden test</title>
    </head>
    <body>
        <h1>COMP30023</h1>
        <p>This one should be fairly easy to pass...</p>
    </body>
</html>
* Closing connection 0
```
Status code must be `200`.

### `no-headers`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v -H 'User-Agent:' -H 'Accept:' -H 'Host:' http://127.0.0.1:8000/project-2-test.html`
#### Example output
```
(same as previous)
```
Status code must be `200`.

### `rfc3986-unreserved`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_~.html`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_~.html HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 208
< Content-Type: text/html
<
<!DOCTYPE html>
<html>
    <head>
        <title>HTML used in hidden test</title>
    </head>
    <body>
        <h1>COMP30023</h1>
        <p>This one should be fairly easy to pass...</p>
    </body>
</html>
* Closing connection 0
```
Status code must be `200`.

### `2kB-request`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -H "Host:" -H "Accept:" -H "User-Agent: $(printf 'a%.s' {1..1954})" -v http://127.0.0.1:8000/request2k.html`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /request2k.html HTTP/1.0
> User-Agent: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 404 Not Found
< Content-Length: 0
<
* Closing connection 0
```
Status code must be `404`.

### `server-404`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/server`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /server HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 404 Not Found
< Content-Length: 0
<
* Closing connection 0
```
Status code must be `404`.

### `path-escape-middle`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v --path-as-is http://127.0.0.1:8000/tricky/../../../www/hidden/project-2-test.html`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /tricky/../../../www/hidden/project-2-test.html HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 404 Not Found
< Content-Length: 0
<
* Closing connection 0
```
Status code must be `404`.

### `inaccessible-file`
For this test, ensure that `www/hidden/inaccessible` is not readable. This can be done by running
`chmod 000 www/hidden/inaccessible`.
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/inaccessible`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /inaccessible HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 404 Not Found
< Content-Length: 0
<
* Closing connection 0
```
Status code must be `403` or `404`.

## Task 1 (C)
These tests provide unusual inputs and verify that the server does something sensible (i.e., doesn't
crash and responds to requests afterwards). The responses to the bad requests are not required to be
valid HTTP.

In all of these cases, the server is run with the command `./server 4 8000 www/hidden`, and after
each set of unusual inputs, a request is made with the command `curl --http1.0 -v
http://127.0.0.1:8000/one.html`. In all cases, the output of that request should look something like
this:

```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /one.html HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 1
< Content-Type: text/html
<
1
* Closing connection 0
```

* `connect-disconnect`: Connects to the server and writes nothing.
* `partial-get`: Writes various substrings of a valid GET request:
    * `G`
    * `GET`
    * `GET `
    * `GET /`
    * `GET /one.html HTTP`
    * `GET /one.html HTTP/1.0`
    * `GET /one.html HTTP/1.0\r\n`
* `invalid-header-crlf`: Writes various incomplete or invalid request lines:
    * `\r\n\r\n`
    * `G\r\n\r\n`
    * `GET\r\n\r\n`
    * `GET /\r\n\r\n`
    * `GET / HTTP/1.0 ?\r\n\r\n`
    * `G /index HTTP/1.0\r\n\r\n`
    * `GE / HTTP\r\n\r\n`
* `nonprint1`: Writes the following invalid data:
    * `<0x0>` (where `<0x0>` is the null byte).
    * `<0x1>\r\n\r\n`
    * `\\r\\n\\r\\n\r\n\r\n` (that is, the string `"\r\n\r\n"` followed by two actual CRLF
      sequences).
* `nonprint2`: Writes the following:
  `<0x1><0x2><0x3> <0x4><0x5><0x6>...<0x20><0x7f> HTTP/1.0\r\n\r\n`
* `omit-method`: Writes the following:
  `/one.html HTTP/1.0\r\n\r\n`
* `invalid-header`: Writes the following:
  `GET /one.html HTTP/1.0\r\nH??\r\n\r\n`
* `http-0.9`: Writes the following:
  `GET /one.html`
* `BREW`: Writes the following:
  `BREW /coffee HTCPCP/1.0`

## Task 2

### `mime-js`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/uom.js -O --no-progress-meter`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /uom.js HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 122354
< Content-Type: text/javascript
<
{ [65536 bytes data]
* Closing connection 0
```
`Content-Type` must be one of the following:
* `text/javascript`
* `application/javascript`
* `application/ecmascript`
* `application/x-javascript`

### `mime-css`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/v7_uom.css -O --no-progress-meter`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /v7_uom.css HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 372933
< Content-Type: text/css
<
{ [65536 bytes data]
* Closing connection 0
```
`Content-Type` must be `text/css`.

### `2G.bin`
This test uses a `2 GiB` file of random data which is not included with the test cases. It can be generated on a
Linux machine by running `head -c 2G < /dev/urandom > www/hidden/2G.bin`.

#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/2G.bin -O --no-progress-meter`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /2G.bin HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 2147483648
< Content-Type: application/octet-stream
<
{ [65536 bytes data]
* Closing connection 0
```
`Content-Type` must be `application/octet-stream`.

### `.d`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/.d -O --no-progress-meter`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /.d HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 160
< Content-Type: application/octet-stream
<
{ [160 bytes data]
* Closing connection 0
```
`Content-Type` must be `application/octet-stream`.

### `d.`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/d. -O --no-progress-meter`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /d. HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 160
< Content-Type: application/octet-stream
<
{ [160 bytes data]
* Closing connection 0
```
`Content-Type` must be `application/octet-stream`.

### `mime-lecture`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/lectures/2022-S1-WK8-LEC2-Sockets-Flow-Ctrl-revised.pdf -O --no-progress-meter`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /lectures/2022-S1-WK8-LEC2-Sockets-Flow-Ctrl-revised.pdf HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 1307069
< Content-Type: application/pdf
<
{ [65536 bytes data]
* Closing connection 0
```
`Content-Type` must be `application/octet-stream` or `application/pdf`.

### `./`
#### Commands
* `./server 4 8000 www/vis`
* `curl --http1.0 --path-as-is -v http://127.0.0.1:8000/./subdir/././other.html`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /./subdir/././other.html HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 129
< Content-Type: text/html
<
<!DOCTYPE html>
<html>
<head>
    <title>Another test page</title>
</head>
<body>
    <h1>Another test page</h1>
</body>
</html>
* Closing connection 0
```
`Content-Type` must be `text/html`.

### `dir-contains-mime`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/tricky/my.html/0 -O --no-progress-meter`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /tricky/my.html/0 HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 160
< Content-Type: application/octet-stream
<
{ [160 bytes data]
* Closing connection 0
```
`Content-Type` must be `application/octet-stream`.

### `index.html.css`
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/tricky/my.html/index.html.css`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /tricky/my.html/index.html.css HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 60
< Content-Type: text/css
<
html body > #comp30023 {
    content: 'Computer Systems';
}
* Closing connection 0
```
`Content-Type` must be `text/css`.

### `ultra-long-path`
Some setup is required for this one. Run the following commands:
```
a=$(printf 'a%.0s' {1..255})
a_last=$(printf 'a%.0s' {1..250})
url_prefix="$a/$a/$a/$a/$a/$a"
mkdir -p "www/hidden/$url_prefix"
cp www/hidden/project-2-test.html "www/hidden/$url_prefix/$a_last.html"
```
#### Commands
* `./server 4 8000 www/hidden`
* `curl --http1.0 -v http://127.0.0.1:8000/$url_prefix/$a_last.html`
#### Example output
```
*   Trying 127.0.0.1:8000...
* Connected to 127.0.0.1 (127.0.0.1) port 8000 (#0)
> GET /aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.html HTTP/1.0
> Host: 127.0.0.1:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 208
< Content-Type: text/html
<
<!DOCTYPE html>
<html>
    <head>
        <title>HTML used in hidden test</title>
    </head>
    <body>
        <h1>COMP30023</h1>
        <p>This one should be fairly easy to pass...</p>
    </body>
</html>
* Closing connection 0
```
`Content-Type` must be `text/html`.

## Task 3
All tests in this section are done over IPv6.

### `header-traversal`
#### Commands
* `./server 6 8000 www/hidden`
* `curl --http1.0 --user-agent "curl/1.0 (/../)" -v http://[::1]:8000/project-2-test.html`
#### Example output
```
*   Trying ::1:8000...
* Connected to ::1 (::1) port 8000 (#0)
> GET /project-2-test.html HTTP/1.0
> Host: [::1]:8000
> User-Agent: curl/1.0 (/../)
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 208
< Content-Type: text/html
<
<!DOCTYPE html>
<html>
    <head>
        <title>HTML used in hidden test</title>
    </head>
    <body>
        <h1>COMP30023</h1>
        <p>This one should be fairly easy to pass...</p>
    </body>
</html>
* Closing connection 0
```
Status code must be `200`.

### `header-traversal`
#### Commands
* `./server 6 8000 www/hidden`
* `curl --http1.0 -v http://[::1]:8000/.dotfile`
#### Example output
```
*   Trying ::1:8000...
* Connected to ::1 (::1) port 8000 (#0)
> GET /.dotfile HTTP/1.0
> Host: [::1]:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 404 Not Found
< Content-Length: 0
<
* Closing connection 0
```
Status code must be `404`.

### `...`
#### Commands
* `./server 6 8000 www/hidden`
* `curl --http1.0 -v http://[::1]:8000/...`
#### Example output
```
*   Trying ::1:8000...
* Connected to ::1 (::1) port 8000 (#0)
> GET /... HTTP/1.0
> Host: [::1]:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 0
< Content-Type: application/octet-stream
<
* Closing connection 0
```
`Content-Type` must be `application/octet-stream`.

### `not-path-traversal`
#### Commands
* `./server 6 8000 www/hidden/../hidden`
* `curl --http1.0 -v http://[::1]:8000/tricky/a/b/c/d/e/f/a../.../..b/c.../connect.jpg -O --no-progress-meter`
#### Example output
```
*   Trying ::1:8000...
* Connected to ::1 (::1) port 8000 (#0)
> GET /tricky/a/b/c/d/e/f/a../.../..b/c.../connect.jpg HTTP/1.0
> Host: [::1]:8000
> User-Agent: curl/7.83.1
> Accept: */*
>
* Mark bundle as not supporting multiuse
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< Content-Length: 228293
< Content-Type: image/jpeg
<
{ [32768 bytes data]
* Closing connection 0
```
`Content-Type` must be `image/jpeg`.

## Task 4
* `long_parallel-2`: Makes two simultaneous rate-limited (to a tenth of the file size per second)
  requests for a `64 MiB` file. Both downloads must make simultaenous progress.
* `long_parallel-5`: Same as above, with five simultaneous requests.
* `long_short`: Makes one slow rate-limited request as above, and many small requests at the same
  time. All downloads must make progress at the same time.
* `stall_extended`: Makes a number of simultaneous requests for `/2GB_zeros.bin`, one of which
  disconnects midway through, one of which stalls twice (after writing `G` and after `ET /`, before
  eventually writing the full request), and the third of which simply writes the whole request. The
  server must:
    * Not respond prematurely to the stalled request.
    * Not crash when the first socket is disconnected.
    * Return the correct response (full `2 GB` of zeros) on both the stalled and simple sockets.

  Since this test takes a while, it is only run if the submission is determined to actually be able
  to serve the requested file (via the plain `2GB_zeros.bin` test).

## Extras
All of the tests in this section are worth zero marks. Tests marked (C) are run like the
so-annotated tests in task 1.
* `2GB_zeros.bin`: `/2GB_zeros.bin` (which is a `2 GB` file of zeros) returns `Content-Type` of
  `application/octet-stream`. `stall_extended` is not run if this test fails.
* `extra-./server 4 3000 /www/hidden/inaccessible-dir`: Web root is inaccessible. Server exits
  immediately or returns failure response to all requests.
* `extra-path-escape-start`: `/..` returns status code `404`.
* `extra-!`: `/tricky/my.html/!` returns `Content-Type` of `application/octet-stream`.
* `extra-nested-404`: `/nested/nested/nested/nested/project-2-test.html` returns status code `404`.
* `extra-cr&lf` (C): Makes the following requests:
    * `GET /one.html HTTP/1.0\r\r`
    * `GET /one.html HTTP/1.0\n\n`
* `extra-not-/-uri`: Makes the following request:
    * `GET one.html HTTP/1.0\r\n\r\n`
* `extra-missing-uri`: Makes the following request:
    * `GET HTTP/1.0`
* `extra-directory`: Requests the following paths, which are directories:
    * `/lectures`
    * `/tricky/my.html`
    * And also `/project-2-test.html`, which is not.
* `extra-directory-escape-end`: Requests the following:
    * `/tricky/..`
    * `/project-2-test.html`
