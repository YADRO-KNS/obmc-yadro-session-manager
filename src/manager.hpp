/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2021 YADRO.
 */

#pragma once

#include <boost/asio.hpp>
#include <dbus.hpp>
#include <xyz/openbmc_project/Session/Item/server.hpp>
#include <xyz/openbmc_project/Session/Manager/server.hpp>

#include <chrono>
namespace obmc
{
namespace session
{

using SessionItemServer =
    sdbusplus::xyz::openbmc_project::Session::server::Item;
using SessionManagerServer =
    sdbusplus::xyz::openbmc_project::Session::server::Manager;

class SessionManager;
class SessionItem;

using SessionItemPtr = std::shared_ptr<SessionItem>;
using SessionManagerPtr = std::shared_ptr<SessionManager>;
using SessionManagerWeakPtr = std::weak_ptr<SessionManager>;

using SessionIdentifier = std::size_t;
using namespace obmc::dbus;

class SessionManager final :
    public SessionManagerServer,
    public std::enable_shared_from_this<SessionManager>
{
    static constexpr const char* serviceName =
        "xyz.openbmc_project.SessionManager";
    static constexpr const char* sessionManagerObjectPath =
        "/xyz/openbmc_project/session_manager";
    static constexpr unsigned int invalidSessionId = 0U;
  public:
    using SessionType = SessionItemServer::Type;

    SessionManager() = delete;
    ~SessionManager() = default;

    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
    SessionManager(SessionManager&&) = delete;
    SessionManager& operator=(SessionManager&&) = delete;

    /** @brief Constructs session manager
     *
     * @param[in] bus     - Handle to system dbus
     * @param[in] ioc     - ASIO context
     */
    SessionManager(sdbusplus::bus::bus& bus, boost::asio::io_context& ioc);

    /** @brief Create a session and publish into the dbus.
     *
     *  @param[in] username             - Owner username of the session
     *  @param[in] remoteAddress        - Remote IP address.
     *  @param[in] type                 - The session type.
     *  @param[in] callerPid            - The PID of the caller to observe the
     *                                    session service owner to cleanup when
     *                                    the service process is down.
     *
     *  @return sessionId[std::string]  - Unique session identificator
     */
    std::string create(std::string username, std::string remoteAddress,
                       SessionType type, int32_t callerPid) override;

    /**
     * @brief Remove all sessions of specified type.
     *
     * @param type         - the type of session to close.
     *
     * @return std::size_t - count of closed sessions
     */
    uint32_t closeAllByType(SessionType type) override;

    /**
     * @brief Close a dbus session by specified ID.
     *
     * @param sessionId     - unique session ID to remove from storage
     */
    void close(std::string sessionId) override;

    /**
     * @brief Cast session id hex view to the SessionIdentifier.
     *
     * @throw std::invalid_argument exception
     * @throw std::out_of_range exception
     *
     * @return SessionIdentifier
     */
    static SessionIdentifier parseSessionId(const std::string&);

    /**
     * @brief Get the Session Object Path object
     *
     * @return const std::string - object path of session for specified
     *         identifier
     */
    const std::string getSessionObjectPath(SessionIdentifier) const;
  protected:
    friend class SessionItem;

    /**
     * @brief Create a session and publish into the dbus.
     *
     * @param userName              - the owner user name
     * @param remoteAddress         - the IP address of the session initiator.
     * @param[in] callerPid         - The PID of the caller to observe the
     *                                session service owner to cleanup when the
     *                                service process is down.
     *
     * @throw logic_error           - Build new session is locked.
     * @return SessionItemPtr       - Pointer to the Item object
     */
    SessionItemPtr create(const std::string& userName,
                          const std::string& remoteAddress, pid_t callerPid);

    /**
     * @brief Remove all sessions associated with the specified user.
     *
     * @param userName      - username to close appropriate sessions
     *
     * @throw std::runtime_error not implemented
     *
     * @return std::size_t  - count of closed sessions
     */
    std::size_t removeAll(const std::string& userName);

    /**
     * @brief Remove all sessions which have been opened from the specified IPv4
     *        address.
     *
     * @param remoteAddress - the IP address of the session initiator..
     *
     * @throw std::runtime_error not implemented
     *
     * @return std::size_t  - count of closed sessions
     */
    std::size_t removeAllByRemoteAddress(const std::string& remoteAddress);

    /**
     * @brief Unconditional removes all opened sessions.
     *
     * @throw std::runtime_error not implemented
     *
     * @return std::size_t - count of closed sessions
     */
    std::size_t removeAll();

    /**
     * @brief Generate new session depened on current session manager slug and
     *        timestamp.
     *
     * @return SessionIdentifier - a new session identifier
     */
    SessionIdentifier generateSessionId() const;

    /**
     * @brief Get the Session Manager Object Path object
     *
     * @return const std::string - object path of session manager for specified
     *         identifier
     */
    const std::string getSessionManagerObjectPath() const;

    /**
     * @brief Cast session identifier to the hex view.
     *
     * @return const std::string - a hex view of session identifier.
     */
    static const std::string hexSessionId(SessionIdentifier);

    /**
     * @brief The callback function to check the session owner process is alive
     *        and cleanup sessions of an unavailable service.
     */
    void checkSessionOwnerAlive(const boost::system::error_code&);
  private:
    sdbusplus::bus::bus& bus;
    boost::asio::io_context& ioc;
    std::unique_ptr<sdbusplus::server::manager::manager> dbusManager;

    using SessionItemDict = std::map<SessionIdentifier, SessionItemPtr>;
    SessionItemDict sessionItems;
    boost::asio::steady_timer timer;
};
} // namespace session
} // namespace obmc
