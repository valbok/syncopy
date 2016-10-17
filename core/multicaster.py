import socket
import struct

class Multicaster:
    def __init__(self, addr, port):
        self._addr = addr
        self._port = port
        

    def __exit__(self, exc_type, exc_value, traceback):
        pass
        #self._sock.close()

    def send(self, mess):
        ttl = struct.pack('b', 1)
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)
        sock.sendto(mess, (self._addr, self._port))        
        sock.close()

    def receive(self):
        bind_addr = '0.0.0.0'
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        membership = socket.inet_aton(self._addr) + socket.inet_aton(bind_addr)

        sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, membership)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        sock.bind((self._addr, self._port))

        while True:
            message, address = sock.recvfrom(255)
            print "receive: ", message, address
            return message
