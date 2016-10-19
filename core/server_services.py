"""
" @author VaL Doroshchuk
" @copyright Copyright (C) 2016 VaL::bOK
" @license GNU GPL v2
"""

from file import *
from logger import *

import tempfile
import xmlrpclib
import os
import time

"""
" @return Prepared binary data to be sent via rpc.
"""
def raw(f):
    f.seek(0)

    return xmlrpclib.Binary(f.read())

"""
" @return Tempfile that could be used to patch using rdiff.
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

"""
" @return dict with file info.
"""
def file_table(d):
    fs = _files(d)
    t = {}
    for fn in fs:
        f = File(d + fn)
        t[fn] = {'changed': f.changed, 'size': f.size, 'checksum': f.checksum}

    return t

"""
" @return Filename that file with this name has been purged.
"""
def removed_filename(fn):
    fs = fn.split("/")
    f = fs[len(fs) - 1]
    del fs[len(fs) - 1]
    bs = "/".join(fs)
    if bs:
        bs += "/"

    return "{}.{}.syncopy_removed".format(bs, f)

"""
" @return True if filename seems like of removed file.
"""
def removed_file(fn):
    fn = os.path.basename(fn)
    fs = fn.split(".")

    return fs[0] == "" and fs[len(fs) - 1] == "syncopy_removed"

"""
" @return True if filename is internal and should not be synced.
"""
def synced_file(fn):
    fs = fn.split(".")

    return fs[len(fs) - 1] == "synced"

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
        log_info("Creating signature for {}".format(fn))

        return raw(File(self._dir + fn).signature())

    """
    " Patches file using delta of client's source file.
    " @param string Filename.
    " @param file File handler that open and can be read.
    """
    def delta(self, fn, signature):
        log_info("Creating delta for {}".format(fn))

        return raw(File(self._dir + fn).delta(tmp_file(signature.data)))

    """
    " Patches file using delta of client's source file.
    " @param string Filename.
    " @param file File handler that open and can be read.
    """
    def patch(self, fn, delta, ts):
        log_info("Patching {}".format(fn))

        f = File(self._dir + fn)
        f.create()
        f.patch(tmp_file(delta.data))
        try:
            File(self._dir + removed_filename(fn)).remove()
        except:
            pass

        return True

    """
    " @return dict of info per file.
    """
    def file_table(self):
        return file_table(self._dir)

    """
    " Removes the file and its dir if empty
    """
    def remove_file(self, fn):
        log_info("Removing file {}".format(fn))
        File(self._dir + fn).rename(self._dir + removed_filename(fn))

        return True
