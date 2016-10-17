# chat server using multicast
# python fork of the original ruby implementation
# http://tx.pignata.com/2012/11/multicast-in-ruby-building-a-peer-to-peer-chat-system.html
# receiver.py
# usage : $ python receiver.py  # wait for messages to come in

import socket
import struct
import sys

multicast_addr = '224.0.0.1'
bind_addr = '0.0.0.0'
port = int(sys.argv[1])#3000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
membership = socket.inet_aton(multicast_addr) + socket.inet_aton(bind_addr)

sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, membership)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

sock.bind((bind_addr, port))

while True:
    message, address = sock.recvfrom(255)
    print message
