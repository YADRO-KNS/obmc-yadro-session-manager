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
    // skip publish session for the `root`
    if (userName == "root")
    {
        throw UnknownUser();
    }
    this->username(userName);
}

const std::string SessionItem::getProcPath() const
{
    return "/proc/" + std::to_string(ownerPid);
}
const std::string SessionItem::getOwner() const
{
    return this->username();
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
