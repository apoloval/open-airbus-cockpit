/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012, 2013 Alvaro Polo
 *
 * Open Airbus Cockpit is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Open Airbus Cockpit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <liboac/network/client.h>
#include <liboac/stream/functions.h>

using namespace oac;
using namespace oac::network;

BOOST_AUTO_TEST_SUITE(TcpClientTest)

BOOST_AUTO_TEST_CASE(MustConnectToExistingHostWithAvailablePort)
{
   tcp_client cli("www.google.com", 80);
   stream::write_as_string(*cli.output(), "GET / HTTP/1.1\n");
   stream::write_as_string(*cli.output(), "Host: www.google.com\n");
   stream::write_as_string(*cli.output(), "User-Agent: liboac-network\n");
   stream::write_as_string(*cli.output(), "\n");

   BOOST_CHECK_EQUAL(
         "HTTP/1.1 302",
         stream::read_as_string(*cli.input(), 12));
}

BOOST_AUTO_TEST_CASE(MustFailWhileConnectingToUnexistingHost)
{
   BOOST_CHECK_THROW(
         tcp_client("www.bartolohizocaca.com", 80), // hope it never exists
         network::connection_refused);
}

BOOST_AUTO_TEST_CASE(MustFailWhileConnectingToUnavailablePort)
{
   BOOST_CHECK_THROW(
         tcp_client("localhost", 1234),
         network::connection_refused);
}

BOOST_AUTO_TEST_SUITE_END()
