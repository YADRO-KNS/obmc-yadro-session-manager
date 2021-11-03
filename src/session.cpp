// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 YADRO

#include <dbus.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <session.hpp>

#include <iostream>

namespace obmc
{
namespace session
{

using namespace phosphor::logging;

void SessionItem::setSessionMetadata(std::string username,
                                     std::string remoteIPAddr)
{
    this->adjustSessionOwner(username);
    if (remoteIPAddr.empty())
    {
        throw InvalidArgument();
    }
    this->remoteIPAddr(remoteIPAddr);
}

void SessionItem::adjustSessionOwner(const std::string& userName)
{
    using DBusGetObjectOut = std::map<std::string, std::vector<std::string>>;
    using namespace dbus;

    constexpr const std::array userObjectIfaces = {
        "xyz.openbmc_project.User.Attributes"};
    auto userObjectPath = "/xyz/openbmc_project/user/" + userName;

    DBusGetObjectOut getUserObject;
    auto callMethod =
        bus.new_method_call(object_mapper::service, object_mapper::object,
                            object_mapper::interface, object_mapper::getObject);
    callMethod.append(userObjectPath.c_str(), userObjectIfaces);
    bus.call(callMethod).read(getUserObject);

    if (getUserObject.empty())
    {
        throw UnknownUser();
    }

    this->associations({
        make_tuple("user", "session", userObjectPath),
    });
}

const std::string SessionItem::getProcPath() const
{
    return "/proc/" + std::to_string(ownerPid);
}
const std::string SessionItem::getOwner() const
{
    for (const auto assocTuple : associations())
    {
        const std::string assocType = std::get<0>(assocTuple);
        if (assocType != std::string("user"))
        {
            continue;
        }
        const std::string userObjectPath = std::get<2>(assocTuple);

        return retrieveUserFromObjectPath(userObjectPath);
    }

    throw std::logic_error("The username has not been set.");
}

const std::string
    SessionItem::retrieveUserFromObjectPath(const std::string& objectPath)
{
    return dbus::utils::getLastSegmentFromObjectPath(objectPath);
}

SessionIdentifier
    SessionItem::retrieveIdFromObjectPath(const std::string& objectPath)
{
    return SessionManager::parseSessionId(
        dbus::utils::getLastSegmentFromObjectPath(objectPath));
}

} // namespace session
} // namespace obmc
