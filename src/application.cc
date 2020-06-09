#include "application.h"

Application::Application()
    : _mainLoop(Glib::MainLoop::create())
    , _adapter("hci0") {
}

void Application::run() {
    _adapter.signal_device_added().connect(sigc::mem_fun(this, &Application::on_device_added));
    _adapter.signal_device_removed().connect(sigc::mem_fun(this, &Application::on_device_removed));
    _adapter.startDiscovery();

    _mainLoop->run();
}

void Application::on_device_added(const std::string& address) {
    std::cout << "Added: " << address << std::endl;
}

void Application::on_device_removed(const std::string& address) {
    std::cout << "Removed: " << address << std::endl;
}