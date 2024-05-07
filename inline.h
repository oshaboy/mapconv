#ifndef INLINE_H
#define INLINE_H
#ifdef __OPTIMIZE__
#define INLINE inline
#else
#define INLINE static
#endif
#endif