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
    tmp = tempfile.TemporaryFile()
    tmp.write(data)
    tmp.seek(0)
    return tmp

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
    " @return Signature of the file by \a fn
    """
    def signature(self, fn):
        return raw(File(self._dir + fn).signature())

    """
    " Patches file using delta of client's source file.
    """
    def patch(self, fn, delta):
        File(self._dir + fn).patch(tmp_file(delta.data))

        return True
