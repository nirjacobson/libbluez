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
    inline const std::string BusName = "org.bluez";     ///< Bluez D-Bus bus name
    inline const std::string PathPrefix = "/org/bluez"; ///< Bluez D-Bus path
};

#endif // BLUEZ_H