#ifndef APPLICATION_H
#define APPLICATION_H

#include <vector>

#include <glibmm/refptr.h>
#include <glibmm/main.h>
#include <sigc++/sigc++.h>

#include "adapter.h"
#include "device.h"

class Application {

    public:
        Application();

        void run();

    private:
        Glib::RefPtr<Glib::MainLoop> _mainLoop;

        Bluez::Adapter _adapter;

        void on_device_added(const std::string& address);
        void on_device_removed(const std::string& address);
};

#endif // APPLICATION_H