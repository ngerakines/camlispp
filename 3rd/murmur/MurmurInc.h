//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//
// This variant of MurmurHash3 is based on:
// https://smhasher.googlecode.com/svn/trunk/MurmurHash3.cpp

#ifndef _MURMURINC_H_
#define _MURMURINC_H_

#include <cstring>
#include <stdint.h>
#include <sys/types.h>

// 128-bit variant
class murmur_inc {
	public:
		murmur_inc(uint32_t seed = 0);
		void hash(const void *key, size_t len);
		void finalize(void *out);
	private:
		uint32_t _h1;
		uint32_t _h2;
		uint32_t _h3;
		uint32_t _h4;
		size_t _len;
};

class murmur_inc64 {
	public:
		murmur_inc64(uint32_t seed = 0);
		void hash(const void *key, size_t len);
		void finalize(void *out);
	private:
		uint64_t _h1;
		uint64_t _h2;
		size_t _len;
};

#endif /* _MURMURINC_H_ */
