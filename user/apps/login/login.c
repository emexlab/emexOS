/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: login.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include "libdesktop.h"
#include "exui.h"
#include "login.h"
#include "vad.h"

#define LOGIN_W   300
#define LOGIN_H   164
#define LOGIN_PAD 10
#define FONT FONT8X12

#define APPTITLE "login"

// TODO:
// - read from /emr/config/user.emcg
// - implement a emex config file parser lib
//
//#define LOGIN_USER "emex"
//#define LOGIN_PASS "emex"
#define USERNAME "username: "
#define PASSWORD "password: "
//#define MAX_TRIES 3
//#define BUF_SIZE 64

#define KBD_PATH "/dev/input/keyboard0"

//                   AARRGGBB
#define LG_BG      0xFFD4D0C8u
#define LG_BLACK   0xFF000000u
#define LG_WHITE   0xFFFFFFFFu
#define LG_FOCUSED 0xFF000080u
#define LG_ERR     0xFFCC0000u
#define LG_FG      0xFF000000u

//#define FW FONT8X12_W
//#define FH FONT8X12_H

#define ICON_PATH "/emr/system/desktop/icons/user1.vad"

// icon size, make sure it fits inside the window height
// content area is LOGIN_H minus the titlebar (~18px) and border (~2px)
// so roughly 144px tall, 48 fits easily
#define ICON_SIZE 40

typedef struct {
    unsigned int keycode;
    unsigned int modifiers;
    unsigned char pressed;
} key_event_t;

char log_user[MAX_TRIES][BUF_SIZE];
char log_pass[MAX_TRIES][BUF_SIZE];

static int g_kbd = -1;
static DesktopArea ca;
static vad_image_t g_icon;
static int g_has_icon = 0;

static void draw_login(exui_inputbox_t *user_state, exui_inputbox_t *pass_state, const char *msg, int field)
{
    user_state->focused = (field == 0);
    pass_state->focused = (field == 1);

    int cx  = LOGIN_PAD;
    int cy  = LOGIN_PAD;
    int fh  = font_h(FONT);
    int bw  = ca.w - LOGIN_PAD * 2;

    exui.Clear(LG_WHITE);

    // draw icon top right, clamped so it cant go outside the buffer
    if (g_has_icon) {
        int ix = ca.w - ICON_SIZE - LOGIN_PAD;
        int iy = (ca.h - ICON_SIZE) / 2;
        if (ix < 0) ix = 0;
        if (iy < 0) iy = 0;
        // make sure the icon stays within bounds
        if (ix + ICON_SIZE <= ca.w && iy + ICON_SIZE <= ca.h) {
            vad_set_bg(LG_WHITE);
            vad_draw_scaled(&g_icon, ix, iy, ICON_SIZE, ICON_SIZE);
        }
    }

    exui.Print(cx, cy, "username:", LG_FG, LG_WHITE, FONT);
    cy += fh + 3;
    exui.InputBox(cx, cy, bw, user_state, LG_FG, LG_BG, field == 0 ? LG_FOCUSED : LG_BLACK, FONT, 0);
    cy += fh + 6 + 6;

    exui.Print(cx, cy, "password:", LG_FG, LG_WHITE, FONT);
    cy += fh + 3;
    exui.InputBox(cx, cy, bw, pass_state, LG_FG, LG_BG, field == 1 ? LG_FOCUSED : LG_BLACK, FONT, 0);
    cy += fh + 6 + 6;

    if (msg && msg[0]) exui.Print(cx, cy, msg, LG_ERR, LG_BG, FONT);

    exui.Flush();
}

static int str_eq(const char *a, const char *b)
{
    while (*a && *b) { if (*a != *b) return 0; a++; b++; }
    return *a == '\0' && *b == '\0';
}

int main(void)
{
    printf("debug: login started \n");

    g_kbd = open(KBD_PATH, O_RDONLY);
    if (g_kbd < 0) return 1;

    int scr_w = 1280, scr_h = 720;
    int win_x = (scr_w - LOGIN_W) / 2;
    int win_y = (scr_h - LOGIN_H) / 2;
    if (win_x < 0) win_x = 0;
    if (win_y < 0) win_y = 0;

    desktop.createWindow(APPTITLE, win_x, win_y, LOGIN_W, LOGIN_H, DT_WIN | DT_NOMOVE);
    ca = desktopContentArea(win_x, win_y, LOGIN_W, LOGIN_H, DT_WIN | DT_NOMOVE);
    exui_init(ca.w, ca.h);

    // load icon after exui_init so the buffer is ready
    g_has_icon = (vad_load(ICON_PATH, &g_icon) == 0);
    printf("debug: icon load %s\n", g_has_icon ? "ok" : "failed");
    if (g_has_icon)
        printf("debug: icon size %dx%d\n", g_icon.width, g_icon.height);

    exui_inputbox_t user_state; memset(&user_state, 0, sizeof(user_state));
    exui_inputbox_t pass_state; memset(&pass_state, 0, sizeof(pass_state));
    pass_state.mask = 1;

    int field = 0;
    int attempts = 0;
    char msg[64]; msg[0] = '\0';

    draw_login(&user_state, &pass_state, msg, field);

    for (;;)
    {
        key_event_t ev;
        if ((int)read(g_kbd, &ev, sizeof(ev)) != (int)sizeof(ev)) continue;
        if (!ev.pressed) continue;

        unsigned int kc = ev.keycode;
        //char c = (char)(kc & 0xFF);

        if (kc == '\t') {
            field = field ? 0 : 1;
            draw_login(&user_state, &pass_state, msg, field);
            continue;
        }

        if (kc == '\n' || kc == '\r') {
            if (field == 0) {
                field = 1;
            } else {
                if (str_eq(user_state.buf, LOGIN_USER) && str_eq(pass_state.buf, LOGIN_PASS)) {
                    desktop.closeWindow();
                    if (g_has_icon) vad_free(&g_icon);
                    return 0;
                }
                if (attempts < MAX_TRIES) {
                    strncpy(log_user[attempts], user_state.buf, BUF_SIZE);
                    strncpy(log_pass[attempts], pass_state.buf, BUF_SIZE);
                }

                attempts++;
                memset(&pass_state, 0, sizeof(pass_state));
                pass_state.mask = 1;
                field = 0;
                if (attempts >= MAX_TRIES) {
                    int fd = open(LOG_PATH, O_WRONLY);
                    if (fd < 0) fd = open(LOG_PATH, O_WRONLY | O_CREAT);
                    if (fd >= 0) {
                        write(fd, "[LOGIN] login attempt failed;\n", 30);
                        for (int i = 0; i < MAX_TRIES; i++) {
                            char buf[256];
                            int len = snprintf(buf, sizeof(buf), "[LOGIN] %d:\n[LOGIN] u: %s\n[LOGIN] p: %s\n", i + 1, log_user[i], log_pass[i]);
                            write(fd, buf, len);
                        }
                        close(fd);
                    }
                    desktop.closeWindow();
                    if (g_has_icon) vad_free(&g_icon);
                    return 2;
                }

                const char *pfx = "wrong ("; const char *sfx = " left)";
                int mi = 0;
                for (int k = 0; pfx[k]; k++) msg[mi++] = pfx[k];
                msg[mi++] = '0' + (MAX_TRIES - attempts);

                for (int k = 0; sfx[k]; k++) msg[mi++] = sfx[k];
                msg[mi] = '\0';
            }
            draw_login(&user_state, &pass_state, msg, field);
            continue;
        }

        if (field == 0)
            exui.InputBox(LOGIN_PAD, font_h(FONT) + 3 + LOGIN_PAD, ca.w - LOGIN_PAD * 2, &user_state, LG_FG, LG_BLACK, LG_FOCUSED, FONT, kc);
        else
            exui.InputBox(LOGIN_PAD, (font_h(FONT) + 3) * 2 + font_h(FONT) + 6 + LOGIN_PAD, ca.w - LOGIN_PAD * 2, &pass_state, LG_FG, LG_BLACK, LG_FOCUSED, FONT, kc);

        draw_login(&user_state, &pass_state, msg, field);
    }
}