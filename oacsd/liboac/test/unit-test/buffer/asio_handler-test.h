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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include <boost/thread.hpp>

#include <liboac/buffer/asio_handler.h>
#include <liboac/util/attempt.h>

using namespace oac;
using namespace oac::buffer;

BOOST_AUTO_TEST_SUITE(BufferASIOHandlerTest)

struct let_test
{

   let_test() : _result(0), _index_inc(0)
   {}

   let_test& handle_success(std::size_t nbytes)
   {
      handle(boost::system::error_code(), nbytes);
      return *this;
   }

   let_test& handle_failure(int error)
   {
      handle(
            boost::system::error_code(
                  error,
                  boost::system::system_category()),
            0);
      return *this;
   }

   let_test& assert_bytes_processed(std::size_t nbytes)
   {
      BOOST_CHECK_EQUAL(nbytes, _result.get_value());
      return *this;
   }

   let_test& assert_index_increment(std::size_t nbytes)
   {
      BOOST_CHECK_EQUAL(nbytes, _index_inc);
      return *this;
   }

   template <typename Exception>
   let_test& assert_failed_with()
   {
      BOOST_CHECK_THROW(_result.get_value(), Exception);
      return *this;
   }

private:

   void handle(
         const boost::system::error_code& ec,
         std::size_t nbytes)
   {
      auto io_handler = make_io_handler(
            std::bind(&let_test::handler, this, std::placeholders::_1),
            std::bind(&let_test::update_index, this, std::placeholders::_1));
      io_handler(ec, nbytes);
   }

   void handler(const util::attempt<std::size_t>& result)
   {
      _result = result;
   }

   void update_index(std::size_t inc)
   {
      _index_inc = inc;
   }

   util::attempt<std::size_t> _result;
   std::size_t _index_inc;
};

BOOST_AUTO_TEST_CASE(MustHandleSuccessOperation)
{
   let_test()
      .handle_success(200)
      .assert_bytes_processed(200)
      .assert_index_increment(200);
}

BOOST_AUTO_TEST_CASE(MustHandleEndOfFile)
{
   let_test()
      .handle_failure(boost::asio::error::eof)
      .assert_failed_with<io::eof_error>()
      .assert_index_increment(0);
}

BOOST_AUTO_TEST_CASE(MustHandleConnectionAborted)
{
   let_test()
      .handle_failure(boost::asio::error::connection_aborted)
      .assert_failed_with<network::connection_reset>()
      .assert_index_increment(0);
}

BOOST_AUTO_TEST_CASE(MustHandleConnectionReset)
{
   let_test()
      .handle_failure(boost::asio::error::connection_reset)
      .assert_failed_with<network::connection_reset>()
      .assert_index_increment(0);
}

BOOST_AUTO_TEST_CASE(MustHandleOtherFailures)
{
   let_test()
      .handle_failure(boost::asio::error::connection_refused)
      .assert_failed_with<io::boost_asio_error>()
      .assert_index_increment(0);
}

BOOST_AUTO_TEST_SUITE_END()
