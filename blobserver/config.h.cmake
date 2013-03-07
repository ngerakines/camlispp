#ifndef _BLOBSERVER_CONFIG_H_
#define _BLOBSERVER_CONFIG_H_

#cmakedefine ENABLE_DEBUG

#cmakedefine ENABLE_DEBUG_LOAD

#cmakedefine ENABLE_DUMP

#if defined ENABLE_DEBUG
#define DEBUG 1
#endif

#cmakedefine ENABLE_STATIC

#define DAEMON_NAME "blobserver"

#endif
