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

#include <img4.h>
#include <monocypher-ed25519.h>

const struct img4_device img4_this_device = {
    .chip = IMG4_CHIP,
    .board = IMG4_BOARD,
    .ecid = 0,
    .nonce_hash = { 0 },
    .epoch = 0,
};

static int is_fourcc(const struct der *e,
                     uint32_t fourcc)
{
    uint8_t bytes[4] = { fourcc >> 24, fourcc >> 16, fourcc >> 8, fourcc };
    return e->cls == DER_UNIVERSAL && !e->constructed && e->tag == DER_IA5_STRING && der_is(e, bytes, 4);
}

static int property(const struct der *e,
                    uint32_t *name,
                    struct der *value)
{
    struct der seq, label;
    struct der_list list, fields;

    if(e->cls != DER_PRIVATE || !e->constructed)
    {
        return -1;
    }
    der_open(e, &list);
    if(der_expect(&list, DER_UNIVERSAL, 1, DER_SEQUENCE, &seq) != 0 || !der_end(&list))
    {
        return -1;
    }
    der_open(&seq, &fields);
    if(der_next(&fields, &label) != 1 || !is_fourcc(&label, e->tag) || der_next(&fields, value) != 1 || !der_end(&fields))
    {
        return -1;
    }
    *name = e->tag;
    return 0;
}

static int property_set(const struct der *e,
                        uint32_t *name,
                        struct der *set)
{
    if(property(e, name, set) != 0 || set->cls != DER_UNIVERSAL || !set->constructed || set->tag != DER_SET)
    {
        return -1;
    }
    return 0;
}

#define SEEN_CHIP   (1 << 0)
#define SEEN_BORD   (1 << 1)
#define SEEN_ECID   (1 << 2)
#define SEEN_BNCH   (1 << 3)
#define SEEN_CEPO   (1 << 4)
#define SEEN_ALL    (SEEN_CHIP | SEEN_BORD | SEEN_ECID | SEEN_BNCH | SEEN_CEPO)

static int check_manp(const struct der *set,
                      const struct img4_device *device)
{
    struct der e, value;
    struct der_list list;
    uint32_t name;
    uint64_t number;
    int seen = 0, got;

    der_open(set, &list);
    while((got = der_next(&list, &e)) == 1)
    {
        if(property(&e, &name, &value) != 0)
        {
            return IMG4_ERR_FORMAT;
        }
        int bit = name == IMG4_FOURCC("CHIP") ? SEEN_CHIP :
                  name == IMG4_FOURCC("BORD") ? SEEN_BORD :
                  name == IMG4_FOURCC("ECID") ? SEEN_ECID :
                  name == IMG4_FOURCC("BNCH") ? SEEN_BNCH :
                  name == IMG4_FOURCC("CEPO") ? SEEN_CEPO : 0;
        if(bit == 0)
        {
            continue;
        }
        if(seen & bit)
        {
            return IMG4_ERR_FORMAT;
        }
        seen |= bit;

        if(bit == SEEN_BNCH)
        {
            if(value.cls != DER_UNIVERSAL || value.constructed || value.tag != DER_OCTET_STRING)
            {
                return IMG4_ERR_FORMAT;
            }
            if(!der_is(&value, device->nonce_hash, IMG4_NONCE_HASH_SIZE))
            {
                return IMG4_ERR_DEVICE;
            }
            continue;
        }

        if(der_uint64(&value, &number) != 0)
        {
            return IMG4_ERR_FORMAT;
        }
        if((bit == SEEN_CHIP && number != device->chip) ||
           (bit == SEEN_BORD && number != device->board) ||
           (bit == SEEN_ECID && number != device->ecid))
        {
            return IMG4_ERR_DEVICE;
        }
        if(bit == SEEN_CEPO && number < device->epoch)
        {
            return IMG4_ERR_EPOCH;
        }
    }
    if(got < 0 || seen != SEEN_ALL)
    {
        return IMG4_ERR_FORMAT;
    }
    return IMG4_OK;
}

static int image_digest(const struct der *set,
                        const uint8_t **digest)
{
    struct der e, value;
    struct der_list list;
    uint32_t name;
    int got;

    *digest = NULL;
    der_open(set, &list);
    while((got = der_next(&list, &e)) == 1)
    {
        if(property(&e, &name, &value) != 0)
        {
            return IMG4_ERR_FORMAT;
        }
        if(name != IMG4_FOURCC("DGST"))
        {
            continue;
        }
        if(*digest != NULL || value.cls != DER_UNIVERSAL || value.constructed || value.tag != DER_OCTET_STRING ||
           value.length != IMG4_DIGEST_SIZE)
        {
            return IMG4_ERR_FORMAT;
        }
        *digest = value.content;
    }
    if(got < 0 || *digest == NULL)
    {
        return IMG4_ERR_FORMAT;
    }
    return IMG4_OK;
}

static int check_chain(const struct x509 *chain,
                       int count,
                       const struct img4_root *root)
{
    const uint8_t *issuer_key = root->public_key;
    const uint8_t *issuer_name = root->subject;
    size_t issuer_name_size = root->subject_size;

    for(int i = 0; i < count; i++)
    {
        const struct x509 *cert = &chain[i];
        int last = i == count - 1;

        if(cert->issuer_size != issuer_name_size || !der_same(cert->issuer, issuer_name, issuer_name_size))
        {
            return IMG4_ERR_CHAIN;
        }
        if(x509_check(cert, issuer_key) != 0)
        {
            return IMG4_ERR_CHAIN;
        }

        if(!last)
        {
            /* a CA that may sign certificates, and has room for the CAs below it */
            int below = count - 2 - i;
            if(!cert->is_ca || (cert->key_usage >= 0 && !(cert->key_usage & X509_KEY_CERT_SIGN)) ||
               (cert->path_length >= 0 && below > cert->path_length))
            {
                return IMG4_ERR_CERTIFICATE;
            }
        }
        else
        {
            /* the signing certificate */
            if(cert->is_ca || (cert->key_usage >= 0 && !(cert->key_usage & X509_DIGITAL_SIGNATURE)))
            {
                return IMG4_ERR_CERTIFICATE;
            }
        }

        issuer_key = cert->public_key;
        issuer_name = cert->subject;
        issuer_name_size = cert->subject_size;
    }
    return IMG4_OK;
}

int img4_verify(const uint8_t *file,
                size_t size,
                const char *type,
                const struct img4_root *root,
                const struct img4_device *device,
                struct img4_image *image)
{
    struct der img4, e, im4p, im4m, body, signature, certs, manb, entries;
    struct der_list list, fields, items;
    struct x509 chain[IMG4_MAX_CHAIN];
    uint64_t version;
    uint32_t name;
    int count = 0, got, error;
    uint32_t want = IMG4_FOURCC(type);

    if(der_read(file, size, &img4) != 0 || img4.element_size != size ||
       img4.cls != DER_UNIVERSAL || !img4.constructed || img4.tag != DER_SEQUENCE)
    {
        return IMG4_ERR_FORMAT;
    }
    der_open(&img4, &list);
    if(der_next(&list, &e) != 1 || !is_fourcc(&e, IMG4_FOURCC("IMG4")) ||
       der_expect(&list, DER_UNIVERSAL, 1, DER_SEQUENCE, &im4p) != 0 ||
       der_expect(&list, DER_CONTEXT, 1, 0, &e) != 0)
    {
        return IMG4_ERR_FORMAT;
    }
    der_open(&e, &items);
    if(der_expect(&items, DER_UNIVERSAL, 1, DER_SEQUENCE, &im4m) != 0 || !der_end(&items))
    {
        return IMG4_ERR_FORMAT;
    }

    got = der_next(&list, &e);
    if(got < 0 || (got == 1 && (e.cls != DER_CONTEXT || e.tag != 1 || !der_end(&list))))
    {
        return IMG4_ERR_FORMAT;
    }

    /* IM4P (no keybag andddd no compression yet sadly :<) */
    der_open(&im4p, &fields);
    if(der_next(&fields, &e) != 1 || !is_fourcc(&e, IMG4_FOURCC("IM4P")))
    {
        return IMG4_ERR_FORMAT;
    }
    if(der_expect(&fields, DER_UNIVERSAL, 0, DER_IA5_STRING, &e) != 0 || e.length != 4)
    {
        return IMG4_ERR_FORMAT;
    }
    image->type = IMG4_FOURCC(e.content);
    if(der_expect(&fields, DER_UNIVERSAL, 0, DER_IA5_STRING, &e) != 0)
    {
        return IMG4_ERR_FORMAT;
    }
    image->description = e.content;
    image->description_size = e.length;
    if(der_expect(&fields, DER_UNIVERSAL, 0, DER_OCTET_STRING, &e) != 0 || !der_end(&fields))
    {
        return IMG4_ERR_FORMAT;
    }
    image->data = e.content;
    image->size = e.length;

    /* IM4M */
    der_open(&im4m, &fields);
    if(der_next(&fields, &e) != 1 || !is_fourcc(&e, IMG4_FOURCC("IM4M")) ||
       der_expect(&fields, DER_UNIVERSAL, 0, DER_INTEGER, &e) != 0 || der_uint64(&e, &version) != 0 || version != 0 ||
       der_expect(&fields, DER_UNIVERSAL, 1, DER_SET, &body) != 0 ||
       der_expect(&fields, DER_UNIVERSAL, 0, DER_OCTET_STRING, &signature) != 0 || signature.length != X509_SIGNATURE_SIZE ||
       der_expect(&fields, DER_UNIVERSAL, 1, DER_SEQUENCE, &certs) != 0 || !der_end(&fields))
    {
        return IMG4_ERR_FORMAT;
    }

    /* the chain, then the manifest's signature (nothing in the manifest is used before both hold) */
    der_open(&certs, &items);
    while((got = der_next(&items, &e)) == 1)
    {
        if(count == IMG4_MAX_CHAIN || x509_parse(e.element, e.element_size, &chain[count]) != 0)
        {
            return IMG4_ERR_CERTIFICATE;
        }
        count++;
    }
    if(got < 0 || count == 0)
    {
        return IMG4_ERR_CERTIFICATE;
    }
    error = check_chain(chain, count, root);
    if(error != IMG4_OK)
    {
        return error;
    }
    if(crypto_ed25519_check(signature.content, chain[count - 1].public_key, body.element, body.element_size) != 0)
    {
        return IMG4_ERR_SIGNATURE;
    }

    /* MANB (MANP and the entry for this type) */
    der_open(&body, &items);
    if(der_next(&items, &e) != 1 || property_set(&e, &name, &manb) != 0 || name != IMG4_FOURCC("MANB") || !der_end(&items))
    {
        return IMG4_ERR_FORMAT;
    }
    int seen_manp = 0, seen_image = 0;
    const uint8_t *digest = NULL;
    der_open(&manb, &items);
    while((got = der_next(&items, &e)) == 1)
    {
        if(property_set(&e, &name, &entries) != 0)
        {
            return IMG4_ERR_FORMAT;
        }
        if(name == IMG4_FOURCC("MANP"))
        {
            if(seen_manp++)
            {
                return IMG4_ERR_FORMAT;
            }
            error = check_manp(&entries, device);
            if(error != IMG4_OK)
            {
                return error;
            }
        }
        else if(name == want)
        {
            if(seen_image++)
            {
                return IMG4_ERR_FORMAT;
            }
            error = image_digest(&entries, &digest);
            if(error != IMG4_OK)
            {
                return error;
            }
        }
    }
    if(got < 0 || !seen_manp)
    {
        return IMG4_ERR_FORMAT;
    }
    if(!seen_image || image->type != want)
    {
        return IMG4_ERR_TYPE;
    }

    /* the payload the manifest vouches for (the SHA-512 of the whole IM4P) */
    uint8_t hash[IMG4_DIGEST_SIZE];
    crypto_sha512(hash, im4p.element, im4p.element_size);
    if(!der_same(hash, digest, IMG4_DIGEST_SIZE))
    {
        return IMG4_ERR_DIGEST;
    }
    return IMG4_OK;
}

const char *img4_error(int error)
{
    switch(error)
    {
        case IMG4_OK:
            return "valid";
        case IMG4_ERR_FORMAT:
            return "not a valid Image4 file";
        case IMG4_ERR_CERTIFICATE:
            return "has a certificate that cannot sign it";
        case IMG4_ERR_CHAIN:
            return "is not signed by a chain from this BootROM's root key";
        case IMG4_ERR_SIGNATURE:
            return "has a manifest that is not signed by its chain";
        case IMG4_ERR_DEVICE:
            return "has a manifest for another device";
        case IMG4_ERR_EPOCH:
            return "has a manifest older than this device allows";
        case IMG4_ERR_TYPE:
            return "is not a payload of the type expected";
        case IMG4_ERR_DIGEST:
            return "has a payload its manifest does not vouch for";
        default:
            return "is not valid";
    }
}
