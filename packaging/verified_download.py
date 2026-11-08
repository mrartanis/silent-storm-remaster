"""Bounded retries and atomic, SHA-256-verified release downloads."""

import hashlib
from http.client import HTTPException, IncompleteRead
from pathlib import Path
import tempfile
import time
from urllib.error import HTTPError, URLError
import urllib.request


TRANSIENT_HTTP = {408, 429, 500, 502, 503, 504}


def download(url, destination, sha256, *, timeout=90, attempts=4):
    if attempts < 1:
        raise ValueError('Download attempts must be positive')
    destination = Path(destination)
    for attempt in range(1, attempts + 1):
        temporary = None
        print(f'Downloading {url} (attempt {attempt}/{attempts})', flush=True)
        try:
            request = urllib.request.Request(url, headers={'User-Agent': 'Silent-Storm-release'})
            digest = hashlib.sha256()
            received = 0
            with tempfile.NamedTemporaryFile(dir=destination.parent,
                                             prefix=destination.name + '.', suffix='.part',
                                             delete=False) as stream:
                temporary = Path(stream.name)
                with urllib.request.urlopen(request, timeout=timeout) as response:
                    length = response.headers.get('Content-Length')
                    while block := response.read(1024 * 1024):
                        stream.write(block)
                        digest.update(block)
                        received += len(block)
                    if length is not None and received != int(length):
                        raise IncompleteRead(b'', max(0, int(length) - received))
            if digest.hexdigest() != sha256:
                raise ValueError(f'Unexpected SHA-256 for {url}: {digest.hexdigest()}')
            temporary.replace(destination)
            return destination
        except (URLError, TimeoutError, ConnectionError, HTTPException) as error:
            if isinstance(error, HTTPError):
                error.close()
                if error.code not in TRANSIENT_HTTP:
                    raise
            if attempt == attempts:
                raise
            delay = 2 ** attempt
            if isinstance(error, HTTPError):
                retry_after = error.headers.get('Retry-After', '')
                if retry_after.isdecimal():
                    delay = max(delay, min(30, int(retry_after)))
            print(f'Download failed: {error}; retrying {url} in {delay}s', flush=True)
            time.sleep(delay)
        finally:
            if temporary is not None:
                temporary.unlink(missing_ok=True)
