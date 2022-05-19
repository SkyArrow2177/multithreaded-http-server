# Unit tests for well-formed requests.

from dataclasses import dataclass
import signal
from typing import Optional
import unittest
import subprocess
import subprocess
import time
import requests

from test_env import *

SKIP_LARGE_FILES = True


@dataclass
class Request:
    # Requested path
    path: str
    # Expected values
    code: int
    size: int
    mime: Optional[str]

    def __post_init__(self):
        addr = "[::1]" if IP_VER == 6 else "127.0.0.1"
        self.path = "http://" + addr + ":" + str(PORT) + self.path


class TestValidRequests(unittest.TestCase):
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

    def test_index_html(self):
        req = Request(
            path="/index.html",
            code=HTTP_200,
            size=251,
            mime=MIME_HTML,
        )
        self.valid_helper(req)

    def test_dir_image(self):
        req = Request(
            path="/assets/image.jpg",
            code=HTTP_200,
            size=17868,
            mime=MIME_JPEG,
        )
        self.valid_helper(req)

    def test_css(self):
        req = Request(
            path="/assets/styles.css",
            code=HTTP_200,
            size=82,
            mime=MIME_CSS,
        )
        self.valid_helper(req)

    def test_js(self):
        req = Request(
            path="/assets/script.js",
            code=HTTP_200,
            size=420,
            mime=MIME_JS,
        )
        self.valid_helper(req)

    def test_gif(self):
        req = Request(
            path="/assets/bird.gif",
            code=HTTP_200,
            size=17306121,
            mime=MIME_DEF,
        )
        self.valid_helper(req)

    @unittest.skipIf(SKIP_LARGE_FILES, "")
    def test_large_1(self):
        req = Request(
            path="/suisei.mp4",
            code=HTTP_200,
            size=1362886656,
            mime=MIME_DEF,
        )
        self.valid_helper(req)

    @unittest.skipIf(SKIP_LARGE_FILES, "")
    def test_large_2(self):
        req = Request(
            path="/suisei2.mp4",
            code=HTTP_200,
            size=2146088452,
            mime=MIME_DEF,
        )
        self.valid_helper(req)

    def test_multi_dot_file(self):
        req = Request(
            path="/telephone.clang-format.css.html.js.jpg",
            code=HTTP_200,
            size=4871337,
            mime=MIME_JPEG,
        )
        self.valid_helper(req)

    def test_empty_before_extension(self):
        req = Request(
            path="/special/.css",
            code=HTTP_200,
            size=91,
            mime=MIME_CSS,
        )
        self.valid_helper(req)

    def test_actually_application_octet_stream(self):
        req = Request(
            path="/special/html",
            code=HTTP_200,
            size=367,
            mime=MIME_DEF,
        )
        self.valid_helper(req)

    def test_actually_application_octet_stream_2(self):
        req = Request(
            path="/special/.__html",
            code=HTTP_200,
            size=375,
            mime=MIME_DEF,
        )
        self.valid_helper(req)

    def test_file_404(self):
        req = Request(
            path="/assets/bababoowee.js",
            code=HTTP_404,
            size=0,
            mime=None,
        )
        self.valid_helper(req)

    def test_dir_exists_404(self):
        req = Request(
            path="/assets",
            code=HTTP_404,
            size=0,
            mime=None,
        )
        self.valid_helper(req)

    def test_dir_not_exist_404(self):
        req = Request(
            path="/assets2305783290",
            code=HTTP_404,
            size=0,
            mime=None,
        )
        self.valid_helper(req)

    def test_path_escape_outside_in(self):
        req = Request(
            path="/../www1/subdir/other.html",
            code=HTTP_404,
            size=0,
            mime=None,
        )
        self.valid_helper(req)

    def test_path_escape_inside(self):
        req = Request(
            path="/subdir/../assets/script.js",
            code=HTTP_404,
            size=0,
            mime=None,
        )
        self.valid_helper(req)

    def test_path_escape_ending(self):
        req = Request(
            path="/..",
            code=HTTP_404,
            size=0,
            mime=None,
        )
        self.valid_helper(req)

    def test_path_escape_ending_mix(self):
        req = Request(
            path="/special/..../..",
            code=HTTP_404,
            size=0,
            mime=None,
        )
        self.valid_helper(req)

    def test_path_escape_middle_mix(self):
        req = Request(
            path="/assets/../special/..../example.html",
            code=HTTP_404,
            size=0,
            mime=None,
        )
        self.valid_helper(req)

    def test_do_not_path_escape_file(self):
        req = Request(
            path="/special/...",
            code=HTTP_200,
            size=60,
            mime=MIME_DEF,
        )
        self.valid_helper(req)

    def test_do_not_path_escape_dir(self):
        req = Request(
            path="/special/..../example.html",
            code=HTTP_200,
            size=1447,
            mime=MIME_HTML,
        )
        self.valid_helper(req)

    def test_do_not_path_escape_dotception(self):
        req = Request(
            path="/special/..../...",
            code=HTTP_200,
            size=12,
            mime=MIME_DEF,
        )
        self.valid_helper(req)

    def test_aaa(self):
        req = Request(
            path="/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
            code=HTTP_200,
            size=13,
            mime=MIME_DEF,
        )
        self.valid_helper(req)

    def test_long_path_200(self):
        path_string = "/zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz/yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv/uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu/tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt/file"
        req = Request(
            path=path_string,
            code=HTTP_200,
            size=192,
            mime=MIME_DEF,
        )
        self.valid_helper(req)

    def test_post_invalid_400(self):
        req = Request(
            path="/api/v1/playMusic",
            code=HTTP_400,
            size=0,
            mime=None,
        )
        self.valid_helper(req, "POST")

    def valid_helper(self, req: Request, method: str = "GET"):
        """Prepares requests for testing, ensuring that path escapes remain and are not normalized."""
        s = requests.Session()
        req2 = requests.Request(method=method, url=req.path)
        req3 = req2.prepare()
        req3.url = req.path
        r = s.send(req3, verify=False)
        # print(r.status_code)
        self.assertEqual(req.code, r.status_code)
        self.assertEqual(str(req.size), r.headers["content-length"])
        self.assertEqual(req.size, len(r.content))
        if r.status_code == HTTP_200:
            self.assertEqual(req.mime, r.headers["content-type"])

    @classmethod
    def tearDownClass(cls) -> None:
        cls.server.send_signal(signal.SIGINT)


if __name__ == "__main__":
    unittest.main()
