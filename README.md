
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

## Options and Configuration

Some compile-time options include:

* **ENABLE_DEBUG**: Enables general debugging. Without this, very little will be logged.
* **ENABLE_DUMP**: Enables extra dumping of internal objects (blob index, blob, blob key, etc).
* **ENABLE_MD5**: Enables use of md5 to has blobs, enabled by default. Can be turned off because md5 isn't that strong.
* **ENABLE_STATIC**: Enables use of static site, mostly for debugging. This is enabled by default.
* **ENABLE_DEBUG_LOAD**: Enables a feature that will attempt to create blobs for all files in a given directory.

Some run-time options include:

```
  --help                 produce help message
  --directory arg        The directory to save blobs in and serve them from
  --ip arg               The ip address to bind to
  --port arg             The port to serve requests on
  --threads arg          The number of threads to use
  --static_directory arg The directory to serve static files from
  --load_directory arg   The directory to load data from
```


## Roadmap / TODO

1. Create base HTTP server (done)
1. Integrate cityhash (done)
1. Support retreival of blobs by hash type and hash. (done)
1. Support stat (done)
1. Support upload (done)
1. Support upload resume (skipped)
1. Support blob enumeration (done)
1. Add md5 and sha hashing (done)
1. Improve blob storage (disk backed) and index storage (disk backed) (done)
1. Support p2p sync/replication operations (done)
1. Support sharding and conditional blob routing (skipped)
1. Support large files (skipped)
1. Support daemon configuration files
1. Support static site, mostly for debugging (done)
1. Support accept-encoding http header

## Testing

    $ ./blobserver/src/blobserver --port 8081 --directory /home/ngerakines/tmp/source/data1/ --load_directory /home/ngerakines/tmp/source --sync http://127.0.0.1:8082 --sync-delay 30

    $ ./blobserver/src/blobserver --port 8082 --directory /home/ngerakines/tmp/source/data2/


# Errata

Please use the following astyle config when contributing:

    alias style='astyle -A2 --indent=tab -C -N -Y -f -F -xd -j -J -p -H -n'

To enable object dumping for debug, use the following options when generating build files:

    cmake . -DENABLE_DEBUG=ON -DENABLE_DUMP=ON -DENABLE_DEBUG_LOAD=ON

# License

Copyright (c) 2013 Nick Gerakines <nick@gerakines.net>

This project and its contents are open source under the MIT license.
