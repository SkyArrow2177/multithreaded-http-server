import nclib
import requests
import unittest
import http.client

PORT = 8000


def main():
    nc = nclib.Netcat(("localhost", PORT), verbose=True)
    out = nc.send("GET /comp30023-2022-project-2/.clang-format")
    print(out)

    out = nc.send(" HTTP/1.1\r\n")
    print(out)

    response = nc.recv()
    print(str(response))


if __name__ == "__main__":
    main()