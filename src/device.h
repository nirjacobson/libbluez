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

            /// @brief "Device paired/connected/disconnected" signal type.
            typedef sigc::signal<void()> sig_connection;

            const std::string& address() const; ///< Getter for ::_address.
            const std::string& alias() const;   ///< Getter for ::_alias.
            bool connected() const;             ///< Getter for ::_connected.
            bool paired() const;                ///< Getter for ::_paired.

            /// @brief Connect to this Bluetooth device.
            /// You must /;;pair() before you /;;connect().
            void connect() const;

            /// @brief Pair with this Bluetooth device.
            /// You can ::connect() after you ::pair().
            /// @see connect().
            void pair() const;

            /// @brief Disconnect from this Bluetooth device.
            void disconnect() const;

            sig_connection signal_connected();    ///< Getter for ::_signal_connected.
            sig_connection signal_paired();       ///< Getter for ::_signal_paired.
            sig_connection signal_disconnected(); ///< Getter for ::_signal_disconnected.

        private:
            /// @brief Bluez device D-Bus interface name.
            inline static const std::string Interface = "org.bluez.Device1";

            class Properties;
            class Methods;

            /// @brief D-Bus proxy for the device object.
            Glib::RefPtr<Gio::DBus::Proxy> _device_proxy;

            std::string _path; ///< The path to the device D-Bus object.
            bool _connected;   ///< Whether the device is currently connected.
            bool _paired;      ///< Whether the device is currently paired.

            sig_connection _signal_connected;    ///< Emitted upon successful connection.
            sig_connection _signal_paired;       ///< Emitted upon successful pairing.
            sig_connection _signal_disconnected; ///< Emitted upon disconnection.

            /// @brief Adapter constructor.
            /// @param [in] path The path to the device D-Bus object.
            Device(const std::string& path);

            std::string _address; ///< The device MAC address.
            std::string _alias;   ///< The device alias (readable name).

            /// @brief Called when properties change on the Device object (pairing and connection status, etc.).
            /// @param [in] changed_properties     Properties that have changed.
            /// @param [in] invalidated_properties Properties that are no longer valid.
            void on_device_properties_changed (const Gio::DBus::Proxy::MapChangedProperties& changed_properties, const std::vector<Glib::ustring>& invalidated_properties);

    };

    /// @brief D-Bus property names supported by the D-Bus Device interface.
    class Device::Properties {
        public:
            inline static const std::string Address = "Address";        ///< Address property name.
            inline static const std::string Alias = "Alias";            ///< Alias property name.
            inline static const std::string Connected = "Connected";    ///< Connected property name.
            inline static const std::string Paired = "Paired";          ///< Paired property name.
    };

    /// @brief D-Bus method names supported by the D-Bus Device interface.
    class Device::Methods {
        public:
            inline static const std::string Connect = "Connect";        ///< Connect method name.
            inline static const std::string Pair = "Pair";              ///< Pair method name.
            inline static const std::string Disconnect = "Disconnect";  ///< Disconnect method name.
    };

}

#endif // DEVICE_H
