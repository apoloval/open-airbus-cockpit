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

#include <thread>

#include <liboac/thread/channel.h>

using namespace oac;

BOOST_AUTO_TEST_SUITE(ChannelTest)

struct let_test
{

   ~let_test()
   {
      _reader.join();
   }

   template< class Rep, class Period >
   let_test& set_timeout(const std::chrono::duration<Rep,Period>& timeout)
   {
      _chan.set_timeout(timeout);
      return *this;
   }

   let_test& write_message(int msg)
   {
      _chan << msg;
      return *this;
   }

   let_test& read(int expected_message)
   {
      _reader = std::thread([this, expected_message]()
      {
         BOOST_CHECK_EQUAL(expected_message, _chan.read());
      });
      return *this;
   }

   template< class Rep, class Period >
   let_test& read_for(
         int expected_message,
         const std::chrono::duration<Rep,Period>& timeout)
   {
      _reader = std::thread([this, expected_message, timeout]()
      {
         BOOST_CHECK_EQUAL(expected_message, _chan.read_for(timeout));
      });
      return *this;
   }

   template< class Rep, class Period >
   let_test& read_for_and_timeout(
         const std::chrono::duration<Rep,Period>& timeout)
   {
      _reader = std::thread([this, timeout]()
      {
         BOOST_CHECK_THROW(
               _chan.read_for(timeout),
               thread::channel_timeout_error);
      });
      return *this;
   }

private:

   thread::channel<int> _chan;
   int _received_message;
   std::thread _reader;
};

BOOST_AUTO_TEST_CASE(MustReceiveMessageAlreadyInMailbox)
{
   let_test()
         .write_message(1234)
         .read_for(1234, std::chrono::milliseconds(50));
}

BOOST_AUTO_TEST_CASE(MustWaitForMessageDeliveredAfterReceptionStart)
{
   let_test()
         .read_for(1234, std::chrono::milliseconds(50))
         .write_message(1234);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenNoMessageIsDeliverdAndTimedOut)
{
   let_test()
         .read_for_and_timeout(std::chrono::milliseconds(50));
}

BOOST_AUTO_TEST_CASE(MustHonourConfiguredTimeout)
{
   let_test()
         .set_timeout(std::chrono::milliseconds(50))
         .read(1234)
         .write_message(1234);
}

BOOST_AUTO_TEST_SUITE_END()
