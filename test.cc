#include <glibmm/init.h>

#include "src/application.h"

int main(int argc, const char** argv) {
    Glib::init();

    Application app;

    app.run();

    return 0;
}