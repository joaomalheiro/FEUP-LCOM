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
 * @brief Draws the chosen bitmap in the chosed coordinates
 * @param  bmplocation The absolute location of the bmp to draw
 * @param  x           The x at which to draw the bitmap
 * @param  y           The y at which to draw the bitmap
 * @return             0 if successful, not 0 otherwise
 */
int vg_draw_bitmap(char * bmplocation, int x, int y);

/**
 * @brief Returns the vertical resolution
 * @return The verical resolution of the current video mode
 */
int getVerResolution();

/**
 * @brief Returns the horizontal resolution
 * @return The horizontal resolution of the current video mode
 */
int getHorResolution();

/**
 * @brief Gets the number of bits per pixel of the current video mode
 * @return Returns the number of bits per pixel of the current video mode
 */
unsigned getBitsPerPixel();

/**
 * @brief Returns the size of the VRAM in bytes
 * @return Returns the size of the VRAM in bytes
 */
unsigned int getVramSize();

/**
 * @brief Returns a pointer to the mapped video memory
 * @return Returns a pointer to the mapped graphics buffer
 */
unsigned short * getGraphicsBuffer();

/**
 * @brief Returns a pointer to the secondary buffer
 * @return Returns a pointer to the secondary buffer
 */
unsigned short * getBackBuffer();

/**
 * @brief Swaps the primary and secondary buffers by copying the contents of the secondary buffer into the primary buffer
 */
void swap_buffers();

/**
 * @brief Returns a "snapshot" of the video_mem, resulting in a "print screen" buffer
 * @return The snapshotted buffer (copy of video mem) or NULL if the allocation failed
 */
unsigned short * snapshot_video_mem();

/**
 * @brief Draws the given buffer directly, so it must be a "true buffer" - the size of vram (into the double buffer)
 * @param buffer The buffer to redraw
 */
void redraw_buffer(unsigned short * buffer);

/**
 * @brief Makes the given buffer black and white
 * @param buffer  The buffer to make black and white
 * @param n_bytes The size of the buffer to make black and white
 */
void make_bw(unsigned short * buffer, unsigned int n_bytes);

/**
 * @brief Changes the brightness of the passed buffer
 * @param buffer       Buffer to change the brightness of
 * @param n_bytes      Number of bytes of the passed buffer
 * @param gamma_factor The factor by which to alter the brightness of the buffer (1.0 makes no changes, less than that darkens and more brightens)
 */
void change_gamma(unsigned short * buffer, unsigned int n_bytes, double gamma_factor);

/**
 * @brief Fills screen with given color
 * @param color Color to fill the screen with (in 5R 6G 5B format)
 */
void vg_fill_screen(short color);

 /** @} end of video_gr */

#endif /* __VIDEO_GR_H */
