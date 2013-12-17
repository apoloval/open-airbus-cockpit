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

#include <liboac/thread/task.h>

using namespace oac;

BOOST_AUTO_TEST_SUITE(ThreadTaskTest)

struct let_test
{

   OAC_DECL_EXCEPTION(foobar_exception, oac::exception, "foobar");

   ~let_test()
   {
      stop_loop();
   }

   let_test& submit_success_task_by_callback()
   {
      _executor.submit_task<int>(
            [this]() { return 123; },
            [this](const util::attempt<int>& n) { _number = n.get_value(); });
      return *this;
   }

   let_test& submit_failing_task_by_callback()
   {
      _executor.submit_task<int>(
            [this]() -> int { OAC_THROW_EXCEPTION(foobar_exception()); },
            [this](const util::attempt<int>& n)
            {
               try { n.get_value(); }
               catch (const foobar_exception& e)
               { _error = std::make_exception_ptr(e); }
            });
      return *this;
   }

   let_test& submit_success_task_by_future()
   {
      _number = _executor.submit_task<int>([this]() { return 123; }).get();
      return *this;
   }

   let_test& submit_failing_task_by_future()
   {
      try
      {
         _executor.submit_task<int>(
            [this]() -> int { OAC_THROW_EXCEPTION(foobar_exception()); }
         ).get();
      }
      catch (...) { _error = std::current_exception(); }
      return *this;
   }

   let_test& loop_in_background()
   {
      _loop_thread = std::thread([this]() {  _executor.loop(); });
      return *this;
   }

   let_test& check_task_is_done()
   {
      sleep(50);
      BOOST_CHECK_EQUAL(123, _number);
      return *this;
   }

   let_test& check_task_failed()
   {
      sleep(50);
      BOOST_CHECK(_error);
      BOOST_CHECK_THROW(std::rethrow_exception(_error), foobar_exception);
      return *this;
   }

   let_test& stop_loop()
   {
      _executor.stop_loop();
      _loop_thread.join();
      return *this;
   }

   let_test& submit_tasks(int ntasks)
   {
      if (ntasks)
      {
         auto fnumber = _executor.submit_task<int>(
            [ntasks]() { return ntasks; }
         );
         submit_tasks(ntasks - 1);
         BOOST_CHECK_EQUAL(ntasks, fnumber.get());
      }
      return *this;
   }

private:

   thread::task_executor _executor;
   std::atomic_int _number;
   std::exception_ptr _error;
   std::thread _loop_thread;

   void sleep(long millis)
   {
      std::this_thread::sleep_for(std::chrono::milliseconds(millis));
   }
};

BOOST_AUTO_TEST_CASE(MustExecuteSuccessActionByCallback)
{
   let_test()
      .loop_in_background()
      .submit_success_task_by_callback()
      .check_task_is_done();
}

BOOST_AUTO_TEST_CASE(MustExecuteFailingActionByCallback)
{
   let_test()
      .loop_in_background()
      .submit_failing_task_by_callback()
      .check_task_failed();
}

BOOST_AUTO_TEST_CASE(MustExecuteSuccessActionByFuture)
{
   let_test()
      .loop_in_background()
      .submit_success_task_by_future()
      .check_task_is_done();
}

BOOST_AUTO_TEST_CASE(MustExecuteFailingActionByFuture)
{
   let_test()
      .loop_in_background()
      .submit_failing_task_by_future()
      .check_task_failed();
}

BOOST_AUTO_TEST_CASE(MustExecuteBunchOfActions)
{
   let_test()
      .loop_in_background()
      .submit_tasks(1000);
}

BOOST_AUTO_TEST_SUITE_END()
