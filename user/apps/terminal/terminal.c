/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: terminal.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "exui.h"
#include "libdesktop.h"
#include "libfont.h"

#include <emx/fb.h>
#include <emx/tty.h>

//-////////////////////////////////////////-//
//-//                                    //-//
//-//             TERMINAL :)            //-//
//-//       emex user-space shell        //-//
//-//                                    //-//
//-////////////////////////////////////////-//

#define BUFFER 256
#define SHELL_PROMPT "[pc@emexos]$ "
#define SHELL_CONFIG "/.config/terminal/"
#define BIN_PATH "/bin/"
#define KBD_PATH "/dev/input/keyboard0"
#define VT_CTRL_PATH "/dev/vt/control"
#define SHELL_W 700
#define SHELL_H 450

#define WELCOME_MESSAGE                                                        \
  "\n\033[0m Welcome to shelly, emexOS's default shell.\n Type \"ls /bin\" "   \
  "for a list of commands.\n\n"

// shell colors (ARGB)
#define SH_BG 0xFF111111u
#define SH_FG 0xFFCCCCCCu
#define SH_PROMPT 0xFF55FF55u
#define SH_ERR 0xFFFF5555u
#define SH_INFO 0xFF5599FFu

// text buffer
#define TBUF_ROWS 200
#define TBUF_COLS 100

// color indices into sh_colors[]
#define CI_FG 0
#define CI_PROMPT 1
#define CI_ERR 2
#define CI_INFO 3

#define FONT FONT8X12

static const unsigned int sh_colors[4] = {SH_FG, SH_PROMPT, SH_ERR, SH_INFO};

typedef struct {
  unsigned int keycode;
  unsigned int modifiers;
  unsigned char pressed;
} key_event_t;

static char t_text[TBUF_ROWS][TBUF_COLS + 1];
static unsigned char t_clr[TBUF_ROWS][TBUF_COLS];
static int t_row = 0;
static int t_col = 0;
static int t_vtop = 0;

static int g_kbd = -1;
static int g_vt_ctrl = -1;
static unsigned long long g_vt_id = (unsigned long long)-1;
static int g_vt_out = -1;
static int g_vt_in  = -1;
static char ibuf[BUFFER];
static int ilen = 0;

static void ensure_visible(void)
{
	int vrows = exui_height() / font_h(FONT);
	if (vrows <= 0) vrows = 1;
	if (t_row >= t_vtop + vrows) t_vtop = t_row - vrows + 1;
}

static void next_row(void)
{
	t_row++;
	t_col = 0;
	if (t_row >= TBUF_ROWS) {
		for (int i = 0; i < TBUF_ROWS - 1; i++) {
		    memcpy(t_text[i], t_text[i + 1], TBUF_COLS + 1);
		    memcpy(t_clr[i], t_clr[i + 1], TBUF_COLS);
		}
		t_text[TBUF_ROWS - 1][0] = '\0';
		memset(t_clr[TBUF_ROWS - 1], 0, TBUF_COLS);
		t_row = TBUF_ROWS - 1;
		if (t_vtop > 0) t_vtop--;
	}
	t_text[t_row][0] = '\0';
	memset(t_clr[t_row], 0, TBUF_COLS);
	ensure_visible();
}

static void buf_putc(char c, int ci)
{
	if (c == '\n') {
		next_row();
		return;
	}
	if (c == '\r') return;
	if (c == '\b') {
		if (t_col > 0) {
			t_col--;
			t_text[t_row][t_col] = '\0';
		}
		return;
	}

	int vcols = exui_width() / font_w(FONT);

	if (vcols <= 0 || vcols > TBUF_COLS) vcols = TBUF_COLS;
	if (t_col >= vcols) next_row();
	if (t_col < TBUF_COLS) {
		t_text[t_row][t_col] = c;
		t_clr[t_row][t_col] = (unsigned char)ci;
		t_col++;
		t_text[t_row][t_col] = '\0';
	}
}

static void buf_puts(const char *s, int ci)
{
  	while (*s) buf_putc(*s++, ci);
}

static void redraw(void)
{
	int fw = font_w(FONT);
	int fh = font_h(FONT);
	int vrows = exui_height() / fh;
	char tmp[2];
	tmp[1] = '\0';

	exui.Clear(SH_BG);

	for (int r = t_vtop; r <= t_row && r < t_vtop + vrows; r++) {
		int vis = r - t_vtop;
		int py = vis * fh;
		for (int c = 0; c < TBUF_COLS && t_text[r][c]; c++) {
			unsigned int fg = sh_colors[t_clr[r][c] < 4 ? t_clr[r][c] : 0];
			tmp[0] = t_text[r][c];
			exui.Print(c * fw, py, tmp, fg, SH_BG, FONT);
		}
	}

	int vis = t_row - t_vtop;
	if (vis >= 0 && vis < vrows) exui.DrawCursor(t_col * fw, vis * fh, SH_FG, FONT);

	exui.Flush();
}

static void drain_vt_output(void)
{
	if (g_vt_out < 0) return;
	char vt_buf[512];
	int n;
	while ((n = (int)read(g_vt_out, vt_buf, sizeof(vt_buf) - 1)) > 0) {
		vt_buf[n] = '\0';
		buf_puts(vt_buf, CI_FG);
	}
}

static void vt_build_path(char *out, unsigned long long id, const char *suffix) {
	int i = 0;
	const char *pfx = "/dev/vt/";
	while (*pfx) out[i++] = *pfx++;

	char tmp[24]; int ti = 0;
	unsigned long long v = id;
	if (!v) { tmp[ti++] = '0'; }

	else { while (v) { tmp[ti++] = '0' + (int)(v % 10); v /= 10; } }
	for (int a = 0, b = ti - 1; a < b; a++, b--) { char c = tmp[a]; tmp[a] = tmp[b]; tmp[b] = c; }
	for (int k = 0; k < ti; k++) out[i++] = tmp[k];

	out[i++] = '/';
	while (*suffix) out[i++] = *suffix++;
	out[i] = '\0';
}

static int parse_args(char *buf, char **argv, int maxn)
{
	int argc = 0;
	char *p = buf;
	while (*p && argc < maxn - 1)
	{
		while (*p == ' ') p++;
		if (!*p) break;
		argv[argc++] = p;

		while (*p && *p != ' ') p++;
		if (*p == ' ')
		*p++ = '\0';
	}

	argv[argc] = NULL;
	return argc;
}

static int file_exists(const char *path)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0) return 0;
	close(fd);
	return 1;
}

static void run_cmd(void)
{
	if (!ilen) return;

	char buf[BUFFER];
	memcpy(buf, ibuf, (unsigned)(ilen + 1));

	char *argv[32];
	int argc = parse_args(buf, argv, 32);

	if (!argc) return;

	if (!strcmp(argv[0], "exec")) {
		if (!argv[1])
		{
			buf_puts("exec: path required\n", CI_ERR);
			return;
		}
		char *const envp[] = {(char *)0};
		pid_t pid = fork();

		if (pid == 0) {
			execve(argv[1], argv + 1, envp);
			_exit(1);
		}
		if (pid > 0) {
			waitpid(pid, NULL, 0);
			drain_vt_output();
			redraw();
		}
		return;
	}

	char path[BUFFER];
	size_t bl = sizeof(BIN_PATH) - 1;
	size_t cl = strlen(argv[0]);
	if (bl + cl + 5 > BUFFER) {
		buf_puts(argv[0], CI_ERR);
		buf_puts(": name too long\n", CI_ERR);
		return;
	}
	memcpy(path, BIN_PATH, bl);
	memcpy(path + bl, argv[0], cl);
	memcpy(path + bl + cl, ".elf", 5);

	if (!file_exists(path)) {
		buf_puts(argv[0], CI_ERR);
		buf_puts(": command not found\n", CI_ERR);
		return;
	}

	char *const envp[] = {(char *)0};
	pid_t pid = fork();
	if (pid == 0) {
		execve(path, argv, envp);
		_exit(127);
	} else if (pid > 0) {
		waitpid(pid, NULL, 0);
		drain_vt_output();
		redraw();
	} else {
		buf_puts("fork failed\n", CI_ERR);
	}
}

static void vt_setup(void)
{
	g_vt_ctrl = open(VT_CTRL_PATH, O_RDWR);
	if (g_vt_ctrl < 0) return;

	if (ioctl(g_vt_ctrl, VT_CREATE, &g_vt_id) != 0)
	{
		close(g_vt_ctrl);
		g_vt_ctrl = -1;
		return;
	}

	char path_out[64], path_in[64];
	vt_build_path(path_out, g_vt_id, "output");
	vt_build_path(path_in, g_vt_id,  "input");

	g_vt_out = open(path_out, O_RDWR);
	g_vt_in = open(path_in, O_WRONLY);

	ioctl(g_vt_ctrl, VT_FOCUS, &g_vt_id);

	// bind this process and all its children to the vt
	// so write(1,...) in child processes goes into g_vt_out
	vt_setproc(g_vt_id);
}

static void vt_teardown(void)
{
	if (g_vt_ctrl >= 0 && g_vt_id != (unsigned long long)-1)
		ioctl(g_vt_ctrl, VT_DESTROY, &g_vt_id);
	if (g_vt_out  >= 0) close(g_vt_out);
	if (g_vt_in   >= 0) close(g_vt_in);
	if (g_vt_ctrl >= 0) close(g_vt_ctrl);
}

int main(void)
{
	// printf(WELCOME_MESSAGE);
	// g_fb = open("/dev/fb0", O_RDWR);
	g_kbd = open(KBD_PATH, O_RDONLY);
	if (g_kbd < 0)
	return 1;

	//g_ptmx = open("/dev/ptmx", O_RDONLY);

	int scr_w = 1280, scr_h = 720;
	int win_x = (scr_w - SHELL_W) / 2;
	int win_y = (scr_h - SHELL_H) / 2;

	if (win_x < 0)
		win_x = 0;
	if (win_y < 0)
		win_y = 0;

	desktop.createWindow("terminal", win_x, win_y, SHELL_W, SHELL_H, DT_WIN);
	DesktopArea ca = desktopContentArea(win_x, win_y, SHELL_W, SHELL_H, DT_WIN);
	exui_init(ca.w, ca.h);

	for (int i = 0; i < TBUF_ROWS; i++) {
		t_text[i][0] = '\0';
		memset(t_clr[i], 0, TBUF_COLS);
	}

	buf_puts("terminal :3\n", CI_INFO);
	buf_puts("type \"ls /bin\" for available commands.\n\n", CI_FG);

	vt_setup();
	//vt_setproc(g_vt_id);

	buf_puts("[pc@emexos]$ ", CI_PROMPT);
	redraw();

	for (;;) {
		key_event_t ev;
		if ((int)read(g_kbd, &ev, sizeof(ev)) != (int)sizeof(ev))
		    continue;
		if (!ev.pressed)
		    continue;

		if (g_vt_in >= 0)
		    write(g_vt_in, &ev, sizeof(ev));

		unsigned int kc = ev.keycode;
		char c = (char)(kc & 0xFF);

		if (kc == '\n' || kc == '\r') {
		    buf_putc('\n', CI_FG);
		    run_cmd();
		    ilen = 0;
		    ibuf[0] = '\0';
		    buf_puts("[pc@emexos]$ ", CI_PROMPT);
		    redraw();

		} else if (kc == '\b') {
		    if (ilen > 0) {
			    ilen--;
			    ibuf[ilen] = '\0';
			    buf_putc('\b', CI_FG);
			    redraw();
		    }
		} else if (c >= 0x20 && c <= 0x7E) {
		    if (ilen < BUFFER - 1) {
			    ibuf[ilen++] = c;
			    ibuf[ilen] = '\0';
			    buf_putc(c, CI_FG);
			    redraw();
		    }
		}
	}

	vt_teardown();
	return 0;
}