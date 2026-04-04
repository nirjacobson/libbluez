#include "adapter.h"

Bluez::Adapter::Adapter(const std::string& iface)
    : _iface(iface)
    , _object_manager_proxy(nullptr)
    , _adapter_proxy(nullptr) {
    Gio::DBus::Proxy::create_for_bus(
        Gio::DBus::BusType::SYSTEM,
        Bluez::BusName,
        "/",
        Freedesktop::ObjectManager::Interface,
    [&](Glib::RefPtr<Gio::AsyncResult>& result) {
        _object_manager_proxy = Gio::DBus::Proxy::create_for_bus_finish(result);

        init_devices();
        _object_manager_proxy->signal_signal().connect(sigc::mem_fun(*this, &Adapter::on_signal));
    });
    Gio::DBus::Proxy::create_for_bus(
        Gio::DBus::BusType::SYSTEM,
        Bluez::BusName,
        Bluez::PathPrefix + "/" + iface,
        Adapter::Interface,
    [&](Glib::RefPtr<Gio::AsyncResult>& result) {
        _adapter_proxy = Gio::DBus::Proxy::create_for_bus_finish(result);
    });
}

void Bluez::Adapter::start_discovery() const {
    if (_adapter_proxy) _adapter_proxy->call_sync(Methods::StartDiscovery);
}

void Bluez::Adapter::stop_discovery() const {
    if (_adapter_proxy) _adapter_proxy->call_sync(Methods::StopDiscovery);
}

std::vector<std::string> Bluez::Adapter::devices() const {
    std::vector<std::string> devices;

    std::transform(
        _address_to_device_map.begin(),
        _address_to_device_map.end(),
        std::back_inserter(devices),
    [](const std::map<std::string, DeviceEntry>::value_type pair) {
        return pair.first;
    });

    return devices;
}

const std::string& Bluez::Adapter::alias(const std::string& address) const {
    try {
        return _address_to_device_map.at(address).alias;
    } catch (const std::out_of_range& e) {
        throw DeviceNotFound();
    }
}

Bluez::Device* Bluez::Adapter::device(const std::string& address) {
    try {
        return new Device(_address_to_device_map.at(address).path);
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
    const Glib::VariantContainerBase result = _object_manager_proxy->call_sync(Freedesktop::ObjectManager::Methods::GetManagedObjects);
    Glib::VariantContainerBase objects;
    result.get_child(objects);

    for (unsigned int i = 0; i < objects.get_n_children(); i++) {
        Glib::VariantContainerBase object;
        objects.get_child(object, i);

        Glib::Variant<std::string> path;
        object.get_child(path, 0);

        Glib::VariantContainerBase iface_dict;
        object.get_child(iface_dict, 1);

        for (unsigned j = 0; j < iface_dict.get_n_children(); j++) {
            Glib::VariantContainerBase iface_dict_entry;
            iface_dict.get_child(iface_dict_entry, j);

            Glib::Variant<std::string> iface;
            Glib::VariantContainerBase properties;
            iface_dict_entry.get_child(iface, 0);
            iface_dict_entry.get_child(properties, 1);

            if (iface.get() != Device::Interface)
                continue;

            std::string device_address;
            std::string device_path = path.get();
            std::string device_alias;
            get_property_value(properties, Bluez::Device::Properties::Address, device_address);
            get_property_value(properties, Bluez::Device::Properties::Alias, device_alias);

            _address_to_device_map.insert(std::pair<std::string, DeviceEntry>(device_address, {device_path, device_alias}));
            _path_to_address_map.insert(std::pair<std::string, std::string>(device_path, device_address));
        }
    }
}

void Bluez::Adapter::on_signal (const Glib::ustring& sender_name, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters) {
    Glib::Variant<std::string> path;
    parameters.get_child(path, 0);

    if (signal_name == Freedesktop::ObjectManager::Signals::InterfacesAdded.data()) {
        on_interfaces_added(parameters);
    } else if (signal_name == Freedesktop::ObjectManager::Signals::InterfacesRemoved.data()) {
        on_interfaces_removed(parameters);
    }
}

void Bluez::Adapter::on_interfaces_added(const Glib::VariantContainerBase& parameters) {
    Glib::Variant<std::string> path;
    parameters.get_child(path, 0);

    Glib::VariantContainerBase iface_dict;
    parameters.get_child(iface_dict, 1);

    for (unsigned i = 0; i < iface_dict.get_n_children(); i++) {
        Glib::VariantContainerBase iface_dict_entry;
        iface_dict.get_child(iface_dict_entry, i);

        Glib::Variant<std::string> iface;
        Glib::VariantContainerBase properties;
        iface_dict_entry.get_child(iface, 0);
        iface_dict_entry.get_child(properties, 1);

        if (iface.get() != Device::Interface)
            continue;

        std::string device_address;
        std::string device_path = path.get();
        std::string device_alias;
        get_property_value(properties, Bluez::Device::Properties::Address, device_address);
        get_property_value(properties, Bluez::Device::Properties::Alias, device_alias);

        _address_to_device_map.insert(std::pair<std::string, DeviceEntry>(device_address, {device_path, device_alias}));
        _path_to_address_map.insert(std::pair<std::string, std::string>(device_path, device_address));

        _signal_device_added.emit(device_address);

        break;
    }
}

void Bluez::Adapter::on_interfaces_removed(const Glib::VariantContainerBase& parameters) {
    Glib::Variant<std::string> path;
    parameters.get_child(path, 0);

    Glib::VariantContainerBase iface_arr;
    parameters.get_child(iface_arr, 1);

    for (unsigned int i = 0; i < iface_arr.get_n_children(); i++) {
        Glib::Variant<std::string> iface;
        iface_arr.get_child(iface, i);

        if (iface.get() != Device::Interface)
            continue;

        std::string device_path = path.get();
        std::string address = _path_to_address_map[device_path];
        _path_to_address_map.erase(device_path);
        _address_to_device_map.erase(address);
        _signal_device_removed.emit(address);
        break;
    }
}

template <typename T>
void Bluez::Adapter::get_property_value(const Glib::VariantContainerBase& properties, const std::string& propName,  T& propValue) {
    for (unsigned int i = 0; i < properties.get_n_children(); i++) {
        Glib::VariantContainerBase property;
        properties.get_child(property, i);

        Glib::Variant<std::string> property_name;
        Glib::VariantContainerBase property_value;
        property.get_child(property_name, 0);
        property.get_child(property_value, 1);

        std::string property_name_value = property_name.get();

        if (property_name_value == propName) {
            Glib::Variant<T> variant;
            property_value.get_child(variant);

            propValue = variant.get();
        }
    }
}