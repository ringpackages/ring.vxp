/* Minimal dynamic array macros for C */

#ifndef DARR_H
#define DARR_H

#include <stdlib.h>
#include <string.h>

typedef struct {
	size_t len;
	size_t cap;
} darr__hdr;

#define darr__h(a)      ((darr__hdr *)((char *)(a) - sizeof(darr__hdr)))
#define arrlen(a)       ((a) ? (int)darr__h(a)->len : 0)
#define arrfree(a)      ((void)((a) ? (free(darr__h(a)), (a) = NULL) : 0))
#define arrput(a, v)    (darr__grow(a, 1), (a)[darr__h(a)->len++] = (v))

#define darr__grow(a, n) \
	(((a) && darr__h(a)->len + (n) <= darr__h(a)->cap) \
	  ? 0 \
	  : ((a) = darr__growf((a), sizeof(*(a)), (n))))

static inline void *darr__growf(void *a, size_t esize, size_t addlen) {
	size_t len = a ? darr__h(a)->len : 0;
	size_t need = len + addlen;
	size_t cap = a ? darr__h(a)->cap : 0;
	if (need <= cap) return a;
	if (need < cap * 2) need = cap * 2;
	if (need < 4) need = 4;
	darr__hdr *h = (darr__hdr *)realloc(a ? darr__h(a) : NULL,
		sizeof(darr__hdr) + esize * need);
	h->cap = need;
	if (!a) h->len = 0;
	return (char *)h + sizeof(darr__hdr);
}

#endif /* DARR_H */