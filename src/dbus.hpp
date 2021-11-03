// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 YADRO

#pragma once

#include <sdbusplus/bus.hpp>
#include <xyz/openbmc_project/Common/error.hpp>

using InvalidArgument =
    sdbusplus::xyz::openbmc_project::Common::Error::InvalidArgument;
using NotAllowed = sdbusplus::xyz::openbmc_project::Common::Error::NotAllowed;
using InternalFailure =
    sdbusplus::xyz::openbmc_project::Common::Error::InternalFailure;

namespace obmc
{
namespace dbus
{

namespace object_mapper
{
constexpr const char* service = "xyz.openbmc_project.ObjectMapper";
constexpr const char* interface = "xyz.openbmc_project.ObjectMapper";
constexpr const char* object = "/xyz/openbmc_project/object_mapper";

constexpr const char* getObject = "GetObject";
constexpr const char* getSubTree = "GetSubTree";
constexpr const char* getSubTreePaths = "GetSubTreePaths";
} // namespace object_mapper
namespace freedesktop
{
constexpr const char* propertyIface = "org.freedesktop.DBus.Properties";
constexpr const char* objectManagerIface = "org.freedesktop.DBus.ObjectManager";

constexpr const char* get = "Get";
constexpr const char* getAll = "GetAll";
constexpr const char* getManagedObjects = "GetManagedObjects";

using DbusVariantType =
    std::variant<std::vector<std::tuple<std::string, std::string, std::string>>,
                 std::vector<std::string>, std::vector<double>, std::string,
                 int64_t, uint64_t, double, int32_t, uint32_t, int16_t,
                 uint16_t, uint8_t, bool>;

using DBusPropertiesMap = std::map<std::string, DbusVariantType>;
using DBusInteracesMap = std::map<std::string, DBusPropertiesMap>;
using ManagedObjectType =
    std::vector<std::pair<sdbusplus::message::object_path, DBusInteracesMap>>;

} // namespace freedesktop

using DBusGetObjectOut = std::map<std::string, std::vector<std::string>>;
using DBusSubTreeOut = std::map<std::string, DBusGetObjectOut>;
using UserAssociation = std::tuple<std::string, std::string, std::string>;
using UserAssociationList = std::vector<UserAssociation>;
using DBusSessionDetailsMap =
    std::map<std::string,
             std::variant<std::string, uint32_t, UserAssociationList>>;

namespace utils
{
/**
 * @brief Retrieve the last segment of dbus object path
 *
 * @param[in] objectPath        - The DBus object path to retrieve the last
 *                                segment
 * @return const std::string    - The last segment of dbus object path
 */
static const std::string
    getLastSegmentFromObjectPath(const std::string& objectPath)
{
    std::size_t pos = objectPath.find_last_of('/');
    if (pos == std::string::npos)
    {
        throw std::logic_error("Invalid format of dbus object path.");
    }
    return objectPath.substr(pos + 1);
}
} // namespace utils
} // namespace dbus
} // namespace obmc
