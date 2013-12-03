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

#include <liboac/buffer/ring.h>
#include <liboac/network/async_server.h>
#include <liboac/network/client.h>
#include <liboac/stream/functions.h>

using namespace oac;
using namespace oac::buffer;
using namespace oac::network;

BOOST_AUTO_TEST_SUITE(AsyncTcpServerTest)

template <typename ConnectionHandler, typename ClientHandler>
void test(
      const ConnectionHandler& conn_handler,
      const ClientHandler& client_handler,
      std::uint32_t millis_before_stop)
{
   auto port = rand() % 7000 + 1025;
   async_tcp_server server(port, conn_handler);
   boost::thread bg_thread([&server]() {
      server.io_service().run();
   });

   {
      tcp_client cli("localhost", port);
      client_handler(cli.connection());
   }

   /* Must wait for the server to respond before stopping it. */
   boost::this_thread::sleep_for(
            boost::chrono::milliseconds(millis_before_stop));

   server.io_service().stop();
   bg_thread.join();
}

void write_msg(
      const async_tcp_connection_ptr& conn,
      const std::string& msg)
{
   auto buff = std::make_shared<ring_buffer>(512);
   stream::write_as_string(*buff, "Hello World!");
   conn->write(*buff, [buff](const attempt<std::size_t>& nbytes) {});
}

template <typename OnReceivedHandler,
          typename StreamBufferPtr>
void receive_msg(
      const async_tcp_connection_ptr& conn,
      const StreamBufferPtr& buff,
      const std::string& expected_msg,
      OnReceivedHandler handler)
{
   conn->read(*buff, [conn, buff, expected_msg, handler](
      const attempt<std::size_t>& nbytes)
  {
      auto expected_len = sizeof(char) * expected_msg.length();
      if (buff->available_for_read() < expected_len)
         receive_msg(conn, buff, expected_msg, handler);
      else
      {
         BOOST_CHECK_EQUAL(
                  expected_msg,
                  stream::read_as_string(*buff, expected_len));
         handler();
      }
  });
}

template <typename OnReceivedHandler,
          typename StreamBufferPtr>
void receive_dwords(
      const async_tcp_connection_ptr& conn,
      const StreamBufferPtr& buff,
      std::size_t expected_dwords_count,
      OnReceivedHandler handler)
{
   auto to_read = expected_dwords_count * sizeof(std::uint32_t);
   conn->read(*buff, [conn, buff, handler, to_read, expected_dwords_count](
         const attempt<std::size_t>& nbytes)
   {
      if (buff->available_for_read() < to_read)
         receive_dwords(conn, buff, expected_dwords_count, handler);
      else
      {
         for (std::uint32_t i = 0; i < expected_dwords_count; i++)
            BOOST_CHECK_EQUAL(i, stream::read_as<std::uint32_t>(*buff));
         handler();
      }
   });
}

BOOST_AUTO_TEST_CASE(ServerCreationMustFailWhenPortIsUnavailable)
{
   auto handler = [](const async_tcp_connection_ptr&){};
   async_tcp_server server(9000, handler);
   BOOST_CHECK_THROW(
            async_tcp_server(9000, handler),
            network::bind_error);

}

BOOST_AUTO_TEST_CASE(ServerShouldServeMessage)
{
   test(
         [](const async_tcp_connection_ptr& conn)
         {
            write_msg(conn, "Hello World!");
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
         [&consumed](const async_tcp_connection_ptr& conn){
            receive_msg(
                  conn,
                  std::make_shared<ring_buffer>(64),
                  "Hello World!",
                  [&consumed]() { consumed = true; });
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
         [&consumed](const async_tcp_connection_ptr& conn){
            receive_msg(
                  conn,
                  std::make_shared<ring_buffer>(64),
                  "Hello World!",
                  [&consumed]() { consumed = true; });
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
         [&consumed](const async_tcp_connection_ptr& conn){
            receive_dwords(
                  conn,
                  std::make_shared<ring_buffer>(65536 * sizeof(std::uint32_t)),
                  65536,
                  [&consumed]() { consumed = true; });
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
