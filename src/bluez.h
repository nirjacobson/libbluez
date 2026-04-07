/**
 * @file bluez.h
 * @author Nir Jacobson
 * @date 2026-04-07
 */

#ifndef BLUEZ_H
#define BLUEZ_H

#include <string>

/// @brief Bluez D-Bus information
namespace Bluez {
    inline const std::string BusName = "org.bluez";
    inline const std::string PathPrefix = "/org/bluez";
};

#endif // BLUEZ_H