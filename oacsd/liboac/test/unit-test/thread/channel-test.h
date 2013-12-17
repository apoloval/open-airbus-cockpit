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
      for (auto& reader : _readers)
      {
         if (reader.get_id() != std::thread::id {} && reader.joinable())
            reader.join();
	   }
   }

   template <class Rep, class Period>
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

   template <class Rep, class Period>
   let_test& write_message_and_wait(
         int msg,
         const std::chrono::duration<Rep,Period>& time)
   {
      write_message(msg);
      std::this_thread::sleep_for(time);
      return *this;
   }

   let_test& read(
         int expected_message,
         const thread::channel<int>::message_predicate& pred =
               thread::channel<int>::any_message_pred)
   {
      _readers.push_back(std::thread([this, expected_message, pred]()
      {
         int msg;
         BOOST_CHECK_NO_THROW(msg = _chan.read(pred));
         BOOST_CHECK_EQUAL(expected_message, msg);
      }));
      return *this;
   }

   template <class Rep, class Period>
   let_test& read_for(
         int expected_message,
         const std::chrono::duration<Rep,Period>& timeout,
         const thread::channel<int>::message_predicate& pred =
               thread::channel<int>::any_message_pred)
   {
      _readers.push_back(std::thread([this, expected_message, timeout, pred]()
      {
         BOOST_CHECK_EQUAL(expected_message, _chan.read_for(timeout, pred));
      }));
      return *this;
   }

   template <class Rep, class Period>
   let_test& read_for_and_timeout(
         const std::chrono::duration<Rep,Period>& timeout,
         const thread::channel<int>::message_predicate& pred =
               thread::channel<int>::any_message_pred)
   {
      using namespace std::chrono;
      _readers.push_back(std::thread([this, timeout, pred]()
      {
         auto t1 = steady_clock::now();
         BOOST_CHECK_THROW(
               _chan.read_for(timeout, pred),
               thread::channel_timeout_error);
         auto t2 = steady_clock::now();
         auto elapsed = duration_cast<std::chrono::duration<Rep,Period>>(t2 - t1);
         // elapsed time is not greater than timeout + 15ms
         BOOST_CHECK_GE(15, std::abs(elapsed.count() - timeout.count()));

      }));
      return *this;
   }

private:

   thread::channel<int> _chan;
   int _received_message;
   std::list<std::thread> _readers;
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

BOOST_AUTO_TEST_CASE(MustThrowWhenMessageWasAlreadyReadAndNoMoreAreDelivered)
{
   let_test()
      .write_message(1234)
      .read_for(1234, std::chrono::milliseconds(50))
      .read_for_and_timeout(std::chrono::milliseconds(50));
}

BOOST_AUTO_TEST_CASE(MustHonourConfiguredTimeout)
{
   let_test()
         .set_timeout(std::chrono::milliseconds(50))
         .read(1234)
         .write_message(1234);
}

BOOST_AUTO_TEST_CASE(MustIgnoreFilteredMessages)
{
   let_test()
         .write_message(20)
         .write_message(30)
         .write_message(40)
         .write_message(1234)
         .read(1234, [](const int& msg) { return msg > 1024; });
}

BOOST_AUTO_TEST_CASE(MustFilterButNotDropMessages)
{
   let_test()
         .write_message(20)
         .write_message(30)
         .write_message(40)
         .write_message(1234)
         .read(1234, [](const int& msg) { return msg > 1024; })
         .read(20, [](const int& msg) { return msg < 1024; });
}

BOOST_AUTO_TEST_CASE(MustHonourConfiguredWhileIgnoringMessages)
{
   let_test()
         .read_for_and_timeout(std::chrono::milliseconds(50), [](const int& msg)
         {
            return msg > 1024;
         })
         .write_message_and_wait(20, std::chrono::milliseconds(20))
         .write_message_and_wait(30, std::chrono::milliseconds(20))
         .write_message_and_wait(40, std::chrono::milliseconds(20))
         .write_message_and_wait(50, std::chrono::milliseconds(20))
         .write_message_and_wait(60, std::chrono::milliseconds(20));
}

BOOST_AUTO_TEST_SUITE_END()
