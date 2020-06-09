#ifndef DEVICE_H
#define DEVICE_H

#include <assert.h>
#include <iostream>

#include <glibmm/refptr.h>
#include <glibmm/variant.h>
#include <giomm/dbusproxy.h>
#include <sigc++-2.0/sigc++/sigc++.h>

#include "freedesktop.h"
#include "bluez.h"

namespace Bluez {

    class Device {

        friend class Adapter;

        public:

            typedef sigc::signal<void> sig_connection;

            const std::string& address() const;
            const std::string& alias() const;
            bool connected() const;
            bool paired() const;

            void connect() const;
            void pair() const;
            void disconnect() const;

            sig_connection signal_connected();
            sig_connection signal_paired();
            sig_connection signal_disconnected();

        private:
            inline static const std::string Interface = "org.bluez.Device1";

            class Properties;
            class Methods;

            Glib::RefPtr<Gio::DBus::Proxy> _deviceProxy;

            std::string _path;
            bool _connected;
            bool _paired;

            sig_connection _signal_connected;
            sig_connection _signal_paired;
            sig_connection _signal_disconnected;

            Device(const std::string& path);

            std::string _address;
            std::string _alias;

            void on_device_properties_changed (const Gio::DBus::Proxy::MapChangedProperties& changed_properties, const std::vector<Glib::ustring>& invalidated_properties);

    };

    class Device::Properties {
        public:
            inline static const std::string Address = "Address";
            inline static const std::string Alias = "Alias";
            inline static const std::string Connected = "Connected";
            inline static const std::string Paired = "Paired";
    };

    class Device::Methods {
        public:
            inline static const std::string Connect = "Connect";
            inline static const std::string Pair = "Pair";
            inline static const std::string Disconnect = "Disconnect";
    };

}

#endif // DEVICE_H
