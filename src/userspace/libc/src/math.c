#include <math.h>

double fabs(double x) {
    return x < 0 ? -x : x;
}

double sqrt(double x) {
    if (x < 0) return 0;
    double res;
    __asm__ ("sqrtsd %1, %0" : "=x"(res) : "x"(x));
    return res;
}

double sin(double x) {
    double res;
    __asm__ ("fsin" : "=t"(res) : "0"(x));
    return res;
}

double cos(double x) {
    double res;
    __asm__ ("fcos" : "=t"(res) : "0"(x));
    return res;
}

double pow(double x, double y) {
    double res;
    // x^y = 2^(y * log2(x))
    // This is a simplified x87 implementation
    __asm__ (
        "fyl2x;"            // st(1) = y * log2(x), pop st(0)
        "fld %%st(0);"      // duplicate y*log2(x)
        "frndint;"          // round to integer
        "fsubr %%st(1), %%st(0);" // get fractional part
        "f2xm1;"            // 2^(fractional) - 1
        "fld1;"
        "faddp;"             // 2^(fractional)
        "fscale;"            // st(0) * 2^st(1)
        "fstp %%st(1);"      // pop integer part
        : "=t"(res)
        : "0"(x), "u"(y)
    );
    return res;
}

double floor(double x) {
    if (x >= 0) {
        return (double)(long)x;
    } else {
        long i = (long)x;
        if (x == (double)i) return x;
        return (double)(i - 1);
    }
}
