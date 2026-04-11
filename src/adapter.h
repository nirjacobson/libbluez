/**
 * @file adapter.h
 * @author Nir Jacobson
 * @date 2026-04-07
 */

#ifndef ADAPTER_H
#define ADAPTER_H

#include <assert.h>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <iostream>
#include <algorithm>

#include <glibmm/refptr.h>
#include <giomm/dbusproxy.h>
#include <glibmm/variant.h>
#include <sigc++/sigc++.h>

#include "bluez.h"
#include "freedesktop.h"
#include "device.h"

namespace Bluez {

    /// @brief Bluetooth adapter proxy
    class Adapter {

        public:
            typedef sigc::signal<void(std::string)> sig_device;

            /// @brief Thrown when an unknown device is requested
            struct DeviceNotFound : public std::exception {
                const char* what() const throw() {
                    return "The device with the given MAC address is unknown.";
                }
            };

            /// @brief Adapter constructor
            /// @param [in] iface The name of the adapter, i.e. hci0
            Adapter(const std::string& iface);

            /// @brief Causes newly discovered Bluetooth devices to register with this Adapter.
            void start_discovery() const;

            /// @brief Causes the Adapter to stop discovering new devices.
            void stop_discovery() const;

            /// @brief Returns the MAC addresses of registered Bluetooth devices
            /// @return a list of MAC addresses
            std::vector<std::string> devices() const;

            /// @brief Return the alias (reported name) of a given Bluetooth device
            /// @param [in] address the MAC address of the device
            /// @return the device alias
            const std::string& alias(const std::string& address) const;

            /// @brief Return a Bluetooth device proxy for a given device
            /// @param [in] address the MAC address of the device
            /// @return a Bluetooth device proxy
            Device* device(const std::string& address);

            sig_device signal_device_added();   ///< Getter for ::_signal_device_added.
            sig_device signal_device_removed(); ///< Getter for ::_signal_device_removed.

        private:
            /// @brief The D-Bus Bluetooth adapter interface
            inline static const std::string Interface = "org.bluez.Adapter1";

            class Methods;

            /// @brief Device D-Bus path and alias (readable name).
            struct DeviceEntry {
                std::string path;  ///< The D-Bus path to the device
                std::string alias; ///< The alias (readable name) of the device
            };

            std::string _iface; ///< The name of the adapter, i.e. hci0
            Glib::RefPtr<Gio::DBus::Proxy> _object_manager_proxy; ///< D-Bus proxy for the adapter object using the manager interface
            Glib::RefPtr<Gio::DBus::Proxy> _adapter_proxy;        ///< D-Bus proxy for the adapter object using the adapter interface

            std::map<std::string, DeviceEntry> _address_to_device_map; ///< Device information by MAC address
            std::map<std::string, std::string> _path_to_address_map;   ///< Mapping of device D-Bus path to MAC address

            sig_device _signal_device_added;   ///> Emitted when a new device appears
            sig_device _signal_device_removed; ///> Emitted when a device disappears

            /// @brief Initialize an internal list of preregistered Bluetooth device information
            void init_devices();

            /// @brief Called when the object manager interface emits a signal
            /// @param [in] signal_name The signal emitted
            /// @param [in] parameters  The signal arguments
            /// @see Freedesktop::ObjectManager::Signals
            void on_signal (const Glib::ustring&, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters);

            /// @brief A delegate called when the object manager emits that new devices have been discovered
            /// @param [in] parameters The signal arguments
            void on_interfaces_added(const Glib::VariantContainerBase& parameters);

            /// @brief A delegate called when the object manager emits that devices have been removed
            /// @param [in] parameters The signal arguments
            void on_interfaces_removed(const Glib::VariantContainerBase& parameters);

            /// @brief A convenience method to retrieve a property from a Glib::VariantContainerBase with a given type
            /// @tparam T The data type of the property
            /// @param [in]  properties the set of properties
            /// @param [in]  propName   the property to retrieve 
            /// @param [out] propValue  the property value
            template <typename T>
            static void get_property_value(const Glib::VariantContainerBase& properties, const std::string& propName, T& propValue);
    };

    /// @brief D-Bus method names supported by the D-Bus Adapter interface
    class Adapter::Methods {
        public:
            inline static const std::string StartDiscovery = "StartDiscovery";
            inline static const std::string StopDiscovery = "StopDiscovery";
    };

}

#endif // ADAPTER_H