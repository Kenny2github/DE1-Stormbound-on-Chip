/**
 * Routines for interfacing with the VGA port.
 */
#ifndef VGA_H
#define VGA_H

/**** Constants ****/

#define SCREEN_W 320
#define SCREEN_H 240

/**** Exported functions ****/

/**
 * @brief Configure VGA.
 * Clears the screens for both front and back buffers 
*/
void configure_vga(void);

/**
 * @brief Draw non-RLE image map on VGA screen
 * @param x x-coordinate of where the image is to be drawn
 * @param y y-coordinate of where the image is to be drawn
 * @param height Height of the image
 * @param width Width of the image
 * @param img_map The 2D array mapping of the image to be drawn
 */
void draw_img_map(int x, int y, int height, int width, int img_map[height][width]);

/**
 * @brief Draw RLE image map on VGA screen
 * @param x x-coordinate of where the image is to be drawn
 * @param y y-coordinate of where the image is to be drawn
 * @param width Width of the image
 * @param rle_img_map The 2D array mapping of the image to be drawn in RLE
 */
void draw_RLE_img_map(int x, int y, int width, int rle_img_map[])

#endif
