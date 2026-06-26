/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: user.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "gen.h"
#include "../shells/shells.h"
#include "../bootscreen/boot.h"

#include <kernel/include/assembly.h>
#include <kernel/packages/elf/loader.h>

#include <kernel/mem/memlog.h>
#include <kernel/file_systems/vfs/vfs.h>


// path to the init app in the VFS (loaded from initrd.cpio)
#define SYSTEMLOCATE "/emr/system/system.elf"
//mod
//#define LOGINLOCATE "/emr/bin/login.elf"

void uproc(void)
{
    #if ENABLE_ULIME
        if (!proc_mgr || !ulime) {
            log("[INIT]", "proc_mgr or ulime not ready\n", error);
            return;
        }

        log("[INIT]", "loading " SYSTEMLOCATE "...\n", d);

		static u8 init_elf_buf[512 * 1024];

        int fd = fs_open(SYSTEMLOCATE, O_RDONLY);
        if (fd < 0) {
        	log("[INIT]", "failed to load " SYSTEMLOCATE "\n", error);
            //while (1) __asm__ volatile("cli; hlt");
            // hcf();
            return;
        }
		ssize_t elf_size = fs_read(fd, init_elf_buf, sizeof(init_elf_buf));
		fs_close(fd);
		ulime_proc_t *init_proc = proc_create_proc(proc_mgr, (u8*)"system", 0, USERPRIORITY);
		if (elf_size <= 0) {
		    log("[INIT]", "empty/unreadable\n", error);
		    return;
		}
		if (!init_proc) {
		    log("[INIT]", "failed proc\n", error);
		    return;
		}
		if (elf_load(init_proc, init_elf_buf, (u64)elf_size) != 0) {
		    log("[INIT]", "elf_load failed\n", error);
		    return;
		}

        log("[INIT]", "jumping to userspace\n", success);

        //memlog_print_map();

        bootscreen_services:
        {
        	u32 fw     = get_fb_width();
	        u32 fh     = get_fb_height();
	        u32 half_w     = fw / 2;
	        u32 half_h     = fh / 2;

			bs_screens[BS1].visible = 1;
			bs_screens[BS2].visible = 1;
			bs_screens[BS3].visible = 1;
			bs_screens[BS4].visible = 0;
		    clear(BS1, 0xff000000);
		    clear(BS2, 0xff000000);
			clear(BS3, 0xff000000);
			clear(BS4, 0xff000000);
      		bs_backbuf_clear(0x00000000);

	        uninit_bootscreen();
	        bs_set_region(BS1, 0, 0,      half_w,half_h);
	        bs_set_region(BS2, 0, half_h, half_w,fh - half_h);
			bs_set_region(USER_SCREEN_MODE, 0, 0, fw, fh);
			bs_set_region(BS4, 0, 0, fw, fh);

	        bs_switch(USER_SCREEN_MODE);
			//clear(BS3, 0xff000033); // blue ig?
        }

        ulime->ptr_proc_curr = init_proc;

		#if BS_DEBUG == 1
			dump_kprocesses();
		   	proc_list_procs(proc_mgr);
			log("[INIT]", "jumping to init_proc\n", d);
        #endif
        #if JUMPTOUSER == 1
        	if (mt)
         	{
	            mt_add_task(mt, init_proc); // register init with mt
	            mt_start(mt); // and launch
				//never returns
	        }
            JumpToUserspace(init_proc); // fallback if mt is not up

        #endif

        //Fallback
        emergency_shell();
        recovery_shell();

        __builtin_unreachable();
    #endif
}
