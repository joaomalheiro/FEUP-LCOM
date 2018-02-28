#include <minix/syslib.h>
#include <minix/drivers.h>
#include <stdint.h>
#include <machine/int86.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <stdio.h>
#include <math.h>
#include "vbe.h"
#include "video_utils.h"
#include "read_xpm.h"
#include "video_test.h"

/* Constants for VBE 0x105 mode */

/* The physical address may vary from VM to VM.
 * At one time it was 0xD0000000
 *  #define VRAM_PHYS_ADDR    0xD0000000
 * Currently on lab B107 is 0xF0000000
 * Better run my version of lab5 as follows:
 *     service run `pwd`/lab5 -args "mode 0x105"
 */
#define VRAM_PHYS_ADDR	0xE0000000
#define H_RES     1024
#define V_RES		  768
#define BITS_PER_PIXEL	  8

/* Private global variables */

static char *video_mem;		/* Process (virtual) address to which VRAM is mapped */
static unsigned h_res;		/* Horizontal screen resolution in pixels */
static unsigned v_res;		/* Vertical screen resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */

/* Public global variables */
unsigned HRES;
unsigned VRES;

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
  //Also setting the 'public' global variables (so that read_xpm has access to the resolution, for example)
  HRES = h_res;
  VRES = v_res;

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

  return video_mem;
}

void vg_fill_pixel(long x, long y, unsigned long color){

  //printf("fill_pixel : x:%lu\ty:%lu\tcolor:%lu\n", x, y, color);

  //Checking if the coordinates are within the resolution limits
  if (x < h_res && x >= 0 && y < v_res && y>= 0){
    unsigned long pixelposition = (x + h_res * y) * (bits_per_pixel/8);

    //Using palette mode
    if(bits_per_pixel == 8){
        video_mem[pixelposition] = (unsigned char) color;
    } else {
        //Using rgb mode
        unsigned long * base_video_mem = (unsigned long *) video_mem;
        base_video_mem[pixelposition] = color;
    }
  }
}

void vg_draw_rect(long xi, long yi, long xf, long yf, unsigned long color){

  printf("draw_rect: xi: %ld yi: %ld xf: %ld yf: %ld\n", xi, yi, xf, yf);
  long i = xi;
  long j = yi;

  for(i = xi; i <= xf; ++i){
    for(j = yi; j <= yf; ++j){
      vg_fill_pixel(i, j, color);
    }
  }

}

void vg_draw_line(unsigned long xi, unsigned long yi, unsigned long xf, unsigned long yf, unsigned long color){


  //Ensuring that when the line has no length a pixel is drawn
  if(xi == xf && yi == yf){
    vg_fill_pixel(xi, yi, color);
    return;
  }

  //Implementation using the DDA (Digital Difference Analyzer) algorithm
  //Based on the source material found at: https://www.tutorialspoint.com/computer_graphics/line_generation_algorithm.htm

  long delta_x = xf - xi;
  long delta_y = yf - yi;

  //Calculating the number of iterations
  unsigned long iterations = abs(MAX_VAL(delta_x, delta_y));

  //How much each axis increases per iteration
  double xincrease = (double) delta_x / iterations;
  double yincrease = (double) delta_y / iterations;
  //printf("xin = %f\tyinc = %f\n", yincrease);

  //Using decimal variables to be able to increase with decimal steps
  long double x = xi;
  long double y = yi;

  unsigned long i;
  for (i = 0; i < iterations; ++i){
    //Incrementing or decrementing x and y (depends on the value of xincrease and yincrease)
    x += xincrease;
    y += yincrease;
    //Since we can't draw in between the pixels, we have to round x and y (ROUND defined in video_utils.h)
    vg_fill_pixel(ROUND(x), ROUND(y), color);
  }

  /*
  //Old method: this did not work and new one is simpler
  printf("xi:%ld\tyi:%ld\txf:%ld\tyf:%ld\n", xi, yi, xf, yf);

  if(xf < xi) {
    swap(unsigned long, xi, xf);
    swap(unsigned long, yi, yf);
  }

  printf("v2 xi:%ld\tyi:%ld\txf:%ld\tyf:%ld\n", xi, yi, xf, yf);

  long delta_x = (long)(xf - xi);
  long delta_y = (long)(yf - yi);

  printf("d_x: %ld\td_y: %ld\n", delta_x, delta_y);

  unsigned long decl = ceil((float)(max(delta_x, delta_y)/min(delta_x, delta_y)));
  printf("b = %ld - %f * %ld\n", yf, 1/decl, xf);
  unsigned long b = yf - ceil(1/decl)*xf;

  printf("min:%ld max:%ld\n", min(delta_x, delta_y), max(delta_x, delta_y));

  printf("decl: %ld\tb: %ld\n", decl, b);

  if(abs(delta_x) >= abs(delta_y)){
    if(xf < xi) {
      swap(unsigned long, xi, xf);
      swap(unsigned long, yi, yf);
    }
    int decl = abs(delta_x/delta_y);
    int b = yf - decl*xf;
    while(xi<=xf){
      printf("xi = %lu , yi = %lu , xf = %lu , yf = %lu \n",xi,yi,xf,yf);
      yi = decl*xi + b;
      vg_fill_pixel(xi,yi,color);
      xi++;
    }
  } else {
    if(yf < yi) {
      swap(unsigned long, xi, xf);
      swap(unsigned long, yi, yf);
    }
    int decl = abs(delta_y/delta_x);
    int b = yf - decl*xf;
    while(yi<=yf){

      printf("xi = %lu , yi = %lu , xf = %lu , yf = %lu \n",xi,yi,xf,yf);
      xi = (yi-b)/decl;
      vg_fill_pixel(xi,yi,color);
      yi++;
    }
  }
  */
}

void vg_draw_xpm(char *xpm[], unsigned short xi, unsigned short yi){
  int width, height;

  // get the pix map from the XPM
  char *map = read_xpm(xpm, &width, &height);

  int i=0;
  int j=0;
  while (j < height) {
    i=0;
    while (i < width) {
      unsigned long color = map[j * width + i];
      vg_fill_pixel(xi+i, yi+j, color);
      i++;
    }
    j++;
  }

  //Freeing the memory where the pix map was stored
  free(map);
}

int vg_exit() {
  struct reg86u reg86;

  reg86.u.b.intno = 0x10; /* BIOS video services */
  reg86.u.b.ah = 0x00;    /* Set Video Mode function */
  reg86.u.b.al = 0x03;    /* 80x25 text mode*/

  if( sys_int86(&reg86) != OK ) {
      printf("\tvg_exit(): sys_int86() failed \n");
      return 1;
  } else
      return 0;
}
