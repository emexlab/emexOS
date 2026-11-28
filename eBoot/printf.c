/*
 * SPDX-License-Identifier: LicenseRef-EPSL-1.2
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * Licensed under the emexLabs Public Source License, version 1.2
 * (EPSL 1.2).
 *
 * Private use and modification are permitted under the license.
 * Redistribution of source or binaries, modified or unmodified,
 * requires prior written permission from emexLabs.
 *
 * External deployment is subject to the source-availability
 * requirements and other conditions of the license.
 *
 * Provided "AS IS", without warranty, to the extent permitted by law.
 *
 * See LICENSE for the full license terms.
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "printf.h"

#define F_LEFT      (1 << 0)
#define F_ZERO      (1 << 1)
#define F_PLUS      (1 << 2)
#define F_SPACE     (1 << 3)
#define F_ALT       (1 << 4)
#define F_UPPER     (1 << 5)
#define F_POINTER   (1 << 6)

enum length {
    LEN_INT,
    LEN_CHAR,
    LEN_SHORT,
    LEN_LONG,
};

static void put(size_t *count,
                char c)
{
    eboot_putc(c);
    (*count)++;
}

static void pad(size_t *count,
                char c,
                int n)
{
    while(n-- > 0)
    {
        put(count, c);
    }
}

static void put_number(size_t *count,
                       uint64_t value,
                       int negative,
                       unsigned base,
                       int flags,
                       int width,
                       int precision)
{
    const char *set = (flags & F_UPPER) ? "0123456789ABCDEF" : "0123456789abcdef";
    char digits[24];
    int n = 0;
    int nonzero = value != 0;

    if(value != 0 || precision != 0)
    {
        do
        {
            digits[n++] = set[value % base];
            value /= base;
        } while(value != 0);
    }

    int zeros = precision > n ? precision - n : 0;
    char sign = negative ? '-' : (flags & F_PLUS) ? '+' : (flags & F_SPACE) ? ' ' : 0;

    int prefix = 0;
    if(base == 16 && ((flags & F_POINTER) || ((flags & F_ALT) && nonzero)))
    {
        prefix = 2;
    }
    if(base == 8 && (flags & F_ALT) && zeros == 0 && (n == 0 || digits[n - 1] != '0'))
    {
        zeros = 1;
    }

    int length = n + zeros + (sign != 0) + prefix;
    int fill = width > length ? width - length : 0;
    int zero_fill = (flags & F_ZERO) && !(flags & F_LEFT) && precision < 0;

    if(!(flags & F_LEFT) && !zero_fill)
    {
        pad(count, ' ', fill);
    }
    if(sign != 0)
    {
        put(count, sign);
    }
    if(prefix != 0)
    {
        put(count, '0');
        put(count, (flags & F_UPPER) ? 'X' : 'x');
    }
    if(zero_fill)
    {
        pad(count, '0', fill);
    }
    pad(count, '0', zeros);
    while(n > 0)
    {
        put(count, digits[--n]);
    }
    if(flags & F_LEFT)
    {
        pad(count, ' ', fill);
    }
}

static int64_t signed_arg(va_list *ap,
                          enum length length)
{
    if(length == LEN_LONG)
    {
        return va_arg(*ap, long long);
    }
    int value = va_arg(*ap, int);
    if(length == LEN_CHAR)
    {
        return (signed char)value;
    }
    if(length == LEN_SHORT)
    {
        return (short)value;
    }
    return value;
}

static uint64_t unsigned_arg(va_list *ap,
                             enum length length)
{
    if(length == LEN_LONG)
    {
        return va_arg(*ap, unsigned long long);
    }
    unsigned int value = va_arg(*ap, unsigned int);
    if(length == LEN_CHAR)
    {
        return (unsigned char)value;
    }
    if(length == LEN_SHORT)
    {
        return (unsigned short)value;
    }
    return value;
}

static int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int vprintf(const char *fmt,
            va_list ap)
{
    size_t count = 0;
    va_list args;

    va_copy(args, ap);

    while(*fmt != 0)
    {
        if(*fmt != '%')
        {
            put(&count, *fmt++);
            continue;
        }
        const char *start = fmt++;

        int flags = 0;
        for(;; fmt++)
        {
            if(*fmt == '-')
            {
                flags |= F_LEFT;
            }
            else if(*fmt == '0')
            {
                flags |= F_ZERO;
            }
            else if(*fmt == '+')
            {
                flags |= F_PLUS;
            }
            else if(*fmt == ' ')
            {
                flags |= F_SPACE;
            }
            else if(*fmt == '#')
            {
                flags |= F_ALT;
            }
            else
            {
                break;
            }
        }

        int width = 0;
        if(*fmt == '*')
        {
            width = va_arg(args, int);
            if(width < 0)
            {
                flags |= F_LEFT;
                width = -width;
            }
            fmt++;
        }
        else
        {
            while(is_digit(*fmt))
            {
                width = width * 10 + (*fmt++ - '0');
            }
        }

        int precision = -1;
        if(*fmt == '.')
        {
            fmt++;
            precision = 0;
            if(*fmt == '*')
            {
                precision = va_arg(args, int);
                if(precision < 0)
                {
                    precision = -1;
                }
                fmt++;
            }
            else
            {
                while(is_digit(*fmt))
                {
                    precision = precision * 10 + (*fmt++ - '0');
                }
            }
        }

        enum length length = LEN_INT;
        if(*fmt == 'h')
        {
            fmt++;
            length = LEN_SHORT;
            if(*fmt == 'h')
            {
                fmt++;
                length = LEN_CHAR;
            }
        }
        else if(*fmt == 'l')
        {
            fmt++;
            length = LEN_LONG;
            if(*fmt == 'l')
            {
                fmt++;
            }
        }
        else if(*fmt == 'z' || *fmt == 'j' || *fmt == 't')
        {
            fmt++;
            length = LEN_LONG;
        }

        char conversion = *fmt;
        if(conversion == 0)
        {
            while(*start != 0)
            {
                put(&count, *start++);
            }
            break;
        }
        fmt++;

        if(conversion == 'd' || conversion == 'i')
        {
            int64_t value = signed_arg(&args, length);
            uint64_t magnitude = value < 0 ? (uint64_t)0 - (uint64_t)value : (uint64_t)value;
            put_number(&count, magnitude, value < 0, 10, flags, width, precision);
        }
        else if(conversion == 'u')
        {
            put_number(&count, unsigned_arg(&args, length), 0, 10, flags & ~(F_PLUS | F_SPACE), width, precision);
        }
        else if(conversion == 'x' || conversion == 'X')
        {
            int upper = conversion == 'X' ? F_UPPER : 0;
            put_number(&count, unsigned_arg(&args, length), 0, 16, (flags | upper) & ~(F_PLUS | F_SPACE), width, precision);
        }
        else if(conversion == 'o')
        {
            put_number(&count, unsigned_arg(&args, length), 0, 8, flags & ~(F_PLUS | F_SPACE), width, precision);
        }
        else if(conversion == 'p')
        {
            uint64_t value = (uint64_t)(uintptr_t)va_arg(args, void *);
            put_number(&count, value, 0, 16, (flags | F_POINTER) & ~(F_PLUS | F_SPACE), width, precision);
        }
        else if(conversion == 'c')
        {
            char c = (char)va_arg(args, int);
            if(!(flags & F_LEFT))
            {
                pad(&count, ' ', width - 1);
            }
            put(&count, c);
            if(flags & F_LEFT)
            {
                pad(&count, ' ', width - 1);
            }
        }
        else if(conversion == 's')
        {
            const char *s = va_arg(args, const char *);
            if(s == 0)
            {
                s = "(null)";
            }
            int n = 0;
            while(s[n] != 0 && (precision < 0 || n < precision))
            {
                n++;
            }
            if(!(flags & F_LEFT))
            {
                pad(&count, ' ', width - n);
            }
            for(int i = 0; i < n; i++)
            {
                put(&count, s[i]);
            }
            if(flags & F_LEFT)
            {
                pad(&count, ' ', width - n);
            }
        }
        else if(conversion == '%')
        {
            put(&count, '%');
        }
        else
        {
            while(start < fmt)
            {
                put(&count, *start++);
            }
        }
    }

    va_end(args);
    return (int)count;
}

int printf(const char *fmt,
           ...)
{
    va_list ap;
    va_start(ap, fmt);
    int count = vprintf(fmt, ap);
    va_end(ap);
    return count;
}
