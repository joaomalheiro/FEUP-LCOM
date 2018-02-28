#include <minix/syslib.h>
#include <minix/drivers.h>
#include <stdint.h>
#include <machine/int86.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <stdio.h>
#include <math.h>
#include "video_gr.h"
#include "vbe.h"
#include "video_utils.h"
#include "bitmap.h"

/* Private global variables */

static unsigned short *back_buffer; /* Secondary buffer to use for double buffering */
static unsigned short *video_mem;		/* Process (virtual) address to which VRAM is mapped */
static unsigned h_res;		/* Horizontal screen resolution in pixels */
static unsigned v_res;		/* Vertical screen resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */

/* VBE call macros */
#define VBE_CALL_SUPPORTED 0x4F
#define VBE_CALL_SUCCESSFUL 0x00
#define VBE_CALL_FAILED 0x01
#define VBE_CALL_NOT_SUPPORTED_CURRENT_HW 0x02
#define VBE_INVALID_IN_VIDEO_MODE 0x03

void *vg_init(unsigned short mode){

  struct reg86u r;
  r.u.w.ax = VBE_SET_MODE; // VBE call, function 02 -- set VBE mode
  r.u.w.bx = (1<<14) | mode; // set bit 14: linear framebuffer
  r.u.b.intno = VIDEO_INT;

  if(sys_int86(&r) != OK) {
    printf("vg_init::Error in sys_int86() call\n");
    return NULL;
  }

  if(r.u.b.al != VBE_CALL_SUPPORTED){
    printf("vg_init::VBE call not supported!\n");
    return NULL;
  }

  if(r.u.b.ah != VBE_CALL_SUCCESSFUL){
    if(r.u.b.ah == VBE_CALL_FAILED){
      printf("vg_init::VBE call failed\n");
      return NULL;
    } else if(r.u.b.ah == VBE_CALL_NOT_SUPPORTED_CURRENT_HW){
      printf("vg_init::VBE call not supported in current HW configuration\n");
      return NULL;
    } else if(r.u.b.ah == VBE_INVALID_IN_VIDEO_MODE){
      printf("vg_init::VBE call invalid in current video mode\n");
      return NULL;
    }
  }

  vbe_mode_info_t mode_info;

  vbe_get_mode_info(mode, &mode_info);

  h_res = mode_info.XResolution;
	v_res = mode_info.YResolution;
  bits_per_pixel = mode_info.BitsPerPixel;

  // VRAM size, basically number of pixels * the bytes per pixel
  unsigned int vram_size = v_res * h_res * (bits_per_pixel / 8);

  /* Allocating memory to map to */
  struct mem_range mr;
  mr.mr_base = mode_info.PhysBasePtr;
  mr.mr_limit = mr.mr_base + vram_size;
  int r1;
  if((r1 = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)) != OK)
    panic("sys_privctl (ADD_MEM) failed: %d\n", r1);

  /* Memory mapping */

  video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);

  if(video_mem == MAP_FAILED)
    panic("couldn't map video memory");

  //Finally, allocating the secondary buffer to use for double buffering
  back_buffer = malloc(vram_size); //Using already calculated vram_size variable

  if(back_buffer == NULL) {
    panic("Secondary buffer could not be allocated!!");
  }

  return video_mem;
}

int vg_draw_bitmap(char * bmplocation, int x, int y){

  //Create a bitmap
  Bitmap * mybmp = loadBitmap(bmplocation);

  if(mybmp == NULL){
    return -1;
  }

  //Draw a bitmap
  drawBitmap(mybmp, x, y);

  //Delete a bitmap
  deleteBitmap(mybmp);

  //Return 0 on success
  return 0;
}

int getVerResolution() {
  return v_res;
}

int getHorResolution() {
  return h_res;
}

unsigned short * getGraphicsBuffer() {
  return video_mem;
}

unsigned short * getBackBuffer() {
  return back_buffer;
}

unsigned getBitsPerPixel() {
  return bits_per_pixel;
}

unsigned int getVramSize() {
  return v_res * h_res * (bits_per_pixel / 8);
}

void swap_buffers() {
  memcpy(video_mem, back_buffer, h_res*v_res*(bits_per_pixel/8));
}

unsigned short * snapshot_video_mem() {
  unsigned short * snapshot = malloc(h_res*v_res*(bits_per_pixel/8));

  if(snapshot == NULL) {
    return NULL;
  }

  memcpy(snapshot, video_mem, h_res*v_res*(bits_per_pixel/8));
  return snapshot;
}

void redraw_buffer(unsigned short * buffer) {
  if(buffer == NULL) {
    return;
  }

  memcpy(back_buffer, buffer, h_res*v_res*(bits_per_pixel/8));
}

void make_bw(unsigned short * buffer, unsigned int n_bytes) {
  if(buffer == NULL || n_bytes <= 0) {
    return;
  }

  ////To convert an image to black and white we just need to make each pixel's colors the average of all the components

  //n_bytes / bytes_per_pixel = n_pixels
  int n_pixels = n_bytes/(bits_per_pixel/8);
  int i;
  for(i = 0; i < n_pixels; i++) {
    //Getting each color component
    int r;
    int g;
    int b;
    pixel_to_rgb(buffer[i], &r, &g, &b);
    //Calculating the average
    int avg = (r + g + b)/3;
    //Changing the pixel to be the average of the 3 colors
    //(Because green has one more bit, we have to convert its color to the correct range otherwise we would get a purple tint)
    //It is indifferent if the use MAX_RED or MAX_BLUE, since only green has 1 more bit of color
    buffer[i] = rgb_to_pixel(avg, avg * (double)(MAX_GREEN / MAX_RED), avg);
  }
}

void change_gamma(unsigned short * buffer, unsigned int n_bytes, double gamma_factor) {
  if(buffer == NULL || n_bytes <= 0) {
    return;
  }

  //n_bytes / bytes_per_pixel = n_pixels
  int n_pixels = n_bytes/(bits_per_pixel/8);
  int i;
  for(i = 0; i < n_pixels; i++) {
    //Getting each color component
    int r;
    int g;
    int b;
    pixel_to_rgb(buffer[i], &r, &g, &b);

    //Changing the colors
    //(Because green has one more bit, we have to convert its color to the correct range otherwise we would get a weird tint)
    buffer[i] = rgb_to_pixel(r * gamma_factor, g * gamma_factor, b * gamma_factor);
  }
}

int vg_exit() {
  struct reg86u reg86;

  reg86.u.b.intno = 0x10; /* BIOS video services */
  reg86.u.b.ah = 0x00;    /* Set Video Mode function */
  reg86.u.b.al = 0x03;    /* 80x25 text mode*/

  if(sys_int86(&reg86) != OK) {
    printf("\tvg_exit(): sys_int86() failed \n");
    return 1;
  } else {
    return 0;
  }

  //Deallocating the secondary buffer
  free(back_buffer);
}

void vg_fill_screen(short color) {
  unsigned short * video_buf = getBackBuffer();

  //Creating a line of all pixels set in the given color
  unsigned short * colored_line = (unsigned short *) malloc(getHorResolution() * sizeof(unsigned short));
  int currcol;
  for(currcol = 0; currcol < getHorResolution(); currcol++){
    colored_line[currcol] = color;
  }

  int currline;
  //Filling each line with the given color
  for(currline = 0; currline < getVerResolution(); currline++){
    //Copying a line
    memcpy(video_buf, colored_line, getHorResolution() * sizeof(unsigned short));
    //Advancing a line with the base memory address
    video_buf += getHorResolution();
  }

  free(colored_line);
}
