
# About

The camlispp project provides camlistore services and components written in c++.

For more information on what camlistore is and how it works, see: [https://camlistore.org/](https://camlistore.org/)

# Goals and Motivation

This project was kicked off to get me back into the world of c++. I've been following the camlistore project for a little while now and believe that some great things will come from it.

# Roadmap

1. Implement a blob storage daemon. (in-progress)
1. Implement a schema daemon.
1. Implement the index-search daemon.

# Blob Storage Daemon

## Roadmap / TODO

1. Create base HTTP server (done)
1. Integrate cityhash (done)
1. Support retreival of blobs by hash type and hash. (done)
1. Support stat (done)
1. Support upload (done, mostly)
1. Support upload resume
1. Support blob enumeration
1. Add md5 and sha hashing
1. Improve blob storage (disk backed) and index storage (disk backed)
1. Support p2p sync/replication operations
1. Support sharding and conditional blob routing
1. Support large files
1. Support daemon configuration files

# Errata

alias style='astyle -A2 --indent=tab -C -N -Y -f -F -xd -j -J -p -H -n'

# License

Copyright (c) 2013 Nick Gerakines <nick@gerakines.net>

This project and its contents are open source under the MIT license.
