#include "exui.h"
#include "libdesktop.h"
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <GL/gl.h>
#include <zbuffer.h>

static GLfloat view_rotx = 20.0, view_roty = 30.0;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.0;

static void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
                 GLint teeth, GLfloat tooth_depth) {
  GLint i;
  GLfloat r0, r1, r2;
  GLfloat angle, da;
  GLfloat u, v, len;

  r0 = inner_radius;
  r1 = outer_radius - tooth_depth / 2.0;
  r2 = outer_radius + tooth_depth / 2.0;
  da = 2.0 * M_PI / teeth / 4.0;

  glNormal3f(0.0, 0.0, 1.0);
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;
    glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
  }
  glEnd();

  glBegin(GL_QUADS);
  for (i = 0; i < teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;
    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
  }
  glEnd();

  glNormal3f(0.0, 0.0, -1.0);
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),
               -width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
  }
  glEnd();

  glBegin(GL_QUADS);
  for (i = 0; i < teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),
               -width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),
               -width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
  }
  glEnd();

  glBegin(GL_QUAD_STRIP);
  for (i = 0; i < teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;
    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
    u = r2 * cos(angle + da) - r1 * cos(angle);
    v = r2 * sin(angle + da) - r1 * sin(angle);
    len = sqrt(u * u + v * v);
    u /= len;
    v /= len;
    glNormal3f(v, -u, 0.0);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
    glNormal3f(cos(angle), sin(angle), 0.0);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),
               -width * 0.5);
    u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
    v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
    glNormal3f(v, -u, 0.0);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),
               -width * 0.5);
    glNormal3f(cos(angle), sin(angle), 0.0);
  }
  glVertex3f(r1 * cos(0), r1 * sin(0), width * 0.5);
  glVertex3f(r1 * cos(0), r1 * sin(0), -width * 0.5);
  glEnd();

  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;
    glNormal3f(-cos(angle), -sin(angle), 0.0);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
  }
  glEnd();
}

void draw_scene() {
  angle += 2.0;
  glPushMatrix();
  glRotatef(view_rotx, 1.0, 0.0, 0.0);
  glRotatef(view_roty, 0.0, 1.0, 0.0);

  glPushMatrix();
  glTranslatef(-3.0, -2.0, 0.0);
  glRotatef(angle, 0.0, 0.0, 1.0);
  glCallList(gear1);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(3.1, -2.0, 0.0);
  glRotatef(-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
  glCallList(gear2);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-3.1, 4.2, 0.0);
  glRotatef(-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
  glCallList(gear3);
  glPopMatrix();

  glPopMatrix();
}

void init_scene() {
  static GLfloat pos[4] = {5, 5, 10, 0.0};
  static GLfloat red[4] = {1.0, 0.0, 0.0, 0.0};
  static GLfloat green[4] = {0.0, 1.0, 0.0, 0.0};
  static GLfloat blue[4] = {0.0, 0.0, 1.0, 0.0};
  static GLfloat white[4] = {1.0, 1.0, 1.0, 0.0};
  static GLfloat shininess = 5;

  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
  glLightfv(GL_LIGHT0, GL_SPECULAR, white);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);

  gear1 = glGenLists(1);
  glNewList(gear1, GL_COMPILE);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
  glMaterialfv(GL_FRONT, GL_SPECULAR, white);
  glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
  glColor3fv(blue);
  gear(1.0, 4.0, 1.0, 20, 0.7);
  glEndList();

  gear2 = glGenLists(1);
  glNewList(gear2, GL_COMPILE);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
  glMaterialfv(GL_FRONT, GL_SPECULAR, white);
  glColor3fv(red);
  gear(0.5, 2.0, 2.0, 10, 0.7);
  glEndList();

  gear3 = glGenLists(1);
  glNewList(gear3, GL_COMPILE);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
  glMaterialfv(GL_FRONT, GL_SPECULAR, white);
  glColor3fv(green);
  gear(1.3, 2.0, 0.5, 10, 0.7);
  glEndList();
}

int main() {
  int win_w = 450, win_h = 350;

  // Center window
  int scr_w = 1280, scr_h = 800; // Expected resolution
  int win_x = (scr_w - win_w) / 2;
  int win_y = (scr_h - win_h) / 2;

  if (desktop.createWindow("TinyGL Gears", win_x, win_y, win_w, win_h, DT_WIN) <
      0) {
    printf("Failed to create window\n");
    return 1;
  }

  DesktopArea ca = desktopContentArea(win_x, win_y, win_w, win_h, DT_WIN);
  exui_init(ca.w, ca.h);

  int w = ca.w;
  int h = ca.h;

  ZBuffer *frameBuffer = ZB_open(w, h, ZB_MODE_RGBA, 0);
  if (!frameBuffer) {
    printf("ZB_open failed\n");
    return 1;
  }

  glInit(frameBuffer);

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glViewport(0, 0, w, h);
  glShadeModel(GL_SMOOTH);

  GLfloat aspect = (GLfloat)h / (GLfloat)w;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-1.0, 1.0, -aspect, aspect, 5.0, 60.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -45.0);

  init_scene();
  glSetEnableSpecular(GL_TRUE);

  unsigned int *pixel_buffer = malloc(w * h * 4);
  if (!pixel_buffer) {
    printf("Failed to allocate pixel buffer\n");
    return 1;
  }

  while (1) {
    // Poll events
    dt_event_t ev;
    /*
    while (desktop.pollEvents(&ev, 1) > 0) {
        if (ev.type == DT_EV_KEY && ev.keycode == 27) { // ESC
            goto cleanup;
        }
    }
    */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_scene();

    ZB_copyFrameBuffer(frameBuffer, pixel_buffer, w * 4);

    // Force alpha to 0xFF
    for (int i = 0; i < w * h; i++) {
      pixel_buffer[i] |= 0xFF000000;
    }

    exui.BlitPixels(0, 0, w, h, pixel_buffer, w, h);
    exui.Flush();

    // Target ~60fps to reduce CPU/IO load
    // emexOS might not have usleep, but I saw it in shell.c
    // Wait, I'll use a simple loop if usleep is missing
    for (volatile int i = 0; i < 100000; i++)
      ;
  }

cleanup:
  ZB_close(frameBuffer);
  desktop.closeWindow();
  return 0;
}
