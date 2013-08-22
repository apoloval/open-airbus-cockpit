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

#include <atomic>
#include <functional>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "network.h"

using namespace oac;

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



BOOST_AUTO_TEST_SUITE(TcpServerTest)

void test(const std::function<void(const ptr<tcp_connection>&)>& server_handler,
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

BOOST_AUTO_TEST_SUITE(AsyncTcpClientTest)

struct let_test
{

   let_test()
      : _io_srv(std::make_shared<boost::asio::io_service>())
   {
   }

   let_test& connect(const network::hostname& hostname, network::tcp_port port)
   {
      _client = std::unique_ptr<async_tcp_client>(
            new async_tcp_client(hostname, port, _io_srv));
      return *this;
   }

   let_test& close()
   {
      _client.reset();
      return *this;
   }

   let_test& send_http_get()
   {
      linear_buffer output_buffer(4096);

      auto on_write = [](
            const boost::system::error_code& ec,
            std::size_t nbytes)
      {
         BOOST_CHECK(!ec);
         BOOST_CHECK_EQUAL(15+21+27+1, nbytes);
      };


      stream::write_as_string(output_buffer, "GET / HTTP/1.1\n");
      stream::write_as_string(output_buffer, "Host: www.google.com\n");
      stream::write_as_string(output_buffer, "User-Agent: liboac-network\n");
      stream::write_as_string(output_buffer, "\n");

      _client->connection().write(output_buffer, on_write);
      _io_srv->reset();
      _io_srv->run();
      return *this;
   }

   let_test& send_http_get_with_future()
   {
      linear_buffer output_buffer(4096);

      stream::write_as_string(output_buffer, "GET / HTTP/1.1\n");
      stream::write_as_string(output_buffer, "Host: www.google.com\n");
      stream::write_as_string(output_buffer, "User-Agent: liboac-network\n");
      stream::write_as_string(output_buffer, "\n");

      auto result =_client->connection().write(output_buffer);

      _io_srv->reset();
      _io_srv->run();

      BOOST_CHECK_EQUAL(15+21+27+1, result.get());

      return *this;
   }

   let_test& receive_http_response()
   {
      linear_buffer input_buffer(4096);

      auto on_read = [](
            const boost::system::error_code& ec,
            std::size_t nbytes)
      {
      };

      _client->connection().read(input_buffer, on_read);
      _io_srv->reset();
      _io_srv->run();

      BOOST_CHECK_EQUAL(
         "HTTP/1.1 302",
         stream::read_as_string(input_buffer, 12));

      return *this;
   }

   let_test& receive_http_response_with_future()
   {
      linear_buffer input_buffer(4096);

      auto result = _client->connection().read(input_buffer);
      _io_srv->reset();
      _io_srv->run();

      result.get();

      BOOST_CHECK_EQUAL(
         "HTTP/1.1 302",
         stream::read_as_string(input_buffer, 12));

      return *this;
   }

private:

   std::shared_ptr<boost::asio::io_service> _io_srv;
   std::unique_ptr<async_tcp_client> _client;
};

BOOST_AUTO_TEST_CASE(MustConnectToExistingHostWithAvailablePort)
{
   let_test()
         .connect("www.google.com", 80)
         .send_http_get()
         .receive_http_response()
         .close();
}

BOOST_AUTO_TEST_CASE(MustConnectToExistingHostWithAvailablePortUsingFutures)
{
   let_test()
         .connect("www.google.com", 80)
         .send_http_get_with_future()
         .receive_http_response_with_future()
         .close();
}

BOOST_AUTO_TEST_CASE(MustFailWhileConnectingToUnexistingHost)
{
   BOOST_CHECK_THROW(
         let_test()
               .connect("www.bartolohizocaca.com", 80), // hope it never exists
         network::connection_refused);
}

BOOST_AUTO_TEST_CASE(MustFailWhileConnectingToUnavailablePort)
{
   BOOST_CHECK_THROW(
         let_test()
               .connect("localhost", 1234),
         network::connection_refused);
}

BOOST_AUTO_TEST_SUITE_END()



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
      const async_tcp_connection::ptr_type& conn,
      const std::string& msg)
{
   auto buff = ring_buffer::create(512);
   stream::write_as_string(*buff, "Hello World!");
   conn->write(*buff, [buff](
      const boost::system::error_code& ec,
      std::size_t nbytes)
   {

   });
}

template <typename OnReceivedHandler,
          typename StreamBufferPtr>
void receive_msg(
      const async_tcp_connection::ptr_type& conn,
      const StreamBufferPtr& buff,
      const std::string& expected_msg,
      OnReceivedHandler handler)
{
   conn->read(*buff, [conn, buff, expected_msg, handler](
      const boost::system::error_code& ec,
      std::size_t nbytes)
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
      const async_tcp_connection::ptr_type& conn,
      const StreamBufferPtr& buff,
      std::size_t expected_dwords_count,
      OnReceivedHandler handler)
{
   auto to_read = expected_dwords_count * sizeof(std::uint32_t);
   conn->read(*buff, [conn, buff, handler, to_read, expected_dwords_count](
         const boost::system::error_code& ec,
         std::size_t nbytes)
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
   auto handler = [](const async_tcp_connection::ptr_type&){};
   async_tcp_server server(9000, handler);
   BOOST_CHECK_THROW(
            async_tcp_server(9000, handler),
            network::bind_error);

}

BOOST_AUTO_TEST_CASE(ServerShouldServeMessage)
{
   test(
         [](const async_tcp_connection::ptr_type& conn)
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
         [&consumed](const async_tcp_connection::ptr_type& conn){
            receive_msg(
                  conn,
                  ring_buffer::create(64),
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
         [&consumed](const async_tcp_connection::ptr_type& conn){
            receive_msg(
                  conn,
                  ring_buffer::create(64),
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
         [&consumed](const async_tcp_connection::ptr_type& conn){
            receive_dwords(
                  conn,
                  ring_buffer::create(65536 * sizeof(std::uint32_t)),
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
