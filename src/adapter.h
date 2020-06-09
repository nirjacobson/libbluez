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
#include <sigc++-2.0/sigc++/sigc++.h>

#include "bluez.h"
#include "freedesktop.h"
#include "device.h"

namespace Bluez {

class Adapter {

    public:
        typedef sigc::signal<void, std::string> sig_device;

        struct DeviceNotFound : public std::exception {
            const char* what() const throw() {
                return "The given device is unrecognized.";
            }
        };

        Adapter(const std::string& iface);

        void startDiscovery() const;
        void stopDiscovery() const;

        std::vector<std::string> devices() const;
        const std::string& alias(const std::string& address) const;

        Device* device(const std::string& address);

        sig_device signal_device_added();
        sig_device signal_device_removed();

    private:
        inline static const std::string Interface = "org.bluez.Adapter1";
        class Methods;

        struct DeviceEntry {
            std::string path;
            std::string alias;
        };

        std::string _iface;
        Glib::RefPtr<Gio::DBus::Proxy> _objectManagerProxy;
        Glib::RefPtr<Gio::DBus::Proxy> _adapterProxy;

        std::map<std::string, DeviceEntry> _addressToDeviceMap;
        std::map<std::string, std::string> _pathToAddressMap;

        sig_device _signal_device_added;
        sig_device _signal_device_removed;

        void init_devices();

        void on_signal (const Glib::ustring& sender_name, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters);

        void on_interfaces_added(const Glib::VariantContainerBase& parameters);
        void on_interfaces_removed(const Glib::VariantContainerBase& parameters);

        template <typename T>
        static void get_property_value(const Glib::VariantContainerBase& properties, const std::string& propName, T& propValue);
};

class Adapter::Methods {
    public:
        inline static const std::string StartDiscovery = "StartDiscovery";
        inline static const std::string StopDiscovery = "StopDiscovery";
};

}

#endif // ADAPTER_H