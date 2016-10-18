#!/usr/bin/env python
"""
" @author VaL Doroshchuk
" @copyright Copyright (C) 2016 VaL::bOK
" @license GNU GPL v2
"""

import argparse
import core

"""
" Tools to sync files using rsync between server and few clients.
" 1. First start server
"    $ ./synco.py /path/to/sync
" 2. Start watching the local dir:
"    $ ./synco.py /watch/dir --server http://admin:password@127.0.0.1:9999
"""
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Synchronizes files between server and its clients using HTTP")
    parser.add_argument('dir', metavar='/path/to/dir', type=str, help='Dir')
    parser.add_argument('--listen', metavar='admin:password@0.0.0.0:9999', type=str, default="admin:password@0.0.0.0:9999",
        help='Run server')
    parser.add_argument('--server', metavar='http://admin:password@127.0.0.1:9999', type=str, help='Connect to master')
    args = parser.parse_args()    
    
    watch_dir = args.dir
    host = args.listen
    node_type = core.ServerNode
    if args.server:
        node_type = core.ClientNode
        host = args.server

    node = node_type(host, watch_dir)
    node.start()
