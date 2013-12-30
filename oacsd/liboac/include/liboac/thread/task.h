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

OAC_DECL_ABSTRACT_EXCEPTION(task_exception);

template <typename T>
using task_callback = std::function<void(const util::attempt<T>&)>;

template <typename T>
using task_action = std::function<T(void)>;

class task_executor
{
public:

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

using task_executor_ptr = std::shared_ptr<task_executor>;

OAC_DECL_EXCEPTION(task_state_error, task_exception,
      "an operation was invoked on a task that was on a unexpected state");

/** A task that can be recurrently executed. */
template <typename T>
class recurrent_task
{
public:

   /** Create a recurrent ask object with no task at all. */
   recurrent_task() {}

   template <typename Rep,
             typename Period = std::ratio<1>>
   recurrent_task(const task_executor_ptr& executor,
                 const task_action<T>& action,
                 const task_callback<T>& callback,
                 const std::chrono::duration<Rep, Period>& delay)
    : _executor { executor },
      _action { action },
      _callback { callback },
      _delay { delay }
   {}

   template <typename Rep,
             typename Period = std::ratio<1>>
   recurrent_task(const task_executor_ptr& executor,
                 const task_action<T>& action,
                 const std::chrono::duration<Rep, Period>& delay)
    : recurrent_task
   {
      executor,
      action,
      [](const util::attempt<T>&) {},
      delay
   }
   {}

   recurrent_task(recurrent_task&& task)
    : _executor { std::move(task._executor) },
      _action { std::move(task._action) },
      _callback { std::move(task._callback) },
      _delay { task._delay },
      _continue { task._continue },
      _thread { std::move(task._thread) }
   {}

   recurrent_task& operator = (recurrent_task&& task)
   {
      swap(task);
      return *this;
   }

   ~recurrent_task()
   {
      if (is_running())
         stop();
   }

   bool is_valid() const
   { return !!_executor; }

   bool is_running() const
   { return _thread.get_id() != std::thread::id(); }

   void start()
   {
      if (!is_valid() || is_running())
         OAC_THROW_EXCEPTION(task_state_error());
      _continue = true;
      _thread = std::thread { std::bind(&recurrent_task::loop, this) };
   }

   void stop()
   {
      if (!is_valid() || !is_running())
         OAC_THROW_EXCEPTION(task_state_error());
      _continue = false;
      _thread.join();
   }

   void swap(recurrent_task& task)
   {
      std::swap(task._executor, _executor);
      std::swap(task._action, _action);
      std::swap(task._callback, _callback);
      std::swap(task._delay, _delay);
      std::swap(task._continue, _continue);
      std::swap(task._thread, _thread);
   }

private:

   void loop()
   {
      while (_continue)
      {
         std::this_thread::sleep_for(_delay);
         _executor->submit_task(_action, _callback);
      }
   }

   task_executor_ptr _executor;
   task_action<T> _action;
   task_callback<T> _callback;
   std::chrono::milliseconds _delay;

   std::atomic_bool _continue;
   std::thread _thread;
};

}} // namespace oac::thread

#endif
