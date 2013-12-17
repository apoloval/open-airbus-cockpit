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

#ifndef OAC_THREAD_TASK_H
#define OAC_THREAD_TASK_H

#include <functional>
#include <future>

#include <liboac/thread/channel.h>
#include <liboac/util/attempt.h>

namespace oac { namespace thread {

class task_executor
{
public:

   template <typename T>
   using task_callback = std::function<void(const util::attempt<T>&)>;

   template <typename T>
   using task_action = std::function<T(void)>;

   template <typename T>
   void submit_task(
         const task_action<T>& action,
         const task_callback<T>& callback)
   {
      _actions << std::move(task<T> { action, callback });
   }

   template <typename T>
   std::future<T> submit_task(
         const task_action<T>& action)
   {
      auto promise = std::make_shared<std::promise<T>>();
      auto future = promise->get_future();
      submit_task<T>(action, fill_promise(promise));
      return future;
   }

   template <typename Predicate>
   void loop_until(const Predicate& pred)
   {
      do
      {
         try
         {
            auto action = _actions.read();
            action();
         }
         catch (const channel_timeout_error&) {}
      } while (pred());
   }

   void loop() 
   { 
      _stop = false;
      loop_until([this]() { return !_stop; });
   }

   void stop_loop() 
   { 
      submit_task<void>([this]() { _stop = true; });
   }

private:

   template <typename T>
   struct abstract_task
   {
      using action_type = task_action<T>;
      using callback_type = task_callback<T>;

      action_type action;
      callback_type callback;

      abstract_task(const action_type& act, const callback_type& cb)
       : action { act }, callback { cb }
      {}
   };

   template <typename T>
   struct task : abstract_task<T>
   {
      task(const action_type& act, const callback_type& cb)
       : abstract_task<T>(act, cb)
      {}

      void operator()()
      {
         try
         {
            auto result = action();
            callback(util::make_success(action()));
         }
         catch (...)
         {
            callback(util::make_failure<T>(std::current_exception()));
         }
      }
   };

   template <>
   struct task<void> : abstract_task<void>
   {
      task(const action_type& act, const callback_type& cb)
       : abstract_task<void>(act, cb)
      {}

      void operator()()
      {
         try
         {
            action();
            callback(util::make_success());
         }
         catch (...)
         {
            callback(util::make_failure<void>(std::current_exception()));
         }
      }
   };

   using actions_channel = channel<task_action<void>>;

   actions_channel _actions;
   std::atomic_bool _stop;

   template <typename T>
   task_callback<T> fill_promise(const std::shared_ptr<std::promise<T>>& p)
   {
      return [p](const util::attempt<T>& result)
      {
         try { p->set_value(result.get_value()); }
         catch (...) { p->set_exception(std::current_exception()); }
      };
   }

   template <>
   task_callback<void> fill_promise(const std::shared_ptr<std::promise<void>>& p)
   {
      return [p](const util::attempt<void>& result)
      {
         try
         {
            result.get_value();
            p->set_value();
         }
         catch (...) { p->set_exception(std::current_exception()); }
      };
   }
};

}} // namespace oac::thread

#endif
