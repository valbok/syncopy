"""
" @author VaL
" @copyright Copyright (C) 2016 VaL::bOK
" @license GNU GPL v2
"""

from file import *

import tempfile
import xmlrpclib

"""
" @return Prepared binary data to be sent via rpc.s
"""
def raw(f):
    f.seek(0)

    return xmlrpclib.Binary(f.read())

"""
" @return Prepared binary data to be sent via rpc.s
"""
def tmp_file(data):
    tmp = tempfile.SpooledTemporaryFile(max_size=1024 ** 2 * 5, mode='w+')
    tmp.write(data)
    tmp.seek(0)

    return tmp

def _files(d):
    result = []
    for (root, dirs, files) in os.walk(d):
        for file in files:
            result.append(os.path.join(root[len(d):], file))

    return result

def file_table(d):
    fs = _files(d)
    t = {}
    for fn in fs:
        f = File(d + fn)
        t[fn] = {'changed': f.changed, 'size': f.size}

    return t

"""
" Handles requests from remote clients.
"""
class ServerServices:

    """
    " @param Directory where files should be created.
    """
    def __init__(self, d):
        self._dir = d

    """
    " @return Signature of the file by \a fn.
    " @param string Filename.
    """
    def signature(self, fn):
        return raw(File(self._dir + fn).signature())

    """
    " Patches file using delta of client's source file.
    " @param string Filename.
    " @param file File handler that open and can be read.
    """
    def delta(self, fn, signature):
        return raw(File(self._dir + fn).delta(tmp_file(signature.data)))

    """
    " Patches file using delta of client's source file.
    " @param string Filename.
    " @param file File handler that open and can be read.
    """
    def patch(self, fn, delta):
        File(self._dir + fn).patch(tmp_file(delta.data))

        return True

    def file_table(self):
        return file_table(self._dir)
