#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "_syscall.h"

// bump allocator via brk
static char *_heap = NULL;


void *malloc(size_t n) {
    if (!n) return NULL;
    if (!_heap) _heap = (char *)_sc1(_SCAL_BRK, 0);
    n = (n + 15) & ~(size_t)15; // align 16
    char *p = _heap, *end = _heap + n;
    if ((char *)_sc1(_SCAL_BRK, (long)end) < end) { errno = ENOMEM; return NULL; }
    _heap = end;
    return p;
}

void *calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void *p = malloc(total);
    if (p) memset(p, 0, total);
    return p;
}

void *realloc(void *ptr, size_t n) {
    if (!ptr) return malloc(n);
    void *p = malloc(n);
    if (p) memcpy(p, ptr, n); // may copy garbage if n > old size, acceptable
    return p;
}

void free(void *ptr) { (void)ptr; } // no-op with bump allocator


// _exit in unistd.c
extern void _exit(int status) __attribute__((noreturn));

void exit(int status) { _exit(status); __builtin_unreachable(); }

void abort(void) { _exit(1); }

long strtol(const char *s, char **end, int base) {
    while (*s == ' ') s++;
    int neg = (*s == '-'); if (neg || *s == '+') s++;
    if (base == 0) {
        if (*s == '0' && (s[1]=='x'||s[1]=='X')) { base=16; s+=2; }
        else if (*s == '0') { base=8; }
        else base=10;
    } else if (base==16 && *s=='0' && (s[1]=='x'||s[1]=='X')) s+=2;
    long v = 0;
    while (*s) {
        int d;
        if (*s>='0'&&*s<='9') d=*s-'0';
        else if (*s>='a'&&*s<='z') d=*s-'a'+10;
        else if (*s>='A'&&*s<='Z') d=*s-'A'+10;
        else break;
        if (d >= base) break;
        v = v*base + d; s++;
    }
    if (end) *end = (char *)s;
    return neg ? -v : v;
}

static unsigned int rng_state = 123456789;

void srand(unsigned int seed)
{
    rng_state = seed;
}

int rand(void)
{
    rng_state = rng_state * 1103515245 + 12345;
    return (int)((rng_state >> 16) & 0x7FFF);
}

double strtod(const char *nptr, char **endptr)
{
    while (
    	*nptr == ' ' 	||
     	*nptr == '\t' 	||
      	*nptr == '\n' 	||
       	*nptr == '\r'
    ) {  nptr++; }

    int sign = 1;

    if (*nptr == '-')
    {
        sign = -1;
        nptr++;
    } else if (*nptr == '+') {
        nptr++;
    }

    double value = 0.0;

    while (*nptr >= '0' && *nptr <= '9') {
        value = value * 10.0 + (*nptr - '0');
        nptr++;
    }

    if (*nptr == '.')
    {
        nptr++;
        double fraction = 1.0;

        while (*nptr >= '0' && *nptr <= '9')
        {
            fraction *= 10.0;
            value += (*nptr - '0') / fraction;
            nptr++;
        }
    }

    if (*nptr == 'e' || *nptr == 'E')
    {
        nptr++;
        int exp_sign = 1;

        if (*nptr == '-')
        {
            exp_sign = -1;
            nptr++;
        } else if (*nptr == '+')
        {
            nptr++;
        }

        int exp = 0;
        while (*nptr >= '0' && *nptr <= '9')
        {
            exp = exp * 10 + (*nptr - '0');
            nptr++;
        }

        double multiplier = 1.0;
        for (int i = 0; i < exp; i++)
        {
            multiplier *= 10.0;
        }

        if (exp_sign == -1)
        {
            value /= multiplier;
        } else
        {
            value *= multiplier;
        }
    }

    if (endptr)
    {
        *endptr = (char *)nptr;
    }

    return sign * value;
}

double atof(const char *nptr)
{
    return strtod(nptr, NULL);
}

int atoi(const char *s) { return (int)strtol(s, NULL, 10); }
long atol(const char *s)  { return strtol(s, NULL, 10); }
int abs (int x) { return x < 0 ? -x : x; }
long labs(long x) { return x < 0 ? -x : x; }

/* just there for DOOOOOOOOOOOOOM fahhh*/
char *getenv(const char *name) {
	(void)name;
	return NULL;
}

void qsort(
	void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *)
){
    unsigned char *b = (unsigned char *)base;
    unsigned char  tmp[256]; /* max element size we support */
    if (size > sizeof(tmp)) return;

    for (size_t i = 1; i < nmemb; i++)
    {
        memcpy(tmp, b + i * size, size);
        size_t j = i;

        while (j > 0 && compar(b + (j - 1) * size, tmp) > 0)
        {
            memcpy(b + j * size, b + (j - 1) * size, size);
            j--;
        }
        memcpy(b + j * size, tmp, size);
    }
}

