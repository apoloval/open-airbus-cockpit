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
#include <liboac/network/server.h>
#include <liboac/stream/functions.h>

using namespace oac;
using namespace oac::network;

BOOST_AUTO_TEST_SUITE(TcpServerTest)

void test(const std::function<void(const tcp_connection_ptr&)>& server_handler,
          const std::function<void(tcp_connection&)>& client_handler)
{
   auto port = rand() % 7000 + 1025;
   auto server = network::make_tcp_server(port, server_handler);
   server->run_in_background();

   {
      tcp_client cli("localhost", port);
      client_handler(cli.connection());
   }

   // Sleep a little while to give time to the server to respond
   boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
   server->stop();
}

BOOST_AUTO_TEST_CASE(ServerCreationMustFailWhenPortIsUnavailable)
{
   auto server_1 = network::make_tcp_server(9000, [](tcp_connection&){});
   BOOST_CHECK_THROW(
         network::make_tcp_server(9000, [](tcp_connection&){}),
         network::bind_error);
}

BOOST_AUTO_TEST_CASE(ServerShouldServeMessage)
{
   test(
         [](const tcp_connection_ptr& conn){
            stream::write_as_string(*conn->output(), "Hello World!");
         },
         [](tcp_connection& conn){
            BOOST_CHECK_EQUAL("Hello World!",
                              stream::read_as_string(*conn.input(), 12));
         }
   );
}

BOOST_AUTO_TEST_CASE(ServerShouldConsumeMessage)
{
   test(
         [](const tcp_connection_ptr& conn){
            BOOST_CHECK_EQUAL("Hello World!",
                              stream::read_as_string(*conn->input(), 12));
         },
         [](tcp_connection& conn){
            stream::write_as_string(*conn.output(), "Hello World!");
         }
   );
}

BOOST_AUTO_TEST_CASE(ServerShouldEchoMessage)
{
   test(
         [](const tcp_connection_ptr& conn){
            auto msg = stream::read_as_string(*conn->input(), 12);
            BOOST_CHECK_EQUAL("Hello World!", msg);
            stream::write_as_string(*conn->output(), msg);
         },
         [](tcp_connection& conn){
            std::string msg("Hello World!");
            stream::write_as_string(*conn.output(), msg);
            BOOST_CHECK_EQUAL(msg, stream::read_as_string(*conn.input(), 12));
         }
   );
}

BOOST_AUTO_TEST_CASE(ServerShouldDetectEmptyMessage)
{
   test(
         [](const tcp_connection_ptr& conn){
            std::uint8_t buff[12];
            auto nread = conn->input()->read(buff, 12);
            BOOST_CHECK_EQUAL(0, nread);
         },
         [](tcp_connection& conn){
         }
   );
}

BOOST_AUTO_TEST_CASE(ServerShouldDetectIncompleteMessage)
{
   test(
         [](const tcp_connection_ptr& conn){
            std::uint8_t buff[12];
            auto nread = conn->input()->read(buff, 12);
            BOOST_CHECK_EQUAL(5, nread);
         },
         [](tcp_connection& conn){
            stream::write_as_string(*conn.output(), "Hello");
         }
   );
}

BOOST_AUTO_TEST_CASE(ClientShouldDetectEmptyMessage)
{
   test(
         [](const tcp_connection_ptr& conn){
         },
         [](tcp_connection& conn){
            std::uint8_t buff[12];
            auto nread = conn.input()->read(buff, 12);
            BOOST_CHECK_EQUAL(0, nread);
         }
   );
}

BOOST_AUTO_TEST_CASE(ClientShouldDetectIncompleteMessage)
{
   test(
         [](const tcp_connection_ptr& conn){
            stream::write_as_string(*conn->output(), "Hello");
         },
         [](tcp_connection& conn){
            std::uint8_t buff[12];
            auto nread = conn.input()->read(buff, 12);
            BOOST_CHECK_EQUAL(5, nread);
         }
   );
}

BOOST_AUTO_TEST_SUITE_END()
