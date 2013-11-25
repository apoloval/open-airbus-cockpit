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

#include <liboac/thread/monitor.h>

using namespace oac;

BOOST_AUTO_TEST_SUITE(MonitorTest)

struct counter
{
   int value;
   counter() : value(0) {}

   int inc() { return value++; }
   int dec() { return value--; }
};

BOOST_AUTO_TEST_CASE(MustNotCauseConcurrencyIssues)
{
   static const int loop_to = 1000000;
   thread::monitor<counter> c;
   std::thread t1([&c](){
      for (int i = 0; i < loop_to; i++)
         c->inc();
   });
   std::thread t2([&c](){
      for (int i = 0; i < loop_to; i++)
         c->dec();
   });
   t1.join();
   t2.join();
   BOOST_CHECK_EQUAL(0, c->value);
}

BOOST_AUTO_TEST_CASE(MustNotCauseConcurrencyIssuesEvenWithConcurrentLocks)
{
   static const int loop_to = 1000000;
   thread::monitor<counter> c;
   std::thread t1([&c](){
      for (int i = 0; i < loop_to; i++)
         auto j = c->inc() + c->dec();
   });
   std::thread t2([&c](){
      for (int i = 0; i < loop_to; i++)
         auto j = c->dec() + c->inc();
   });
   t1.join();
   t2.join();
   BOOST_CHECK_EQUAL(0, c->value);
}

BOOST_AUTO_TEST_SUITE_END()
