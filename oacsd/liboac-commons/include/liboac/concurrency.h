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
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OAC_CONCURRENCY_H
#define OAC_CONCURRENCY_H

#include <functional>
#include <future>
#include <queue>

#include <boost/thread.hpp>

namespace oac {

class async_executor
{
public:

   async_executor()
   { reset(); }

   void reset() { _must_run = true; }

   void run()
   {
      while (true)
      {
         boost::unique_lock<boost::mutex> lock(_mutex);
         while (!_jobs.empty())
         {
            _jobs.front()->execute();
            _jobs.pop();
         }
         if (!_must_run)
            break;
         _new_job.wait(lock);
      }
   }

   void run_in_background()
   {
      _bg_thread = boost::thread(&async_executor::run, this);
   }

   void stop(bool wait_for_completion = true)
   {
      {
         boost::unique_lock<boost::mutex> lock(_mutex);
         _must_run = false;
         _new_job.notify_one();
      }
      if (wait_for_completion)
         _bg_thread.join();
   }

   template <typename Function>
   auto execute(const Function& func) -> std::future<decltype(func())>
   { return do_execute<decltype(func())>(func); }

   template <typename Function, typename Type1>
   auto execute(
         const Function& func,
         Type1 arg1) -> std::future<decltype(func(arg1))>
   { return do_execute<decltype(func(arg1))>(std::bind(func, arg1)); }

   template <typename Function, typename Type1, typename Type2>
   auto execute(
         const Function& func,
         Type1 arg1,
         Type2 arg2) -> std::future<decltype(func(arg1, arg2))>
   {
      return do_execute<decltype(func(arg1, arg2))>(
            std::bind(func, arg1, arg2));
   }

   template <typename Function, typename Type1, typename Type2, typename Type3>
   auto execute(
         const Function& func,
         Type1 arg1,
         Type2 arg2,
         Type3 arg3) -> std::future<decltype(func(arg1, arg2, arg3))>
   {
      return do_execute<decltype(func(arg1, arg2, arg3))>(
            std::bind(func, arg1, arg2, arg3));
   }

private:

   class job_base
   {
   public:
      virtual ~job_base() {}
      virtual void execute() = 0;
   };

   template <typename>
   class job;

   template <typename RetType>
   class job<RetType(void)> : public job_base
   {
   public:

      typedef RetType result_type;
      typedef std::function<RetType(void)> function_type;

      job(const function_type& func) : _func(func) {}

      void execute() override final
      {
         auto result = _func();
         _promise.set_value(result);
      }

      std::future<result_type> get_future()
      { return _promise.get_future(); }

   private:

      function_type _func;
      std::promise<result_type> _promise;
   };

   std::queue<std::shared_ptr<job_base>> _jobs;
   boost::mutex _mutex;
   boost::condition_variable _new_job;
   boost::thread _bg_thread;
   bool _must_run;


   template <typename RetType>
   std::future<RetType> do_execute(const std::function<RetType(void)>& func)
   {
      auto j = std::make_shared<job<RetType(void)>>(func);

      boost::unique_lock<boost::mutex> lock(_mutex);
      _jobs.push(j);
      _new_job.notify_one();
      return j->get_future();
   }
};

} // namespace oac

#endif
