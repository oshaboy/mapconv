#ifndef UTF8_H
#define UTF8_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "inline.h"
INLINE uint32_t byte_reverse(uint32_t a){
    char buf[sizeof(uint32_t)];
    memcpy(buf, &a, sizeof(uint32_t));
    for (int i=0; i<(sizeof(uint32_t)/2); i++){
        char tmp=buf[i];
        buf[i]=buf[sizeof(uint32_t)-1-i];
        buf[sizeof(uint32_t)-1-i]=tmp;
    }
    memcpy(&a, buf, sizeof(uint32_t));
    return a;
}


INLINE size_t codepoint_length_in_utf8(int codepoint){
    static const unsigned char results[]={
        1,1,1,1,1,1,1,1,
        2,2,2,2,
        3,3,3,3,3,
        4,4,4,4,4,
        0,0,0,0,0,0,0,0,0,0

    };
    return results[32-__builtin_clz(codepoint)];
}

INLINE size_t find_length_utf8(unsigned char c){
    static const unsigned char results[]={1,0,2,3,4,0,0,0};
    return results[__builtin_clz(((unsigned int)(~c))<<24)];
}

INLINE int parse_utf8(const unsigned char * str, size_t * s){
    size_t _;
    if (!s) s=&_;

    *s = find_length_utf8(*str);
    if (*s==0) return -1;
    uint32_t buf=0;
    memcpy(&buf, str, sizeof(uint32_t));
    const static uint32_t validation_bitmasks[]={0,0xc0, 0xc0c0,0xc0c0c0 };
    const static uint32_t validation_checks[]={0,0x80, 0x8080,0x808080 };
    const static uint32_t stow_bitmasks[]={0x7f, 0x1f3f, 0xf3f3f, 0x73f3f3f};
    buf=byte_reverse(buf)>>((4-*s)*8);
    if ((buf & validation_bitmasks[*s-1]) != validation_checks[*s-1]){
        *s=0;
        return -1;
    }
    buf&=stow_bitmasks[*s-1];
    for (int i=0; i<4; i++){
        uint32_t mask = (unsigned)0xff<<(i*8);
        buf=((buf&(~mask)) | ((buf & mask)>>(i*2)));
    }
    return buf;
}

typedef struct {
    unsigned char str[5];
    unsigned char len : 3;
} UTF8_Bytes;

INLINE UTF8_Bytes put_utf8(int codepoint){
    size_t s=codepoint_length_in_utf8(codepoint);
    if (s == 0){
        return (UTF8_Bytes) {
            .str={-1,-1,-1,-1,-1},
            .len=0
        };
    }
    const static uint32_t bitmasks[]={0,0xc080, 0xe08080,0xf0808080};
    uint32_t buf=0;
    const unsigned char mask=(s==1?0x7f:0x3f);
    for (int i=0; i<4; i++)
        buf |= ((codepoint & (mask<<(i*6)))<<(i*2));
    
    buf|=bitmasks[s-1];
    buf=byte_reverse(buf)>>((4-s)*8);
    UTF8_Bytes result={
        .str={0},
        .len=s
    };
    memcpy(result.str, &buf, sizeof(uint32_t));
    return result;
}
#endif