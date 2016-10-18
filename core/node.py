from SimpleXMLRPCServer import SimpleXMLRPCServer, SimpleXMLRPCRequestHandler, SimpleXMLRPCDispatcher
from base64 import b64decode
from xmlrpclib import ServerProxy

import os

import server_services
from file import *

import time

_g_server_user = ""
_g_server_password = ""

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

def _files(d):
    result = []
    for (root, dirs, files) in os.walk(d):
        for file in files:
            result.append(os.path.join( root[len(d):], file ))

    return result

class Node:
    def __init__(self, host, watch_dir):
        self._host = host
        if watch_dir[-1] != '/':
            watch_dir += '/'
        
        self._dir = watch_dir

class ServerNode(Node):
    def start(self):
        global _g_server_user, _g_server_password
        (_g_server_user, _g_server_password), (host, port) =_parse_host(self._host)

        print "Starting as master on " + host + ":" + str(port) + " for dir " + self._dir

        server = SimpleXMLRPCServer((host, port), requestHandler = RequestHandler)
        server.register_instance(server_services.ServerServices(self._dir))
        server.serve_forever()

class ClientNode(Node):
    def start(self):
        server = ServerProxy(self._host)

        print "Starting monitoring " + self._dir + "*"

        while True:
            print
            files = _files(self._dir)
            for fn in files:
                f = File(self._dir + fn)
                print "Syncing:", fn
                signature = server.signature(fn)
                tmp = server_services.tmp_file(signature.data)
                d = f.delta(tmp)
                server.patch(fn, server_services.raw(d))

            time.sleep(5)


