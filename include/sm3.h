#ifndef SM3_H
#define SM3_H

#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PUTU32(p, V)                                                                                                   \
    ((p)[0] = (uint8_t)((V) >> 24), (p)[1] = (uint8_t)((V) >> 16), (p)[2] = (uint8_t)((V) >> 8), (p)[3] = (uint8_t)(V))
#define GETU32(p) ((uint32_t)(p)[0] << 24 | (uint32_t)(p)[1] << 16 | (uint32_t)(p)[2] << 8 | (uint32_t)(p)[3])
#define ROL32(a, n) (((a) << (n)) | (((a)&0xffffffff) >> (32 - (n))))

#define SM3_IS_BIG_ENDIAN 1

#define SM3_DIGEST_SIZE 32
#define SM3_BLOCK_SIZE 64
#define SM3_STATE_WORDS 8
#define SM3_HMAC_SIZE (SM3_DIGEST_SIZE)

struct SM3Context {
    uint32_t digest[SM3_STATE_WORDS];
    uint64_t nblocks;
    uint8_t block[SM3_BLOCK_SIZE];
    size_t num;
};

typedef struct SM3Context SM3_CTX;

void SM3_Init(SM3_CTX *);
void SM3_Update(SM3_CTX *, unsigned const char *, unsigned);
void SM3_Final(SM3_CTX *, unsigned char digest[SM3_DIGEST_SIZE]);
void SM3_Digest(const unsigned char *, size_t, unsigned char dgst[SM3_DIGEST_SIZE]);
#ifdef __cplusplus
}
#endif

#endif /* SM3_H */
