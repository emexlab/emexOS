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

#include "elfreloc.h"

#define EI_NIDENT            16
#define EI_CLASS             4
#define EI_DATA              5

#define ELFCLASS64           2
#define ELFDATA2LSB          1
#define ET_DYN               3

#define EM_EMEX64            0x0E64

#define R_EMEX64_NONE        0
#define R_EMEX64_64          1
#define R_EMEX64_GLOB_DAT    8
#define R_EMEX64_JUMP_SLOT   9
#define R_EMEX64_RELATIVE    10

#define PT_LOAD              1
#define PT_DYNAMIC           2

#define SHT_DYNSYM           11

#define SHN_UNDEF            0
#define SHN_ABS              0xFFF1

#define STB_LOCAL            0
#define STB_GLOBAL           1
#define STB_WEAK             2

#define DT_NULL              0
#define DT_PLTRELSZ          2
#define DT_HASH              4
#define DT_STRTAB            5
#define DT_SYMTAB            6
#define DT_RELA              7
#define DT_RELASZ            8
#define DT_RELAENT           9
#define DT_STRSZ             10
#define DT_SYMENT            11
#define DT_PLTREL            20
#define DT_JMPREL            23
#define DT_GNU_HASH          0x6ffffef5

#define ELF64_R_SYM(i)       ((uint32_t)((i) >> 32))
#define ELF64_R_TYPE(i)      ((uint32_t)((i) & 0xffffffffUL))
#define ELF64_ST_BIND(i)     ((uint8_t)((i) >> 4))

typedef struct {
    uint8_t e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} Elf64_Shdr;

typedef struct {
    int64_t d_tag;
    uint64_t d_val;
} Elf64_Dyn;

typedef struct {
    uint64_t r_offset;
    uint64_t r_info;
    int64_t r_addend;
} Elf64_Rela;

typedef struct {
    uint32_t st_name;
    uint8_t st_info;
    uint8_t st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint64_t st_size;
} Elf64_Sym;

static uint64_t align_up(uint64_t val,
                         uint64_t align)
{
    if(align <= 1)
    {
        return val;
    }
    return (val + align - 1) & ~(align - 1);
}

static int sym_streq(const char *a,
                     const char *b)
{
    while(*a != '\0' && *b != '\0')
    {
        if(*a != *b)
        {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

static const char *get_dynstr_name(const char *dynstr,
                                   size_t dynstr_size,
                                   uint32_t offset)
{
    if(dynstr == 0 || (size_t)offset >= dynstr_size)
    {
        return 0;
    }
    for(size_t i = offset; i < dynstr_size; i++)
    {
        if(dynstr[i] == '\0')
        {
            return dynstr + offset;
        }
    }
    return 0;
}

static size_t count_dynsym_gnu_hash(uint64_t load_bias,
                                    uint64_t gnu_hash_vaddr,
                                    uint64_t min_vaddr,
                                    uint64_t max_vaddr)
{
    if(gnu_hash_vaddr < min_vaddr || gnu_hash_vaddr + 16 > max_vaddr)
    {
        return 0;
    }

    const uint32_t *hdr = (const uint32_t *)(load_bias + gnu_hash_vaddr);
    uint32_t nbuckets = hdr[0];
    uint32_t symoffset = hdr[1];
    uint32_t bloom_size = hdr[2];

    uint64_t header_bytes = 16 + (uint64_t)bloom_size * sizeof(uint64_t) + (uint64_t)nbuckets * sizeof(uint32_t);
    if(header_bytes > max_vaddr - gnu_hash_vaddr)
    {
        return 0;
    }

    const uint32_t *buckets = (const uint32_t *)(load_bias + gnu_hash_vaddr + 16 + (uint64_t)bloom_size * sizeof(uint64_t));
    const uint32_t *chains  = buckets + nbuckets;

    uint32_t max_sym = 0;
    for(uint32_t i = 0; i < nbuckets; i++)
    {
        if(buckets[i] > max_sym)
        {
            max_sym = buckets[i];
        }
    }

    if(max_sym < symoffset)
    {
        return (size_t)symoffset;
    }

    uint64_t chains_vaddr = gnu_hash_vaddr + header_bytes;
    uint32_t chain_idx = max_sym - symoffset;

    while(1)
    {
        uint64_t entry_vaddr = chains_vaddr + (uint64_t)chain_idx * sizeof(uint32_t);
        if(entry_vaddr + sizeof(uint32_t) > max_vaddr)
        {
            return 0;
        }
        if(chains[chain_idx] & 1U)
        {
            return (size_t)symoffset + (size_t)chain_idx + 1;
        }
        chain_idx++;
    }
}

static size_t count_dynsym_shdr(const uint8_t *elf,
                                size_t size,
                                const Elf64_Ehdr *ehdr)
{
    if(ehdr->e_shoff == 0 || ehdr->e_shnum == 0 || ehdr->e_shentsize != sizeof(Elf64_Shdr))
    {
        return 0;
    }
    if(ehdr->e_shoff > size || (uint64_t)ehdr->e_shnum * sizeof(Elf64_Shdr) > size - ehdr->e_shoff)
    {
        return 0;
    }

    const Elf64_Shdr *shdrs = (const Elf64_Shdr *)(elf + ehdr->e_shoff);
    for(uint16_t i = 0; i < ehdr->e_shnum; i++)
    {
        if(shdrs[i].sh_type == SHT_DYNSYM && shdrs[i].sh_entsize == sizeof(Elf64_Sym))
        {
            return (size_t)(shdrs[i].sh_size / sizeof(Elf64_Sym));
        }
    }
    return 0;
}

void elf_symtab_init(elf_symtab_t *tab)
{
    if(tab == 0)
    {
        return;
    }
    tab->image_count = 0;
    tab->manual_count = 0;
}

int elf_symtab_add_image(elf_symtab_t *tab,
                         const elf_image_t *img)
{
    if(tab == 0 || img == 0 || tab->image_count >= ELF_SYMTAB_MAX_IMAGES)
    {
        return 0;
    }
    tab->images[tab->image_count++] = *img;
    return 1;
}

int elf_symtab_add_symbol(elf_symtab_t *tab,
                          const char *name,
                          uint64_t addr)
{
    if(tab == 0 || name == 0 || tab->manual_count >= ELF_SYMTAB_MAX_MANUAL)
    {
        return 0;
    }
    tab->manual[tab->manual_count].name = name;
    tab->manual[tab->manual_count].addr = addr;
    tab->manual_count++;
    return 1;
}

int elf_symtab_lookup(const elf_symtab_t *tab,
                      const char *name,
                      uint64_t *addr_out)
{
    if(tab == 0 || name == 0 || addr_out == 0)
    {
        return 0;
    }

    for(size_t i = 0; i < tab->manual_count; i++)
    {
        if(sym_streq(tab->manual[i].name, name))
        {
            *addr_out = tab->manual[i].addr;
            return 1;
        }
    }

    int found_weak = 0;
    uint64_t weak_addr = 0;

    for(size_t img_idx = 0; img_idx < tab->image_count; img_idx++)
    {
        const elf_image_t *img = &tab->images[img_idx];
        const Elf64_Sym *syms = (const Elf64_Sym *)img->dynsym;
        if(syms == 0 || img->dynstr == 0 || img->dynsym_count <= 1)
        {
            continue;
        }

        for(size_t s = 1; s < img->dynsym_count; s++)
        {
            const Elf64_Sym *sym = &syms[s];
            if(sym->st_shndx == SHN_UNDEF)
            {
                continue;
            }

            uint8_t bind = ELF64_ST_BIND(sym->st_info);
            if(bind != STB_GLOBAL && bind != STB_WEAK)
            {
                continue;
            }

            const char *sym_name = get_dynstr_name(img->dynstr, img->dynstr_size, sym->st_name);
            if(sym_name == 0 || !sym_streq(sym_name, name))
            {
                continue;
            }

            uint64_t resolved = (sym->st_shndx == SHN_ABS)
                                    ? sym->st_value
                                    : (img->load_bias + sym->st_value);
            if(bind == STB_GLOBAL)
            {
                *addr_out = resolved;
                return 1;
            }
            if(!found_weak)
            {
                found_weak = 1;
                weak_addr = resolved;
            }
        }
    }

    if(found_weak)
    {
        *addr_out = weak_addr;
        return 1;
    }

    return 0;
}

static const char *apply_rela_table(const Elf64_Rela *rela,
                                    size_t count,
                                    uint64_t load_bias,
                                    uint64_t min_vaddr,
                                    uint64_t max_vaddr,
                                    const Elf64_Sym *dynsym,
                                    size_t dynsym_count,
                                    const char *dynstr,
                                    size_t dynstr_size,
                                    const elf_symtab_t *imports,
                                    const char **missing_sym_out)
{
    for(size_t i = 0; i < count; i++)
    {
        uint32_t r_type = ELF64_R_TYPE(rela[i].r_info);
        uint32_t r_sym  = ELF64_R_SYM(rela[i].r_info);

        if(r_type == R_EMEX64_NONE)
        {
            continue;
        }
        if(rela[i].r_offset < min_vaddr || rela[i].r_offset + sizeof(uint64_t) > max_vaddr)
        {
            return "has an out-of-bounds relocation offset";
        }

        uint64_t *target = (uint64_t *)(load_bias + rela[i].r_offset);

        if(r_type == R_EMEX64_RELATIVE)
        {
            if(r_sym != 0)
            {
                return "has a symbol-indexed RELATIVE relocation";
            }
            *target = load_bias + (uint64_t)rela[i].r_addend;
            continue;
        }

        if(r_type != R_EMEX64_64 &&
           r_type != R_EMEX64_GLOB_DAT &&
           r_type != R_EMEX64_JUMP_SLOT)
        {
            return "contains an unsupported dynamic relocation";
        }

        if(dynsym == 0 || r_sym == 0 || (size_t)r_sym >= dynsym_count)
        {
            return "references an out-of-bounds dynamic symbol";
        }

        const Elf64_Sym *sym = &dynsym[r_sym];
        const char *sym_name = get_dynstr_name(dynstr, dynstr_size, sym->st_name);
        if(sym_name == 0)
        {
            return "has a corrupt symbol string offset";
        }

        uint64_t sym_addr = 0;
        int resolved = 0;

        if(ELF64_ST_BIND(sym->st_info) == STB_GLOBAL && imports != 0)
        {
            resolved = elf_symtab_lookup(imports, sym_name, &sym_addr);
        }

        if(!resolved)
        {
            if(sym->st_shndx != SHN_UNDEF)
            {
                sym_addr = (sym->st_shndx == SHN_ABS) ? sym->st_value : (load_bias + sym->st_value);
                resolved = 1;
            }
            else if(imports != 0)
            {
                resolved = elf_symtab_lookup(imports, sym_name, &sym_addr);
            }
        }

        if(!resolved)
        {
            if(ELF64_ST_BIND(sym->st_info) == STB_WEAK)
            {
                sym_addr = 0;
            }
            else
            {
                if(missing_sym_out != 0)
                {
                    *missing_sym_out = sym_name;
                }
                return "has an unresolved symbol";
            }
        }

        *target = sym_addr + (uint64_t)rela[i].r_addend;
    }

    return 0;
}

const char *elfreloc_load_ex(const uint8_t *elf,
                             size_t size,
                             uint64_t *cursor,
                             uint64_t limit,
                             const elf_symtab_t *imports,
                             elf_image_t *image_out,
                             const char **missing_sym_out)
{
    if(missing_sym_out != 0)
    {
        *missing_sym_out = 0;
    }
    if(elf == 0 || cursor == 0 || size < sizeof(Elf64_Ehdr))
    {
        return "is truncated";
    }

    const Elf64_Ehdr *ehdr = (const Elf64_Ehdr *)elf;
    if(ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
       ehdr->e_ident[2] != 'L'  || ehdr->e_ident[3] != 'F')
    {
        return "is not an ELF image";
    }
    if(ehdr->e_ident[EI_CLASS] != ELFCLASS64 || ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
    {
        return "is not a 64-bit little-endian ELF";
    }
    if(ehdr->e_type != ET_DYN)
    {
        return "is not a shared/PIE (ET_DYN) ELF";
    }
    if(ehdr->e_machine != EM_EMEX64)
    {
        return "targets the wrong CPU architecture";
    }
    if(ehdr->e_phentsize != sizeof(Elf64_Phdr) || ehdr->e_phnum == 0 ||
       ehdr->e_phoff > size ||
       (uint64_t)ehdr->e_phnum * sizeof(Elf64_Phdr) > size - ehdr->e_phoff)
    {
        return "has invalid program headers";
    }

    const Elf64_Phdr *phdrs = (const Elf64_Phdr *)(elf + ehdr->e_phoff);
    uint64_t min_vaddr = UINT64_MAX;
    uint64_t max_vaddr = 0;
    uint64_t max_align = 0x1000;
    int has_load = 0;

    for(uint16_t i = 0; i < ehdr->e_phnum; i++)
    {
        const Elf64_Phdr *ph = &phdrs[i];
        if(ph->p_type != PT_LOAD)
        {
            continue;
        }
        if(ph->p_memsz < ph->p_filesz || ph->p_offset > size || ph->p_filesz > size - ph->p_offset)
        {
            return "has a corrupt PT_LOAD segment";
        }
        if(ph->p_vaddr < min_vaddr)
        {
            min_vaddr = ph->p_vaddr;
        }
        if(ph->p_vaddr + ph->p_memsz > max_vaddr)
        {
            max_vaddr = ph->p_vaddr + ph->p_memsz;
        }
        if(ph->p_align > max_align)
        {
            max_align = ph->p_align;
        }
        has_load = 1;
    }

    if(!has_load || max_vaddr <= min_vaddr)
    {
        return "has no loadable segments";
    }

    uint64_t span = max_vaddr - min_vaddr;
    uint64_t base = align_up(*cursor, max_align);
    if(base > limit || span > limit - base)
    {
        return "does not fit in available memory";
    }

    uint8_t *dst_base = (uint8_t *)base;
    for(uint64_t i = 0; i < span; i++)
    {
        dst_base[i] = 0;
    }

    uint64_t load_bias = base - min_vaddr;
    const Elf64_Phdr *dyn_phdr = 0;

    for(uint16_t i = 0; i < ehdr->e_phnum; i++)
    {
        const Elf64_Phdr *ph = &phdrs[i];
        if(ph->p_type == PT_LOAD)
        {
            const uint8_t *src = elf + ph->p_offset;
            uint8_t *dst = (uint8_t *)(load_bias + ph->p_vaddr);
            for(uint64_t b = 0; b < ph->p_filesz; b++)
            {
                dst[b] = src[b];
            }
        }
        else if(ph->p_type == PT_DYNAMIC)
        {
            dyn_phdr = ph;
        }
    }

    const Elf64_Sym *dynsym = 0;
    size_t dynsym_count = 0;
    const char *dynstr = 0;
    size_t dynstr_size = 0;

    if(dyn_phdr != 0)
    {
        if(dyn_phdr->p_vaddr < min_vaddr || dyn_phdr->p_vaddr + dyn_phdr->p_memsz > max_vaddr)
        {
            return "has an out-of-bounds PT_DYNAMIC segment";
        }

        const Elf64_Dyn *dyn = (const Elf64_Dyn *)(load_bias + dyn_phdr->p_vaddr);
        size_t max_dyn = dyn_phdr->p_memsz / sizeof(Elf64_Dyn);

        uint64_t rela_vaddr = 0;
        uint64_t rela_size = 0;
        uint64_t rela_ent = sizeof(Elf64_Rela);
        uint64_t jmprel_vaddr = 0;
        uint64_t pltrel_size = 0;
        uint64_t pltrel_type = DT_RELA;
        uint64_t symtab_vaddr = 0;
        uint64_t syment = sizeof(Elf64_Sym);
        uint64_t strtab_vaddr = 0;
        uint64_t strsz = 0;
        uint64_t hash_vaddr = 0;
        uint64_t gnuhash_vaddr = 0;

        for(size_t i = 0; i < max_dyn && dyn[i].d_tag != DT_NULL; i++)
        {
            switch(dyn[i].d_tag)
            {
                case DT_RELA:
                    rela_vaddr = dyn[i].d_val;
                    break;
                case DT_RELASZ:
                    rela_size = dyn[i].d_val;
                    break;
                case DT_RELAENT:
                    rela_ent = dyn[i].d_val;
                    break;
                case DT_JMPREL:
                    jmprel_vaddr = dyn[i].d_val;
                    break;
                case DT_PLTRELSZ:
                    pltrel_size = dyn[i].d_val;
                    break;
                case DT_PLTREL:
                    pltrel_type = dyn[i].d_val;
                    break;
                case DT_SYMTAB:
                    symtab_vaddr = dyn[i].d_val;
                    break;
                case DT_SYMENT:
                    syment = dyn[i].d_val;
                    break;
                case DT_STRTAB:
                    strtab_vaddr = dyn[i].d_val;
                    break;
                case DT_STRSZ:
                    strsz = dyn[i].d_val;
                    break;
                case DT_HASH:
                    hash_vaddr = dyn[i].d_val;
                    break;
                case DT_GNU_HASH:
                    gnuhash_vaddr = dyn[i].d_val;
                    break;
                default:
                    break;
            }
        }

        if(strtab_vaddr != 0 && strsz != 0)
        {
            if(strtab_vaddr < min_vaddr || strtab_vaddr + strsz > max_vaddr)
            {
                return "has an out-of-bounds .dynstr table";
            }
            dynstr = (const char *)(load_bias + strtab_vaddr);
            dynstr_size = (size_t)strsz;
        }

        if(symtab_vaddr != 0)
        {
            if(syment != sizeof(Elf64_Sym) || symtab_vaddr < min_vaddr || symtab_vaddr >= max_vaddr)
            {
                return "has an invalid .dynsym table";
            }

            if(hash_vaddr != 0)
            {
                if(hash_vaddr < min_vaddr || hash_vaddr + 8 > max_vaddr)
                {
                    return "has an invalid DT_HASH table";
                }
                const uint32_t *sysv_hash = (const uint32_t *)(load_bias + hash_vaddr);
                dynsym_count = (size_t)sysv_hash[1];
            }
            else if(gnuhash_vaddr != 0)
            {
                dynsym_count = count_dynsym_gnu_hash(load_bias, gnuhash_vaddr, min_vaddr, max_vaddr);
            }

            if(dynsym_count == 0)
            {
                dynsym_count = count_dynsym_shdr(elf, size, ehdr);
            }

            if(dynsym_count > 0)
            {
                if((uint64_t)dynsym_count * sizeof(Elf64_Sym) > max_vaddr - symtab_vaddr)
                {
                    return "has an out-of-bounds .dynsym table";
                }
                dynsym = (const Elf64_Sym *)(load_bias + symtab_vaddr);
            }
        }

        if(rela_vaddr != 0 && rela_size != 0)
        {
            if(rela_ent != sizeof(Elf64_Rela) ||
               rela_vaddr < min_vaddr ||
               rela_vaddr + rela_size > max_vaddr)
            {
                return "has an invalid .rela.dyn table";
            }

            const char *err = apply_rela_table((const Elf64_Rela *)(load_bias + rela_vaddr), (size_t)(rela_size / sizeof(Elf64_Rela)), load_bias, min_vaddr, max_vaddr, dynsym, dynsym_count, dynstr, dynstr_size, imports, missing_sym_out);
            if(err != 0)
            {
                return err;
            }
        }

        if(jmprel_vaddr != 0 && pltrel_size != 0)
        {
            if(pltrel_type != DT_RELA ||
               jmprel_vaddr < min_vaddr ||
               jmprel_vaddr + pltrel_size > max_vaddr)
            {
                return "has an invalid .rela.plt table";
            }

            const char *err = apply_rela_table((const Elf64_Rela *)(load_bias + jmprel_vaddr), (size_t)(pltrel_size / sizeof(Elf64_Rela)), load_bias, min_vaddr, max_vaddr, dynsym, dynsym_count, dynstr, dynstr_size, imports, missing_sym_out);
            if(err != 0)
            {
                return err;
            }
        }
    }

    if(ehdr->e_entry < min_vaddr || ehdr->e_entry >= max_vaddr)
    {
        return "has an invalid entry point";
    }

    if(image_out != 0)
    {
        image_out->base = base;
        image_out->span = span;
        image_out->load_bias = load_bias;
        image_out->entry = load_bias + ehdr->e_entry;
        image_out->dynsym = dynsym;
        image_out->dynsym_count = dynsym_count;
        image_out->dynstr = dynstr;
        image_out->dynstr_size = dynstr_size;
    }

    *cursor = (base + span + 0x1FFF) & ~0x1FFFUL;
    return 0;
}

const char *elfreloc_load(const uint8_t *elf,
                          size_t size,
                          uint64_t *cursor,
                          uint64_t limit,
                          uint64_t *entry_out)
{
    elf_image_t img;
    const char *err = elfreloc_load_ex(elf, size, cursor, limit, 0, &img, 0);
    if(err != 0)
    {
        return err;
    }
    if(entry_out != 0)
    {
        *entry_out = img.entry;
    }
    return 0;
}
