#ifndef IMAGE_H
#define IMAGE_H

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Pixel;

typedef struct
{
    int width;
    int height;
    Pixel *data;
} Image;

Image *create_image(int width, int height);
void free_image(Image *image);
Image *copy_image(Image *image);
Image *load_bmp(const char *filename);
int save_bmp(const char *filename, Image *image);

#endif