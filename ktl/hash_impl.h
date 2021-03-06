#pragma once

#include <ktl_core.h>
/*
 * Derived from, with unnecessary code stripped out:
 * Copyright 2020 王一 Wang Yi <godspeed_china@yeah.net>
 * This is free and unencumbered software released into the public domain.http://unlicense.org/
 * See github.com/wangyi-fudan/wyhash/LICENSE
 */

#include <cstdint>
#include <emmintrin.h>

extern "C"
{
    extern __int64 _mm_popcnt_u64(UINT64);
}

namespace ktl
{
//#pragma intrinsic(_umul128)
//#pragma intrinsic(_mm_popcnt_u64)

    //mum function
    static inline uint64_t _wyrot(uint64_t x) { return (x >> 32) | (x << 32); }

    static inline void _wymum(uint64_t* A, uint64_t* B) {
#if defined(_M_X64)
        uint64_t  a, b;
        a = _umul128(*A, *B, &b);
        *A ^= a;  *B ^= b;
#else
        uint64_t ha = *A >> 32, hb = *B >> 32, la = (uint32_t)*A, lb = (uint32_t)*B, hi, lo;
        uint64_t rh = ha * hb, rm0 = ha * lb, rm1 = hb * la, rl = la * lb, t = rl + (rm0 << 32), c = t < rl;
        lo = t + (rm1 << 32); c += lo < t; hi = rh + (rm0 >> 32) + (rm1 >> 32) + c;
        * A ^= lo;  *B ^= hi;
#endif
    }

    static inline uint64_t _wymix(uint64_t A, uint64_t B) { _wymum(&A, &B); return A ^ B; }
    //read functions
#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    static inline uint64_t _wyr8(const uint8_t* p) { uint64_t v; memcpy(&v, p, 8); return v; }
    static inline uint64_t _wyr4(const uint8_t* p) { unsigned v; memcpy(&v, p, 4); return v; }
#else
    static inline uint64_t _wyr8(const uint8_t* p) { uint64_t v; memcpy(&v, p, 8); return _byteswap_uint64(v); }
    static inline uint64_t _wyr4(const uint8_t* p) { unsigned v; memcpy(&v, p, 4); return _byteswap_ulong(v); }
#endif
    static inline uint64_t _wyr3(const uint8_t* p, unsigned k) { return (((uint64_t)p[0]) << 16) | (((uint64_t)p[k >> 1]) << 8) | p[k - 1]; }
    //wyhash function
    static inline uint64_t wyhash(const void* key, uint64_t len, uint64_t seed, const uint64_t* secret) {
        const uint8_t* p = (const uint8_t*)key;  uint64_t a, b; seed ^= *secret;
        if (len <= 16) {
            if (len <= 8) {
                if (len >= 4) { a = _wyr4(p); b = _wyr4(p + len - 4); }
                else if (len) { a = _wyr3(p, (unsigned)len); b = 0; }
                else a = b = 0;
            }
            else { a = _wyr8(p); b = _wyr8(p + len - 8); }
        }
        else {
            uint64_t i = len;
            if (i > 48) {
                uint64_t see1 = seed, see2 = seed;
                do {
                    seed = _wymix(_wyr8(p) ^ secret[1], _wyr8(p + 8) ^ seed);
                    see1 = _wymix(_wyr8(p + 16) ^ secret[2], _wyr8(p + 24) ^ see1);
                    see2 = _wymix(_wyr8(p + 32) ^ secret[3], _wyr8(p + 40) ^ see2);
                    p += 48; i -= 48;
                } while (i > 48);
                seed ^= see1 ^ see2;
            }
            while (i > 16) { seed = _wymix(_wyr8(p) ^ secret[1], _wyr8(p + 8) ^ seed);	i -= 16; p += 16; }
            a = _wyr8(p + i - 16); b = _wyr8(p + i - 8);
        }
        return _wymix(secret[1] ^ len, _wymix(a ^ secret[1], b ^ seed));
    }

    //utility functions
    static const uint64_t _wyp[5] = { 0xa0761d6478bd642full, 0xe7037ed1a0b428dbull, 0x8ebc6af09c88c6e3ull, 0x589965cc75374cc3ull, 0x1d8e4e27c47d124full };
    static inline uint64_t wyhash64(uint64_t A, uint64_t B) { A ^= _wyp[0]; B ^= _wyp[1];  _wymum(&A, &B);  return _wymix(A ^ _wyp[0], B ^ _wyp[1]); }
    static inline uint64_t wyrand(uint64_t* seed) { *seed += _wyp[0]; return _wymix(*seed, *seed ^ _wyp[1]); }
    static inline double wy2u01(uint64_t r) { const double _wynorm = 1.0 / (1ull << 52); return (r >> 12) * _wynorm; }
    static inline double wy2gau(uint64_t r) { const double _wynorm = 1.0 / (1ull << 20); return ((r & 0x1fffff) + ((r >> 21) & 0x1fffff) + ((r >> 42) & 0x1fffff)) * _wynorm - 3.0; }
    static inline uint64_t wy2u0k(uint64_t r, uint64_t k) { _wymum(&r, &k); return k; }
    static inline void make_secret(uint64_t seed, uint64_t* secret) {
        uint8_t c[] = { 15, 23, 27, 29, 30, 39, 43, 45, 46, 51, 53, 54, 57, 58, 60, 71, 75, 77, 78, 83, 85, 86, 89, 90, 92, 99, 101, 102, 105, 106, 108, 113, 114, 116, 120, 135, 139, 141, 142, 147, 149, 150, 153, 154, 156, 163, 165, 166, 169, 170, 172, 177, 178, 180, 184, 195, 197, 198, 201, 202, 204, 209, 210, 212, 216, 225, 226, 228, 232, 240 };
        for (size_t i = 0; i < 5; i++) {
            uint8_t ok;
            do {
                ok = 1; secret[i] = 0;
                for (size_t j = 0; j < 64; j += 8) secret[i] |= ((uint64_t)c[wyrand(&seed) % sizeof(c)]) << j;
                if (secret[i] % 2 == 0) { ok = 0; continue; }
                for (size_t j = 0; j < i; j++)
#if defined(_M_X64)
                    if (_mm_popcnt_u64(secret[j] ^ secret[i]) != 32) { ok = 0; break; }
#endif
                if (!ok)continue;
                for (uint64_t j = 3; j < 0x100000000ull; j += 2) if (secret[i] % j == 0) { ok = 0; break; }
            } while (!ok);
        }
    }
}