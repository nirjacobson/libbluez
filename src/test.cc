#include <glibmm/init.h>
#include <giomm-2.68/giomm.h>

#include "application.h"

int main(int argc, const char** argv) {
    Glib::init();
    Gio::init();

    Application app;

    app.run();

    return 0;
}