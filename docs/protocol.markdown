
# Protocol

This is unnoficial documentation for the camlistore blob protocol. I started this to condense some of the documentation and keep my own notes and observerations in a single place. Some of the content found in this document has been copied over from the camlistore project. I make no claims to the content contained therein.

## Overview

The blob server protocol describes a series of APIs used to interact with immutable data.

Highlights include:

* content-addressable blobs only
  - no notion of "files", filenames, dates, streams, encryption,
    permissions, metadata.
* immutable
* only operations:
  - store(digest, bytes)
  - check(digest) => bool (have it or not)
  - get(digest) => bytes
  - list([start_digest]) => [(digest[, size]), ...]+
* amenable to implementation on ordinary filesystems (e.g. ext3, vfat,
  ntfs) or on Amazon S3, BigTable, AppEngine Datastore, Azure, Hadoop
  HDFS, etc.

## Terms

**blob**
: an immutable sequence of 0 or more bytes, with no extra metadata

**blobref**
: a reference to a blob, consisting of a cryptographic hash function name and that hash function's digest of the blob's bytes, in hex. Examples of valid blobrefs include: 

**blob server**
: The simplest and lowest layer of the Camlistore servers. A blob server, while potentially shared between users, is logically private to a single user and holds that user's blobs (whatever they may represent). 

**digest**
: A compilation or summary of a blob.

**digest type**
: bar

**base url**
: The base entry point of the blob server.

## Service Discovery

Service discovery appertains to the self-description of a server implementing this protocol and the act of providing that description to potential consumers of the service.

Currently, clients may make an HTTP GET request to the base url of a server that implements the blob protocol. That request must set the HTTP Accept header to value to "text/x-camli-configuration". Doing so tells the server to return a json structure describing the services provided by the server.

***An example request***
```
GET / HTTP/1.1
Accept: text/x-camli-configuration
```

***An example response***
```json
{
  "blobRoot": "/",
  "ownerName": "Nick Gerakines"
}
```

## API

### Get

The GET api provides a mechanism to retreive a named blob's contents.

```plain
url ::= <base url> <blob ref>
```

*Sample Get Request:*

```plain
GET /sha1-D336EE2D38070D9D7B29612D7E1293C0E7CA3812
```

*Sample Get Response:*
```plain
HTTP/1.1 200 OK
Content-Length: 42
<http headers>

<blob contents>
```

If the blob server does not contain the given blob, a 404 is returned. Otherwise a status code 200 is returned. The HTTP header "Content-Length" is provided by the server as well.

#### HEAD alternative

Alternatively, the HEAD method can be used in place of the GET method to allow clients to quickly determine if a given blob exists on the server. In either case, the HTTP header "Content-Length" will be provided by the server.

```plain
HEAD /sha1-D336EE2D38070D9D7B29612D7E1293C0E7CA3812
```

*Sample Get Response:*
```plain
HTTP/1.1 200 OK
Content-Length: 42
<http headers>
```

### Enumerate

The enumerate endpoint provides a way to cycle through all or a subset of known blobs.

```plain
parameter-key ::= "after" | "limit" | "maxwaitsec"
parameter ::= <parameter-key> "=" <value>
query-string ::= <parameter> | <parameter> "&" <query-string>
url ::= <base url> "enumerate-blobs" | <base url> "enumerate-blobs?" <query-string>
```

The above BNF describes the composition of an enumerate request. The *after*, *limit* and *maxwaitsec* query string parameters are all optional and may be overriden by the server.

**after**
: A blobref used to indicate where the list of blobrefs returned by the server should start.

**limit**
: Limit the number of returned blobrefs. The server may have its own lower limit, however, so be sure to pay attention to the presence of a "continueAfter" key in the JSON response.

**maxwaitsec**
: The client may send this, an integer max number of seconds the client is willing to wait for the arrival of blobs. If the server supports long-polling (an optional feature), then the server will return immediately if any blobs or available, else it will wait for this number of seconds. It is an error to send this option with a non-zero value along with the 'after' option.

The server's reply must include "canLongPoll" set to true if the server supports this feature. Even if the server supports long polling, the server may cap 'maxwaitsec' and wait for less time than requested by the client.

*Sample Enumerate Request:*
```plain
GET /enumerate-blobs
```

*Sample Enumerate Response:*
```plain
HTTP/1.1 200 OK
Content-Type: text/javascript
<http headers>

{
  "blobs": [
    {"blobRef": "sha1-956A0643F15008BE8BB572645B2D34FEEAF91E31", "size": 336},
    {"blobRef": "sha1-F0ECED23F8B235ADFEE9A95E2145C81F416863E9", "size": 1650},
  ],
  "continueAfter": "sha1-F0ECED23F8B235ADFEE9A95E2145C81F416863E9",
  "canLongPoll": true,
}
```

*An example request of 25 blobrefs.*
```plain
GET /enumerate-blobs?limit=25
```

*An example request of 25 blobrefs after a given blobref.*
```plain
GET /enumerate-blobs?limit=25&after=sha1-F0ECED23F8B235ADFEE9A95E2145C81F416863E9
```

The response includes a JSON datastructure containing a list of zero or more blob refs.

They're returned in sorted order, sorted by (digest type and digest value). That is, md5-acbd18db4cc2f85cedef654fccc4a4d8 sorts
before sha1-0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33 because "m" sorts before "s", even though "0" sorts before "a".

The json response may also include the *continueAfter* and *canLongPoll* keys. The value associated with the *continueAfter* key will be a blob ref to pass in the a subsequent enumerate request as the *last* query string parameter. The value associate with the *canLongPoll* key indicates that the server supports long polling.

#### Errata

If the blob server has multiple blob refs for the same blob then enumeration will include all possible refs that may lead to duplicate blob transfers. This may occur if the blob server supports multiple digests (sha1 and sha256, etc). Currently, there is no way to determine if a given blob ref has sibling blob refs.

If the blob server supports multiple digest types (ie sha1 and sha256, etc), there is no way to iterate over blob refs of a specific digest type.

The *last* query string parameter must contain a blob ref of a blob that is known to exist. Partial blob refs (ie "sha1-77F5242") may cause unexpected behavior.

### Stat

The 'stat' method is both a stat on the server itself (server capability/property discovery), as well as a multi-stat() on blobs.

```plain
blob-key = "blob" <number>
parameter-key ::= blob-key | "maxwaitsec"
parameter ::= <parameter-key> "=" <value>
query-string ::= <parameter> | <parameter> "&" <query-string>
url ::= <base url> "stat" | <base url> "stat?camliversion=1&" <query-string>
```

It is important to note that stat requests can be made with either the HTTP GET or HTTP POST methods. In either case, the request parameters are the same. GET is more correct but be aware that HTTP servers and proxies start to suck around the 2k and 4k URL lengths. If you're stat'ing more than ~37 blobs, using POST would be safest.

If the HTTP POST method is used, the HTTP header "Content-Type" should be set to "application/x-www-form-urlencoded".

Stat requests have the following request parameters:

**camliversion**
: The required parameter represents the version of the protocol being referenced. It must be included and should be set to the base protocol version of "1".

**maxwaitsec**
: This optional query string parameter can be used by clients to consume events from servers that support long polling.

If the server supports long-polling (an optional feature), then the server will return immediately if all the requested blobs or available, or wait up until this amount of time for the blobs to become available. The server's reply must include "canLongPoll" set to true if the server supports this feature. Even if the server supports long polling, the server may cap 'maxwaitsec' and wait for less time than requested by the client.

**blob<n>**
: Zero or more blob refs starting at 1 and incrimenting without gaps and without zero padding.

There's no defined limit on how many you include here, but servers may return a 400 Bad Request if you ask for too many. All servers should support <= 1000 though.

*Sample Stat GET Request:*
```plain
GET /stat?camliversion=1&blob1=sha1-956A0643F15008BE8BB572645B2D34FEEAF91E31 HTTP/1.1
<http headers>
```

*Sample Stat POST Request:*
```plain
POST /camli/stat HTTP/1.1
Content-Type: application/x-www-form-urlencoded
<http headers>

camliversion=1&
blob1=sha1-956A0643F15008BE8BB572645B2D34FEEAF91E31&
blob2=sha1-F0ECED23F8B235ADFEE9A95E2145C81F416863E9
```

*Sample Stat Response:*
```plain
HTTP/1.1 200 OK
Content-Type: text/javascript
<http headers>

{
	"stat": [
		{"blobRef": "sha1-956A0643F15008BE8BB572645B2D34FEEAF91E31", "size": 336},
		{"blobRef": "sha1-F0ECED23F8B235ADFEE9A95E2145C81F416863E9", "size": 1650},
	],
	"maxUploadSize": 1048576,
	"uploadUrl": "http://127.0.0.1:3179/upload",
	"uploadUrlExpirationSeconds": 7200,
	"canLongPoll": false
}
```

The json structure returned includes the following components:

**stat** (required)
: An array of blob ref objects including the "blobRef" key and "size" key.

**maxUploadSize** (required)
: An integer of max byte size for whole request payload, which may be one or more blobs.

**uploadUrl** (required)
: The next URL to use to upload any more blobs.

**uploadUrlExpirationSeconds** (required)
: How long the upload URL will be valid for, in seconds.

**canLongPoll**
: Set to true (type boolean) if the server supports long polling. If not true, the server ignores the client's "maxwaitsec" parameter.

### Upload

Adding blobs to a blob server is a two step process:

1. A stat request is made to determine which blobs are necessary to upload. Although this step is technically not required, all clients should perform both parts of this step to reduce upload traffic. Alternatively, servers that support HTTP pipelining could have a series of HEAD requests against blob refs to determine if the server has them already.
2. The upload is executed through an HTTP POST multipart/form-data request.

Things to note about the request:

* You MUST provide a "name" parameter in each multipart part's Content-Disposition value. The part's name matters and is the blobref ("digest-hexhexhexhex") of your blob. The bytes MUST match the blobref and the server MUST reject it if they don't match.
* You (currently) MUST provide a Content-Type for each multipart part. It doesn't matter what it is (it's thrown away), but it's necessary to satisfy various HTTP libraries. Easiest is to just set it to "application/octet-stream" Server implementions SHOULD fail if you clients forget it, to encourage clients to remember it for compatibility with all blob servers.
* You (currently) MUST provide a "filename" parameter in each multipart's Content-Disposition value, unique per blob, but it will also be thrown away and exists purely to satisfy various HTTP libraries (mostly App Engine). It's recommended to either set this to an increasing number (e.g. "blob1", "blob2") or just repeat the blobref value here.

Some of these requirements may be relaxed in the future.

*Sample Upload Request:*
```plain
POST /upload HTTP/1.1
Content-Type: multipart/form-data; boundary=randomboundaryXYZ
<http headers>

--randomboundaryXYZ
Content-Disposition: form-data; name="sha1-9b03f7aca1ac60d40b5e570c34f79a3e07c918e8"; filename="blob1"
Content-Type: application/octet-stream

(binary or text blob data)
--randomboundaryXYZ
Content-Disposition: form-data; name="sha1-deadbeefdeadbeefdeadbeefdeadbeefdeadbeef"; filename="blob2"
Content-Type: application/octet-stream

(binary or text blob data)
--randomboundaryXYZ--
```

*Sample Upload Response:*
```plain
HTTP/1.1 200 OK
Content-Type: text/javascript
<http headers>

{
   "received": [
      {"blobRef": "sha1-9b03f7aca1ac60d40b5e570c34f79a3e07c918e8",
       "size": 12312},
      {"blobRef": "sha1-deadbeefdeadbeefdeadbeefdeadbeefdeadbeef",
       "size": 29384933}
   ],
   "maxUploadSize": 1048576,
   "uploadUrl": "http://127.0.0.1:3179/upload",
   "uploadUrlExpirationSeconds": 7200,
}
```

The json structure returned includes the following components:

**received** (required)
: An array of blob ref objects including the "blobRef" key and "size" key.

**maxUploadSize** (required)
: An integer of max byte size for whole request payload, which may be one or more blobs.

**uploadUrl** (required)
: The next URL to use to upload any more blobs.

**uploadUrlExpirationSeconds** (required)
: How long the upload URL will be valid for, in seconds.

**errorText**
: String error message for protocol errors not relating to a particular blob. Mostly for debugging clients.

If connection drops during a POST to an upload URL, you should re-do a stat request to verify which objects were received by the server and which were not. Also, the URL you received from stat before might no longer work, so stat is required to a get a valid upload URL.

### Upload Resume

TBD

## Server to Server Features

### Replication / Sync

TBD

### Garbage Collection

TBD
