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

#include <x509.h>
#include <monocypher-ed25519.h>

static const uint8_t ed25519_algorithm[] = { 0x06, 0x03, 0x2B, 0x65, 0x70 };

static const uint8_t oid_basic_constraints[] = { 0x55, 0x1D, 0x13 };
static const uint8_t oid_key_usage[] = { 0x55, 0x1D, 0x0F };

static int is_ed25519(const struct der *e)
{
    return e->cls == DER_UNIVERSAL && e->constructed && e->tag == DER_SEQUENCE && der_is(e, ed25519_algorithm, sizeof(ed25519_algorithm));
}

static const uint8_t *bit_string(const struct der *e,
                                 size_t size)
{
    if(e->cls != DER_UNIVERSAL || e->constructed || e->tag != DER_BIT_STRING ||
       e->length != size + 1 || e->content[0] != 0)
    {
        return NULL;
    }
    return e->content + 1;
}

static int basic_constraints(const struct der *value,
                             struct x509 *cert)
{
    struct der seq, e;
    struct der_list list, fields;

    der_open(value, &list);
    if(der_expect(&list, DER_UNIVERSAL, 1, DER_SEQUENCE, &seq) != 0 || !der_end(&list))
    {
        return -1;
    }

    der_open(&seq, &fields);
    int got = der_next(&fields, &e);
    if(got < 0)
    {
        return -1;
    }
    if(got == 1 && e.tag == DER_BOOLEAN && e.cls == DER_UNIVERSAL)
    {
        /* (DER leaves out cA when it is FALSE) */
        if(der_bool(&e, &cert->is_ca) != 0 || !cert->is_ca)
        {
            return -1;
        }
        got = der_next(&fields, &e);
        if(got < 0)
        {
            return -1;
        }
    }
    if(got == 1)
    {
        uint64_t length;
        if(!cert->is_ca || der_uint64(&e, &length) != 0 || length > 255 || !der_end(&fields))
        {
            return -1;
        }
        cert->path_length = (int)length;
    }
    return 0;
}

static int key_usage(const struct der *value,
                     struct x509 *cert)
{
    struct der bits;
    struct der_list list;

    der_open(value, &list);
    if(der_expect(&list, DER_UNIVERSAL, 0, DER_BIT_STRING, &bits) != 0 || !der_end(&list) ||
       bits.length < 2 || bits.length > 3 || bits.content[0] > 7)
    {
        return -1;
    }

    int usage = 0;
    for(size_t i = 1; i < bits.length; i++)
    {
        for(int b = 0; b < 8; b++)
        {
            if(bits.content[i] & (0x80 >> b))
            {
                usage |= 1 << ((i - 1) * 8 + b);
            }
        }
    }

    cert->key_usage = usage;
    return 0;
}

static int extensions(const struct der *wrapper,
                      struct x509 *cert)
{
    struct der all, ext, oid, e, value;
    struct der_list list, exts, fields;
    int seen_basic = 0, seen_usage = 0;

    der_open(wrapper, &list);
    if(der_expect(&list, DER_UNIVERSAL, 1, DER_SEQUENCE, &all) != 0 || !der_end(&list) || all.length == 0)
    {
        return -1;
    }

    der_open(&all, &exts);
    int got;
    while((got = der_next(&exts, &ext)) == 1)
    {
        int critical = 0;

        if(ext.cls != DER_UNIVERSAL || !ext.constructed || ext.tag != DER_SEQUENCE)
        {
            return -1;
        }
        der_open(&ext, &fields);
        if(der_expect(&fields, DER_UNIVERSAL, 0, DER_OID, &oid) != 0 || der_next(&fields, &e) != 1)
        {
            return -1;
        }
        if(e.cls == DER_UNIVERSAL && e.tag == DER_BOOLEAN)
        {
            if(der_bool(&e, &critical) != 0 || !critical || der_next(&fields, &e) != 1)
            {
                return -1;
            }
        }
        if(e.cls != DER_UNIVERSAL || e.constructed || e.tag != DER_OCTET_STRING || !der_end(&fields))
        {
            return -1;
        }
        value = e;

        if(der_is(&oid, oid_basic_constraints, sizeof(oid_basic_constraints)))
        {
            if(seen_basic++ || basic_constraints(&value, cert) != 0)
            {
                return -1;
            }
        }
        else if(der_is(&oid, oid_key_usage, sizeof(oid_key_usage)))
        {
            if(seen_usage++ || key_usage(&value, cert) != 0)
            {
                return -1;
            }
        }
        else if(critical)
        {
            return -1;
        }
    }
    return got;
}

int x509_parse(const uint8_t *p,
               size_t size,
               struct x509 *cert)
{
    struct der certificate, tbs, algorithm, signature, e, spki, spki_algorithm, key;
    struct der_list list, fields, spki_fields;
    uint64_t version;

    cert->is_ca = 0;
    cert->path_length = -1;
    cert->key_usage = -1;

    if(der_read(p, size, &certificate) != 0 || certificate.element_size != size ||
       certificate.cls != DER_UNIVERSAL || !certificate.constructed || certificate.tag != DER_SEQUENCE)
    {
        return -1;
    }

    der_open(&certificate, &list);
    if(der_expect(&list, DER_UNIVERSAL, 1, DER_SEQUENCE, &tbs) != 0 ||
       der_expect(&list, DER_UNIVERSAL, 1, DER_SEQUENCE, &algorithm) != 0 || !is_ed25519(&algorithm) ||
       der_expect(&list, DER_UNIVERSAL, 0, DER_BIT_STRING, &signature) != 0 || !der_end(&list))
    {
        return -1;
    }

    cert->tbs = tbs.element;
    cert->tbs_size = tbs.element_size;
    cert->signature = bit_string(&signature, X509_SIGNATURE_SIZE);
    if(cert->signature == NULL)
    {
        return -1;
    }

    der_open(&tbs, &fields);
    struct der_list version_list;
    if(der_expect(&fields, DER_CONTEXT, 1, 0, &e) != 0)
    {
        return -1;
    }

    der_open(&e, &version_list);
    if(der_expect(&version_list, DER_UNIVERSAL, 0, DER_INTEGER, &e) != 0 || der_uint64(&e, &version) != 0 ||
       version != 2 || !der_end(&version_list))
    {
        return -1;
    }

    if(der_expect(&fields, DER_UNIVERSAL, 0, DER_INTEGER, &e) != 0 || e.length == 0 || e.length > 20 ||
       der_expect(&fields, DER_UNIVERSAL, 1, DER_SEQUENCE, &e) != 0 || !is_ed25519(&e) ||
       der_expect(&fields, DER_UNIVERSAL, 1, DER_SEQUENCE, &e) != 0)
    {
        return -1;
    }
    cert->issuer = e.element;
    cert->issuer_size = e.element_size;
    if(der_expect(&fields, DER_UNIVERSAL, 1, DER_SEQUENCE, &e) != 0 ||
       der_expect(&fields, DER_UNIVERSAL, 1, DER_SEQUENCE, &e) != 0)
    {
        return -1;
    }
    cert->subject = e.element;
    cert->subject_size = e.element_size;

    if(der_expect(&fields, DER_UNIVERSAL, 1, DER_SEQUENCE, &spki) != 0)
    {
        return -1;
    }
    der_open(&spki, &spki_fields);
    if(der_expect(&spki_fields, DER_UNIVERSAL, 1, DER_SEQUENCE, &spki_algorithm) != 0 || !is_ed25519(&spki_algorithm) ||
       der_expect(&spki_fields, DER_UNIVERSAL, 0, DER_BIT_STRING, &key) != 0 || !der_end(&spki_fields))
    {
        return -1;
    }
    cert->public_key = bit_string(&key, X509_KEY_SIZE);
    if(cert->public_key == NULL)
    {
        return -1;
    }

    int got, last = 0;
    while((got = der_next(&fields, &e)) == 1)
    {
        if(e.cls != DER_CONTEXT || e.tag <= (uint32_t)last || e.tag > 3)
        {
            return -1;
        }
        last = (int)e.tag;
        if(e.tag == 3 && (!e.constructed || extensions(&e, cert) != 0))
        {
            return -1;
        }
    }

    return got;
}

int x509_check(const struct x509 *cert,
               const uint8_t issuer_key[X509_KEY_SIZE])
{
    return crypto_ed25519_check(cert->signature, issuer_key, cert->tbs, cert->tbs_size) == 0 ? 0 : -1;
}
