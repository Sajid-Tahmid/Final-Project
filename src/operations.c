#include <stdlib.h>
#include "operations.h"

void grayscale(Image *image)
{
    int x, y, gray;

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            Pixel *pixel = &image->data[y * image->width + x];
            gray = 0.299 * pixel->r + 0.587 * pixel->g + 0.114 * pixel->b;
            pixel->r = gray;
            pixel->g = gray;
            pixel->b = gray;
        }
    }
}

void brightness(Image *image, int value)
{
    int i, total, r, g, b;

    total = image->width * image->height;

    for (i = 0; i < total; i++)
    {
        r = image->data[i].r + value;
        g = image->data[i].g + value;
        b = image->data[i].b + value;

        if (r > 255) r = 255;
        if (r < 0) r = 0;
        if (g > 255) g = 255;
        if (g < 0) g = 0;
        if (b > 255) b = 255;
        if (b < 0) b = 0;

        image->data[i].r = r;
        image->data[i].g = g;
        image->data[i].b = b;
    }
}

void invert_image(Image *image)
{
    int total = image->width * image->height;

    for (int i = 0; i < total; i++)
    {
        image->data[i].r = 255 - image->data[i].r;
        image->data[i].g = 255 - image->data[i].g;
        image->data[i].b = 255 - image->data[i].b;
    }
}

void flip_horizontal(Image *image)
{
    int x, y, opposite_x;
    Pixel temp;

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width / 2; x++)
        {
            opposite_x = image->width - 1 - x;
            temp = image->data[y * image->width + x];
            image->data[y * image->width + x] = image->data[y * image->width + opposite_x];
            image->data[y * image->width + opposite_x] = temp;
        }
    }
}

void flip_vertical(Image *image)
{
    int x, y, opposite_y;
    Pixel temp;

    for (y = 0; y < image->height / 2; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            opposite_y = image->height - 1 - y;
            temp = image->data[y * image->width + x];
            image->data[y * image->width + x] = image->data[opposite_y * image->width + x];
            image->data[opposite_y * image->width + x] = temp;
        }
    }
}

Image *rotate_90(Image *image)
{
    Image *rotated;
    int x, y, new_x, new_y;

    rotated = malloc(sizeof(Image));
    if (rotated == NULL) return NULL;

    rotated->width = image->height;
    rotated->height = image->width;
    rotated->data = malloc(rotated->width * rotated->height * sizeof(Pixel));

    if (rotated->data == NULL)
    {
        free(rotated);
        return NULL;
    }

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            new_x = image->height - 1 - y;
            new_y = x;
            rotated->data[new_y * rotated->width + new_x] = image->data[y * image->width + x];
        }
    }

    return rotated;
}

Image *crop_image(Image *image, int x, int y, int width, int height)
{
    Image *cropped;
    int i, j;

    if (x < 0 || y < 0) return NULL;
    if (width <= 0 || height <= 0) return NULL;
    if (x + width > image->width) return NULL;
    if (y + height > image->height) return NULL;

    cropped = malloc(sizeof(Image));
    if (cropped == NULL) return NULL;

    cropped->width = width;
    cropped->height = height;
    cropped->data = malloc(width * height * sizeof(Pixel));

    if (cropped->data == NULL)
    {
        free(cropped);
        return NULL;
    }

    for (j = 0; j < height; j++)
        for (i = 0; i < width; i++)
            cropped->data[j * width + i] = image->data[(y + j) * image->width + (x + i)];

    return cropped;
}

void blur_image(Image *image)
{
    Image *blurred;
    int x, y, nx, ny, count, r, g, b;

    blurred = malloc(sizeof(Image));
    if (blurred == NULL) return;

    blurred->width = image->width;
    blurred->height = image->height;
    blurred->data = malloc(image->width * image->height * sizeof(Pixel));

    if (blurred->data == NULL)
    {
        free(blurred);
        return;
    }

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            r = 0;
            g = 0;
            b = 0;
            count = 0;

            for (ny = y - 1; ny <= y + 1; ny++)
            {
                for (nx = x - 1; nx <= x + 1; nx++)
                {
                    if (nx >= 0 && nx < image->width && ny >= 0 && ny < image->height)
                    {
                        r += image->data[ny * image->width + nx].r;
                        g += image->data[ny * image->width + nx].g;
                        b += image->data[ny * image->width + nx].b;
                        count++;
                    }
                }
            }

            blurred->data[y * image->width + x].r = r / count;
            blurred->data[y * image->width + x].g = g / count;
            blurred->data[y * image->width + x].b = b / count;
        }
    }

    for (y = 0; y < image->height; y++)
        for (x = 0; x < image->width; x++)
            image->data[y * image->width + x] = blurred->data[y * image->width + x];

    free(blurred->data);
    free(blurred);
}

void sharpen_image(Image *image)
{
    Image *sharpened;
    int kernel[3][3] = { {0, -1, 0}, {-1, 5, -1}, {0, -1, 0} };
    int x, y, nx, ny, i, j, r, g, b;

    sharpened = malloc(sizeof(Image));
    if (sharpened == NULL) return;

    sharpened->width = image->width;
    sharpened->height = image->height;
    sharpened->data = malloc(image->width * image->height * sizeof(Pixel));

    if (sharpened->data == NULL)
    {
        free(sharpened);
        return;
    }

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            r = 0;
            g = 0;
            b = 0;

            for (j = -1; j <= 1; j++)
            {
                for (i = -1; i <= 1; i++)
                {
                    nx = x + i;
                    ny = y + j;

                    if (nx >= 0 && nx < image->width && ny >= 0 && ny < image->height)
                    {
                        r += image->data[ny * image->width + nx].r * kernel[j + 1][i + 1];
                        g += image->data[ny * image->width + nx].g * kernel[j + 1][i + 1];
                        b += image->data[ny * image->width + nx].b * kernel[j + 1][i + 1];
                    }
                }
            }

            if (r > 255) r = 255;
            if (r < 0) r = 0;
            if (g > 255) g = 255;
            if (g < 0) g = 0;
            if (b > 255) b = 255;
            if (b < 0) b = 0;

            sharpened->data[y * image->width + x].r = r;
            sharpened->data[y * image->width + x].g = g;
            sharpened->data[y * image->width + x].b = b;
        }
    }

    for (y = 0; y < image->height; y++)
        for (x = 0; x < image->width; x++)
            image->data[y * image->width + x] = sharpened->data[y * image->width + x];

    free(sharpened->data);
    free(sharpened);
}