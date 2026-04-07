/**
 * @file device.h
 * @author Nir Jacobson
 * @date 2026-04-07
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <assert.h>
#include <iostream>

#include <glibmm/refptr.h>
#include <glibmm/variant.h>
#include <giomm/dbusproxy.h>
#include <sigc++/sigc++.h>

#include "freedesktop.h"
#include "bluez.h"

namespace Bluez {

    /// @brief Bluetooth device proxy
    class Device {

            friend class Adapter;

        public:

            typedef sigc::signal<void()> sig_connection;

            const std::string& address() const;
            const std::string& alias() const;
            bool connected() const;
            bool paired() const;

            /// @brief Connect to this Bluetooth device.
            /// You must pair() before you connect().
            void connect() const;

            /// @brief Pair with this Bluetooth device.
            /// You can connect after you pair.
            /// @see connect().
            void pair() const;

            /// @brief Disconnect from this Bluetooth device.
            void disconnect() const;

            sig_connection signal_connected();
            sig_connection signal_paired();
            sig_connection signal_disconnected();

        private:
            inline static const std::string Interface = "org.bluez.Device1";

            class Properties;
            class Methods;

            /// @brief D-Bus proxy for the device object
            Glib::RefPtr<Gio::DBus::Proxy> _device_proxy;

            std::string _path; ///< The path to the device D-Bus object
            bool _connected;   ///< Whether the device is currently connected
            bool _paired;      ///< Whether the device is currently paired

            sig_connection _signal_connected;    ///< Emitted upon successful connection
            sig_connection _signal_paired;       ///< Emitted upon successful pairing
            sig_connection _signal_disconnected; ///< Emitted upon disconnection

            /// @brief Adapter constructor
            /// @param [in] path The path to the device D-Bus object
            Device(const std::string& path);

            std::string _address; ///< The device MAC address
            std::string _alias;   ///< The device alias (readable name)

            /// @brief Called when properties change on the Device object (pairing and connection status, etc.)
            /// @param [in] changed_properties     properties that have changed
            /// @param [in] invalidated_properties properties that are no longer valid
            void on_device_properties_changed (const Gio::DBus::Proxy::MapChangedProperties& changed_properties, const std::vector<Glib::ustring>& invalidated_properties);

    };

    /// @brief D-Bus property names supported by the D-Bus Device interface
    class Device::Properties {
        public:
            inline static const std::string Address = "Address";
            inline static const std::string Alias = "Alias";
            inline static const std::string Connected = "Connected";
            inline static const std::string Paired = "Paired";
    };

    /// @brief D-Bus method names supported by the D-Bus Device interface
    class Device::Methods {
        public:
            inline static const std::string Connect = "Connect";
            inline static const std::string Pair = "Pair";
            inline static const std::string Disconnect = "Disconnect";
    };

}

#endif // DEVICE_H
