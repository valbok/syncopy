#!/usr/bin/env python
import argparse
import core


def parse_host(s):
    credits, hosts = s.split("@")
    user, password = credits.split(":")
    host, port = hosts.split(":")
    return (user, password), (host, int(port))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('dir', metavar='/path/to/dir', type=str, help='Dir')
    parser.add_argument('--server', metavar='admin:password@0.0.0.0:9999', type=str, default="admin:password@0.0.0.0:9999",
        help='Server settings')

    args = parser.parse_args()

    (user, password), (host, port) = parse_host(args.server)
    print user, password
    print host, port

    node = core.Node((user, password), (host, port))
    node.start(core.Multicaster("100.0.0.1", 3333))
    #server = SimpleXMLRPCServer(("0.0.0.0", 9000), requestHandler=RequestHandler(user="admin", password="password"))

    #server.serve_forever()
