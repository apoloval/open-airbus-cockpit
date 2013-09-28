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

#include <liboac/buffer/linear.h>
#include <liboac/network/async_client.h>
#include <liboac/stream/functions.h>

using namespace oac;
using namespace oac::buffer;
using namespace oac::network;

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
            const attempt<std::size_t>& nbytes)
      {
         BOOST_CHECK_EQUAL(15+21+27+1, nbytes.get_value());
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

      auto on_read = [](const attempt<std::size_t>& nbytes) {};

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
