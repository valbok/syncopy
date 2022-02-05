# syncopy

Syncopy is a prototype of a tool that supposed to synchronize files between several locations.
Implemented simplified version of rsync.

# Example

      $ ./signature destination_file.txt
      destination file : destination_file.txt
      size             : 1106
      signature file   : destination_file.txt.sig
      window           : 500
      chunks           : 3
      $ ./delta destination_file.txt.sig source_file.txt
      signature file : cmake_install.cmake.sig
      window         : 500
      chunks         : 3

      delta file     : source_file.txt.delta
      md5            : 2000904e4521d67ac7d262bc3a7ede11
      chunks         : 3
      $ ./patch source_file.txt.delta destination_file.txt


# RPC
Start the rpc server

      $ cd path/to/upload; ./server path/to/sync/dir

Start the rpc client

      $ cd where/files/monitored ./client

**todo**

- Don't load files to memory when creating signatures, deltas or when patching.
- Don't use unix dependent code, use std when possible.
