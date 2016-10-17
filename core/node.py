from SimpleXMLRPCServer import SimpleXMLRPCServer, SimpleXMLRPCRequestHandler, SimpleXMLRPCDispatcher
from base64 import b64decode
from threading import Thread, Condition
from thread import start_new_thread
from multicaster import *

import socket
import time

def authenticate(user, password):
    if usr == self._user and pwd == self._password:
        return True

class RequestHandler(SimpleXMLRPCRequestHandler):
    def __init__(self, *args, **kwargs):
        self._user = kwargs.pop("user")
        self._password = kwargs.pop("password")
        print kwargs
        SimpleXMLRPCRequestHandler.__init__(self, *args, **kwargs)

    def parse_request(self):
        if SimpleXMLRPCRequestHandler.parse_request(self):
            basic, foo, encoded = self.headers.get('Authorization', "").partition(' ')
            usr, foo, pwd = b64decode(encoded).partition(':')
            if authenticate(usr, pwd):
                return True

            self.send_error(401, 'Authentication failed')
            return False

def notifier_thread(multicaster):
    while True:
        print "notify"
        multicaster.send("ready")
        time.sleep(3)

def receiver_thread(multicaster):
    m = multicaster.receive()
    print m


class Node(SimpleXMLRPCServer):
    def __init__(self, credits, hosts):
        SimpleXMLRPCServer.__init__(self, hosts, requestHandler=RequestHandler)
        


    def start(self, notifier):        
        start_new_thread(notifier_thread, (notifier,))
        start_new_thread(receiver_thread, (notifier,))

        SimpleXMLRPCServer.serve_forever(self)