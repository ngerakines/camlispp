#ifndef _BLOBSERVER_CONFIG_H_
#define _BLOBSERVER_CONFIG_H_

#cmakedefine ENABLE_DEBUG

#cmakedefine ENABLE_DUMP

#if defined ENABLE_DEBUG
#define DEBUG 1
#endif

#cmakedefine ENABLE_MD5

#cmakedefine ENABLE_STATIC

#if defined ENABLE_MD5
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#endif

#define DAEMON_NAME "blobserver"

#endif
