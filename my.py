from SimpleXMLRPCServer import SimpleXMLRPCServer, SimpleXMLRPCRequestHandler, SimpleXMLRPCDispatcher
import sys
from base64 import b64decode

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


if __name__ == '__main__':
    server = SimpleXMLRPCServer(("0.0.0.0", 9000), requestHandler=RequestHandler(user="admin", password="password"))

    server.serve_forever()
