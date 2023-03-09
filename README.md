# Multithreaded HTTP/1.0 server in C

## Features

- Supports HTTP/1.0 and HTTP/1.1 GET requests for static files.

  - Implements a subset of the HTTP/1.0 specification.
  - HTTP/1.1 is accepted in the request line for convenience as modern browsers
    default to this version string.
  - Only the request line needs to be well-formed.

    - This server does not require nor read any further request headers.

    - Therefore, when reading the GET request, as long as a single `CRLF` is
      encountered at the end of a valid request line, the request will be
      immediately accepted and processed.

    - This server does not wait until a 2x CRLF is reached, which is the end of
      a HTTP request given by RFC 1945.

  - Handles multi-packet requests: GET requests can be > 2KB in size
    (configurable), which is larger than a typical MTU.

- Handles multiple simultaneous downloads up to the process thread limit through
  the use of a dedicated POSIX thread per request.

  - To improve on this further, a thread pool could be used to avoid the
    per-process thread limit. Implementing green threads would be a further
    challenge!

- Incrementally parses the request line by tracking the last-completed stage in
  per-request state machine, improving request processing performance.

- Supports large files (larger than 2 GiB) - file sizes are limited only by the
  maximum supported by the file system (ext4: 16 TiB).

- Performs zero-copy transfers from a file to the response socket using
  `sendfile`, reaching local download speeds to in excess of 3 GB/s on large
  files, saturating the read speed of a PCIe 3.0 NVMe SSD where the test files
  are located!

- Protects against path escape attacks involving `/../` or trailing `/..`, while
  also accepting and processing potentially-legitimate paths such as
  `/folder../`

- Supports extremely long path names up to the system's max limits (255 chars
  per file folder, 4096 chars for the entire path on Linux).

- Safe signal handling with correct use of `sig_atomic_t` - for example,
  keyboard interrupts sent through `netcat` will not remotely terminate the
  running server.

- Timeouts on partial idle requests.

- Runs on amd64 Linux (any 64-bit CPU & Linux distribution should work,
  including arm64 CPUs).

- Tests written in Python 3 (version 3.7+ required).

## Building

1. Clone this repository.
2. `make`

## Testing

### Test results

A comprehensive list of test results is attached in `proj2-final.txt`. Note that
these results and the comments are directed at commit SHA
`8791d22ab8e746653b06b36a5b8244dc970e824d` - I've added a readme and improved
the Makefile since, with no changes to the server functionality.

Clearly, these tests that I wrote weren't for naught!

### Netcat tests

These tests are designed to test the returned status of both well-formed and
intentionally malformed requests, which should return 400 Bad Request, as well
as the exact size of the response.

They also test that given these inputs, the server does not crash, segfault,
leak memory, or otherwise do anything undesirable.

To run these tests:

1. Build the project as above.
2. Install `nclib`: `pip3 install nclib`.
3. In `test_netcat_custom.py`, configure the `SKIP_TIMEOUT` flag. If `True`,
   tests which expect a timeout outcome (such as sending an empty message) will
   not be run, reducing the total test duration.
4. Run the tests: `python3 test_netcat_custom.py`

### Request tests

These test well-formed HTTP/1.0 or HTTP/1.1 GET requests (which also conform to
the subset of the spec supported by this server) against the server. Key areas
of focus are

- Correct `Content-Length` and `Content-Type` header values
- Correct actual size of response
- Large file support
- Path escape protection.

To run these tests:

1. Build the project as above.
2. Install `requests`: `pip3 install requests`.
3. In `test_valid_requests.py`, configure the `SKIP_LARGE_FILES` flag. This flag
   is provided in situations where it is cumbersome to test with > 2 GiB files,
   for example when copying a large file to a remote development environment
   would take too long, be too expensive, or not possible due to a lack of
   space.
4. Run the tests: `python3 test_valid_requests.py`
