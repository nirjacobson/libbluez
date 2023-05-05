#include "device.h"

Bluez::Device::Device(const std::string& path)
    : _path(path)
    , _connected(false)
    , _paired(false) {

    _deviceProxy = Gio::DBus::Proxy::create_for_bus_sync (
                       Gio::DBus::BusType::SYSTEM,
                       Bluez::BusName,
                       path,
                       Device::Interface
                   );

    _deviceProxy->signal_properties_changed().connect(sigc::mem_fun(*this, &Device::on_device_properties_changed));

    Glib::Variant<std::string> stringVariant;
    _deviceProxy->get_cached_property(stringVariant, Properties::Address);
    _address = stringVariant.get();
    _deviceProxy->get_cached_property(stringVariant, Properties::Alias);
    _alias = stringVariant.get();

    Glib::Variant<bool> boolVariant;
    _deviceProxy->get_cached_property(boolVariant, Properties::Connected);
    _connected = boolVariant.get();
    _deviceProxy->get_cached_property(boolVariant, Properties::Paired);
    _paired = boolVariant.get();
}

const std::string& Bluez::Device::address() const {
    return _address;
}

const std::string& Bluez::Device::alias() const {
    return _alias;
}

bool Bluez::Device::connected() const {
    return _connected;
}

bool Bluez::Device::paired() const {
    return _paired;
}

void Bluez::Device::pair() const {
    _deviceProxy->call(Methods::Pair);
}

void Bluez::Device::connect() const {
    _deviceProxy->call(Methods::Connect);
}

void Bluez::Device::disconnect() const {
    _deviceProxy->call(Methods::Disconnect);
}

Bluez::Device::sig_connection Bluez::Device::signal_connected() {
    return _signal_connected;
}

Bluez::Device::sig_connection Bluez::Device::signal_paired() {
    return _signal_paired;
}

Bluez::Device::sig_connection Bluez::Device::signal_disconnected() {
    return _signal_disconnected;
}

void Bluez::Device::on_device_properties_changed (const Gio::DBus::Proxy::MapChangedProperties& changed_properties, const std::vector<Glib::ustring>& invalidated_properties) {
    if (changed_properties.find(Properties::Connected) != changed_properties.end()) {
        Glib::Variant<bool> variant = Glib::Variant<bool>::cast_dynamic<Glib::Variant<bool>>(changed_properties.at(Properties::Connected));
        _connected = variant.get();

        if (_connected) {
            _signal_connected.emit();
        } else {
            _signal_disconnected.emit();
        }
    }

    if (changed_properties.find(Properties::Paired) != changed_properties.end()) {
        Glib::Variant<bool> variant = Glib::Variant<bool>::cast_dynamic<Glib::Variant<bool>>(changed_properties.at(Properties::Paired));
        _paired = variant.get();

        if (_paired) {
            _signal_paired.emit();
        } else {
            _signal_disconnected.emit();
        }
    }
}