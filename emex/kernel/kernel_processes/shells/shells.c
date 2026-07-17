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

#include "shells.h"
#include <drivers/ps2/keyboard/keyboard.h>

void emergency_shell(void) {
	//dump_kprocesses();
    log("[KERNEL]", "FAILED TO JUMP TO USERMODE\n\n", error);
    log("::", "emexOS couldn't load \"" SYSTEMLOCATE2 "\"\n", _d);
    log("::", "you are now being dropped into the emergencyshell...\n", _d);
    print("\n", SHELL_FG_COLOR);

    //first clear screen
    //clear(0xff000000);

    print("At the top you should see a list like this\n", SHELL_FG_COLOR);
    print("Process List:\n", SHELL_FG_COLOR);
    print("PID   State       Name            Entry Point\n", SHELL_FG_COLOR);
    print("----  ----------  --------------  -----------\n", SHELL_FG_COLOR);
    print("3     READY       app.elf         0x400A0000\n", SHELL_FG_COLOR);
    print("2     READY       __rt            0x40001000\n", SHELL_FG_COLOR);
    print("1     READY       kernel          0x40000000\n\n", SHELL_FG_COLOR);

    print(" If your list looks the same as this, the jump likely failed.\n", SHELL_FG_COLOR);
    print(" In that case, try rebooting the system.\n\n", SHELL_FG_COLOR);

    print(" If your list looks different, there may be a hardware problem.\n", SHELL_FG_COLOR);
    print(" Please open an issue and include as much information as possible.\n\n", SHELL_FG_COLOR);

    print(" If rebooting multiple times does not fix the problem, also open an issue:\n", SHELL_FG_COLOR);
    print(" > https://github.com/emexos/emexOS/issues\n\n", SHELL_FG_COLOR);


    char ibuffer[128];
    int index =0;

    print(EPROMPT, SHELL_FG_COLOR);

    while (1) {
    	if (!keyboard_has_key()) continue;

     	char c = keyboard_get_key();

		if (c == '\n')
		{
			ibuffer[index] = '\0';
			if (index > 0 && ibuffer[index -1] == '\n') {
				ibuffer[index -1] = '\0';
			}
			index = 0;

			print("\n", SHELL_FG_COLOR);
			print(EPROMPT, SHELL_FG_COLOR);
		}
		else if (c == '\b') {
			if (index > 0) {
				index--;
				print("\b \b", SHELL_FG_COLOR);

			}
		}
		else if (str_equals(ibuffer,"exit") == 1) {
			print("\n:: {debug: ==1;}", SHELL_FG_COLOR);
			print("\n:: exiting recovery shell", SHELL_FG_COLOR);

			print("\n", SHELL_FG_COLOR);

			return;
		}
		// everything else just echo's
		else {
			ibuffer[index++] = c;
			char echostr[2] = {c, 0};

			print(echostr, SHELL_FG_COLOR);
		}
    }

    //__builtin_unreachable();
}

void recovery_shell(void) {
	//dump_kprocesses();
	//
	log("::", "you are now being dropped into the recoveryshell...\n", _d);
	print("\n", SHELL_FG_COLOR);


	print(RPROMPT, SHELL_FG_COLOR);

	char ibuffer[128];
    int index =0;

    while (1) {
    	if (!keyboard_has_key()) continue;

     	char c = keyboard_get_key();

		if (c == '\n')
		{
			ibuffer[index] = '\0';
			if (index > 0 && ibuffer[index -1] == '\n') {
				ibuffer[index -1] = '\0';
			}
			index = 0;

			print("\n", SHELL_FG_COLOR);
			print(RPROMPT, SHELL_FG_COLOR);
		}
		else if (c == '\b') {
			if (index > 0) {
				index--;
				print("\b \b", SHELL_FG_COLOR);

			}
		}
		else if (str_equals(ibuffer,"exit") == 1) {
			print("\n:: {debug: ==1;}", SHELL_FG_COLOR);
			print("\n:: exiting recovery shell", SHELL_FG_COLOR);

			print("\n", SHELL_FG_COLOR);

			return;
		}
		// everything else just echo's
		else {
			ibuffer[index++] = c;
			char echostr[2] = {c, 0};

			print(echostr, SHELL_FG_COLOR);
		}
    }

	//}

    //__builtin_unreachable();
}
