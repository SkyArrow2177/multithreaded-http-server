from dataclasses import dataclass
from email import header
from pprint import pprint
import signal
from typing import Optional
import unittest
import subprocess
import subprocess
import time
import requests

IP_VER: int = 4
PORT: int = 9000
ROOT: str = "./www1"

MIME_HTML = "text/html"
MIME_JPEG = "image/jpeg"
MIME_CSS = "text/css"
MIME_JS = "text/javascript"
MIME_DEF = "application/octet-stream"

HTTP_200 = 200
HTTP_400 = 400
HTTP_404 = 404


@dataclass
class Request:
    path: str
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
                "/home/k/compsys/comp30023-2022-project-2/server",
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

    def test_aaa(self):
        req = Request(
            path="/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
            code=HTTP_200,
            size=13,
            mime=MIME_DEF,
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

    def test_gif(self):
        req = Request(
            path="/assets/office.gif",
            code=HTTP_200,
            size=8752538,
            mime=MIME_DEF,
        )
        self.valid_helper(req)

    def valid_helper(self, req: Request):
        s = requests.Session()
        req2 = requests.Request(method="GET", url=req.path)
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
