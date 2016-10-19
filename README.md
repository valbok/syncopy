# syncopy

Syncopy is supposed to synchronize files between few locations.
Uses HTTP (with BASIC AUTH) to transfer data and librsync to avoid unnecessary network traffic.

**Topology**

       <Client> - [Master node] - <Client>
                   /    |    \
            <Client> <Client> <Client>

All clients and master node will be syncrhonized.
If new files appear, changed or deleted on a client, all clients will get synchronized.
If to change/modify a file on the master node, all clients will get synchronized too (except removed files, they will be fetched from clients to server again)

# Usage

Start the server

      $ ./synco.py path/to/sync/dir --listen admin:password@0.0.0.0:9999

Start clients

      $ ./synco.py where/files/located --server http://admin:password@127.0.0.1:9999      

# Requires

    apt-get install librsync-dev
    pip install python-librsync
    

**todo**

- SSL for rpc server.
- Hard to transfer big files, sometimes not enough memory.
