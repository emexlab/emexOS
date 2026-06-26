/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: fm.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>

#include "libdesktop.h"
#include "exui.h"
#include "libfont.h"
#include "../../libs/libbmp/bmp.h"

#define APP_TITLE  "file manager"
#define APP_W      540
#define APP_H      360
#define FONT       FONT8X12
#define FONB       FONT8X12_BOLD

#define MAX_ENTRIES  256
#define MAX_PATH     512
#define NAME_MAX_LEN 256
#define HIST_MAX     24

#define TOOLBAR_H   26
#define ROW_H       20
#define ICON_W      16
#define ICON_H      16
#define SCROLLBAR_W 10

#define FM_BG        0xFFD4D0C8u
#define FM_BG_ALT    0xFFC8C4BCu
#define FM_BG_HOVER  0xFFDFDCDAu
#define FM_SEL       0xFF000080u
#define FM_SEL_TXT   0xFFFFFFFFu
#define FM_FG        0xFF000000u
#define FM_FG_DIM    0xFF666666u
#define FM_BORDER    0xFF808080u
#define FM_BORDER_LT 0xFFFFFFFFu
#define FM_WHITE     0xFFFFFFFFu

#define ICO_FILE    "/emr/system/desktop/icons/file.bmp"
#define ICO_FOLDER  "/emr/system/desktop/icons/folder.bmp"
#define ICO_SYSFILE "/emr/system/desktop/icons/system_file.bmp"
#define ICO_LOCKED  "/emr/system/desktop/icons/locked_folder.bmp"
#define ICO_ARROW_L "/emr/system/desktop/icons/arrow_left.bmp"
#define ICO_ARROW_R "/emr/system/desktop/icons/arrow_right.bmp"

#define POPUP_RESULT "/tmp/fm_popup_result"

typedef struct {
    char name[NAME_MAX_LEN];
    int  is_dir;
} fm_entry_t;

static bmp_image_t 	ico_file;
static bmp_image_t 	ico_folder;
static bmp_image_t 	ico_sysfile;
static bmp_image_t 	ico_locked;
static bmp_image_t 	ico_arrow_l;
static bmp_image_t 	ico_arrow_r;

static fm_entry_t entries[MAX_ENTRIES];
static int  entry_count 	= 0;
static int  sel_idx     	= -1;
static int  hover_idx   	= -1;
static int  scroll_top  	= 0;

static char 	cwd[MAX_PATH];
static char 	history[HIST_MAX][MAX_PATH];
static int  	hist_pos  = 0;
static int  	hist_size = 0;

static DesktopArea g_ca;

static int cmx = 0;
static int cmy = 0;
static unsigned char cur_buttons  = 0;
static unsigned char prev_buttons = 0;

static void load_icons(void)
{
    bmp_load(ICO_FILE, &ico_file);
    bmp_load(ICO_FOLDER, &ico_folder);
    bmp_load(ICO_SYSFILE, &ico_sysfile);
    bmp_load(ICO_LOCKED, &ico_locked);
    bmp_load(ICO_ARROW_L, &ico_arrow_l);
    bmp_load(ICO_ARROW_R, &ico_arrow_r);
}

static bmp_image_t *pick_icon(fm_entry_t *e)
{
    if (e->is_dir)
    {
        if (strcmp(e->name, "emr") == 0 || strcmp(e->name, "dev") == 0)
            return &ico_locked;
        return &ico_folder;
    }

    int nlen = (int)strlen(e->name);
    if (nlen > 4 && strcmp(e->name + nlen - 4, ".elf") == 0)  return &ico_sysfile;
    if (nlen > 4 && strcmp(e->name + nlen - 4, ".emx") == 0)  return &ico_sysfile;

    return &ico_file;
}

static void path_join(const char *base, const char *name, char *out, int sz)
{
    int bl = (int)strlen(base);
    int nl = (int)strlen(name);
    if (bl + nl + 2 > sz) return;
    memcpy(out, base, bl);
    if (bl > 0 && out[bl - 1] != '/') out[bl++] = '/';
    memcpy(out + bl, name, nl + 1);
}

static void load_dir(void)
{
    DIR *dir = opendir(cwd);
    entry_count = 0;
    sel_idx = -1;
    hover_idx = -1;
    scroll_top = 0;

    if (!dir) return;

    struct dirent *ent;

    while (entry_count < MAX_ENTRIES && (ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0) continue;
        strncpy(entries[entry_count].name, ent->d_name, NAME_MAX_LEN - 1);
        entries[entry_count].name[NAME_MAX_LEN - 1] = '\0';
        entries[entry_count].is_dir = (ent->d_type == DT_DIR) ? 1 : 0;
        entry_count++;
    }

    closedir(dir);
}

static void navigate_to(const char *p)
{
    if (hist_pos < hist_size - 1) hist_size = hist_pos + 1;

    if (hist_size < HIST_MAX) {
        strncpy(history[hist_size], cwd, MAX_PATH - 1);
        hist_size++;
    }

    hist_pos = hist_size - 1;
    strncpy(cwd, p, MAX_PATH - 1);
    cwd[MAX_PATH - 1] = '\0';

    load_dir();
}

static void nav_back(void)
{
    if (hist_pos <= 0 || hist_size == 0) return;
    if (hist_pos < hist_size) strncpy(history[hist_pos], cwd, MAX_PATH - 1);
    hist_pos--;
    strncpy(cwd, history[hist_pos], MAX_PATH - 1);

    load_dir();
}

static void nav_forward(void)
{
    if (hist_pos >= hist_size - 1) return;
    hist_pos++;
    strncpy(cwd, history[hist_pos], MAX_PATH - 1);

    load_dir();
}

static void nav_up(void)
{
    int n = (int)strlen(cwd);
    if (n <= 1) return;

    char par[MAX_PATH];
    strncpy(par, cwd, MAX_PATH - 1);
    int i = n - 1;

    if (par[i] == '/') i--;

    while (i > 0 && par[i] != '/') i--;

    if (i == 0) { par[0] = '/'; par[1] = '\0'; }
    else{ par[i] = '\0'; }

    navigate_to(par);
}

static int visible_rows(void)
{
    int h = g_ca.h - TOOLBAR_H;
    return h < ROW_H ? 0 : h / ROW_H;
}

static void clamp_scroll(void)
{
    int rows = visible_rows();

    if (sel_idx >= 0)
    {
        if (sel_idx < scroll_top) scroll_top = sel_idx;
        if (sel_idx >= scroll_top + rows) scroll_top = sel_idx - rows + 1;
    }

    if (scroll_top < 0) scroll_top = 0;

    int ms = entry_count - rows;
    if (ms < 0) ms = 0;
    if (scroll_top > ms) scroll_top = ms;
}

static int row_at(int my_coord)
{
    int ly = TOOLBAR_H;
    int lh = g_ca.h - TOOLBAR_H;

    if (my_coord < ly || my_coord >= ly + lh) return -1;

    int ei = scroll_top + (my_coord - ly) / ROW_H;

    return (ei >= 0 && ei < entry_count) ? ei : -1;
}

static void open_entry(int ei)
{
    if (ei < 0 || ei >= entry_count) return;

    fm_entry_t *e = &entries[ei];

    if (e->is_dir)
    {
        if (strcmp(e->name, "..") == 0) {
        	nav_up();
        }
        else
        {
            char np[MAX_PATH];
            path_join(cwd, e->name, np, MAX_PATH);
            navigate_to(np);
        }
    } else
    {
        int nlen = (int)strlen(e->name);
        if (nlen > 4 && strcmp(e->name + nlen - 4, ".elf") == 0)
        {
            char fp[MAX_PATH];
            path_join(cwd, e->name, fp, MAX_PATH);
            char *const argv[] = { fp, (char *)0 };
            char *const envp[] = { (char *)0 };
            pid_t pid = fork();
            if (pid == 0) { execve(fp, argv, envp); _exit(1); }
        }
    }
}


static int dclick_idx   	= -1;
static int dclick_timer 	= 0;


#define DC_TICKS 22

static void on_click(int ei)
{
    if (ei < 0) {
        sel_idx = -1;
        dclick_idx = -1;
        dclick_timer = 0;
        return;
    }

    sel_idx = ei;


    if (ei == dclick_idx && dclick_timer > 0)
    {
        dclick_timer 	= 0;
        dclick_idx   	= -1;
        open_entry(ei);
    } else {
        dclick_idx   = ei;
        dclick_timer = DC_TICKS;
    }
}

static const char *ctx_items[] = { "open", ":", "copy path" };
#define CTX_COUNT 3


static void popup_child(int sx, int sy)
{
    int mw, mh;
    exui_menu_t menu;
    /*
     * eXui popups
     */
    exui.MenuSize(CTX_COUNT, &mw, &mh);

    desktop.createPopup(sx, sy, mw, mh);

    DesktopArea ca = desktopContentArea(sx, sy, mw, mh, DT_POPUP);

    exui_init(ca.w, ca.h);

    menu.items = ctx_items;
    menu.count = CTX_COUNT;
    menu.hover = -1;

    int pmx = 0, pmy = 0;
    unsigned char pb = 0, ppb = 0;

    for (;;)
    {
        ppb = pb;

        dt_event_t evs[16];
        int n = desktop.pollEvents(evs, 16);

        for (int i = 0; i < n; i++)
        {
            if (evs[i].type == DT_EV_MOUSE)
            {
                pmx = evs[i].mx;
                pmy = evs[i].my;
                pb  = evs[i].buttons;
            }
        }

        int pclicked = (pb & DT_BTN_LEFT) && !(ppb & DT_BTN_LEFT);

        int result = exui.MenuDraw(&menu, pmx, pmy, pclicked);
        exui.Flush();

        if (result >= 0 || result == -2)
        {
            int fd = open(POPUP_RESULT, O_WRONLY | O_CREAT);
            if (fd >= 0)
            {
                char rb[4];
                rb[0] = (char)('0' + (result >= 0 ? result : 9));
                rb[1] = '\n';
                write(fd, rb, 2);
                close(fd);
            }
            break;
        }

        for (volatile int i = 0; i < 1000; i++) __asm__ volatile("pause");
    }

    desktop.closeWindow();
    _exit(0);
}

static int run_popup(int sx, int sy)
{
    unlink(POPUP_RESULT);

    pid_t pid = fork();
    char rb[4];
    int fd = open(POPUP_RESULT, O_RDONLY);

    if (pid == 0) popup_child(sx, sy);
    waitpid(pid, NULL, 0);

    if (fd < 0) return -1;


    int n = (int)read(fd, rb, sizeof(rb) - 1);

    close(fd);
    unlink(POPUP_RESULT);

    if (n <= 0) return -1;

    int idx = rb[0] - '0';
    return idx == 9 ? -1 : idx;
}

static void draw(void)
{
    int cw = g_ca.w;
    int ch = g_ca.h;
    int fw = font_w(FONT);
    int fh = font_h(FONT);

    exui.Clear(FM_BG);
    exui.DrawRectangle(0, 0, cw, TOOLBAR_H, FM_BG);
    exui.HSep(0, TOOLBAR_H - 1, cw, FM_BORDER);

    int bsz 	= TOOLBAR_H - 6;
    int bx  	= 3;
    int by  	= 3;

    int lclick = (cur_buttons & DT_BTN_LEFT) && !(prev_buttons & DT_BTN_LEFT);

    exui.IconButton(
    	bx, by, bsz, bsz,
        ico_arrow_l.pixels, ico_arrow_l.width, ico_arrow_l.height,
        cmx, cmy, lclick, NULL
    );

    if (
    	lclick && hist_pos > 0 &&
        cmx >= bx && cmx < bx + bsz && cmy
        >= by && cmy < by + bsz
    ){
        nav_back();
        return;
    }
    bx += bsz + 2;

    exui.IconButton(
    	bx, by, bsz, bsz,
        ico_arrow_r.pixels, ico_arrow_r.width, ico_arrow_r.height,
        cmx, cmy, lclick, NULL
    );

    if (
    	lclick && hist_pos < hist_size - 1 &&
        cmx >= bx && cmx < bx + bsz && cmy
        >= by && cmy < by + bsz
    ){
        nav_forward();
        return;
    }
    bx += bsz + 2;


    exui.Button(
    	bx, by, bsz, bsz,
     	"^", FONB, cmx, cmy, lclick, NULL
    );

    if (
    	lclick && (int)strlen(cwd) > 1 &&
        cmx >= bx && cmx < bx + bsz && cmy >= by && cmy < by + bsz
    )
    {
        nav_up();
        return;
    }
    bx += bsz + 5;

    int pbw = cw - bx - 3;
    int pbh = bsz;
    exui.DrawRectangle(bx, 	      by,       pbw, pbh, FM_WHITE );
    exui.DrawRectangle(bx,        by,       pbw, 1,   FM_BORDER);
    exui.DrawRectangle(bx,        by,       1,   pbh, FM_BORDER);
    exui.DrawRectangle(bx,        by+pbh-1, pbw, 1,   FM_BORDER_LT);
    exui.DrawRectangle(bx+pbw-1,  by,       1,   pbh, FM_BORDER_LT);

    {
        int mc   = (pbw - 6) / fw;
        int pl   = (int)strlen(cwd);

        if (mc < 1) mc = 1;
        const char *ps = (pl > mc) ? cwd + pl - mc : cwd;

        exui.Print(bx + 3, by + (pbh - fh) / 2, ps, FM_FG, FM_WHITE, FONT);
    }

    int ly   	= TOOLBAR_H;
    int lh   	= ch - TOOLBAR_H;
    int rows 	= visible_rows();
    int nsb  	= (entry_count > rows);
    int lw   	= cw - (nsb ? SCROLLBAR_W : 0);

    for (int i = 0; i < rows; i++)
    {
        int ei  = scroll_top + i;
        int ry  = ly + i * ROW_H;

        if (ei >= entry_count) {
            exui.DrawRectangle(0, ry, lw, ROW_H, i % 2 ? FM_BG_ALT : FM_BG);
            continue;
        }

        fm_entry_t *e  = &entries[ei];
        int sel  = (ei == sel_idx);
        int hov  = (ei == hover_idx);

        unsigned int bg =
            sel ? FM_SEL     				:
            hov ? FM_BG_HOVER				:
            (i % 2 ? FM_BG_ALT : FM_BG)
        ;

        unsigned int tc = sel ? FM_SEL_TXT : FM_FG;
        unsigned int dc = sel ? FM_SEL_TXT : FM_FG_DIM;

        exui.DrawRectangle(0, ry, lw, ROW_H, bg);

        bmp_image_t *ico = pick_icon(e);

        if (ico && ico->pixels)
        {
        	exui.BlitPixels(
         		2, ry + (ROW_H - ICON_H) / 2,
                ico->width, ico->height, ico->pixels,
                ICON_W, ICON_H
         	);
        }

        int name_x   = 2 + ICON_W + 4;
        int max_name = (lw - name_x - 30) / fw;
        if (max_name < 1) max_name = 1;

        char 	nb[NAME_MAX_LEN];
        int nlen = (int)strlen(e->name);

        if (nlen > max_name && max_name > 4)
        {
            strncpy(nb, e->name, (unsigned)(max_name - 3));
            nb[max_name - 3] = '\0';
            strcat(nb, "...");
        } else
        {
            strncpy(nb, e->name, NAME_MAX_LEN - 1);
            nb[NAME_MAX_LEN - 1] = '\0';
        }

        exui.Print(name_x, ry + (ROW_H - fh) / 2, nb, tc, bg, e->is_dir ? FONB : FONT);

        if (e->is_dir)exui.Print(lw - 3 * fw - 3, ry + (ROW_H - fh) / 2, "DIR", dc, bg, FONT);
    }

    if (nsb)
    {
        int sbx = cw - SCROLLBAR_W;

        exui.DrawRectangle(sbx, ly, SCROLLBAR_W, lh, FM_BG_ALT);
        exui.DrawRectangle(sbx, ly, 1,           lh, FM_BORDER);

        int th  = (rows * lh) / entry_count;
        if (th < 12) th = 12;

        int rng = entry_count - rows;
        int ty  = ly + (rng > 0 ? (scroll_top * (lh - th)) / rng : 0);

        exui.DrawRectangle(sbx + 1,               ty,      SCROLLBAR_W - 2, 	th, FM_BG);
        exui.DrawRectangle(sbx + 1,               ty,      SCROLLBAR_W - 2, 	1,  FM_BORDER_LT);
        exui.DrawRectangle(sbx + 1,               ty,      1,               	th, FM_BORDER_LT);
        exui.DrawRectangle(sbx + 1,               ty+th-1, SCROLLBAR_W - 2, 	1,  FM_BORDER);
        exui.DrawRectangle(sbx + SCROLLBAR_W - 2, ty,    1,                 	th, FM_BORDER);
    }

    exui.Flush();
}

int main(void)
{
    load_icons();

    strncpy(cwd, "/", MAX_PATH - 1);
    hist_size = 0;
    hist_pos  = 0;
    load_dir();

    int scr_w = 1280, scr_h = 720;
    int win_x = (scr_w - APP_W) / 2;
    int win_y = (scr_h - APP_H) / 2;

    desktop.createWindow(APP_TITLE, win_x, win_y, APP_W, APP_H, DT_WIN);
    g_ca = desktopContentArea(win_x, win_y, APP_W, APP_H, DT_WIN);
    exui_init(g_ca.w, g_ca.h);

    draw();

    for (;;)
    {
        if (dclick_timer > 0) dclick_timer--;

        prev_buttons = cur_buttons;

        dt_event_t evs[DT_EVENT_QUEUE_MAX];
        int n = desktop.pollEvents(evs, DT_EVENT_QUEUE_MAX);

        int lclicked = 0;
        int rclicked = 0;

        for (int i = 0; i < n; i++)
        {
            dt_event_t *ev = &evs[i];

            if (ev->type == DT_EV_MOUSE)
            {
                cmx = ev->mx;
                cmy = ev->my;
                cur_buttons = ev->buttons;

                if ((cur_buttons  & DT_BTN_LEFT)  && !(prev_buttons & DT_BTN_LEFT))  lclicked = 1;
                if ((cur_buttons  & DT_BTN_RIGHT)  && !(prev_buttons & DT_BTN_RIGHT)) rclicked = 1;

                prev_buttons = cur_buttons;
                hover_idx    = row_at(cmy);
            }

            if (ev->type == DT_EV_KEY && ev->pressed)
            {
                unsigned int kc = ev->keycode;

                if (kc == 0x1B) goto done;

                if ((kc == 0x100 || kc == 'w' || kc == 'W') && sel_idx > 0)
                {
                    sel_idx--;
                    clamp_scroll();
                }
                if ((kc == 0x101 || kc == 's' || kc == 'S') && sel_idx < entry_count - 1)
                {
                    sel_idx++;
                    clamp_scroll();
                }
                if ((kc == '\n' || kc == '\r') && sel_idx >= 0)open_entry(sel_idx);

                if (kc == '\b') nav_up();
            }
        }

        if (lclicked && cmy >= TOOLBAR_H)
            on_click(row_at(cmy));

        if (rclicked && sel_idx >= 0 && cmy >= TOOLBAR_H)
        {
            int sx = 0, sy = 0;
            desktop.getCurrentMousePos(&sx, &sy);
            int choice = run_popup(sx, sy);
            if (choice == 0) open_entry(sel_idx);
        }

        draw();

        for (volatile int i = 0; i < 2000; i++) __asm__ volatile("pause");
    }

done:
    desktop.closeWindow();

    bmp_free(&ico_file);
    bmp_free(&ico_folder);
    bmp_free(&ico_sysfile);
    bmp_free(&ico_locked);
    bmp_free(&ico_arrow_l);
    bmp_free(&ico_arrow_r);

    return 0;
}