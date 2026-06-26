#pragma once
#include <stddef.h>

void *malloc(size_t n);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t n);
void free (void *ptr);

void exit(int status) __attribute__((noreturn));
void abort(void) __attribute__((noreturn));

void srand(unsigned int seed);
int rand(void);

int atoi(const char *s);
long atol(const char *s);
long strtol(const char *s, char **end, int base);
double strtod(const char *nptr, char **endptr);
double atof(const char *nptr);

int  abs(int x);
long labs(long x);

// for screen clear command
//int system(const char *cmd);

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

char *getenv(const char *name);
void  qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
