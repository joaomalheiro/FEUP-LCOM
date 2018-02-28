#ifndef __VIDEO_GR_H
#define __VIDEO_GR_H

/** @defgroup video_gr video_gr
 * @{
 *
 * Functions for outputing data to screen in graphics mode
 */

/**
 * @brief Initializes the video module in graphics mode
 *
 * Uses the VBE INT 0x10 interface to set the desired
 *  graphics mode, maps VRAM to the process' address space and
 *  initializes static global variables with the resolution of the screen,
 *  and the number of colors
 *
 * @param mode 16-bit VBE mode to set
 * @return Virtual address VRAM was mapped to. NULL, upon failure.
 */
void *vg_init(unsigned short mode);

 /**
 * @brief Returns to default Minix 3 text mode (0x03: 25 x 80, 16 colors)
 *
 * @return 0 upon success, non-zero upon failure
 */
int vg_exit(void);

/**
 * @brief Fills a pixel at coordinates x,y with color color
 * @param x     The x of the pixel to fill
 * @param y     The y of the pixel to fill
 * @param color The color to fill the pixel with
 */
void vg_fill_pixel(unsigned long x, unsigned long y, unsigned long color);

void vg_draw_rect(unsigned long xi, unsigned long yi, unsigned long xf, unsigned long yf, unsigned long color);

void vg_draw_line(unsigned long xi, unsigned long yi, unsigned long xf, unsigned long yf, unsigned long color);

void vg_draw_xpm(char *xpm[], unsigned short xi, unsigned short yi);

/**
 * Public variables for accessing the resolution of the screen
 */
 extern unsigned HRES;
 extern unsigned VRES;

 /** @} end of video_gr */

#endif /* __VIDEO_GR_H */
