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

#include <functional>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "network.h"

using namespace oac;

/*
BOOST_AUTO_TEST_SUITE(TcpServerTest)

void test(const std::function<void(const ptr<tcp_connection>&)>& server_handler,
          const std::function<void(tcp_connection&)>& client_handler)
{
   auto server = network::make_tcp_server(9000, server_handler);
   server->run_in_background();

   {
      tcp_client cli("localhost", 9000);
      client_handler(cli.connection());
   }

   server->stop();
}

BOOST_AUTO_TEST_CASE(ServerShouldServeMessage)
{
   test(
         [](const ptr<tcp_connection>& conn){
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
         [](const ptr<tcp_connection>& conn){
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
         [](const ptr<tcp_connection>& conn){
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
         [](const ptr<tcp_connection>& conn){
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
         [](const ptr<tcp_connection>& conn){
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
         [](const ptr<tcp_connection>& conn){
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
         [](const ptr<tcp_connection>& conn){
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
*/


BOOST_AUTO_TEST_SUITE(AsyncTcpServerTest)

typedef std::function<
      void(const async_tcp_connection::ptr_type&)> connection_handler;

void test(
      const connection_handler& conn_handler,
      const std::function<void(tcp_connection&)>& client_handler,
      std::uint32_t millis_before_stop)
{
   async_tcp_server<connection_handler> server(9000, conn_handler);
   server.run_in_background();

   {
      tcp_client cli("localhost", 9000);
      client_handler(cli.connection());
   }

   /* Must wait for the server to respond before stopping it. */
   boost::this_thread::sleep_for(boost::chrono::milliseconds(millis_before_stop));
   server.stop();
}

BOOST_AUTO_TEST_CASE(ServerShouldServeMessage)
{
   test(
         [](const async_tcp_connection::ptr_type& conn)
         {
            stream::write_as_string(*conn->output(), "Hello World!");
         },
         [](tcp_connection& conn){
            BOOST_CHECK_EQUAL("Hello World!",
                              stream::read_as_string(*conn.input(), 12));
         },
         0
   );
}

BOOST_AUTO_TEST_CASE(ServerShouldConsumeMessage)
{
   bool consumed = false;
   test(
         [&consumed](const async_tcp_connection::ptr_type& conn){
            conn->read([&consumed](
                  buffer_input_stream<fixed_buffer>& input,
                  std::size_t nbytes)
            {
               if (!nbytes)
                  return 0; // Correct, eof
               BOOST_CHECK_EQUAL(12, nbytes);
               BOOST_CHECK_EQUAL("Hello World!",
                                 stream::read_as_string(input, 12));
               consumed = true;
               return 12;
            });
         },
         [](tcp_connection& conn){
            stream::write_as_string(*conn.output(), "Hello World!");
         },
         200
   );
   BOOST_CHECK(consumed);
}

BOOST_AUTO_TEST_CASE(ServerShouldConsumePartedMessage)
{
   bool consumed = false;
   test(
         [&consumed](const async_tcp_connection::ptr_type& conn){
            conn->read([&consumed](
                  buffer_input_stream<fixed_buffer>& input,
                  std::size_t nbytes) -> std::size_t
            {
               if (nbytes < 12)
                  return 0;
               BOOST_CHECK_EQUAL(12, nbytes);
               BOOST_CHECK_EQUAL(
                     "Hello World!", stream::read_as_string(input, 12));
               consumed = true;
               return 12;
            });
         },
         [](tcp_connection& conn){
            stream::write_as_string(*conn.output(), "Hello ");
            boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
            stream::write_as_string(*conn.output(), "World!");
         },
         200
   );
   BOOST_CHECK(consumed);
}

BOOST_AUTO_TEST_CASE(ServerShouldConsumeBigMessage)
{
   bool consumed = false;
   test(
         [&consumed](const async_tcp_connection::ptr_type& conn){
            conn->read([&consumed](
                  buffer_input_stream<fixed_buffer>& input,
                  std::size_t nbytes) -> std::size_t
            {
               auto to_read = 65536 * sizeof(std::uint32_t);
               if (nbytes < to_read)
                  return 0;
               for (std::uint32_t i = 0; i < 65536; i++)
                  BOOST_CHECK_EQUAL(i, stream::read_as<std::uint32_t>(input));
               consumed = true;
               return to_read;
            });
         },
         [](tcp_connection& conn){
            for (std::uint32_t i = 0; i < 65536; i++)
               stream::write_as(*conn.output(), i);
         },
         200
   );
   BOOST_CHECK(consumed);
}

BOOST_AUTO_TEST_SUITE_END()
