"""
" @author VaL
" @copyright Copyright (C) 2016 VaL::bOK
" @license GNU GPL v2
"""

from SimpleXMLRPCServer import SimpleXMLRPCServer, SimpleXMLRPCRequestHandler, SimpleXMLRPCDispatcher
from base64 import b64decode
from xmlrpclib import ServerProxy

import os
import sys
import time

import server_services
from file import *

"""
" Unfortunatelly could not pass credentials to RequestHandler
" so using global vars.
"""
_g_server_user = ""
_g_server_password = ""

"""
" Handles BASIC AUTH.
"""
class RequestHandler(SimpleXMLRPCRequestHandler):
    def parse_request(self):
        if SimpleXMLRPCRequestHandler.parse_request(self):
            basic, foo, encoded = self.headers.get('Authorization', "").partition(' ')
            user, password = b64decode(encoded).split(':')
            if user == _g_server_user and password == _g_server_password:
                return True

            self.send_error(401, 'Authentication failed')
            return False


def _parse_host(s):
    credits, hosts = s.split("@")
    user, password = credits.split(":")
    host, port = hosts.split(":")
    return (user, password), (host, int(port))

"""
" Base node handler.
"""
class Node:
    def __init__(self, host, watch_dir):
        self._host = host
        if watch_dir[-1] != '/':
            watch_dir += '/'

        self._dir = watch_dir

"""
" Handles connections and makes some actions.
"""
class ServerNode(Node):
    def start(self):
        global _g_server_user, _g_server_password
        (_g_server_user, _g_server_password), (host, port) =_parse_host(self._host)

        print "Starting as master on " + host + ":" + str(port) + " for dir " + self._dir

        server = SimpleXMLRPCServer((host, port), requestHandler = RequestHandler)
        server.register_instance(server_services.ServerServices(self._dir))
        server.serve_forever()

"""
" Node to be handled as client.
" Monitors the dir and uploads changes to server.
"""
class ClientNode(Node):

    def __init__(self,  host, watch_dir):
        Node.__init__(self, host, watch_dir)
        self._server = ServerProxy(self._host)

    """
    " Uploads file to remote dir.
    """
    def _upload(self, f, fn):
        signature = self._server.signature(fn)
        tmp = server_services.tmp_file(signature.data) if signature else False
        delta = f.delta(tmp) if tmp else False
        if not tmp or not self._server.patch(fn, server_services.raw(delta)):
            print "Cound not patch file", fn

    """
    " Downloads file to local dir.
    """
    def _download(self, f, fn):
        signature = f.signature()
        delta = self._server.delta(fn, server_services.raw(signature))
        tmp = server_services.tmp_file(delta.data) if delta else False
        if tmp:
            f.patch(tmp)
        else:
            print "Cound not patch file", fn, tmp

    """
    " Starts monitoring the dir.
    """
    def start(self):
        print "Starting monitoring " + self._dir + "*"

        while True:
            print
            current_files = server_services.file_table(self._dir)
            remote_files = self._server.file_table()

            for fn in current_files:
                f = File(self._dir + fn)
                upload = True
                if fn in remote_files:
                    if f.checksum == remote_files[fn]['checksum']:
                        print ". skipping", fn
                        continue

                    if f.changed < remote_files[fn]['changed']:
                        upload = False

                if upload:
                    print "+ uploading", fn
                    self._upload(f, fn)
                else:
                    print "- downloading", fn
                    self._download(f, fn)

            for fn in remote_files:
                f = File(self._dir + fn)
                if f.exists:
                    continue

                print "- downloading new", fn
                self._download(f, fn)

            print "*",
            for x in range(10, 0, -1):
                print x,
                sys.stdout.flush()
                time.sleep(1)

            print


