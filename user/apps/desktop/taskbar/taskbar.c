/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: taskbar.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "taskbar.h"
#include "../compositor/comp.h"
#include "../config/cfg.h"
#include "../../libs/libfont/libfont.h"

#include <string.h>
#include <unistd.h>

typedef struct
{
    char 	name[TB_ENTRY_NAMELEN];
    char 	exec[TB_ENTRY_EXECLEN];
} tb_entry_t;


static tb_entry_t s_entries[TB_MAX_ENTRIES];
static int	s_entry_count = 0;
static int	s_scr_w = 0;
static int	s_scr_h = 0;
static int	s_tb_y  = 0;

static void setup_entries(void)
{
    s_entry_count = 5;

    entries:
    {
	    strncpy( s_entries[0].name,
	     	"term",
	      	TB_ENTRY_NAMELEN - 1
	    );
	    strncpy( s_entries[0].exec,
	    	"/emr/system/terminal.elf",
	     	TB_ENTRY_EXECLEN - 1
	    );
	} {
	    strncpy( s_entries[1].name,
	    	"fm",
	     	TB_ENTRY_NAMELEN - 1
	    );
	    strncpy( s_entries[1].exec,
	    	"/emr/system/fm.elf",
	     	TB_ENTRY_EXECLEN - 1
	    );
	} {
	    strncpy( s_entries[2].name,
	    	"gears",
	     	TB_ENTRY_NAMELEN - 1
	    );
	    strncpy( s_entries[2].exec,
	    	"/bin/gears.elf",
	     	TB_ENTRY_EXECLEN - 1
	    );
    } {
	    strncpy( s_entries[3].name,
	    	"sysinfo",
	     	TB_ENTRY_NAMELEN - 1
	    );
	    strncpy( s_entries[3].exec,
	    	"/emr/system/sysinfo.elf",
	     	TB_ENTRY_EXECLEN - 1
	    );
    } {
	    strncpy( s_entries[4].name,
	    	"cube3d",
	     	TB_ENTRY_NAMELEN - 1
	    );
	    strncpy( s_entries[4].exec,
	    	"/emr/system/bin/cube.elf",
	     	TB_ENTRY_EXECLEN - 1
	    );
    }
}

void taskbar_init(int scr_w, int scr_h)
{
    s_scr_w 	= scr_w;
    s_scr_h 	= scr_h;
    s_tb_y  	= scr_h - TB_H;

    setup_entries();
}

int taskbar_y(void)
{
    return s_tb_y;
}

// get the x start of button i
static int btn_x(int i)
{
    return TB_BTN_PAD + i * (TB_BTN_W + TB_BTN_PAD);
}

static int hit_btn(int i, int mx, int my)
{
    int bx = btn_x(i);
    int by = s_tb_y + TB_BTN_VPAD;

    return
    	mx >= bx && mx < bx + TB_BTN_W &&
        my >= by && my < by + (TB_H - TB_BTN_VPAD * 2)
    ;
}

void taskbar_draw(int mx, int my, int btn_down)
{
    int y  = s_tb_y;
    int w  = s_scr_w;

    // bar bg
    for (int dy = 0; dy < TB_H; dy++) comp_fill(0, y + dy, w, 1, TB_BACKGROUND);

    // top border line
    comp_fill(0, y, w, 1, TB_TOP_BORDER);

    for (int i = 0; i < s_entry_count; i++)
    {
        int bx  = btn_x(i);
        int by   = y + TB_BTN_VPAD;
        int bh  = TB_H - (TB_BTN_VPAD * 2);
        int hov = hit_btn(i, mx, my);
        int press = hov && btn_down;

        // button face
        comp_fill(bx, by, TB_BTN_W, bh, TB_BACKGROUND);

        // win95 style raised/pressed borders
       	/*{
		    if (!press)
		    {
				comp_fill(bx, by, TB_BTN_W, 1, TB_LIGHT);
        		comp_fill(bx, by + bh - 1, TB_BTN_W, 1, TB_LIGHT);
         		comp_fill(bx, by, 1, bh, TB_LIGHT);
          		comp_fill(bx + TB_BTN_W - 1, by, 1, bh, TB_LIGHT);
		    } else
		    {
				comp_fill(bx, by, TB_BTN_W, 1, TB_LIGHT);
        		comp_fill(bx, by + bh - 1, TB_BTN_W, 1, TB_LIGHT);
          		comp_fill(bx, by, 1, bh, TB_LIGHT);
           		comp_fill(bx + TB_BTN_W - 1, by, 1, bh, TB_LIGHT);
		    }
     	}*/

	    {
	        int show_border = hov || press;

	        if (show_border)
	        {
	            unsigned int col = press ? TB_BTN_TOP : TB_LIGHT;

	            comp_fill(bx, by, TB_BTN_W, TB_BORDER_W, col);
	            //comp_fill(bx, by + bh - TB_BORDER_W, TB_BTN_W, TB_BORDER_W, col);
	            //comp_fill(bx, by, TB_BORDER_W, bh, col);
	            //comp_fill(bx + TB_BTN_W - TB_BORDER_W, by, TB_BORDER_W, bh, col);

				comp_fill(bx, by, TB_BTN_W, bh, TB_BUTTON_BG);
	        }
	    }

        // label, centered
        int fw   	= font_w(FONT8X12_BOLD);
        int fh   	= font_h(FONT8X12_BOLD);
        int nlen 	  = (int)strlen(s_entries[i].name);
        int tw   	= nlen * fw;
        int tx   	= bx + (TB_BTN_W - tw) / 2 + (press ? 1 : 0);
        int ty   	= by  + (bh - fh) / 2 + (press ? 1 : 0);
        // draw chars one by one into the comp backbuffer
        for (int ci = 0; ci < nlen; ci++)
        {
            unsigned char c = (unsigned char)s_entries[i].name[ci] & 0x7Fu;
            for (int row = 0; row < fh; row++)
            {
                uint16_t bits = font_glyph(FONT8X12_BOLD, c, row);
                for (int col = 0; col < fw; col++)
                {
                    unsigned int color = (bits & (1u << col)) ? TB_WHITE : TB_BUTTON_BG;
                    comp_set(tx + ci * fw + col, ty + row, color);
                }
            }
        }
    }
}

int taskbar_click(int mx, int my)
{
    if (my < s_tb_y) return 0;

    for (int i = 0; i < s_entry_count; i++)
    {
        if (!hit_btn(i, mx, my)) continue;

        // just spawn and forget, no wait
        pid_t pid = fork();
        if (pid == 0)
        {
            char *argv[] = { s_entries[i].exec, (char *)0 };
            char *envp[] = { (char *)0 };

            execve(s_entries[i].exec, argv, envp);
            _exit(1);
        }

        return 1;
    }
    return 0;
}