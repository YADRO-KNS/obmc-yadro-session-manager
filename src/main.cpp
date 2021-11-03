// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 YADRO

#include <boost/asio.hpp>
#include <manager.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/asio/connection.hpp>

#include <iostream>

using namespace phosphor::logging;
using namespace sdbusplus::asio;

int main()
{
    boost::asio::io_context io;

    sdbusplus::asio::connection systemConn(io);
    auto sessionManager =
        std::make_shared<obmc::session::SessionManager>(systemConn, io);

    log<level::DEBUG>("io.run()");
    io.run();

    log<level::DEBUG>("Shutdown service 'session-manager'");
    return EXIT_SUCCESS;
}
