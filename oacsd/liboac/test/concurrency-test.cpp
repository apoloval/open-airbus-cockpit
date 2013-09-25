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

#include <liboac/concurrency.h>

// using namespace oac;

BOOST_AUTO_TEST_SUITE(ASyncServiceTest)

int get_seven(void)
{
   std::this_thread::sleep_for(std::chrono::seconds(1));
   return 7;
}

int power_of_two(int i) { return i*i; }

int multiply(int a, int b)
{
   return a*b;
}

BOOST_AUTO_TEST_CASE(ShouldExecuteFunctionWithArityZero)
{
   oac::async_executor srv;
   auto result = srv.execute(&get_seven);
   srv.run_in_background();

   BOOST_CHECK_EQUAL(7, result.get());
   srv.stop();
}

BOOST_AUTO_TEST_CASE(ShouldExecuteLambdaWithArityZero)
{
   oac::async_executor srv;
   auto result = srv.execute([]() { return 7; });
   srv.run_in_background();

   BOOST_CHECK_EQUAL(7, result.get());
   srv.stop();
}

BOOST_AUTO_TEST_CASE(ShouldExecuteFunctionWithArityOne)
{
   oac::async_executor srv;
   auto result = srv.execute(power_of_two, 16);
   srv.run_in_background();

   BOOST_CHECK_EQUAL(256, result.get());
   srv.stop();
}

BOOST_AUTO_TEST_CASE(ShouldExecuteLambdaWithArityOne)
{
   oac::async_executor srv;
   auto result = srv.execute([](int i) { return i*i; }, 16);
   srv.run_in_background();

   BOOST_CHECK_EQUAL(256, result.get());
   srv.stop();
}

BOOST_AUTO_TEST_CASE(ShouldExecuteFunctionWithArityTwo)
{
   oac::async_executor srv;
   auto result = srv.execute(multiply, 4, 3);
   srv.run_in_background();

   BOOST_CHECK_EQUAL(12, result.get());
   srv.stop();
}

BOOST_AUTO_TEST_CASE(ShouldExecuteLambdaWithArityTwo)
{
   oac::async_executor srv;
   auto result = srv.execute([](int a, int b) { return a*b; }, 4, 3);
   srv.run_in_background();

   BOOST_CHECK_EQUAL(12, result.get());
   srv.stop();
}

BOOST_AUTO_TEST_SUITE_END()
