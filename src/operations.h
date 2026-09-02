#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "image.h"

void grayscale(Image *image);
void brightness(Image *image, int value);
void invert_image(Image *image);
void flip_horizontal(Image *image);
void flip_vertical(Image *image);
Image *rotate_90(Image *image);
Image *crop_image(Image *image, int x, int y, int width, int height);
void blur_image(Image *image);
void sharpen_image(Image *image);

#endif