#include "adapter.h"

Bluez::Adapter::Adapter(const std::string& iface)
    : _iface(iface) {
    _objectManagerProxy = Gio::DBus::Proxy::create_for_bus_sync	(
                              Gio::DBus::BusType::BUS_TYPE_SYSTEM,
                              Bluez::BusName,
                              "/",
                              Freedesktop::ObjectManager::Interface);

    _adapterProxy = Gio::DBus::Proxy::create_for_bus_sync	(
                        Gio::DBus::BusType::BUS_TYPE_SYSTEM,
                        Bluez::BusName,
                        Bluez::PathPrefix + "/" + iface,
                        Adapter::Interface);

    init_devices();
    _objectManagerProxy->signal_signal().connect(sigc::mem_fun(this, &Adapter::on_signal));
}

void Bluez::Adapter::startDiscovery() const {
    _adapterProxy->call_sync(Methods::StartDiscovery);
}

void Bluez::Adapter::stopDiscovery() const {
    _adapterProxy->call_sync(Methods::StopDiscovery);
}

std::vector<std::string> Bluez::Adapter::devices() const {
    std::vector<std::string> devices;

    std::transform(
        _addressToDeviceMap.cbegin(),
        _addressToDeviceMap.cend(),
        std::back_inserter(devices),
    [](const std::map<std::string, DeviceEntry>::value_type pair) {
        return pair.first;
    });

    return devices;
}

const std::string& Bluez::Adapter::alias(const std::string& address) const {
    try {
        return _addressToDeviceMap.at(address).alias;
    } catch (const std::out_of_range& e) {
        throw DeviceNotFound();
    }
}

Bluez::Device* Bluez::Adapter::device(const std::string& address) {
    try {
        return new Device(_addressToDeviceMap.at(address).path);
    } catch (const std::out_of_range& e) {
        throw DeviceNotFound();
    }
}

Bluez::Adapter::sig_device Bluez::Adapter::signal_device_added() {
    return _signal_device_added;
}

Bluez::Adapter::sig_device Bluez::Adapter::signal_device_removed() {
    return _signal_device_removed;
}

void Bluez::Adapter::init_devices() {
    Glib::VariantContainerBase result = _objectManagerProxy->call_sync(Freedesktop::ObjectManager::Methods::GetManagedObjects);
    Glib::VariantContainerBase objects;
    result.get_child(objects);

    for (unsigned int i = 0; i < objects.get_n_children(); i++) {
        Glib::VariantContainerBase object;
        objects.get_child(object, i);

        Glib::Variant<std::string> path;
        object.get_child(path, 0);

        Glib::VariantContainerBase ifaceDict;
        object.get_child(ifaceDict, 1);

        for (unsigned j = 0; j < ifaceDict.get_n_children(); j++) {
            Glib::VariantContainerBase ifaceDictEntry;
            ifaceDict.get_child(ifaceDictEntry, j);

            Glib::Variant<std::string> iface;
            Glib::VariantContainerBase properties;
            ifaceDictEntry.get_child(iface, 0);
            ifaceDictEntry.get_child(properties, 1);

            if (iface.get() != Device::Interface)
                continue;

            std::string device_address;
            std::string device_path = path.get();
            std::string device_alias;
            get_property_value(properties, Bluez::Device::Properties::Address, device_address);
            get_property_value(properties, Bluez::Device::Properties::Alias, device_alias);

            _addressToDeviceMap.insert(std::pair<std::string, DeviceEntry>(device_address, {device_path, device_alias}));
            _pathToAddressMap.insert(std::pair<std::string, std::string>(device_path, device_address));
        }
    }
}

void Bluez::Adapter::on_signal (const Glib::ustring& sender_name, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters) {
    Glib::Variant<std::string> path;
    parameters.get_child(path, 0);

    if (signal_name == Freedesktop::ObjectManager::Signals::InterfacesAdded) {
        on_interfaces_added(parameters);
    } else if (signal_name == Freedesktop::ObjectManager::Signals::InterfacesRemoved) {
        on_interfaces_removed(parameters);
    }
}

void Bluez::Adapter::on_interfaces_added(const Glib::VariantContainerBase& parameters) {
    Glib::Variant<std::string> path;
    parameters.get_child(path, 0);

    Glib::VariantContainerBase ifaceDict;
    parameters.get_child(ifaceDict, 1);

    for (unsigned i = 0; i < ifaceDict.get_n_children(); i++) {
        Glib::VariantContainerBase ifaceDictEntry;
        ifaceDict.get_child(ifaceDictEntry, i);

        Glib::Variant<std::string> iface;
        Glib::VariantContainerBase properties;
        ifaceDictEntry.get_child(iface, 0);
        ifaceDictEntry.get_child(properties, 1);

        if (iface.get() != Device::Interface)
            continue;

        std::string device_address;
        std::string device_path = path.get();
        std::string device_alias;
        get_property_value(properties, Bluez::Device::Properties::Address, device_address);
        get_property_value(properties, Bluez::Device::Properties::Alias, device_alias);

        _addressToDeviceMap.insert(std::pair<std::string, DeviceEntry>(device_address, {device_path, device_alias}));
        _pathToAddressMap.insert(std::pair<std::string, std::string>(device_path, device_address));

        _signal_device_added.emit(device_address);

        break;
    }
}

void Bluez::Adapter::on_interfaces_removed(const Glib::VariantContainerBase& parameters) {
    Glib::Variant<std::string> path;
    parameters.get_child(path, 0);

    Glib::VariantContainerBase ifaceArr;
    parameters.get_child(ifaceArr, 1);

    for (unsigned int i = 0; i < ifaceArr.get_n_children(); i++) {
        Glib::Variant<std::string> iface;
        ifaceArr.get_child(iface, i);

        if (iface.get() != Device::Interface)
            continue;

        std::string device_path = path.get();
        std::string address = _pathToAddressMap[device_path];
        _pathToAddressMap.erase(device_path);
        _addressToDeviceMap.erase(address);
        _signal_device_removed.emit(address);
        break;
    }
}

template <typename T>
void Bluez::Adapter::get_property_value(const Glib::VariantContainerBase& properties, const std::string& propName,  T& propValue) {
    for (unsigned int i = 0; i < properties.get_n_children(); i++) {
        Glib::VariantContainerBase property;
        properties.get_child(property, i);

        Glib::Variant<std::string> propertyName;
        Glib::VariantContainerBase propertyValue;
        property.get_child(propertyName, 0);
        property.get_child(propertyValue, 1);

        std::string propertyNameValue = propertyName.get();

        if (propertyNameValue == propName) {
            Glib::Variant<T> variant;
            propertyValue.get_child(variant);

            propValue = variant.get();
        }
    }
}