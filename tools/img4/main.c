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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __APPLE__
#include <sys/random.h>
#endif /* __APPLE__ */

#include <monocypher.h>
#include <monocypher-ed25519.h>
#include <img4.h>

#define SECRET_KEY_SIZE     64

struct buf {
    uint8_t *p;
    size_t n;
    size_t cap;
};

static void put(struct buf *b,
                const void *data,
                size_t n)
{
    if(b->n + n > b->cap)
    {
        size_t cap = b->cap ? b->cap : 256;
        while(cap < b->n + n)
        {
            cap *= 2;
        }
        b->p = realloc(b->p, cap);
        if(b->p == NULL)
        {
            fprintf(stderr, "img4: out of memory\n");
            exit(1);
        }
        b->cap = cap;
    }
    memcpy(b->p + b->n, data, n);
    b->n += n;
}

static void put_byte(struct buf *b,
                     uint8_t byte)
{
    put(b, &byte, 1);
}

static void tlv(struct buf *out,
                int cls,
                int constructed,
                uint32_t tag,
                const void *content,
                size_t length)
{
    uint8_t first = (uint8_t)(cls << 6) | (uint8_t)(constructed << 5);
    if(tag < 0x1F)
    {
        put_byte(out, first | (uint8_t)tag);
    }
    else
    {
        uint8_t digits[5];
        int n = 0;
        do
        {
            digits[n++] = tag & 0x7F;
            tag >>= 7;
        } while(tag != 0);
        put_byte(out, first | 0x1F);
        while(n-- > 0)
        {
            put_byte(out, digits[n] | (n > 0 ? 0x80 : 0));
        }
    }

    if(length < 0x80)
    {
        put_byte(out, (uint8_t)length);
    }
    else
    {
        uint8_t digits[8];
        int n = 0;
        for(size_t l = length; l != 0; l >>= 8)
        {
            digits[n++] = (uint8_t)l;
        }
        put_byte(out, 0x80 | (uint8_t)n);
        while(n-- > 0)
        {
            put_byte(out, digits[n]);
        }
    }
    put(out, content, length);
}

static void wrap(struct buf *out,
                 int cls,
                 int constructed,
                 uint32_t tag,
                 struct buf *content)
{
    tlv(out, cls, constructed, tag, content->p, content->n);
    free(content->p);
    *content = (struct buf){ 0 };
}

static void sequence(struct buf *out,
                     struct buf *content)
{
    wrap(out, DER_UNIVERSAL, 1, DER_SEQUENCE, content);
}

static void integer(struct buf *out,
                    uint64_t value)
{
    uint8_t bytes[9];
    int n = 0;
    do
    {
        bytes[8 - n++] = (uint8_t)value;
        value >>= 8;
    } while(value != 0);
    if(bytes[9 - n] & 0x80)
    {
        bytes[8 - n++] = 0;
    }
    tlv(out, DER_UNIVERSAL, 0, DER_INTEGER, bytes + 9 - n, (size_t)n);
}

static void boolean(struct buf *out,
                    int value)
{
    uint8_t byte = value ? 0xFF : 0x00;
    tlv(out, DER_UNIVERSAL, 0, DER_BOOLEAN, &byte, 1);
}

static void ia5(struct buf *out,
                const char *text)
{
    tlv(out, DER_UNIVERSAL, 0, DER_IA5_STRING, text, strlen(text));
}

static int compare_encodings(const void *a,
                             const void *b)
{
    const struct buf *x = a;
    const struct buf *y = b;
    size_t n = x->n < y->n ? x->n : y->n;
    int c = memcmp(x->p, y->p, n);
    return c != 0 ? c : (x->n > y->n) - (x->n < y->n);
}

static void set(struct buf *out,
                struct buf *elements,
                int count)
{
    struct buf content = { 0 };
    qsort(elements, (size_t)count, sizeof(*elements), compare_encodings);
    for(int i = 0; i < count; i++)
    {
        put(&content, elements[i].p, elements[i].n);
        free(elements[i].p);
        elements[i] = (struct buf){ 0 };
    }
    wrap(out, DER_UNIVERSAL, 1, DER_SET, &content);
}

static void property(struct buf *out,
                     const char *name,
                     struct buf *value)
{
    struct buf seq = { 0 }, body = { 0 };
    ia5(&body, name);
    put(&body, value->p, value->n);
    free(value->p);
    *value = (struct buf){ 0 };
    sequence(&seq, &body);
    wrap(out, DER_PRIVATE, 1, IMG4_FOURCC(name), &seq);
}

static void property_integer(struct buf *out,
                             const char *name,
                             uint64_t number)
{
    struct buf value = { 0 };
    integer(&value, number);
    property(out, name, &value);
}

static void property_bool(struct buf *out,
                          const char *name,
                          int flag)
{
    struct buf value = { 0 };
    boolean(&value, flag);
    property(out, name, &value);
}

static void property_bytes(struct buf *out,
                           const char *name,
                           const uint8_t *bytes,
                           size_t size)
{
    struct buf value = { 0 };
    tlv(&value, DER_UNIVERSAL, 0, DER_OCTET_STRING, bytes, size);
    property(out, name, &value);
}

static int fail(const char *what,
                const char *path)
{
    fprintf(stderr, "img4: %s %s (%s)\n", what, path, strerror(errno));
    return 1;
}

static uint8_t *read_file(const char *path,
                          size_t *size)
{
    FILE *f = fopen(path, "rb");
    if(f == NULL)
    {
        return NULL;
    }
    struct buf b = { 0 };
    uint8_t chunk[65536];
    size_t n;
    while((n = fread(chunk, 1, sizeof(chunk), f)) > 0)
    {
        put(&b, chunk, n);
    }
    int error = ferror(f);
    fclose(f);
    if(error)
    {
        free(b.p);
        return NULL;
    }
    *size = b.n;
    return b.p != NULL ? b.p : malloc(1);
}

static int write_file(const char *path,
                      const void *data,
                      size_t size,
                      int secret)
{
    int fd = secret ? open(path, O_WRONLY | O_CREAT | O_EXCL, 0600) : open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd < 0)
    {
        return fail("cannot create", path);
    }
    if(write(fd, data, size) != (ssize_t)size || close(fd) != 0)
    {
        unlink(path);
        return fail("cannot write", path);
    }
    return 0;
}

static void path_in(char *out,
                    size_t size,
                    const char *folder,
                    const char *name)
{
    snprintf(out, size, "%s/%s", folder, name);
}

static int read_key(const char *folder,
                    const char *name,
                    uint8_t key[SECRET_KEY_SIZE])
{
    char path[4096];
    size_t size;
    path_in(path, sizeof(path), folder, name);
    uint8_t *data = read_file(path, &size);
    if(data == NULL)
    {
        return fail("cannot read", path);
    }
    if(size != SECRET_KEY_SIZE)
    {
        crypto_wipe(data, size);
        free(data);
        fprintf(stderr, "img4: %s is not a key made by img4 init\n", path);
        return 1;
    }
    memcpy(key, data, SECRET_KEY_SIZE);
    crypto_wipe(data, size);
    free(data);
    return 0;
}

struct cert_file {
    uint8_t *der;
    size_t size;
    struct x509 cert;
};

static int read_cert(const char *folder,
                     const char *name,
                     struct cert_file *c)
{
    char path[4096];
    path_in(path, sizeof(path), folder, name);
    c->der = read_file(path, &c->size);
    if(c->der == NULL)
    {
        return fail("cannot read", path);
    }
    if(x509_parse(c->der, c->size, &c->cert) != 0)
    {
        fprintf(stderr, "img4: %s is not a certificate img4 can use\n", path);
        return 1;
    }
    return 0;
}

static const uint8_t ed25519_algorithm[] = { 0x06, 0x03, 0x2B, 0x65, 0x70 };

static void algorithm(struct buf *out)
{
    tlv(out, DER_UNIVERSAL, 1, DER_SEQUENCE, ed25519_algorithm, sizeof(ed25519_algorithm));
}

static void name(struct buf *out,
                 const char *common_name)
{
    static const uint8_t oid_o[] = { 0x55, 0x04, 0x0A };
    static const uint8_t oid_cn[] = { 0x55, 0x04, 0x03 };
    struct buf rdns = { 0 }, attribute = { 0 }, rdn = { 0 };

    tlv(&attribute, DER_UNIVERSAL, 0, DER_OID, oid_o, sizeof(oid_o));
    tlv(&attribute, DER_UNIVERSAL, 0, 12, "emexLabs", 8);
    sequence(&rdn, &attribute);
    wrap(&rdns, DER_UNIVERSAL, 1, DER_SET, &rdn);

    tlv(&attribute, DER_UNIVERSAL, 0, DER_OID, oid_cn, sizeof(oid_cn));
    tlv(&attribute, DER_UNIVERSAL, 0, 12, common_name, strlen(common_name));
    sequence(&rdn, &attribute);
    wrap(&rdns, DER_UNIVERSAL, 1, DER_SET, &rdn);

    sequence(out, &rdns);
}

static void extension(struct buf *out,
                      const uint8_t *oid,
                      size_t oid_size,
                      struct buf *value)
{
    struct buf ext = { 0 };
    tlv(&ext, DER_UNIVERSAL, 0, DER_OID, oid, oid_size);
    boolean(&ext, 1);
    wrap(&ext, DER_UNIVERSAL, 0, DER_OCTET_STRING, value);
    sequence(out, &ext);
}

static void make_certificate(struct buf *out,
                             const char *issuer,
                             const char *subject,
                             const uint8_t public_key[32],
                             int ca,
                             int path_length,
                             int usage,
                             const uint8_t issuer_secret[SECRET_KEY_SIZE])
{
    static const uint8_t oid_basic_constraints[] = { 0x55, 0x1D, 0x13 };
    static const uint8_t oid_key_usage[] = { 0x55, 0x1D, 0x0F };
    struct buf tbs = { 0 }, field = { 0 }, value = { 0 }, exts = { 0 }, cert = { 0 };

    /* version v3 */
    integer(&field, 2);
    wrap(&tbs, DER_CONTEXT, 1, 0, &field);

    /* a random positive serial number */
    uint8_t serial[8];
    if(getentropy(serial, sizeof(serial)) != 0)
    {
        fprintf(stderr, "img4: no randomness\n");
        exit(1);
    }
    serial[0] = (serial[0] & 0x7F) | 0x40;
    tlv(&tbs, DER_UNIVERSAL, 0, DER_INTEGER, serial, sizeof(serial));

    algorithm(&tbs);
    name(&tbs, issuer);

    /* valid from 2026 on, with no end (RFC 5280 4.1.2.5) */
    tlv(&field, DER_UNIVERSAL, 0, 23, "260101000000Z", 13);
    tlv(&field, DER_UNIVERSAL, 0, 24, "99991231235959Z", 15);
    sequence(&tbs, &field);

    name(&tbs, subject);

    /* subjectPublicKeyInfo */
    uint8_t key_bits[33] = { 0 };
    memcpy(key_bits + 1, public_key, 32);
    algorithm(&field);
    tlv(&field, DER_UNIVERSAL, 0, DER_BIT_STRING, key_bits, sizeof(key_bits));
    sequence(&tbs, &field);

    /* basicConstraints, critical */
    if(ca)
    {
        boolean(&field, 1);
        if(path_length >= 0)
        {
            integer(&field, (uint64_t)path_length);
        }
    }
    sequence(&value, &field);
    extension(&exts, oid_basic_constraints, sizeof(oid_basic_constraints), &value);

    /* keyUsage, critical */
    uint8_t usage_bits[3] = { 0 };
    int highest = 0;
    for(int bit = 0; bit < 16; bit++)
    {
        if(usage & (1 << bit))
        {
            usage_bits[1 + bit / 8] |= 0x80 >> (bit % 8);
            highest = bit;
        }
    }
    usage_bits[0] = (uint8_t)(7 - highest % 8);
    tlv(&value, DER_UNIVERSAL, 0, DER_BIT_STRING, usage_bits, (size_t)(2 + highest / 8));
    extension(&exts, oid_key_usage, sizeof(oid_key_usage), &value);

    sequence(&field, &exts);
    wrap(&tbs, DER_CONTEXT, 1, 3, &field);

    struct buf tbs_element = { 0 };
    sequence(&tbs_element, &tbs);

    uint8_t signature[65] = { 0 };
    crypto_ed25519_sign(signature + 1, issuer_secret, tbs_element.p, tbs_element.n);

    put(&cert, tbs_element.p, tbs_element.n);
    free(tbs_element.p);
    algorithm(&cert);
    tlv(&cert, DER_UNIVERSAL, 0, DER_BIT_STRING, signature, sizeof(signature));
    sequence(out, &cert);
}

static int new_key(uint8_t secret[SECRET_KEY_SIZE],
                   uint8_t public_key[32])
{
    uint8_t seed[32];
    if(getentropy(seed, sizeof(seed)) != 0)
    {
        fprintf(stderr, "img4: no randomness\n");
        return 1;
    }
    crypto_ed25519_key_pair(secret, public_key, seed);
    return 0;
}

static int init(const char *folder)
{
    static const char *files[] = { "root.key", "root.der", "ca.key", "ca.der", "signing.key", "signing.der" };
    char path[4096];
    struct stat st;

    if(mkdir(folder, 0700) != 0 && errno != EEXIST)
    {
        return fail("cannot create", folder);
    }
    for(size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++)
    {
        path_in(path, sizeof(path), folder, files[i]);
        if(stat(path, &st) == 0)
        {
            fprintf(stderr, "img4: %s exists already, img4 init does not replace keys\n", path);
            return 1;
        }
    }

    uint8_t root_secret[SECRET_KEY_SIZE], root_public[32];
    uint8_t ca_secret[SECRET_KEY_SIZE], ca_public[32];
    uint8_t signing_secret[SECRET_KEY_SIZE], signing_public[32];
    if(new_key(root_secret, root_public) || new_key(ca_secret, ca_public) || new_key(signing_secret, signing_public))
    {
        return 1;
    }

    struct buf root = { 0 }, ca = { 0 }, signing = { 0 };
    make_certificate(&root, "emexLabs Root CA", "emexLabs Root CA", root_public, 1, -1, X509_KEY_CERT_SIGN | (1 << 6), root_secret);
    make_certificate(&ca, "emexLabs Root CA", "emexLabs Secure Boot CA", ca_public, 1, 0, X509_KEY_CERT_SIGN, root_secret);
    make_certificate(&signing, "emexLabs Secure Boot CA", "emexLabs Secure Boot Signing", signing_public, 0, -1, X509_DIGITAL_SIGNATURE, ca_secret);

    int result = 0;
    path_in(path, sizeof(path), folder, "root.key");
    result |= write_file(path, root_secret, SECRET_KEY_SIZE, 1);
    path_in(path, sizeof(path), folder, "ca.key");
    result |= write_file(path, ca_secret, SECRET_KEY_SIZE, 1);
    path_in(path, sizeof(path), folder, "signing.key");
    result |= write_file(path, signing_secret, SECRET_KEY_SIZE, 1);
    path_in(path, sizeof(path), folder, "root.der");
    result |= write_file(path, root.p, root.n, 0);
    path_in(path, sizeof(path), folder, "ca.der");
    result |= write_file(path, ca.p, ca.n, 0);
    path_in(path, sizeof(path), folder, "signing.der");
    result |= write_file(path, signing.p, signing.n, 0);

    crypto_wipe(root_secret, sizeof(root_secret));
    crypto_wipe(ca_secret, sizeof(ca_secret));
    crypto_wipe(signing_secret, sizeof(signing_secret));
    free(root.p);
    free(ca.p);
    free(signing.p);

    if(result == 0)
    {
        printf("made the Image4 chain in %s (keep root.key and ca.key offline)\n", folder);
    }

    return result;
}

static void bytes_out(FILE *f,
                      const uint8_t *bytes,
                      size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        fprintf(f, "%s0x%02x,%s", i % 12 == 0 ? "    " : " ", bytes[i], i % 12 == 11 || i == size - 1 ? "\n" : "");
    }
}

static int rom(const char *folder,
               const char *header)
{
    struct cert_file root;
    if(read_cert(folder, "root.der", &root) != 0)
    {
        return 1;
    }
    if(x509_check(&root.cert, root.cert.public_key) != 0 || !root.cert.is_ca)
    {
        fprintf(stderr, "img4: %s/root.der is not a self-signed CA certificate\n", folder);
        return 1;
    }

    FILE *h = fopen(header, "w");
    if(h == NULL)
    {
        return fail("cannot create", header);
    }
    fprintf(h,
            "/*\n"
            " * img4_root.h - the root of the BootROM's Image4 chain\n"
            " *\n"
            " * Made by img4 rom from root.der. Only a chain this key signed can sign\n"
            " * what the BootROM boots.\n"
            " */\n"
            "\n"
            "#ifndef IMG4_ROOT_H\n"
            "#define IMG4_ROOT_H\n"
            "\n"
            "#include <img4.h>\n"
            "\n"
            "/* Ed25519 */\n"
            "static const uint8_t img4_root_public_key[32] = {\n");
    bytes_out(h, root.cert.public_key, 32);
    fprintf(h,
            "};\n"
            "\n"
            "/* its certificate's subject, the issuer of the first certificate of a chain */\n"
            "static const uint8_t img4_root_subject[%zu] = {\n", root.cert.subject_size);
    bytes_out(h, root.cert.subject, root.cert.subject_size);
    fprintf(h,
            "};\n"
            "\n"
            "/* (position independent code, as eBoot, sets a struct img4_root up from the arrays itself) */\n"
            "#define IMG4_ROM_ROOT { img4_root_public_key, img4_root_subject, sizeof(img4_root_subject) }\n"
            "\n"
            "#ifndef __PIE__\n"
            "static const struct img4_root img4_rom_root = IMG4_ROM_ROOT;\n"
            "#endif\n"
            "\n"
            "#endif /* IMG4_ROOT_H */\n");
    free(root.der);
    if(fclose(h) != 0)
    {
        return fail("cannot write", header);
    }
    return 0;
}

static int valid_type(const char *type)
{
    if(strlen(type) != 4)
    {
        return 0;
    }
    for(int i = 0; i < 4; i++)
    {
        if(type[i] < 0x21 || type[i] > 0x7E)
        {
            return 0;
        }
    }
    return 1;
}

static int verify_with(const char *folder,
                       const char *type,
                       const struct img4_device *device,
                       const uint8_t *file,
                       size_t size,
                       struct img4_image *image)
{
    struct cert_file root;
    if(read_cert(folder, "root.der", &root) != 0)
    {
        return 1;
    }
    struct img4_root key = { root.cert.public_key, root.cert.subject, root.cert.subject_size };
    int result = img4_verify(file, size, type, &key, device, image);
    free(root.der);
    return result;
}

static int sign(const char *folder,
                const char *type,
                const char *description,
                const char *in,
                const char *out,
                const struct img4_device *device,
                const struct img4_kbag *kbags,
                int kbag_count)
{
    if(!valid_type(type))
    {
        fprintf(stderr, "img4: the type is a 4CC, 4 printable characters like ebot\n");
        return 1;
    }

    uint8_t secret[SECRET_KEY_SIZE];
    struct cert_file ca, signing;
    if(read_key(folder, "signing.key", secret) != 0 ||
       read_cert(folder, "ca.der", &ca) != 0 || read_cert(folder, "signing.der", &signing) != 0)
    {
        return 1;
    }
    if(memcmp(secret + 32, signing.cert.public_key, 32) != 0)
    {
        crypto_wipe(secret, sizeof(secret));
        fprintf(stderr, "img4: %s/signing.key is not the key of signing.der\n", folder);
        return 1;
    }

    size_t size;
    uint8_t *payload = read_file(in, &size);
    if(payload == NULL)
    {
        crypto_wipe(secret, sizeof(secret));
        return fail("cannot read", in);
    }

    /* IM4P */
    struct buf im4p = { 0 }, fields = { 0 };
    ia5(&fields, "IM4P");
    ia5(&fields, type);
    ia5(&fields, description);
    tlv(&fields, DER_UNIVERSAL, 0, DER_OCTET_STRING, payload, size);
    if(kbag_count > 0)
    {
        struct buf kbag_list = { 0 };
        for(int i = 0; i < kbag_count; i++)
        {
            struct buf entry = { 0 };
            integer(&entry, kbags[i].type);
            tlv(&entry, DER_UNIVERSAL, 0, DER_OCTET_STRING, kbags[i].iv, kbags[i].iv_size);
            tlv(&entry, DER_UNIVERSAL, 0, DER_OCTET_STRING, kbags[i].key, kbags[i].key_size);
            sequence(&kbag_list, &entry);
        }
        struct buf kbag_seq = { 0 };
        sequence(&kbag_seq, &kbag_list);
        tlv(&fields, DER_UNIVERSAL, 0, DER_OCTET_STRING, kbag_seq.p, kbag_seq.n);
        free(kbag_seq.p);
    }
    sequence(&im4p, &fields);
    free(payload);

    uint8_t digest[IMG4_DIGEST_SIZE];
    crypto_sha512(digest, im4p.p, im4p.n);

    /* MANP (this chip and board, ECID, BNCH and CEPO 0) */
    struct buf props[8] = { 0 };
    int n = 0;
    property_integer(&props[n++], "CHIP", device->chip);
    property_integer(&props[n++], "BORD", device->board);
    property_integer(&props[n++], "ECID", device->ecid);
    property_bytes(&props[n++], "BNCH", device->nonce_hash, IMG4_NONCE_HASH_SIZE);
    property_integer(&props[n++], "CEPO", device->epoch);
    property_bool(&props[n++], "CPRO", 1);
    property_bool(&props[n++], "CSEC", 1);
    property_integer(&props[n++], "SDOM", 1);
    struct buf manp_set = { 0 };
    set(&manp_set, props, n);

    /* the payload's entry */
    n = 0;
    property_bytes(&props[n++], "DGST", digest, sizeof(digest));
    property_bool(&props[n++], "EPRO", 1);
    property_bool(&props[n++], "ESEC", 1);
    struct buf image_set = { 0 };
    set(&image_set, props, n);

    struct buf entries[2] = { 0 };
    property(&entries[0], "MANP", &manp_set);
    property(&entries[1], type, &image_set);
    struct buf manb_set = { 0 };
    set(&manb_set, entries, 2);

    struct buf manb[1] = { 0 };
    property(&manb[0], "MANB", &manb_set);
    struct buf body = { 0 };
    set(&body, manb, 1);

    /* signed by the signing key, the chain after it */
    uint8_t signature[64];
    crypto_ed25519_sign(signature, secret, body.p, body.n);
    crypto_wipe(secret, sizeof(secret));

    struct buf chain = { 0 }, chain_content = { 0 };
    put(&chain_content, ca.der, ca.size);
    put(&chain_content, signing.der, signing.size);
    sequence(&chain, &chain_content);

    struct buf im4m = { 0 }, wrapped = { 0 };
    ia5(&fields, "IM4M");
    integer(&fields, 0);
    put(&fields, body.p, body.n);
    tlv(&fields, DER_UNIVERSAL, 0, DER_OCTET_STRING, signature, sizeof(signature));
    put(&fields, chain.p, chain.n);
    sequence(&im4m, &fields);
    wrap(&wrapped, DER_CONTEXT, 1, 0, &im4m);

    struct buf img4 = { 0 };
    ia5(&fields, "IMG4");
    put(&fields, im4p.p, im4p.n);
    put(&fields, wrapped.p, wrapped.n);
    sequence(&img4, &fields);

    /* (checked the BootROM's way before it is written) */
    struct img4_image image;
    int result = verify_with(folder, type, device, img4.p, img4.n, &image);
    if(result != IMG4_OK)
    {
        fprintf(stderr, "img4: the file made %s\n", result > 0 ? "cannot be checked" : img4_error(result));
        return 1;
    }
    if(write_file(out, img4.p, img4.n, 0) != 0)
    {
        return 1;
    }

    free(ca.der);
    free(signing.der);
    free(im4p.p);
    free(body.p);
    free(chain.p);
    free(wrapped.p);
    free(img4.p);
    return 0;
}

static int verify(const char *folder,
                  const char *type,
                  const char *path,
                  const struct img4_device *device)
{
    size_t size;
    uint8_t *file = read_file(path, &size);
    if(file == NULL)
    {
        return fail("cannot read", path);
    }

    struct img4_image image;
    int result = verify_with(folder, type, device, file, size, &image);
    if(result > 0)
    {
        free(file);
        return 1;
    }
    if(result != IMG4_OK)
    {
        fprintf(stderr, "img4: %s %s\n", path, img4_error(result));
        free(file);
        return 1;
    }
    printf("%s is valid: %.4s '%.*s', %zu bytes", path, type, (int)image.description_size, (const char *)image.description, image.size);
    if(image.kbag_count > 0)
    {
        printf(", %d kbag(s)", image.kbag_count);
    }
    printf("\n");
    for(int i = 0; i < image.kbag_count; i++)
    {
        printf("  kbag[%d]: type=%llu iv=", i, (unsigned long long)image.kbags[i].type);
        for(size_t j = 0; j < image.kbags[i].iv_size; j++)
        {
            printf("%02x", image.kbags[i].iv[j]);
        }
        printf(" key=");
        for(size_t j = 0; j < image.kbags[i].key_size; j++)
        {
            printf("%02x", image.kbags[i].key[j]);
        }
        printf("\n");
    }
    free(file);
    return 0;
}

static int parse_ecid(const char *s,
                      uint64_t *out)
{
    if(s == NULL || *s == '\0')
    {
        return -1;
    }
    char *end = NULL;
    errno = 0;
    unsigned long long v = strtoull(s, &end, 0);
    if(errno != 0 || *end != '\0')
    {
        return -1;
    }
    *out = (uint64_t)v;
    return 0;
}

static int hex_nibble(char c)
{
    if(c >= '0' && c <= '9')
    {
        return c - '0';
    }
    if(c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    if(c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    return -1;
}

static int parse_hex(const char *s,
                     size_t len,
                     uint8_t *out,
                     size_t max_size,
                     size_t *out_size)
{
    if(len == 0 || (len % 2) != 0 || (len / 2) > max_size)
    {
        return -1;
    }
    for(size_t i = 0; i < len; i += 2)
    {
        int hi = hex_nibble(s[i]);
        int lo = hex_nibble(s[i + 1]);
        if(hi < 0 || lo < 0)
        {
            return -1;
        }
        out[i / 2] = (uint8_t)((hi << 4) | lo);
    }
    *out_size = len / 2;
    return 0;
}

static int parse_kbag_arg(const char *s,
                          struct img4_kbag *out)
{
    if(s == NULL)
    {
        return -1;
    }
    const char *c1 = strchr(s, ':');
    if(c1 == NULL || c1 == s)
    {
        return -1;
    }
    const char *c2 = strchr(c1 + 1, ':');
    if(c2 == NULL || c2 == c1 + 1 || c2[1] == '\0')
    {
        return -1;
    }

    size_t type_len = (size_t)(c1 - s);
    if(type_len == 4 && strncmp(s, "prod", 4) == 0)
    {
        out->type = IMG4_KBAG_PROD;
    }
    else if(type_len == 3 && strncmp(s, "dev", 3) == 0)
    {
        out->type = IMG4_KBAG_DEV;
    }
    else
    {
        char type_buf[32];
        if(type_len >= sizeof(type_buf))
        {
            return -1;
        }
        memcpy(type_buf, s, type_len);
        type_buf[type_len] = '\0';
        if(parse_ecid(type_buf, &out->type) != 0)
        {
            return -1;
        }
    }

    size_t iv_len = (size_t)(c2 - (c1 + 1));
    size_t key_len = strlen(c2 + 1);
    if(parse_hex(c1 + 1, iv_len, out->iv, sizeof(out->iv), &out->iv_size) != 0 ||
       parse_hex(c2 + 1, key_len, out->key, sizeof(out->key), &out->key_size) != 0)
    {
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct img4_device device = img4_this_device;
    struct img4_kbag kbags[IMG4_MAX_KBAGS] = { 0 };
    int kbag_count = 0;
    int argi = 1;

    while(argi < argc && strncmp(argv[argi], "--", 2) == 0)
    {
        if(strcmp(argv[argi], "--ecid") == 0)
        {
            if(argi + 1 >= argc || parse_ecid(argv[argi + 1], &device.ecid) != 0)
            {
                fprintf(stderr, "img4: invalid ecid '%s'\n", argi + 1 < argc ? argv[argi + 1] : "");
                return 2;
            }
            argi += 2;
        }
        else if(strcmp(argv[argi], "--kbag") == 0)
        {
            if(kbag_count >= IMG4_MAX_KBAGS)
            {
                fprintf(stderr, "img4: at most %d keybags are supported\n", IMG4_MAX_KBAGS);
                return 2;
            }
            if(argi + 1 >= argc || parse_kbag_arg(argv[argi + 1], &kbags[kbag_count]) != 0)
            {
                fprintf(stderr, "img4: invalid kbag '%s'\n", argi + 1 < argc ? argv[argi + 1] : "");
                return 2;
            }
            kbag_count++;
            argi += 2;
        }
        else
        {
            break;
        }
    }

    int rem = argc - argi;
    int result = 2;
    if(rem == 2 && strcmp(argv[argi], "init") == 0 && kbag_count == 0)
    {
        result = init(argv[argi + 1]);
    }
    else if(rem == 3 && strcmp(argv[argi], "rom") == 0 && kbag_count == 0)
    {
        result = rom(argv[argi + 1], argv[argi + 2]);
    }
    else if(rem == 6 && strcmp(argv[argi], "sign") == 0)
    {
        result = sign(argv[argi + 1], argv[argi + 2], argv[argi + 3], argv[argi + 4], argv[argi + 5], &device, kbags, kbag_count);
    }
    else if(rem == 4 && strcmp(argv[argi], "verify") == 0 && kbag_count == 0)
    {
        result = verify(argv[argi + 1], argv[argi + 2], argv[argi + 3], &device);
    }
    else
    {
        fprintf(stderr,
                "usage: img4 init <key folder>\n"
                "       img4 rom <key folder> <img4_root.h>\n"
                "       img4 [--ecid <value>] [--kbag <prod|dev|n>:<iv_hex>:<key_hex>] sign <key folder> <type> <description> <payload> <out.img4>\n"
                "       img4 [--ecid <value>] verify <key folder> <type> <file.img4>\n");
    }

    crypto_wipe(kbags, sizeof(kbags));
    return result;
}
