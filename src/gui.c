#include <stdio.h>
#include <stdlib.h>
#include <iup.h>
#include <iupdraw.h>

#include "gui.h"
#include "image.h"
#include "operations.h"

Image *current_image = NULL;
Image *undo_image = NULL;

int undo_available = 0;

Ihandle *canvas;

static Ihandle *current_iup_img = NULL;
static const char *IMG_HANDLE_NAME = "GUI_ACTIVE_IMAGE";

#define CANVAS_BG "24 24 27"
#define CANVAS_MARGIN 20

static void save_undo(void)
{
    if (undo_image != NULL) free_image(undo_image);

    undo_image = copy_image(current_image);
    undo_available = 1;
}

static int canvas_action(Ihandle *ih, float posx, float posy)
{
    int cw, ch;

    (void)posx;
    (void)posy;

    IupDrawBegin(ih);
    IupDrawGetSize(ih, &cw, &ch);

    IupSetAttribute(ih, "DRAWCOLOR", CANVAS_BG);
    IupSetAttribute(ih, "DRAWSTYLE", "FILL");
    IupDrawRectangle(ih, 0, 0, cw, ch);

    if (current_image != NULL && current_iup_img != NULL)
    {
        int iw, img_h, draw_w, draw_h, x, y, avail_w, avail_h;
        double scale, sx, sy;

        iw = current_image->width;
        img_h = current_image->height;

        avail_w = cw - CANVAS_MARGIN * 2;
        avail_h = ch - CANVAS_MARGIN * 2;

        scale = 1.0;

        if (avail_w > 0 && avail_h > 0 && (iw > avail_w || img_h > avail_h))
        {
            sx = (double)avail_w / (double)iw;
            sy = (double)avail_h / (double)img_h;
            scale = (sx < sy) ? sx : sy;
        }

        draw_w = (int)(iw * scale);
        draw_h = (int)(img_h * scale);

        if (draw_w < 1) draw_w = 1;
        if (draw_h < 1) draw_h = 1;

        x = (cw - draw_w) / 2;
        y = (ch - draw_h) / 2;

        IupDrawImage(ih, IMG_HANDLE_NAME, x, y, draw_w, draw_h);
    }
    else
    {
        const char *msg = "\"No Image Loaded\"";
        int text_w, text_h;

        IupSetAttribute(ih, "DRAWCOLOR", "200 200 200");
        IupSetAttribute(ih, "DRAWFONT", "Helvetica, 14");

        IupDrawGetTextSize(ih, msg, -1, &text_w, &text_h);
        IupDrawText(ih, msg, -1, (cw - text_w) / 2, (ch - text_h) / 2, 0, 0);
    }

    IupDrawEnd(ih);

    return IUP_DEFAULT;
}

void display_image(void)
{
    if (current_iup_img != NULL)
    {
        IupSetHandle((char *)IMG_HANDLE_NAME, NULL);
        IupDestroy(current_iup_img);
        current_iup_img = NULL;
    }

    if (current_image != NULL && current_image->data != NULL)
    {
        unsigned char *rgb_data;
        int total, i;

        total = current_image->width * current_image->height;
        rgb_data = malloc(total * 3);

        if (rgb_data != NULL)
        {
            for (i = 0; i < total; i++)
            {
                rgb_data[i * 3 + 0] = current_image->data[i].r;
                rgb_data[i * 3 + 1] = current_image->data[i].g;
                rgb_data[i * 3 + 2] = current_image->data[i].b;
            }

            current_iup_img = IupImageRGB(current_image->width, current_image->height, rgb_data);

            free(rgb_data);

            if (current_iup_img != NULL) IupSetHandle((char *)IMG_HANDLE_NAME, current_iup_img);
        }
    }

    if (canvas != NULL) IupUpdate(canvas);
}

int open_image(Ihandle *self)
{
    Ihandle *file_dialog;
    char *filename;
    Image *new_image;

    file_dialog = IupFileDlg();

    IupSetAttribute(file_dialog, "DIALOGTYPE", "OPEN");
    IupSetAttribute(file_dialog, "FILTER", "*.bmp");
    IupSetAttribute(file_dialog, "FILTERINFO", "BMP Image (*.bmp)");
    IupSetAttribute(file_dialog, "TITLE", "Open Image");

    IupPopup(file_dialog, IUP_CENTER, IUP_CENTER);

    if (IupGetInt(file_dialog, "STATUS") != -1)
    {
        filename = IupGetAttribute(file_dialog, "VALUE");
        new_image = load_bmp(filename);

        if (new_image == NULL)
        {
            IupMessage("Error", "Could not open BMP file.\nOnly 24-bit uncompressed BMP files are supported.");
        }
        else
        {
            if (current_image != NULL) free_image(current_image);

            if (undo_image != NULL)
            {
                free_image(undo_image);
                undo_image = NULL;
            }

            undo_available = 0;
            current_image = new_image;

            printf("Image loaded successfully.\n");
            printf("Width: %d\n", current_image->width);
            printf("Height: %d\n", current_image->height);

            display_image();
        }
    }

    IupDestroy(file_dialog);

    return IUP_DEFAULT;
}

int save_image(Ihandle *self)
{
    Ihandle *file_dialog;
    char *filename;

    if (current_image == NULL)
    {
        IupMessage("Error", "No image has been loaded.");
        return IUP_DEFAULT;
    }

    file_dialog = IupFileDlg();

    IupSetAttribute(file_dialog, "DIALOGTYPE", "SAVE");
    IupSetAttribute(file_dialog, "FILTER", "*.bmp");
    IupSetAttribute(file_dialog, "FILTERINFO", "BMP Image (*.bmp)");
    IupSetAttribute(file_dialog, "TITLE", "Save Image");

    IupPopup(file_dialog, IUP_CENTER, IUP_CENTER);

    if (IupGetInt(file_dialog, "STATUS") != -1)
    {
        filename = IupGetAttribute(file_dialog, "VALUE");

        if (save_bmp(filename, current_image))
            IupMessage("Success", "Image saved successfully.");
        else
            IupMessage("Error", "Could not save image.");
    }

    IupDestroy(file_dialog);

    return IUP_DEFAULT;
}

int grayscale_callback(Ihandle *self)
{
    if (current_image == NULL)
    {
        IupMessage("Error", "No image has been loaded.");
        return IUP_DEFAULT;
    }

    save_undo();
    grayscale(current_image);
    display_image();

    return IUP_DEFAULT;
}

int brightness_callback(Ihandle *self)
{
    int value;

    if (current_image == NULL)
    {
        IupMessage("Error", "No image has been loaded.");
        return IUP_DEFAULT;
    }

    value = 0;

    if (IupGetParam("Brightness", NULL, NULL, "Amount: %i[-255,255]\n", &value, NULL))
    {
        save_undo();
        brightness(current_image, value);
        display_image();
    }

    return IUP_DEFAULT;
}

int invert_callback(Ihandle *self)
{
    if (current_image == NULL)
    {
        IupMessage("Error", "No image has been loaded.");
        return IUP_DEFAULT;
    }

    save_undo();
    invert_image(current_image);
    display_image();

    return IUP_DEFAULT;
}

int flip_horizontal_callback(Ihandle *self)
{
    if (current_image == NULL)
    {
        IupMessage("Error", "No image has been loaded.");
        return IUP_DEFAULT;
    }

    save_undo();
    flip_horizontal(current_image);
    display_image();

    return IUP_DEFAULT;
}

int flip_vertical_callback(Ihandle *self)
{
    if (current_image == NULL)
    {
        IupMessage("Error", "No image has been loaded.");
        return IUP_DEFAULT;
    }

    save_undo();
    flip_vertical(current_image);
    display_image();

    return IUP_DEFAULT;
}

int rotate_callback(Ihandle *self)
{
    Image *new_image;

    if (current_image == NULL)
    {
        IupMessage("Error", "No image has been loaded.");
        return IUP_DEFAULT;
    }

    save_undo();
    new_image = rotate_90(current_image);

    if (new_image == NULL)
    {
        IupMessage("Error", "Could not rotate image.");
        return IUP_DEFAULT;
    }

    free_image(current_image);
    current_image = new_image;
    display_image();

    return IUP_DEFAULT;
}

int crop_callback(Ihandle *self)
{
    Image *cropped;
    int x1, y1, x2, y2, width, height;
    char format[512];
    char title[128];
    char error_msg[256];

    if (current_image == NULL)
    {
        IupMessage("Error", "No image has been loaded.");
        return IUP_DEFAULT;
    }

    printf("Crop button clicked.\n");
    fflush(stdout);

    x1 = 0;
    y1 = 0;
    x2 = current_image->width;
    y2 = current_image->height;

    sprintf(title, "Crop (image is %d x %d)", current_image->width, current_image->height);

    sprintf(
        format,
        "X1 (Left): %%i[0,%d]\n"
        "Y1 (Up): %%i[0,%d]\n"
        "X2 (Right): %%i[1,%d]\n"
        "Y2 (Down): %%i[1,%d]\n",
        current_image->width - 1,
        current_image->height - 1,
        current_image->width,
        current_image->height
    );

    if (IupGetParam(title, NULL, NULL, format, &x1, &y1, &x2, &y2, NULL))
    {
        printf("Crop requested: x1=%d y1=%d x2=%d y2=%d\n", x1, y1, x2, y2);
        fflush(stdout);

        if (x2 > current_image->width) x2 = current_image->width;
        if (y2 > current_image->height) y2 = current_image->height;

        width = x2 - x1;
        height = y2 - y1;

        cropped = (width > 0 && height > 0)
            ? crop_image(current_image, x1, y1, width, height)
            : NULL;

        if (cropped == NULL)
        {
            sprintf(
                error_msg,
                "Invalid crop region.\nX2 (Right) must be greater than X1 (Left), and\nY2 (Down) must be greater than Y1 (Up)."
            );

            IupMessage("Error", error_msg);

            return IUP_DEFAULT;
        }

        save_undo();
        free_image(current_image);
        current_image = cropped;
        display_image();
    }
    else
    {
        printf("Crop dialog was cancelled.\n");
        fflush(stdout);
    }

    return IUP_DEFAULT;
}

int blur_callback(Ihandle *self)
{
    if (current_image == NULL)
    {
        IupMessage("Error", "No image has been loaded.");
        return IUP_DEFAULT;
    }

    save_undo();
    blur_image(current_image);
    display_image();

    return IUP_DEFAULT;
}

int undo_callback(Ihandle *self)
{
    Image *temp;

    if (!undo_available)
    {
        if (undo_image == NULL)
            IupMessage("Undo", "There is nothing to undo.");
        else
            IupMessage("Undo", "Undo can only be done once.\nMake a new edit before undoing again.");

        return IUP_DEFAULT;
    }

    temp = current_image;
    current_image = undo_image;
    undo_image = temp;
    undo_available = 0;

    display_image();

    return IUP_DEFAULT;
}

int sharpen_callback(Ihandle *self)
{
    if (current_image == NULL)
    {
        IupMessage("Error", "No image has been loaded.");
        return IUP_DEFAULT;
    }

    save_undo();
    sharpen_image(current_image);
    display_image();

    return IUP_DEFAULT;
}

int exit_program(Ihandle *self)
{
    return IUP_CLOSE;
}

void create_gui(void)
{
    Ihandle *dialog;
    Ihandle *title;

    Ihandle *open_button;
    Ihandle *save_button;
    Ihandle *gray_button;
    Ihandle *brightness_button;
    Ihandle *invert_button;
    Ihandle *flip_h_button;
    Ihandle *flip_v_button;
    Ihandle *rotate_button;
    Ihandle *crop_button;
    Ihandle *blur_button;
    Ihandle *undo_button;
    Ihandle *sharpen_button;
    Ihandle *exit_button;

    Ihandle *button_box;
    Ihandle *image_frame;
    Ihandle *image_box;
    Ihandle *main_box;

    title = IupLabel("IMAGE MANIPULATION SOFTWARE");

    IupSetAttribute(title, "ALIGNMENT", "ACENTER");
    IupSetAttribute(title, "EXPAND", "HORIZONTAL");
    IupSetAttribute(title, "FONTSIZE", "16");
    IupSetAttribute(title, "PADDING", "10x10");

    open_button = IupButton("Open Image", NULL);
    save_button = IupButton("Save Image", NULL);
    gray_button = IupButton("Grayscale", NULL);
    brightness_button = IupButton("Brightness", NULL);
    invert_button = IupButton("Invert", NULL);
    flip_h_button = IupButton("Flip Horizontal", NULL);
    flip_v_button = IupButton("Flip Vertical", NULL);
    rotate_button = IupButton("Rotate 90", NULL);
    crop_button = IupButton("Crop", NULL);
    blur_button = IupButton("Blur", NULL);
    undo_button = IupButton("Undo", NULL);
    sharpen_button = IupButton("Sharpen", NULL);
    exit_button = IupButton("Exit", NULL);

    IupSetCallback(open_button, "ACTION", open_image);
    IupSetCallback(save_button, "ACTION", save_image);
    IupSetCallback(gray_button, "ACTION", grayscale_callback);
    IupSetCallback(brightness_button, "ACTION", brightness_callback);
    IupSetCallback(invert_button, "ACTION", invert_callback);
    IupSetCallback(flip_h_button, "ACTION", flip_horizontal_callback);
    IupSetCallback(flip_v_button, "ACTION", flip_vertical_callback);
    IupSetCallback(rotate_button, "ACTION", rotate_callback);
    IupSetCallback(crop_button, "ACTION", crop_callback);
    IupSetCallback(blur_button, "ACTION", blur_callback);
    IupSetCallback(undo_button, "ACTION", undo_callback);
    IupSetCallback(sharpen_button, "ACTION", sharpen_callback);
    IupSetCallback(exit_button, "ACTION", exit_program);

    IupSetAttribute(open_button, "RASTERSIZE", "140x35");
    IupSetAttribute(save_button, "RASTERSIZE", "140x35");
    IupSetAttribute(gray_button, "RASTERSIZE", "140x35");
    IupSetAttribute(brightness_button, "RASTERSIZE", "140x35");
    IupSetAttribute(invert_button, "RASTERSIZE", "140x35");
    IupSetAttribute(flip_h_button, "RASTERSIZE", "140x35");
    IupSetAttribute(flip_v_button, "RASTERSIZE", "140x35");
    IupSetAttribute(rotate_button, "RASTERSIZE", "140x35");
    IupSetAttribute(crop_button, "RASTERSIZE", "140x35");
    IupSetAttribute(blur_button, "RASTERSIZE", "140x35");
    IupSetAttribute(undo_button, "RASTERSIZE", "140x35");
    IupSetAttribute(sharpen_button, "RASTERSIZE", "140x35");
    IupSetAttribute(exit_button, "RASTERSIZE", "140x35");

    button_box = IupVbox(
        open_button, save_button, gray_button, brightness_button, invert_button,
        flip_h_button, flip_v_button, rotate_button, crop_button, blur_button,
        undo_button, sharpen_button, IupFill(), exit_button, NULL
    );

    IupSetAttribute(button_box, "MARGIN", "10x10");
    IupSetAttribute(button_box, "GAP", "5");

    canvas = IupCanvas(NULL);

    IupSetAttribute(canvas, "RASTERSIZE", "700x550");
    IupSetAttribute(canvas, "EXPAND", "YES");
    IupSetCallback(canvas, "ACTION", (Icallback)(void *)canvas_action);

    image_frame = IupFrame(canvas);
    IupSetAttribute(image_frame, "EXPAND", "YES");

    image_box = IupVbox(title, image_frame, NULL);
    IupSetAttribute(image_box, "MARGIN", "10x10");
    IupSetAttribute(image_box, "GAP", "5");
    IupSetAttribute(image_box, "EXPAND", "YES");

    main_box = IupHbox(button_box, image_box, NULL);
    IupSetAttribute(main_box, "GAP", "10");
    IupSetAttribute(main_box, "MARGIN", "10x10");

    dialog = IupDialog(main_box);

    IupSetAttribute(dialog, "TITLE", "Image Manipulation Software");
    IupSetAttribute(dialog, "RASTERSIZE", "950x650");
    IupSetAttribute(dialog, "RESIZE", "YES");

    IupShowXY(dialog, IUP_CENTER, IUP_CENTER);
}