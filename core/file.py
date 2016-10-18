"""
" @author VaL
" @copyright Copyright (C) 2016 VaL::bOK
" @license GNU GPL v2
"""

import tempfile
import librsync
import os
import xmlrpclib
import errno

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
        self._path = path
        basedir = os.path.dirname(path)
        if basedir and not os.path.exists(basedir):
            os.makedirs(basedir)

        if not os.path.isfile(path):
            open(path, 'a').close()

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

        return librsync.signature(open(self._path, 'rb'), **kwargs)

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
    " @return tmpfile
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

    def exists(self):
        return os.path.isfile(self._path)

    """
    " @return size of file.
    """
    @property
    def size(self):
        return os.stat(self._path).st_size

    """
    " @return timestampt of file.
    """
    @property
    def changed(self):
        return os.path.getmtime(self._path)