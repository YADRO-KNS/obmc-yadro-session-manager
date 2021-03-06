// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 YADRO

#include <dbus.hpp>
#include <manager.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <session.hpp>
#include <xyz/openbmc_project/Session/Item/client.hpp>
#include <xyz/openbmc_project/Session/Manager/client.hpp>

#include <chrono>
#include <filesystem>
#include <sstream>

namespace obmc
{
namespace session
{

using namespace obmc::dbus;
using namespace phosphor::logging;
using namespace std::chrono_literals;

namespace fs = std::filesystem;

SessionManager::SessionManager(sdbusplus::bus::bus& busIn,
                               boost::asio::io_context& ioc) :
    SessionManagerServer(busIn, sessionManagerObjectPath),
    bus(busIn), ioc(ioc), timer(ioc)
{
    dbusManager = std::make_unique<sdbusplus::server::manager::manager>(
        bus, sessionManagerObjectPath);
    bus.request_name(serviceName);
    timer.expires_from_now(10s);
    timer.async_wait(std::bind(&SessionManager::checkSessionOwnerAlive, this,
                               std::placeholders::_1));
}

std::string SessionManager::create(std::string username,
                                   std::string remoteAddress, SessionType type,
                                   int32_t callerPid)
{
    auto session = this->create(username, remoteAddress, callerPid);
    if (!session)
    {
        throw InvalidArgument();
    }
    session->sessionType(type);
    return session->sessionID();
}

SessionItemPtr SessionManager::create(const std::string& userName,
                                      const std::string& remoteAddress,
                                      pid_t callerPid)
{
    auto sessionId = generateSessionId();

    auto sessionObjectPath = getSessionObjectPath(sessionId);
    auto session =
        std::make_shared<SessionItem>(bus, sessionObjectPath, callerPid);

    session->sessionID(hexSessionId(sessionId));
    session->remoteIPAddr(remoteAddress);

    if (!userName.empty())
    {
        try
        {
            session->adjustSessionOwner(userName);
        }
        catch (const std::exception& ex)
        {
            log<level::DEBUG>(
                "Skip publishing the obmcsess object if user not found",
                entry("USER=%d", userName.c_str()),
                entry("ERROR=%s", ex.what()));
            return SessionItemPtr();
        }
    }

    sessionItems.emplace(sessionId, session);
    return session;
}

uint32_t SessionManager::closeAllByType(SessionType type)
{
    const auto count = std::erase_if(sessionItems, [type](const auto& item) {
        auto const [id, session] = item;
        return session->sessionType() == type;
    });

    return count;
}

void SessionManager::close(std::string sessionId)
{
    log<level::DEBUG>("SessionManager::close()",
                      entry("SESSION-ID=%d", sessionId.c_str()));
    try
    {
        auto numSessId = parseSessionId(sessionId);
        auto foundSessionIt = sessionItems.find(numSessId);
        if (foundSessionIt == sessionItems.end())
        {
            throw InvalidArgument();
        }
        sessionItems.erase(foundSessionIt);
    }
    catch (const std::exception& e)
    {
        log<level::ERR>("Failure to close an obmc session.",
                        entry("SESSION-ID=%s", sessionId.c_str()),
                        entry("ERROR=%s", e.what()));
        throw InternalFailure();
    }
}

std::size_t SessionManager::removeAll(const std::string& userName)
{
    const auto count =
        std::erase_if(sessionItems, [userName](const auto& item) {
            auto const [id, session] = item;
            return session->getOwner() == userName;
        });

    return count;
}

std::size_t
    SessionManager::removeAllByRemoteAddress(const std::string& remoteAddress)
{
    const auto count =
        std::erase_if(sessionItems, [remoteAddress](const auto& item) {
            auto const [id, session] = item;
            return session->remoteIPAddr() == remoteAddress;
        });

    return count;
}

std::size_t SessionManager::removeAll()
{
    size_t handledSessions = sessionItems.size();
    sessionItems.clear();
    return handledSessions;
}

SessionIdentifier SessionManager::generateSessionId() const
{
    auto time = std::chrono::high_resolution_clock::now();
    std::size_t timeHash =
        std::hash<int64_t>{}(time.time_since_epoch().count());
    std::size_t serviceNameHash = std::hash<std::string>{}(serviceName);

    std::size_t result = timeHash ^ (serviceNameHash << 1);
    if (result == invalidSessionId)
    {
        // The hash-collision guard. The Session ID == invalidSessionId is
        // reserved and can't be provided as a valid ID.
        return generateSessionId();
    }

    return result;
}

const std::string
    SessionManager::getSessionObjectPath(SessionIdentifier sessionId) const
{
    return getSessionManagerObjectPath() + "/" + hexSessionId(sessionId);
}

const std::string SessionManager::getSessionManagerObjectPath() const
{
    return std::string(sessionManagerObjectPath);
}

const std::string SessionManager::hexSessionId(SessionIdentifier sessionId)
{
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(sizeof(sessionId) * 2) << std::hex
           << sessionId;
    return stream.str();
}

SessionIdentifier
    SessionManager::parseSessionId(const std::string& hexSessionId)
{
    return std::stoull(hexSessionId, nullptr, 16);
}

void SessionManager::checkSessionOwnerAlive(const boost::system::error_code&)
{
    for (auto it = sessionItems.begin(); it != sessionItems.end();)
    {
        if (it->second && !fs::exists(it->second->getProcPath()))
        {
            const auto sessionId = hexSessionId(it->first);
            log<level::DEBUG>(
                "Found unreachable service",
                entry("SESSION=%s", sessionId.c_str()),
                entry("SVC_PROC=%s", it->second->getProcPath().c_str()));
            it = sessionItems.erase(it);
        }
        else
        {
            ++it;
        }
    }

    timer.expires_from_now(10s);
    timer.async_wait(std::bind(&SessionManager::checkSessionOwnerAlive, this,
                               std::placeholders::_1));
}

} // namespace session
} // namespace obmc
