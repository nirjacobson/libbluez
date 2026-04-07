/**
 * @file application.h
 * @author Nir Jacobson
 * @date 2026-04-07
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <vector>

#include <glibmm/refptr.h>
#include <glibmm/main.h>
#include <sigc++/sigc++.h>

#include "adapter.h"
#include "device.h"

/// @brief An included test application.
/// Use it to test the proper configuration of your system.
class Application {

    public:
        Application();

        void run();

    private:
        Glib::RefPtr<Glib::MainLoop> _main_loop;

        Bluez::Adapter _adapter; ///< The bluetooth Adapter

        /// @brief Called when a new device is registered
        void on_device_added(const std::string& address);

        /// @brief Called when a device is removed
        /// @param [in] address The MAC address of the removed device
        void on_device_removed(const std::string& address);
};

#endif // APPLICATION_H