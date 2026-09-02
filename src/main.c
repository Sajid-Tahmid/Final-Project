#include <iup.h>
#include "gui.h"

int main(int argc, char **argv)
{
    IupOpen(&argc, &argv);
    create_gui();
    IupMainLoop();
    IupClose();

    return 0;
}