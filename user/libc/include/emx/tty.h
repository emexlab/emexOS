#pragma once

/* do:
 *   int fd = open("/dev/vt/control", O_RDWR);
 *   uint64_t vt_id;
 *
 *   ioctl(fd, VT_CREATE, &vt_id);
 * or
 *   ioctl(fd, VT_DESTROY, &vt_id);
 * or
 *   ioctl(fd, VT_FOCUS, &vt_id);
 * or
 *   ioctl(fd, VT_GETFOC, &vt_id);
 *   close(fd);
 *
 * and connect with
 *   /dev/vt/<id>/input
 *   /dev/vt/<id>/output
 * should be self explaining ig
 */

#define VT_CREATE  0x5600
#define VT_DESTROY 0x5601
#define VT_FOCUS   0x5602
#define VT_GETFOC  0x5603


// im not happy with it.... but for now ig it works right? :c
int vt_setproc(unsigned long long vt_id);