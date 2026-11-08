"""Exercise release downloads against a real local HTTP server."""

import hashlib
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import io
from pathlib import Path
import sys
import tarfile
import tempfile
import threading
import unittest
from unittest.mock import patch
from urllib.error import HTTPError, URLError
import urllib.request

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'launcher'))
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'diagnostics'))
import prepare_innoextract
import MacOSRuntime


class DownloadTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        archive = io.BytesIO()
        with tarfile.open(fileobj=archive, mode='w:gz') as bundle:
            notice = b'Test dependency license\n'
            info = tarfile.TarInfo('fixture/LICENSE')
            info.size = len(notice)
            bundle.addfile(info, io.BytesIO(notice))
        self.payload = archive.getvalue()
        self.digest = hashlib.sha256(self.payload).hexdigest()
        self.plan = []
        self.requests = 0
        owner = self

        class Handler(BaseHTTPRequestHandler):
            def do_GET(self):
                owner.requests += 1
                status = owner.plan.pop(0) if owner.plan else 200
                self.send_response(200 if status == 'truncated' else status)
                if status in (200, 'truncated'):
                    self.send_header('Content-Length', str(len(owner.payload)))
                elif status == 429:
                    self.send_header('Retry-After', '3')
                self.end_headers()
                if status == 'truncated':
                    self.wfile.write(owner.payload[:len(owner.payload) // 2])
                    self.close_connection = True
                elif status == 200:
                    self.wfile.write(owner.payload)

            def log_message(self, *_):
                pass

        self.server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
        self.thread = threading.Thread(
            target=lambda: self.server.serve_forever(poll_interval=.01), daemon=True)
        self.thread.start()
        self.addCleanup(self.close_server)
        self.url = f'http://127.0.0.1:{self.server.server_port}/fixture.tar.gz'

    def close_server(self):
        self.server.shutdown()
        self.server.server_close()
        self.thread.join(timeout=2)

    def target(self, kind):
        if kind == 'extractor':
            return self.root / 'extractor.tar.gz'
        return self.root / 'stage/dependencies/homebrew/fixture/fixture.tar.gz'

    def download(self, kind, digest=None):
        digest = self.digest if digest is None else digest
        if kind == 'extractor':
            prepare_innoextract.download(self.url, self.target(kind), digest)
        else:
            prefix = self.root / 'prefix'
            recipe = prefix / '.brew/fixture.rb'
            recipe.parent.mkdir(parents=True, exist_ok=True)
            recipe.write_text(f'  url "https://source.test/fixture.tar.gz"\n  sha256 "{digest}"\n', encoding='utf-8')
            # Keep the production HTTPS formula contract; redirect its transport
            # to our local server without needing an external TLS certificate.
            opener = urllib.request.urlopen
            with patch('urllib.request.urlopen', side_effect=lambda request, **kwargs:
                       opener(self.url, **kwargs)):
                MacOSRuntime.homebrew_sources(prefix, self.root / 'stage')

    @patch('time.sleep')
    def test_504_and_503_recover_for_both_release_paths(self, sleep):
        for kind in ('extractor', 'homebrew'):
            with self.subTest(kind=kind):
                self.plan = [504, 503, 200]
                before = self.requests
                self.download(kind)
                self.assertEqual(self.requests - before, 3)
                self.assertEqual(self.target(kind).read_bytes(), self.payload)
        self.assertEqual(sleep.call_count, 4)
        self.assertEqual((self.root / 'stage/licenses/homebrew/fixture/fixture/LICENSE').read_bytes(),
                         b'Test dependency license\n')

    @patch('time.sleep')
    def test_failure_is_bounded_and_preserves_existing_file(self, sleep):
        target = self.target('extractor')
        target.write_bytes(b'existing valid artifact')
        self.plan = [504] * 8
        with self.assertRaises(HTTPError) as error:
            self.download('extractor')
        self.assertEqual(error.exception.code, 504)
        self.assertEqual(self.requests, 4)
        self.assertEqual(sleep.call_count, 3)
        self.assertEqual(target.read_bytes(), b'existing valid artifact')
        self.assertEqual(list(self.root.iterdir()), [target])

    @patch('time.sleep')
    def test_429_respects_bounded_retry_after(self, sleep):
        self.plan = [429, 200]
        self.download('extractor')
        self.assertEqual(self.requests, 2)
        sleep.assert_called_once_with(3)
        self.assertEqual(self.target('extractor').read_bytes(), self.payload)

    @patch('time.sleep')
    def test_transport_errors_recover(self, sleep):
        opener = urllib.request.urlopen
        for error in (URLError('temporary DNS failure'), TimeoutError('timed out'),
                      ConnectionResetError('connection reset')):
            with self.subTest(error=type(error).__name__):
                calls = []

                def flaky(*args, **kwargs):
                    calls.append(True)
                    if len(calls) == 1:
                        raise error
                    return opener(*args, **kwargs)

                with patch('urllib.request.urlopen', side_effect=flaky):
                    self.download('extractor')
                self.assertEqual(len(calls), 2)
                self.assertEqual(self.target('extractor').read_bytes(), self.payload)
        self.assertEqual(sleep.call_count, 3)

    @patch('time.sleep')
    def test_404_is_not_retried(self, sleep):
        self.plan = [404]
        with self.assertRaises(HTTPError) as error:
            self.download('extractor')
        self.assertEqual(error.exception.code, 404)
        self.assertEqual(self.requests, 1)
        sleep.assert_not_called()
        self.assertFalse(self.target('extractor').exists())

    @patch('time.sleep')
    def test_truncated_download_restarts_from_zero(self, sleep):
        self.plan = ['truncated', 200]
        self.download('extractor')
        self.assertEqual(self.requests, 2)
        self.assertEqual(self.target('extractor').read_bytes(), self.payload)
        self.assertEqual(sleep.call_count, 1)

    @patch('time.sleep')
    def test_bad_checksum_preserves_existing_artifact(self, sleep):
        target = self.target('extractor')
        target.write_bytes(b'existing valid artifact')
        with self.assertRaises(ValueError):
            self.download('extractor', '0' * 64)
        self.assertEqual(target.read_bytes(), b'existing valid artifact')
        self.assertEqual(list(self.root.iterdir()), [target])
        self.assertEqual(self.requests, 1)
        sleep.assert_not_called()

    @patch('time.sleep')
    def test_bad_checksum_is_not_published_or_retried(self, sleep):
        for kind in ('extractor', 'homebrew'):
            with self.subTest(kind=kind):
                before = self.requests
                with self.assertRaises(ValueError):
                    self.download(kind, '0' * 64)
                self.assertEqual(self.requests - before, 1)
                self.assertFalse(self.target(kind).exists())
        sleep.assert_not_called()


if __name__ == '__main__':
    unittest.main()
