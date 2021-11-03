// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 YADRO

#pragma once

#include <unistd.h>

#include <manager.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <xyz/openbmc_project/Association/Definitions/server.hpp>

namespace obmc
{
namespace session
{

using namespace phosphor::logging;

using SessionItemServerObject =
    sdbusplus::server::object::object<SessionItemServer>;
using AssocDefinitionServerObject = sdbusplus::server::object::object<
    sdbusplus::xyz::openbmc_project::Association::server::Definitions>;

class UnknownUser : public std::logic_error
{
    static constexpr auto errDesc = "Unkown username was given.";

  public:
    UnknownUser() : std::logic_error(errDesc)
    {}
    ~UnknownUser() override = default;
};

class SessionItem :
    public SessionItemServerObject,
    public AssocDefinitionServerObject
{
  public:
    SessionItem() = delete;
    SessionItem(const SessionItem&) = delete;
    SessionItem& operator=(const SessionItem&) = delete;
    SessionItem(SessionItem&&) = default;
    SessionItem& operator=(SessionItem&&) = default;

    /** @brief Constructs dbus-object-server of Session Item.
     *
     * @param[in] bus               - Handle to system dbus
     * @param[in] objPath           - The Dbus path that hosts Session Item.
     * @param[in] managerPtr        - The pointer of manager.
     */
    SessionItem(sdbusplus::bus::bus& bus, const std::string& objPath,
                pid_t ownerPid) :
        SessionItemServerObject(bus, objPath.c_str()),
        AssocDefinitionServerObject(bus, objPath.c_str()), bus(bus),
        path(objPath), ownerPid(ownerPid)
    {
        // Nothing to do here
    }

    ~SessionItem() override
    {}

    /** @brief Set Username and Remote IP addres of exist session.
     *
     *  @param[in] username         - Owner username of the session
     *  @param[in] remoteIPAddr     - Remote IP address.
     */
    void setSessionMetadata(std::string username, std::string remoteIPAddr);

    /**
     * @brief Associate user of specified username with the current session.
     *
     * @param userName          - the user name to associate with the
     *                            current session.
     *
     * @throw std::exception    failure on set user object relation to
     *                          the current session
     */
    void adjustSessionOwner(const std::string& userName);

    /**
     * @brief Get the full proc path of session owner process.
     *
     * @return const std::string    - full path to the process: /proc/[pid]
     */
    const std::string getProcPath() const;

    /**
     * @brief Get the session owner username
     *
     * @throw logic_error       - the username has not been set.
     *
     * @return std::string      - the session username
     */
    const std::string getOwner() const;

    static const std::string
        retrieveUserFromObjectPath(const std::string& objectPath);

    static SessionIdentifier
        retrieveIdFromObjectPath(const std::string& objectPath);

  private:
    sdbusplus::bus::bus& bus;
    /** @brief Path of the group instance */
    const std::string path;
    pid_t ownerPid;
};

} // namespace session
} // namespace obmc
