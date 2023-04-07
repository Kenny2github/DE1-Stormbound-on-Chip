/**
 * Routines for interfacing with the VGA port.
 */
#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include "image_data.h"

/**** Constants ****/

/* Screen size */
#define SCREEN_W 320
#define SCREEN_H 240
#define SCREEN_CHAR_W 80
#define SCREEN_CHAR_H 60

/* VGA colors */
#define BLACK 0x0000
#define BACKGROUND 0x00C7
#define TRANSPARENT 0x4947	// Arbitrary sentinel value for transparent pixel
#define WHITE 0xFFFF

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
 * @brief Draw a rectangle.
 *
 * @param x The x-position of the top-left corner.
 * @param y The y-position of the top-left corner.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 * @param color The color to fill the rectangle with.
 */
void draw_rectangle(int x, int y, int w, int h, uint16_t color);

/**
 * @brief Clear all characters on the screen
*/
void clear_char_screen();

/**
 * @brief Draw an image on VGA screen
 *
 * @param x x-coordinate of where the image is to be drawn
 * @param y y-coordinate of where the image is to be drawn
 * @param img The image data to draw
 */
void draw_img_map(int x, int y, struct image img);

/**
 * @brief Writes a string on VGA screen usign left-align
 *
 * @param x x-char-coordinate of where the string starts
 * @param y y-char-coordinate of where the string starts
 * @param str String to be written
 */
void write_string(int x, int y, const char* str);

#endif
