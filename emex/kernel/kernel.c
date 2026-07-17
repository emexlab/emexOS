/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Copyright (C) 2026 emexlab
 *
 * This file is part of emexOS.
 *
 * emexOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * emexOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with emexOS. If not, see <https://www.gnu.org/licenses/>.
 */

#include <kernel/include/assembly.h>
#include <kernel/include/reqs.h>
#include <kernel/include/bootup_sound.h>
#include <kernel/graph/theme.h>
#include <kernel/communication/serial.h>

#include <drivers/drivers.h>

#include <drivers/sound/layer.h>

// configuration files
#include <config/user.h>
#include <config/system.h>
#include <config/user_config.h>

// System services
#include <kernel/limine/cmd.h>

// CPU
#include <kernel/cpu/cpu.h>
#include <kernel/pci/pci.h>
#if X64 == 1
	#include <kernel/arch/x86_64/acpi/acpi.h>
    #include <kernel/arch/x86_64/gdt/gdt.h>
    #include <kernel/arch/x86_64/idt/idt.h>
    #include <kernel/arch/x86_64/exceptions/panic.h>
    #include <kernel/arch/x86_64/exceptions/timer.h>
    #include <kernel/arch/x86_64/syscalls/syscall_init.h>
#elif RISCV == 1
    #include <kernel/arch/riscv/tables/trap.h> // trap Table / exception Table
#elif ARM64 == 1
    #include <kernel/arch/arm64/exception/vectab.h> // vector table
    #include <kernel/arch/arm64/syscalls/syscall_init.h>
#endif

//Devices
#include <kernel/devices/null/null.h>
#include <kernel/devices/zero/zero.h>
#include <kernel/devices/disks/hdd0.h>
#include <kernel/devices/fb0/fb0.h>
#include <kernel/devices/input/kbd.h>
#include <kernel/devices/input/mouse0.h>
#include <kernel/devices/net/eth0.h>
#include <kernel/devices/vt/vt.h>
#include <kernel/devices/random/urandom.h>
#include <kernel/devices/random/random.h>

// usermode stuff
#include <kernel/user/user.h>

// executables
#include <kernel/packages/elf/loader.h>
#include <kernel/packages/cpio/cpio.h>
#include <kernel/packages/gz/gzip.h>


// Memory
#include <kernel/mem/lib/main.h>
#include <kernel/mem/meminclude.h>
#include <kernel/mem/memlog.h>
klime_t *klime = NULL;
ulime_t *ulime = NULL;

#include <kernel/proc/scheduler.h>
scheduler_t *scheduler = NULL;
#define SCHEDQUANT 1

#include <kernel/proc/proc_manager.h>
proc_manager_t *proc_mgr = NULL;

#include <kernel/kernel_processes/kernel/gen.h>
#include <kernel/kernel_processes/loader.h>
#include <kernel/kernel_processes/bootscreen/boot.h>
#include <kernel/kernel_processes/fm/fm.h>

#include <kernel/multitasking/multitasking.h>
mt_t *mt = NULL;

#include <kernel/ipc/ipc.h>

//vFS & fs & disk
#include <kernel/file_systems/vfs/vfs.h>
#include <kernel/file_systems/vfs/config.h>
#include <kernel/kernel_processes/initrd/initrd.h>

#include <kernel/interface/partition.h>
#include <kernel/interface/disk.h>
#include <kernel/interface/mbr.h>
#include <config/disk.h>


// limine modules
#include <kernel/limine/limine.h>
#include <kernel/kernel.h>

kglobal_t kglobal;

void _start(void)
{
    d_boot_screen: { // Initializing Boot screen
        // Temporaly before switchin to glime_t
        // emexOS start
        // Ensure that Limine base revision is supported and that we have a framebuffer
        if (framebuffer_request.response == NULL ||
            framebuffer_request.response->framebuffer_count < 1)
        {
            printf("no response");
            hcf(); // enable text mode
        }
        // Initialize framebuffer graphics
        struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
        {
	        graphics_init(fb);
	        kproc_loader_init();
	        fm_init();
			init_bootscreen();
	        cmd_init(); //cmdline limine
        }
        log("::", "finished loading essr\n", _d); // emex system start recources

        font_scale = 1;

        #if BOOTUP_VISUALS == 1 // verify screenmode
            log("[BOOT]", "BOOTUP_VISUALS == 1\n", warning);
        #else
            log("[BOOT]", "BOOTUP_VISUALS == 0\n", warning);
        #endif

        BOOTUP_PRINT("\n\n ========================\n", white());
        BOOTUP_PRINT(" =  Welcome to ", white());
        BOOTUP_PRINT("emexOS", cyan());
        BOOTUP_PRINT("!  =\n", white());
        BOOTUP_PRINT(" ========================\n\n", white());

        //actually not needed but maybe later (e.g. for testing themes)
        //draw_rect(10, 10, fb_width - 20, fb_height - 20, blue());
    }

    char buf[512]; //for all string operations
    #if ENABLE_ULIME
        scheduler = scheduler_init(ulime, 10);  // 10 tick quantum
        proc_mgr = proc_mng_init(ulime);
    #endif

    memory_ss: { // MADE BY @TSARAKI (github)
        log("[MEM]", "Initializing memory management\n", d);
        // Initialize mem
        physmem_init(memmap_request.response, hhdm_request.response);
        paging_init(hhdm_request.response);

        map_region_alloc(hhdm_request.response, HEAP_START, HEAP_SIZE);

        // kernel lifetime
        klime_t *klime = klime_init((u64 *)HEAP_START, HEAP_SIZE);
        kglobal.klime = klime;

        if (!framebuffer_request.response) {
            panic("Cant initialize glime limine response NULL");
        }

        if (framebuffer_request.response->framebuffer_count < 1) {
            panic("Cant initialize glime limine framebuffer_count 0");
        }

        #if ENABLE_ULIME
            u64 phys_ulime = map_region_alloc(hhdm_request.response, ULIME_START, ULIME_META_SIZE);

		    ulime = ulime_init(hhdm_request.response, klime, phys_ulime);
	        if (!ulime) {
	            BOOTUP_PRINTF("Error: ulime is not initialized");
	            panic("Error: ulime is not initialized");
	        }
        #else
            log("[ULIME]", "skipped (hardware compatibility)\n", warning);
        #endif

        fs_system_init(klime);
    }

    hal: {
	    cpu_detect(); { // update system informatiosn
		    bs_switch(BS2);
		    bs2_draw_info();
		    bs_switch(BS1);
		}
        gdt_init();
        idt_init();
        cpu_enable_sse();
    }

    #if ENABLE_ULIME
        if (ulime) {
            scheduler = scheduler_init(ulime, SCHEDQUANT);
            proc_mgr = proc_mng_init(ulime);

            mt = (mt_t*)klime_create(ulime->klime, sizeof(mt_t));
            if (mt) {
                mt_init(mt, scheduler, ulime);
                //log("[MT]", "multitasking initialized\n", d);
                //ipc_init();
                //ipc_test();
            }

            if (scheduler) {
                char buf[32];
                str_append_uint(buf, SCHEDQUANT);
                log("[SCHED]", "initialized quantum == ", d);
                BOOTUP_PRINT(buf, white());
                BOOTUP_PRINT("\n", white());
            }
            if (proc_mgr) {
                log("[PROCMGR]", "initialized\n", d);
            }

            init_ipc:
           	{
                ipc_messages_init();
				log("[IPC_MESSAGES]", "initialized\n", d);
                ipc_shm_init();
				log("[IPC_SHM]", "initialized\n", d);
                //ipc_test();
            };

            syscall_arch_init(); // SYSCALL/SYSRET
            _init_syscalls_table(ulime);
        }
    #endif

    init_acpi(); // Init ACPI

    u32 freq = 1000;
    timer_init(freq);
    BOOTUP_PRINT_INT(freq, white());
    BOOTUP_PRINT(" 1ms tick)\n", white());
    timer_set_boot_time(); //for uptime command

    pci_init();
    //disks_init();

    limine_module_ss:
    limine_modules_init(); {
        initrd_load();
    }

    fs_mount(NULL, SYS_MOUNT_DEFAULT, SYSFS);

    memlog_print_map();

    module_ss:
    module_init(); {
        // Register device modules
        log("[MOD]", "Init modules...\n", d);

        //module_register(&drive0_module);
        module_register(&null_module);
        module_register(&zero_module);
        module_register(&fb0_module);
        module_register(&kbd_dev_module);
        module_register(&mouse0_module);
        module_register(&eth0_module);
        module_register(&urandom_module);
        module_register(&random_module);
        module_register(&audio0_module);
        module_register(&vt_module);

        log("[MOD]", "found ", d);
        int count = module_get_count();
        str_append_uint(buf, count);
        BOOTUP_PRINT(buf, yellow());
        BOOTUP_PRINT(" module(s)\n", white());

    }

    { // Final things
        fs_register_mods();

        //create test file in /tmp
        fs_create_test_file();

        //buf[0] = '\0'; // clear buffer so it can be used again
        bootup_sound();
        uci();
    }

    kproc();
    init_kernelprocesses2();
    uproc();

    //should not reach here
    //font_manager_set_context(FONT_CONTEXT_PANIC);
    #if USE_HCF == 1
        hcf();
    #else
        panic("USE_HCF; FAILED --> USING PANIC");
    #endif

}
