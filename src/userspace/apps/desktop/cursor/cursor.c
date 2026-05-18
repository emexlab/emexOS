#include "cursor.h"
#include "../compositor/comp.h"

#include <emx/fb.h>
#include <sys/ioctl.h>

// all sprite data comes from data.h now

static cur_type_t g_cur_type = CUR_TYPE_NORMAL;

// returns active sprite
static const unsigned int *cur_px_active(void) {
  switch (g_cur_type) {
  case CUR_TYPE_HRESIZE:
    return cur_hresize_px;
  case CUR_TYPE_VRESIZE:
    return cur_vresize_px;
  case CUR_TYPE_DRESIZE_NWSE:
    return cur_dresize_nwse_px;
  case CUR_TYPE_DRESIZE_NESW:
    return cur_dresize_nesw_px;
  default:
    return cur_normal_px;
  }
}

int cur_w(void) {
  if (g_cur_type == CUR_TYPE_NORMAL)
    return CUR_NORMAL_W;
  return CUR_RESIZE_W;
}

int cur_h(void) {
  if (g_cur_type == CUR_TYPE_NORMAL)
    return CUR_NORMAL_H;
  return CUR_RESIZE_H;
}

void cur_set_type(cur_type_t type) { g_cur_type = type; }

// bg save buffer big enough for largest cursor
static unsigned int bg_save[CUR_RESIZE_W * CUR_RESIZE_H];
static int bg_valid = 0;
static int old_x = 0;
static int old_y = 0;
static int g_fd = -1;
static int g_scr_w = 0;
static int g_scr_h = 0;

static void clamp(int *x, int *y) {
  int cw = cur_w();
  int ch = cur_h();
  if (*x < 0)
    *x = 0;
  if (*y < 0)
    *y = 0;
  if (*x + cw > g_scr_w)
    *x = g_scr_w - cw;
  if (*y + ch > g_scr_h)
    *y = g_scr_h - ch;
}

void cur_init(int fb_fd, int w, int h) {
  g_fd = fb_fd;
  g_scr_w = w;
  g_scr_h = h;
  bg_valid = 0;
}

void cur_undo_from_backbuf(void) {
  if (!bg_valid)
    return;
  comp_put_pixels(old_x, old_y, cur_w(), cur_h(), bg_save);
}

void cur_bake(int x, int y) {
  int cw = cur_w();
  int ch = cur_h();
  const unsigned int *px = cur_px_active();

  clamp(&x, &y);

  for (int row = 0; row < ch; row++) {
    for (int col = 0; col < cw; col++) {
      bg_save[row * cw + col] = comp_get(x + col, y + row);
    }
  }
  for (int row = 0; row < ch; row++) {
    for (int col = 0; col < cw; col++) {
      unsigned int c = px[row * cw + col];
      if (c >> 24)
        comp_set(x + col, y + row, c);
    }
  }

  old_x = x;
  old_y = y;
  bg_valid = 1;
}

void cur_erase_fb(void) {
  if (!bg_valid || g_fd < 0)
    return;
  fb_rect_t r = {.x = (unsigned)old_x,
                 .y = (unsigned)old_y,
                 .w = (unsigned)cur_w(),
                 .h = (unsigned)cur_h(),
                 .pixels = bg_save};
  ioctl(g_fd, FBIO_BLIT, &r);
  bg_valid = 0;
}

void cur_draw_fb(int x, int y) {
  if (g_fd < 0)
    return;

  int cw = cur_w();
  int ch = cur_h();
  const unsigned int *px = cur_px_active();

  clamp(&x, &y);

  // save FB pixels under the cursor
  fb_rect_t save = {.x = (unsigned)x,
                    .y = (unsigned)y,
                    .w = (unsigned)cw,
                    .h = (unsigned)ch,
                    .pixels = bg_save};
  ioctl(g_fd, FBIO_READ_RECT, &save);

  // draws the cursor to the FB
  fb_rect_t draw = {.x = (unsigned)x,
                    .y = (unsigned)y,
                    .w = (unsigned)cw,
                    .h = (unsigned)ch,
                    .pixels = (unsigned int *)px};

  ioctl(g_fd, FBIO_BLIT, &draw);

  old_x = x;
  old_y = y;
  bg_valid = 1;
}

int cur_valid(void) { return bg_valid; }