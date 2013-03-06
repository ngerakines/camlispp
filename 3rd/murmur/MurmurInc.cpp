//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//
// This variant of MurmurHash3 is based on:
// https://smhasher.googlecode.com/svn/trunk/MurmurHash3.cpp
//
// Note: The incremental version does not produce the same result as
// MurmurHash3_x86_128(), unless the step sizes are always 16-byte aligned

#include "MurmurInc.h"

#define	FORCE_INLINE __attribute__((always_inline))
#define BIG_CONSTANT(x) (x##LLU)

static inline FORCE_INLINE uint32_t
rotl32(uint32_t x, int8_t r) {
	return (x << r) | (x >> (32 - r));
}

static inline FORCE_INLINE uint64_t
rotl64(uint64_t x, int8_t r) {
	return (x << r) | (x >> (64 - r));
}

static FORCE_INLINE uint32_t
fmix(uint32_t h) {
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

static FORCE_INLINE uint64_t
fmix(uint64_t k) {
	k ^= k >> 33;
	k *= BIG_CONSTANT(0xff51afd7ed558ccd);
	k ^= k >> 33;
	k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
	k ^= k >> 33;
	return k;
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

murmur_inc::murmur_inc(uint32_t seed)
	: _h1(seed), _h2(seed), _h3(seed), _h4(seed), _len(0) {
}

void
murmur_inc::hash(const void *key, size_t len) {
	const uint32_t c1 = 0x239b961b;
	const uint32_t c2 = 0xab0e9789;
	const uint32_t c3 = 0x38b34ae5;
	const uint32_t c4 = 0xa1e38b93;
	const uint8_t * data = (const uint8_t*)key;
	const ssize_t nblocks = len / 16;
	const uint32_t * blocks = (const uint32_t *)(data + nblocks * 16);

	for (ssize_t i = -nblocks; i; i++) {
		uint32_t k1 = blocks[i * 4 + 0];
		uint32_t k2 = blocks[i * 4 + 1];
		uint32_t k3 = blocks[i * 4 + 2];
		uint32_t k4 = blocks[i * 4 + 3];
		k1 *= c1;
		k1  = ROTL32(k1, 15);
		k1 *= c2;
		_h1 ^= k1;
		_h1 = ROTL32(_h1, 19);
		_h1 += _h2;
		_h1 = _h1 * 5 + 0x561ccd1b;
		k2 *= c2;
		k2  = ROTL32(k2, 16);
		k2 *= c3;
		_h2 ^= k2;
		_h2 = ROTL32(_h2, 17);
		_h2 += _h3;
		_h2 = _h2 * 5 + 0x0bcaa747;
		k3 *= c3;
		k3  = ROTL32(k3, 17);
		k3 *= c4;
		_h3 ^= k3;
		_h3 = ROTL32(_h3, 15);
		_h3 += _h4;
		_h3 = _h3 * 5 + 0x96cd1c35;
		k4 *= c4;
		k4  = ROTL32(k4, 18);
		k4 *= c1;
		_h4 ^= k4;
		_h4 = ROTL32(_h4, 13);
		_h4 += _h1;
		_h4 = _h4 * 5 + 0x32ac3b17;
	}

	const uint8_t * tail = (const uint8_t*)(data + nblocks * 16);
	uint32_t k1 = 0;
	uint32_t k2 = 0;
	uint32_t k3 = 0;
	uint32_t k4 = 0;

	switch (len & 15) {
	case 15:
		k4 ^= tail[14] << 16;

	case 14:
		k4 ^= tail[13] << 8;

	case 13:
		k4 ^= tail[12] << 0;
		k4 *= c4;
		k4  = ROTL32(k4, 18);
		k4 *= c1;
		_h4 ^= k4;

	case 12:
		k3 ^= tail[11] << 24;

	case 11:
		k3 ^= tail[10] << 16;

	case 10:
		k3 ^= tail[ 9] << 8;

	case  9:
		k3 ^= tail[ 8] << 0;
		k3 *= c3;
		k3  = ROTL32(k3, 17);
		k3 *= c4;
		_h3 ^= k3;

	case  8:
		k2 ^= tail[ 7] << 24;

	case  7:
		k2 ^= tail[ 6] << 16;

	case  6:
		k2 ^= tail[ 5] << 8;

	case  5:
		k2 ^= tail[ 4] << 0;
		k2 *= c2;
		k2  = ROTL32(k2, 16);
		k2 *= c3;
		_h2 ^= k2;

	case  4:
		k1 ^= tail[ 3] << 24;

	case  3:
		k1 ^= tail[ 2] << 16;

	case  2:
		k1 ^= tail[ 1] << 8;

	case  1:
		k1 ^= tail[ 0] << 0;
		k1 *= c1;
		k1  = ROTL32(k1, 15);
		k1 *= c2;
		_h1 ^= k1;
	};

	_len += len;
}

void
murmur_inc::finalize(void *out) {
	_h1 ^= _len;
	_h2 ^= _len;
	_h3 ^= _len;
	_h4 ^= _len;
	_h1 += _h2;
	_h1 += _h3;
	_h1 += _h4;
	_h2 += _h1;
	_h3 += _h1;
	_h4 += _h1;
	_h1 = fmix(_h1);
	_h2 = fmix(_h2);
	_h3 = fmix(_h3);
	_h4 = fmix(_h4);
	_h1 += _h2;
	_h1 += _h3;
	_h1 += _h4;
	_h2 += _h1;
	_h3 += _h1;
	_h4 += _h1;
	((uint32_t*)out)[0] = _h1;
	((uint32_t*)out)[1] = _h2;
	((uint32_t*)out)[2] = _h3;
	((uint32_t*)out)[3] = _h4;
}

murmur_inc64::murmur_inc64(uint32_t seed)
	: _h1(seed), _h2(seed), _len(0) {
}

void
murmur_inc64::hash(const void *key, size_t len) {
	const uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
	const uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);
	const uint8_t * data = (const uint8_t*)key;
	const size_t nblocks = len / 16;
	const uint64_t * blocks = (const uint64_t *)(data);

	for (size_t i = 0; i < nblocks; i++) {
		uint64_t k1 = blocks[i * 2 + 0];
		uint64_t k2 = blocks[i * 2 + 1];
		k1 *= c1;
		k1  = ROTL64(k1, 31);
		k1 *= c2;
		_h1 ^= k1;
		_h1 = ROTL64(_h1, 27);
		_h1 += _h2;
		_h1 = _h1 * 5 + 0x52dce729;
		k2 *= c2;
		k2  = ROTL64(k2, 33);
		k2 *= c1;
		_h2 ^= k2;
		_h2 = ROTL64(_h2, 31);
		_h2 += _h1;
		_h2 = _h2 * 5 + 0x38495ab5;
	}

	const uint8_t * tail = (const uint8_t*)(data + nblocks * 16);
	uint64_t k1 = 0;
	uint64_t k2 = 0;

	switch (len & 15) {
	case 15:
		k2 ^= uint64_t(tail[14]) << 48;

	case 14:
		k2 ^= uint64_t(tail[13]) << 40;

	case 13:
		k2 ^= uint64_t(tail[12]) << 32;

	case 12:
		k2 ^= uint64_t(tail[11]) << 24;

	case 11:
		k2 ^= uint64_t(tail[10]) << 16;

	case 10:
		k2 ^= uint64_t(tail[ 9]) << 8;

	case  9:
		k2 ^= uint64_t(tail[ 8]) << 0;
		k2 *= c2;
		k2  = ROTL64(k2, 33);
		k2 *= c1;
		_h2 ^= k2;

	case  8:
		k1 ^= uint64_t(tail[ 7]) << 56;

	case  7:
		k1 ^= uint64_t(tail[ 6]) << 48;

	case  6:
		k1 ^= uint64_t(tail[ 5]) << 40;

	case  5:
		k1 ^= uint64_t(tail[ 4]) << 32;

	case  4:
		k1 ^= uint64_t(tail[ 3]) << 24;

	case  3:
		k1 ^= uint64_t(tail[ 2]) << 16;

	case  2:
		k1 ^= uint64_t(tail[ 1]) << 8;

	case  1:
		k1 ^= uint64_t(tail[ 0]) << 0;
		k1 *= c1;
		k1  = ROTL64(k1, 31);
		k1 *= c2;
		_h1 ^= k1;
	};

	_len += len;
}

void
murmur_inc64::finalize(void *out) {
	_h1 ^= _len;
	_h2 ^= _len;
	_h1 += _h2;
	_h2 += _h1;
	_h1 = fmix(_h1);
	_h2 = fmix(_h2);
	_h1 += _h2;
	_h2 += _h1;
	((uint64_t*)out)[0] = _h1;
	((uint64_t*)out)[1] = _h2;
}
