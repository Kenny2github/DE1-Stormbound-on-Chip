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

#endif
