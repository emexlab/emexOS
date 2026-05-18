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

//-////////////////////////////////////////-//
//-//                                    //-//
//-//              SHELLY :)             //-//
//-//       emex user-space shell        //-//
//-//                                    //-//
//-////////////////////////////////////////-//

#define BUFFER 256
#define SHELL_PROMPT "[pc@emexos]$ "
#define SHELL_CONFIG "/.config/shelly/"
#define BIN_PATH "/bin/"
#define KBD_PATH "/dev/input/keyboard0"
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
static int g_ptmx = -1;
static char ibuf[BUFFER];
static int ilen = 0;

static void ensure_visible(void) {
  int vrows = exui_height() / font_h(FONT);
  if (vrows <= 0)
    vrows = 1;
  if (t_row >= t_vtop + vrows)
    t_vtop = t_row - vrows + 1;
}

static void next_row(void) {
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
    if (t_vtop > 0)
      t_vtop--;
  }
  t_text[t_row][0] = '\0';
  memset(t_clr[t_row], 0, TBUF_COLS);
  ensure_visible();
}

// append char into text buffer only, no drawing
static void buf_putc(char c, int ci) {
  if (c == '\n') {
    next_row();
    return;
  }
  if (c == '\r')
    return;
  if (c == '\b') {
    if (t_col > 0) {
      t_col--;
      t_text[t_row][t_col] = '\0';
    }
    return;
  }
  int vcols = exui_width() / font_w(FONT);
  if (vcols <= 0 || vcols > TBUF_COLS)
    vcols = TBUF_COLS;
  if (t_col >= vcols)
    next_row();
  if (t_col < TBUF_COLS) {
    t_text[t_row][t_col] = c;
    t_clr[t_row][t_col] = (unsigned char)ci;
    t_col++;
    t_text[t_row][t_col] = '\0';
  }
}

static void buf_puts(const char *s, int ci) {
  while (*s)
    buf_putc(*s++, ci);
}

static void redraw(void) {
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

  // block cursor
  int vis = t_row - t_vtop;
  if (vis >= 0 && vis < vrows)
    exui.DrawCursor(t_col * fw, vis * fh, SH_FG, FONT);

  exui.Flush();
}

static int parse_args(char *buf, char **argv, int maxn) {
  int argc = 0;
  char *p = buf;
  while (*p && argc < maxn - 1) {
    while (*p == ' ')
      p++;
    if (!*p)
      break;
    argv[argc++] = p;

    // find end of token
    while (*p && *p != ' ')
      p++;
    if (*p == ' ')
      *p++ = '\0';
  }

  argv[argc] = NULL;
  return argc;
}

// check if a file exists in the vfs by trying to open it
static int file_exists(const char *path) {
  int fd = open(path, O_RDONLY);
  if (fd < 0)
    return 0;
  close(fd);
  return 1;
}

static void run_cmd(void) {
  if (!ilen)
    return;

  char buf[BUFFER];
  memcpy(buf, ibuf, (unsigned)(ilen + 1));

  char *argv[32];
  int argc = parse_args(buf, argv, 32);
  if (!argc)
    return;

  /*// builtin: cd
  if (!strcmp(argv[0], "cd")) {
      const char *path = argv[1] ? argv[1] : "/";
      if (chdir(path) != 0)
          term_puts("cd: no such directory\n", CI_ERR);
      return;
  }*/

  // builtin: exec /abs/path [args]
  if (!strcmp(argv[0], "exec")) {
    if (!argv[1]) {
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
      redraw();
    }
    return;
  }

  // built-in command Handling or absolute path
  char path[BUFFER];
  if (argv[0][0] == '/' || argv[0][0] == '.') {
    strncpy(path, argv[0], BUFFER);
  } else {
    // build /bin/<cmd>.elf path
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
  }

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
    
    // Drain child output from PTY master
    if (g_ptmx >= 0) {
        char pty_buf[512];
        int n;
        while ((n = read(g_ptmx, pty_buf, sizeof(pty_buf) - 1)) > 0) {
            pty_buf[n] = '\0';
            buf_puts(pty_buf, CI_FG);
        }
    }
    
    redraw();
  } else {
    buf_puts("fork failed\n", CI_ERR);
  }
}

int main(void) {
  // printf(WELCOME_MESSAGE);
  // g_fb = open("/dev/fb0", O_RDWR);
  g_kbd = open(KBD_PATH, O_RDONLY);
  if (g_kbd < 0)
    return 1;
    
  g_ptmx = open("/dev/ptmx", O_RDONLY);

  int scr_w = 1280, scr_h = 720;
  int win_x = (scr_w - SHELL_W) / 2;
  int win_y = (scr_h - SHELL_H) / 2;
  if (win_x < 0)
    win_x = 0;
  if (win_y < 0)
    win_y = 0;

  desktop.createWindow("shelly", win_x, win_y, SHELL_W, SHELL_H, DT_WIN);
  DesktopArea ca = desktopContentArea(win_x, win_y, SHELL_W, SHELL_H, DT_WIN);
  exui_init(ca.w, ca.h);

  for (int i = 0; i < TBUF_ROWS; i++) {
    t_text[i][0] = '\0';
    memset(t_clr[i], 0, TBUF_COLS);
  }

  buf_puts("shelly :3\n", CI_INFO);
  buf_puts("type \"ls /bin\" for available commands.\n\n", CI_FG);
  buf_puts("[pc@emexos]$ ", CI_PROMPT);
  redraw();

  for (;;) {
    key_event_t ev;
    if ((int)read(g_kbd, &ev, sizeof(ev)) != (int)sizeof(ev))
      continue;
    if (!ev.pressed)
      continue;

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
}