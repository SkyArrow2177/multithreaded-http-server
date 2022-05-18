import signal
import subprocess
import nclib
import time
import unittest

from test_env import *

FILE: str = "/special/..../example.html"
CRLF: str = "\r\n"
SP: str = " "
HTTP_VER = "HTTP/1.0"
GET = "GET"
POST = "POST"
FULL = "GET /special/..../example.html HTTP/1.0\r\n\r\n"
LONG = "GET /special/..../example.html HTTP/1.0\r\nHost: abcdef.xyz\r\n\r\n"

SKIP_TIMEOUT = False


def ss():
    time.sleep(0.3)


def ss_timeout():
    time.sleep(12)


def ncstart():
    return nclib.Netcat(("localhost", PORT), verbose=True)


class TestNetcat(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.server = subprocess.Popen(
            [
                SERVER,
                str(IP_VER),
                str(PORT),
                ROOT,
            ]
        )
        time.sleep(0.01)

    @classmethod
    def tearDownClass(cls) -> None:
        cls.server.send_signal(signal.SIGINT)

    def status_in_header(self, status, response):
        self.assertTrue(status in response.decode("ascii"))

    @unittest.skipIf(SKIP_TIMEOUT, "")
    def test_0_empty_timeout(self):
        nc = ncstart()
        nc.send("")

        res = nc.recv()
        self.status_in_header(HTTP_400_TEXT, res)

        nc.close()

    def test_1_ok(self):
        nc = ncstart()
        nc.send(FULL)

        res = nc.recv()
        self.status_in_header(HTTP_200_TEXT, res)

        nc.recv()
        nc.close()

    def test_2_ok(self):
        nc = ncstart()
        nc.send(FULL[:2])
        ss()
        nc.send(FULL[2:])

        res = nc.recv()
        self.status_in_header(HTTP_200_TEXT, res)

        nc.recv()
        nc.close()

    @unittest.skipIf(SKIP_TIMEOUT, "")
    def test_3_send_only_prefix_timeout(self):
        nc = ncstart()
        nc.send(FULL[:2])

        res = nc.recv()
        self.status_in_header(HTTP_400_TEXT, res)

        nc.close()

    def test_4_send_nothing_first(self):
        nc = ncstart()
        nc.send("")
        ss()
        nc.send(FULL)

        res = nc.recv()
        self.status_in_header(HTTP_200_TEXT, res)

        nc.recv()
        nc.close()

    def test_5_send_GE_then_before_space_then_rest(self):
        nc = ncstart()
        nc.send(FULL[:2])
        ss()
        nc.send(FULL[2:15])
        ss()
        nc.send(FULL[15:])

        res = nc.recv()
        self.status_in_header(HTTP_200_TEXT, res)

        nc.recv()
        nc.close()

    def test_6_send_GE_then_space_then_rest(self):
        nc = ncstart()
        nc.send(FULL[:2])
        ss()
        nc.send(FULL[2:31])
        ss()
        nc.send(FULL[31:])

        res = nc.recv()
        self.status_in_header(HTTP_200_TEXT, res)

        nc.recv()
        nc.close()

    def test_7_send_GE_then_CR_then_rest_LF(self):
        nc = ncstart()
        nc.send(FULL[:2])
        ss()
        nc.send(FULL[2:41])
        ss()
        nc.send(FULL[41:])

        res = nc.recv()
        self.status_in_header(HTTP_200_TEXT, res)

        nc.recv()
        nc.close()

    def test_8_garbage(self):
        nc = ncstart()
        nc.send("qwertyuiop")

        res = nc.recv()
        self.status_in_header(HTTP_400_TEXT, res)

        nc.close()

    def test_9_invalid_prefix_and_slash(self):
        nc = ncstart()
        nc.send("GET #")
        ss()
        nc.send("speci")  # placing recv() before or after this will give the 400.

        res = nc.recv()
        self.status_in_header(HTTP_400_TEXT, res)

        nc.close()

    def test_a0_garbage_prefix(self):
        nc = ncstart()
        nc.send("G")
        ss()
        nc.send("E ")

        res = nc.recv()
        self.status_in_header(HTTP_400_TEXT, res)

        nc.close()

    def test_a1_send_GE_then_space_then_not_HTTP(self):
        nc = ncstart()
        nc.send(FULL[:2])
        ss()
        nc.send(FULL[2:31])
        ss()
        nc.send("HAHA")

        res = nc.recv()
        self.status_in_header(HTTP_400_TEXT, res)

        nc.close()

    def test_a2_send_GE_then_HTTP_then_not_CR(self):
        nc = ncstart()
        nc.send(FULL[:2])
        ss()
        nc.send(FULL[2:31])
        ss()
        nc.send("HTTP/1.1 \r\n\r\n")

        res = nc.recv()
        self.status_in_header(HTTP_400_TEXT, res)

        nc.close()

    def test_a3_send_GE_then_space_then_another_space_400(self):
        nc = ncstart()
        nc.send(FULL[:2])
        ss()
        nc.send(FULL[2:31])
        ss()
        nc.send(" HTTP/1.1\r\n\r\n")

        res = nc.recv()
        self.status_in_header(HTTP_400_TEXT, res)

        nc.close()

    def test_a4_send_GE_then_CR_then_not_LF(self):
        nc = ncstart()
        nc.send(FULL[:2])
        ss()
        nc.send(FULL[2:31])
        ss()
        nc.send("HTTP/1.1\r \n\r\n")

        res = nc.recv()
        self.status_in_header(HTTP_400_TEXT, res)

        nc.close()

    @unittest.skipIf(SKIP_TIMEOUT, "")
    def test_a5_200_TO_CHANGE_TO_400_TIMEOUT(self):
        nc = ncstart()
        nc.send(LONG[:2])
        ss()
        nc.send(LONG[2:31])
        ss()
        nc.send(LONG[31:51])

        res = nc.recv()
        self.status_in_header(HTTP_400_TEXT, res)

        nc.close()

    def test_a6_long_200(self):
        nc = ncstart()
        nc.send(LONG[:2])
        ss()
        nc.send(LONG[2:31])
        ss()
        nc.send(LONG[31:])

        res = nc.recv()
        self.status_in_header(HTTP_200_TEXT, res)

        nc.recv()
        nc.close()


if __name__ == "__main__":
    unittest.main()
