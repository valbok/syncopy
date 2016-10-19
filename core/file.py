"""
" @author VaL Doroshchuk
" @copyright Copyright (C) 2016 VaL::bOK
" @license GNU GPL v2
"""

import tempfile
import librsync
import os
import xmlrpclib
import errno
import hashlib
import time

def _tmp_file():
    return tempfile.SpooledTemporaryFile(max_size=1024 ** 2 * 5, mode='w+')

"""
" File handler to read and write using librsync to sync source with destination.
" There are 3 steps to synchronize a file.
" 1. Generate a signature for the destination file.
" 2. Generate a delta for the source file (using the signature).
" 3. Patch the destination file using the generated delta.
"
" @example:
"   src = File(src_path)
"   dst = File(dst_path)
"   dst.patch(src.delta(dst.signature()))
"""
class File:

    """
    " @param Path to file. If does not exist, will be created.
    """
    def __init__(self, path):
        self._path = path.replace("../", "")

    """
    " Creates file
    """
    def create(self, ts = 0):
        basedir = os.path.dirname(self._path)
        if basedir and not os.path.exists(basedir):
            os.makedirs(basedir)

        self.touch(ts)

    """
    " Creates a signature of a file.
    " Usually it is remote.
    " @return tmpfile
    """
    def signature(self, block_size = None):
        kwargs = {}
        if block_size:
            kwargs['block_size'] = block_size

        kwargs['s'] = _tmp_file()
        f = open(self._path, 'rb') if self.exists() else _tmp_file()

        return librsync.signature(f, **kwargs)

    """
    " Creates delta versus signature of remote/source file.
    " @param Signature is a file handler.
    " @return tmpfile
    """
    def delta(self, signature):
        kwargs = {}
        kwargs['d'] = _tmp_file()

        return librsync.delta(open(self._path, 'rb'), signature, **kwargs)

    """
    " Patches current file by delta of source file.
    " Usually called on server side.
    " @param Delta is a file handler.
    """
    def patch(self, delta):
        with (tempfile.NamedTemporaryFile(prefix=os.path.basename(self._path), suffix='.sync',
                dir=os.path.dirname(self._path), delete=False)) as synced:
            try:
                with open(self._path, 'w+') as current:
                    librsync.patch(current, delta, synced)
                    os.rename(synced.name, self._path)
            finally:
                try:
                    os.remove(synced.name)
                except OSError as e:
                    if e.errno != errno.ENOENT:
                        raise

    """
    " Checks if file exists
    """
    def exists(self):
        return os.path.isfile(self._path)

    """
    " @return size of file.
    """
    @property
    def size(self):
        return os.stat(self._path).st_size

    """
    " @return timestamp of file.
    """
    @property
    def changed(self):
        return os.path.getmtime(self._path)

    """
    " @return checksum to compare files.
    """
    @property
    def checksum(self):
        hash = hashlib.md5()
        with open(self._path, "rb") as f:
            for block in iter(lambda: f.read(65536), b""):
                hash.update(block)
        return hash.hexdigest()

    """
    " Changes modification time.
    """
    def touch(self, ts = 0):
        if not ts:
            ts = time.time()

        with open(self._path, 'a'):
            os.utime(self._path, (ts,ts))

    """
    " Removes current file
    """
    def remove(self):
        os.remove(self._path)
        try:
            basedir = fn
            while True:
                basedir = os.path.dirname(basedir)
                if not basedir:
                    break
                os.rmdir(self._dir + basedir)
        except:
            pass

    """
    " Renames current file.
    """
    def rename(self, fn):
        os.rename(self._path, fn)
        File(fn).touch()
