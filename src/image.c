#include <stdio.h>
#include <stdlib.h>
#include "image.h"

Image *create_image(int width, int height)
{
    Image *image = malloc(sizeof(Image));
    if (image == NULL) return NULL;

    image->width = width;
    image->height = height;
    image->data = malloc(width * height * sizeof(Pixel));

    if (image->data == NULL)
    {
        free(image);
        return NULL;
    }

    return image;
}

void free_image(Image *image)
{
    if (image != NULL)
    {
        free(image->data);
        free(image);
    }
}

Image *copy_image(Image *image)
{
    if (image == NULL) return NULL;

    Image *copy = create_image(image->width, image->height);
    if (copy == NULL) return NULL;

    int total = image->width * image->height;
    for (int i = 0; i < total; i++) copy->data[i] = image->data[i];

    return copy;
}

Image *load_bmp(const char *filename)
{
    FILE *file;
    unsigned char file_header[14];
    unsigned char info_header[40];
    int width, height;
    short bits_per_pixel;
    int compression;
    int pixel_offset;
    int row_size, padding;
    int x, y;
    unsigned char b, g, r;
    Image *image;

    file = fopen(filename, "rb");
    if (file == NULL) return NULL;

    if (fread(file_header, sizeof(unsigned char), 14, file) != 14)
    {
        fclose(file);
        return NULL;
    }

    if (file_header[0] != 'B' || file_header[1] != 'M')
    {
        fclose(file);
        return NULL;
    }

    pixel_offset =
        (int)file_header[10] |
        ((int)file_header[11] << 8) |
        ((int)file_header[12] << 16) |
        ((int)file_header[13] << 24);

    if (fread(info_header, sizeof(unsigned char), 40, file) != 40)
    {
        fclose(file);
        return NULL;
    }

    width =
        (int)info_header[4] |
        ((int)info_header[5] << 8) |
        ((int)info_header[6] << 16) |
        ((int)info_header[7] << 24);

    height =
        (int)info_header[8] |
        ((int)info_header[9] << 8) |
        ((int)info_header[10] << 16) |
        ((int)info_header[11] << 24);

    bits_per_pixel = info_header[14] | ((short)info_header[15] << 8);

    compression =
        (int)info_header[16] |
        ((int)info_header[17] << 8) |
        ((int)info_header[18] << 16) |
        ((int)info_header[19] << 24);

    if (bits_per_pixel != 24)
    {
        fclose(file);
        return NULL;
    }

    if (compression != 0)
    {
        fclose(file);
        return NULL;
    }

    if (width <= 0 || height <= 0)
    {
        fclose(file);
        return NULL;
    }

    image = create_image(width, height);
    if (image == NULL)
    {
        fclose(file);
        return NULL;
    }

    row_size = width * 3;
    padding = (4 - (row_size % 4)) % 4;

    if (fseek(file, pixel_offset, SEEK_SET) != 0)
    {
        free_image(image);
        fclose(file);
        return NULL;
    }

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            if (fread(&b, sizeof(unsigned char), 1, file) != 1)
            {
                free_image(image);
                fclose(file);
                return NULL;
            }

            if (fread(&g, sizeof(unsigned char), 1, file) != 1)
            {
                free_image(image);
                fclose(file);
                return NULL;
            }

            if (fread(&r, sizeof(unsigned char), 1, file) != 1)
            {
                free_image(image);
                fclose(file);
                return NULL;
            }

            image->data[(height - 1 - y) * width + x].r = r;
            image->data[(height - 1 - y) * width + x].g = g;
            image->data[(height - 1 - y) * width + x].b = b;
        }

        if (fseek(file, padding, SEEK_CUR) != 0)
        {
            free_image(image);
            fclose(file);
            return NULL;
        }
    }

    fclose(file);
    return image;
}

int save_bmp(const char *filename, Image *image)
{
    FILE *file;
    unsigned char file_header[14];
    unsigned char info_header[40];
    int row_size, padding, file_size;
    int x, y;
    unsigned char zero = 0;

    if (image == NULL) return 0;

    file = fopen(filename, "wb");
    if (file == NULL) return 0;

    row_size = image->width * 3;
    padding = (4 - (row_size % 4)) % 4;
    file_size = 14 + 40 + (row_size + padding) * image->height;

    for (x = 0; x < 14; x++) file_header[x] = 0;

    file_header[0] = 'B';
    file_header[1] = 'M';
    file_header[2] = file_size;
    file_header[3] = file_size >> 8;
    file_header[4] = file_size >> 16;
    file_header[5] = file_size >> 24;
    file_header[10] = 54;

    fwrite(file_header, sizeof(unsigned char), 14, file);

    for (x = 0; x < 40; x++) info_header[x] = 0;

    info_header[0] = 40;
    info_header[4] = image->width;
    info_header[5] = image->width >> 8;
    info_header[6] = image->width >> 16;
    info_header[7] = image->width >> 24;
    info_header[8] = image->height;
    info_header[9] = image->height >> 8;
    info_header[10] = image->height >> 16;
    info_header[11] = image->height >> 24;
    info_header[12] = 1;
    info_header[14] = 24;
    info_header[20] = 0;

    fwrite(info_header, sizeof(unsigned char), 40, file);

    for (y = image->height - 1; y >= 0; y--)
    {
        for (x = 0; x < image->width; x++)
        {
            fwrite(&image->data[y * image->width + x].b, sizeof(unsigned char), 1, file);
            fwrite(&image->data[y * image->width + x].g, sizeof(unsigned char), 1, file);
            fwrite(&image->data[y * image->width + x].r, sizeof(unsigned char), 1, file);
        }

        for (x = 0; x < padding; x++) fwrite(&zero, sizeof(unsigned char), 1, file);
    }

    fclose(file);
    return 1;
}