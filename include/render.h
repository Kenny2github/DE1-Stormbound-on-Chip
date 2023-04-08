/**
 * Routines for optimizing rendering.
 *
 * All pixel rendering must be done by pushing to the render stack.
 * Surfaces pushed later will be rendered higher - i.e.,
 * pushing surface B after A means B will cover A.
 * Flushing the stack should be done only once per frame.
 */
#ifndef RENDER_H
#define RENDER_H
#include <stdbool.h>
#include "image_data.h"

/**** Data structures ****/

/**
 * @brief An image with position information.
 */
struct surface {
	// coordinates at which to draw the image
	int x, y;
	// image data to draw
	const struct image* data;
};

/**** Exported functions ****/

/**
 * @brief Flush the render stack and draw to the screen.
 * This also calls wait_for_vsync().
 */
void render_stack(void);

/**
 * @brief Push a surface to the render stack.
 * Surfaces pushed later will be rendered higher.
 *
 * @param surf The surface to render.
 */
void r_stack_push(struct surface surf);

/**
 * @brief Convenience method for initializing surface from image
 * and pushing it to the render stack.
 *
 * @param x x-position of image
 * @param y y-position of image
 * @param img Image data
 */
void push_image(int x, int y, const struct image* img);

/**
 * @brief Test if two rectangles intersect (collide).
 * Note that (0, 0, 2, 2) and (2, 0, 2, 2) do NOT collide.
 *
 * @param x1 x-position of rect 1.
 * @param y1 y-position of rect 1.
 * @param w1 width of rect 1.
 * @param h1 height of rect 1.
 * @param x2 x-position of rect 2.
 * @param y2 y-position of rect 2.
 * @param w2 width of rect 2.
 * @param h2 height of rect 2.
 * @return true if the rectangles collide;
 * @return false if the rectangles do not collide.
 */
bool rects_collide(
	int x1, int y1, int w1, int h1,
	int x2, int y2, int w2, int h2
);

#endif
