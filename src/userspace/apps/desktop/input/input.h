#pragma once

#include <sys/types.h>
#include <emx/mouse.h>
#include "../win/win.h"
#include "../cursor/cursor.h"

typedef struct
{
    int valid;
    pid_t pid;
    int wx, wy, ww, wh;
} drag_info_t;

// resize edge flags
#define RESIZE_NONE   0
#define RESIZE_LEFT   (1 << 0)
#define RESIZE_RIGHT  (1 << 1)
#define RESIZE_TOP    (1 << 2)
#define RESIZE_BOTTOM (1 << 3)

#define RESIZE_ZONE 5 // i dont really know....


#define DT_NORESIZE 0x08

typedef struct
{
    int cx, cy;
    int win_changed;

    int sel_active;
    int sel_x0,  sel_y0;    // start point
    int sel_x1,  sel_y1;    // current end point
    int sel_px1, sel_py1;   // previous end point
    int _sel_was_active;    // tracks previous frame state for release clear
} input_state_t;

void input_frame_begin(input_state_t *is);

void input_init(void);
int input_drain(int mfd, input_state_t *is);
int win_get_resize_edge(int idx, int mx, int my);

