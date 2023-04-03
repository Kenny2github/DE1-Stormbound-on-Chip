/**
 * Routines for interfacing with the VGA port.
 */
#ifndef VGA_H
#define VGA_H

#include <stdint.h>

/**** Constants ****/

/* Screen size */
#define SCREEN_W 320
#define SCREEN_H 240
#define SCREEN_CHAR_W 80
#define SCREEN_CHAR_H 60

/* VGA colors */
#define BLACK 0x0000
#define BACKGROUND 0x00C7

/**** Exported functions ****/

/**
 * @brief Wait for vsync to occur by polling S bit
 */
void wait_for_vsync(void);

/**
 * @brief Update back buffer, usually after waiting for vsync
 */
void update_back_buffer(void);

/**
 * @brief Configure VGA.
 * Clears the screens for both front and back buffers 
 */
void configure_vga(void);

/**
 * @brief Fill entire screen with a single color
 * @param color Color to fill screen with
 */
void fill_screen(uint16_t color);

/**
 * @brief Clear all characters on the screen
*/
void clear_char_screen();

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
void draw_RLE_img_map(int x, int y, int width, int size, int rle_img_map[]);


/**
 * @brief Writes a string on VGA screen usign left-align
 * 
 * @param x x-char-coordinate of where the string starts
 * @param y y-char-coordinate of where the string starts
 * @param str String to be written
 */
void write_string(int x, int y, char* str);

#endif
