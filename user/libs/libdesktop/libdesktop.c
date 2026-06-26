/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: libdesktop.c
 * CREATED BY: emex
 * MODIFIED BY: --
 *
 */

#include "libdesktop.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define DT_CMD         "/tmp/dt/cmd"
#define DT_DIRTY       "/tmp/dt/dirty"
#define DT_CURSOR      "/tmp/dt/cursor"
#define DT_WBUF_PFX    "/tmp/dt/wbuf_"
#define DT_INPUT_PFX   "/tmp/dt/input_"

static void _itoa(int v, char *out)
{
    char tmp[16]; int i=0, j=0, neg=(v<0);
    if (v==0) { out[0]='0'; out[1]='\0'; return; }
    if (neg) v=-v;
    while (v) { tmp[i++]='0'+v%10; v/=10; }
    if (neg) tmp[i++]='-';
    while (i>0) out[j++]=tmp[--i];
    out[j]='\0';
}

static int _slen(const char *s) { int n=0; while(s[n]) n++; return n; }

static void _pid_path(const char *pfx, char *out)
{
    int i = 0;
    int j = 0;

    while (*pfx) out[i++]=*pfx++;
    char ps[12]; _itoa((int)getpid(), ps);
    while (ps[j]) out[i++]=ps[j++];
    out[i]='\0';
}

static void _app_int(char *buf, int *pos, int v)
{
    char tmp[12]; int i=0; _itoa(v,tmp);
    while (tmp[i]) buf[(*pos)++]=tmp[i++];
}

static void _app_str(char *buf, int *pos, const char *s)
{
    while (*s) buf[(*pos)++]=*s++;
}

// handles structural commands (OCTM)
static void _cmd_append(const char *line)
{
    static char existing[4096];
    int fd=open(DT_CMD, O_RDONLY), elen=0;
    if (fd>=0) {
        int r=(int)read(fd, existing, sizeof(existing)-1);
        close(fd);
        if (r>0) { existing[r]='\0'; elen=r; }
    }
    fd=open(DT_CMD, O_WRONLY|O_CREAT);
    if (fd<0) return;
    if (elen>0) write(fd, existing, (unsigned)elen);
    write(fd, line, (unsigned)_slen(line));
    close(fd);
}

static int _createWindow(const char *title, int x, int y, int w, int h, unsigned int style)
{
    char buf[256]; int p=0;
    buf[p++]='O'; buf[p++]=' ';
    _app_int(buf,&p,(int)getpid()); buf[p++]=' ';
    _app_int(buf,&p,(int)style);    buf[p++]=' ';
    _app_int(buf,&p,x); buf[p++]=' ';
    _app_int(buf,&p,y); buf[p++]=' ';
    _app_int(buf,&p,w); buf[p++]=' ';
    _app_int(buf,&p,h); buf[p++]=' ';
    _app_str(buf,&p,title);
    buf[p++]='\n'; buf[p]='\0';
    _cmd_append(buf);
    return 0;
}

static int _createPopup(int x, int y, int w, int h)
{
    return _createWindow("", x, y, w, h, DT_POPUP);
}

static void _closeWindow(void)
{
    char buf[32]; int p=0;
    buf[p++]='C'; buf[p++]=' ';
    _app_int(buf,&p,(int)getpid());
    buf[p++]='\n'; buf[p]='\0';
    _cmd_append(buf);
}

static void _setTitle(const char *title)
{
    char buf[256]; int p=0;
    buf[p++]='T'; buf[p++]=' ';
    _app_int(buf,&p,(int)getpid()); buf[p++]=' ';
    _app_str(buf,&p,title);
    buf[p++]='\n'; buf[p]='\0';
    _cmd_append(buf);
}

static void _markDirty(void)
{
    int fd=open(DT_DIRTY, O_WRONLY|O_CREAT);
    if (fd>=0) { write(fd,"1\n",2); close(fd); }
}

static void _winbuf_write(const unsigned int *pixels, int w, int h)
{
    if (!pixels||w<=0||h<=0) return;
    char path[64]; _pid_path(DT_WBUF_PFX, path);
    int fd=open(path, O_WRONLY|O_CREAT);
    if (fd<0) return;
    int hdr[2]; hdr[0]=w; hdr[1]=h;
    write(fd, hdr, 8);
    write(fd, pixels, (unsigned)(w*h*4));
    close(fd);
    _markDirty();
}

static int _pollEvents(dt_event_t *buf, int max)
{
    if (!buf||max<=0) return 0;
    char path[64]; _pid_path(DT_INPUT_PFX, path);
    int fd=open(path, O_RDONLY);
    if (fd<0) return 0;

    int count=0;
    dt_event_t ev;
    while (count<max && (int)read(fd,&ev,sizeof(ev))==(int)sizeof(ev)) buf[count++]=ev;
    close(fd);

    if (count>0) {
        fd=open(path, O_WRONLY|O_CREAT);
        if (fd>=0) close(fd);
    }
    return count;
}

static void _getCurrentMousePos(int *out_x, int *out_y)
{
    if (!out_x||!out_y) return;
    *out_x=0; *out_y=0;

    int fd=open(DT_CURSOR, O_RDONLY);
    if (fd<0) return;
    char buf[32];
    int n=(int)read(fd, buf, sizeof(buf)-1);
    close(fd);
    if (n<=0) return;
    buf[n]='\0';

    const char *p=buf;
    int x=0, y=0;
    while (*p>='0'&&*p<='9') x=x*10+(*p++-'0');
    while (*p==' ') p++;
    while (*p>='0'&&*p<='9') y=y*10+(*p++-'0');
    *out_x=x; *out_y=y;
}

// for "desktop."
Desktop desktop = {
    .createWindow       = _createWindow,
    .createPopup        = _createPopup,
    .closeWindow        = _closeWindow,
    .setTitle           = _setTitle,
    .markDirty          = _markDirty,
    .winbuf_write       = _winbuf_write,
    .pollEvents         = _pollEvents,
    .getCurrentMousePos = _getCurrentMousePos,
};