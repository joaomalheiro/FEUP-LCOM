#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <minix/drivers.h>
//User includes
#include "test5.h"
#include "pixmap.h"

static int proc_args(int argc, char **argv);
static unsigned long parse_ulong(char *str, int base);
static long parse_long(char *str, int base);
static void print_usage(char **argv);

int main(int argc, char **argv) {
/* initializing service */
sef_startup();

if (argc == 1) {					/* Prints usage of the program if no arguments are passed */
  print_usage(argv);
  return 0;
  }  else
  return proc_args(argc, argv);
}

static void print_usage(char **argv) {
printf("Usage: one of the following:\n"
    "\t service run %s -args \"init <hexadecimal no. - VBE mode to set> <decimal no. - seconds after which returns to text mode>\"\n"
    "\t service run %s -args \"square <decimal no. - x center coordinate> <decimal no. - y center coordinate> <decimal no. - size of each side in pixels> <decimal no. - color to set the pixel\"\n"
    "\t service run %s -args \"line <decimal no. - xi coordinate of first endpoint> <decimal no. - yi coordinate of first endpoint> <decimal no. - xf coordinate of last endpoint> <decimal no. - yf coordinate of last endpoint> <decimal no. - color of the line segment to draw>\"\n"
    "\t service run %s -args \"xpm <string - name of the xpm> <decimal no. - xi coordinate of first endpoint> <decimal no. - yi coordinate of first endpoint>\"\n"
    "\t service run %s -args \"move <string no. - pointer to the image> <decimal no. - xi coordinate of first endpoint> <decimal no. - yi coordinate of first endpoint> <decimal no. - xf coordinate of last endpoint> <decimal no. - yf coordinate of last endpoint> <decimal no. - speed> <decimal no. - frame_rate(fps)>\"\n"
    "\t service run %s -args \"controller\"\n"
    ,argv[0], argv[0], argv[0], argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char **argv) {
if (strncmp(argv[1], "init", strlen("init")) == 0) {
  if (argc != 4) {
    printf("video: wrong no. of arguments for video_test_init()\n");
    return 1;
  }
  unsigned long vbemode = parse_ulong(argv[2], 16);						/* Parses string to unsigned long */
  unsigned long nsecs = parse_ulong(argv[3], 10);
  if ((vbemode == ULONG_MAX) || (nsecs == ULONG_MAX))
    return 1;
  printf("video::video_test_init(%x, %lu)\n", vbemode, nsecs);
  return video_test_init(vbemode, nsecs);
}
else if (strncmp(argv[1], "square", strlen("square")) == 0) {
  if (argc != 6) {
    printf("video: wrong no. of arguments for video_test_square()\n");
    return 1;
  }
  unsigned long xcord = parse_ulong(argv[2], 10);						/* Parses string to unsigned long */
  unsigned long ycord = parse_ulong(argv[3], 10);
  unsigned long pixel_size = parse_ulong (argv[4],10);
  unsigned long color = parse_ulong (argv[5],10);
  if ((xcord == ULONG_MAX) || (ycord == ULONG_MAX) || (pixel_size == ULONG_MAX) || (color == ULONG_MAX) )
    return 1;
  printf("video::video_test_square(%lu, %lu, %lu, %lu)\n", xcord, ycord, pixel_size, color);
  return video_test_square(xcord,ycord,pixel_size,color);
}
else if (strncmp(argv[1], "line", strlen("line")) == 0) {
  if (argc != 7) {
    printf("video: wrong no. of arguments for video_test_line()\n");
    return 1;
  }
  unsigned long xfirst = parse_ulong(argv[2], 10);						/* Parses string to unsigned long */
  unsigned long yfirst = parse_ulong(argv[3], 10);
  unsigned long xlast = parse_ulong(argv[4], 10);
  unsigned long ylast = parse_ulong(argv[5], 10);
  unsigned long color = parse_ulong(argv[6], 10);
if ((xfirst == ULONG_MAX) || (yfirst == ULONG_MAX) ||(xlast == ULONG_MAX)||(ylast == ULONG_MAX)||(color == ULONG_MAX))
    return 1;
  printf("video::video_test_line(%lu, %lu, %lu, %lu, %lu)\n", xfirst, yfirst, xlast , ylast , color);
  return video_test_line(xfirst, yfirst, xlast , ylast , color);
} else if (strncmp(argv[1],"xpm", strlen("xpm")) == 0) {
  if (argc != 5) {
    printf("video: wrong no. of arguments for video_test_xpm()\n");
    return 1;
  }
  unsigned long xi = parse_ulong(argv[3], 10);						/* Parses string to unsigned long */
  unsigned long yi = parse_ulong(argv[4], 10);

  if ((xi == ULONG_MAX) || (yi == ULONG_MAX))
      return 1;

  if (strncmp(argv[2], "pic1", strlen("pic1")) == 0) {
    printf("video::video_test_xpm(pic1, %lu, %lu)\n", xi, yi);
    return video_test_xpm(pic1, xi,yi);
  } else if (strncmp(argv[2], "pic2", strlen("pic2")) == 0) {
    printf("video::video_test_xpm(pic2, %lu, %lu)\n", xi, yi);
    return video_test_xpm(pic2, xi, yi);
  } else if (strncmp(argv[2], "cross", strlen("cross")) == 0) {
      printf("video::video_test_xpm(cross, %lu, %lu)\n", xi, yi);
      return video_test_xpm(cross, xi,yi);
  } else if (strncmp(argv[2], "pic3", strlen("pic3")) == 0) {
      printf("video::video_test_xpm(pic3, %lu, %lu)\n", xi, yi);
      return video_test_xpm(pic3, xi,yi);
  } else if (strncmp(argv[2], "penguin", strlen("penguin")) == 0) {
      printf("video::video_test_xpm(penguin, %lu, %lu)\n", xi, yi);
      return video_test_xpm(penguin, xi,yi);
  } else {
      printf("video::video_test_xpm - can't regognize xpm image\n");
      return 1;
  }

} else if (strncmp(argv[1],"move", strlen("move")) == 0) {
  if (argc != 9) {
    printf("video: wrong no. of arguments for video_test_move()\n");
    return 1;
  }
  unsigned long xi = parse_ulong(argv[3], 10);						/* Parses string to unsigned long */
  unsigned long yi = parse_ulong(argv[4], 10);
  unsigned long xf = parse_ulong(argv[5],10);
  unsigned long yf = parse_ulong(argv[6],10);
  long speed = parse_long(argv[7],10);
  unsigned long frame_rate = parse_ulong(argv[8],10);

if ((xi == ULONG_MAX) || (yi == ULONG_MAX) || (xf == ULONG_MAX) || (yf == ULONG_MAX) || (speed == LONG_MAX) || (frame_rate == ULONG_MAX))
    return 1;

if (strncmp(argv[2], "pic1", strlen("pic1")) == 0) {
  printf("video::video_test_move(pic1,%lu,%lu,%lu,%lu,%ld,%lu)\n", xi, yi, xf, yf, speed, frame_rate);
  return video_test_move(pic1, xi,yi, xf, yf, speed, frame_rate);
}
if (strncmp(argv[2], "pic2", strlen("pic2")) == 0) {
    printf("video::video_test_move(pic2,%lu,%lu,%lu,%lu,%ld,%lu)\n", xi, yi, xf, yf, speed, frame_rate);
  return video_test_move(pic2, xi,yi, xf, yf, speed, frame_rate);
}
if (strncmp(argv[2], "cross", strlen("cross")) == 0) {
    printf("video::video_test_move(cross,%lu,%lu,%lu,%lu,%ld,%lu)\n", xi, yi, xf, yf, speed, frame_rate);
    return video_test_move(cross, xi,yi, xf, yf, speed, frame_rate);
}
if (strncmp(argv[2], "pic3", strlen("pic3")) == 0) {
    printf("video::video_test_move(pic3,%lu,%lu,%lu,%lu,%ld,%lu)\n", xi, yi, xf, yf, speed, frame_rate);
    return video_test_move(pic3, xi,yi, xf, yf, speed, frame_rate);
}
if (strncmp(argv[2], "penguin", strlen("penguin")) == 0) {
    printf("video::video_test_move(penguin,%lu,%lu,%lu,%lu,%ld,%lu)\n", xi, yi, xf, yf, speed, frame_rate);
    return video_test_move(penguin, xi,yi, xf, yf, speed, frame_rate);
}
else {
    printf("video::video_test_move -> no xpm matching given arguments found\n");
    return 1;
  }
}
else if (strncmp(argv[1], "controller", strlen("controller")) == 0) {
  if (argc != 2) {
    printf("video: wrong no. of arguments for test_controller()\n");
    return 1;
  }
  printf("video::test_controller()\n");
  return test_controller();
}
else {
  printf("video: %s - no valid function!\n", argv[1]);
  return 1;
	}
}


static long parse_long(char *str, int base) {
char *endptr;
long val;

/* Convert string to long */
val = strtol(str, &endptr, base);

/* Check for conversion errors */
if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
  perror("strtol");
  return LONG_MAX;
}

if (endptr == str) {
  printf("parse_long: no digits were found in %s\n", str);
  return LONG_MAX;
}

	/* Successful conversion */
	return val;
}

static unsigned long parse_ulong(char *str, int base) {
char *endptr;
unsigned long val;

/* Convert string to unsigned long */
val = strtoul(str, &endptr, base);

/* Check for conversion errors */
if ((errno == ERANGE && val == ULONG_MAX) || (errno != 0 && val == 0)) {
  perror("strtoul");
  return ULONG_MAX;
}

if (endptr == str) {
  printf("parse_ulong: no digits were found in %s\n", str);
  return ULONG_MAX;
}

/* Successful conversion */
return val;
}
