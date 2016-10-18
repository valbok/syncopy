#!/usr/bin/env python
import argparse
import core
import sys
import tempfile

import librsync
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='')
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
    
    
    """
    f1 = core.File('/opt/valbok/syncopy/fuck/big')
    
    f2 = core.File('/ssd/sync/dig')


    #r = tempfile.SpooledTemporaryFile(max_size=1024 ** 2 * 5, mode='w+')
    s1 = f2.signature()
    #print "Sig",s1
    #print r
    #print r.read(10)
    #r.seek(0)
    d = f1.delta(s1)
    #print d
    #dl = open('/ssd/sync/delta', 'rb')
    #dl.seek(0)
    #d = f1.delta(r)
    #current = open('/ssd/sync/dig', 'wr+')
    #synced = open('/ssd/sync/dig.synced', 'wb+')
    #librsync.patch(current, dl, synced)
    f2.patch(d)

    sys.exit()"""

    node = node_type(host, watch_dir)

    node.start()
