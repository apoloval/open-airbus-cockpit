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

#ifndef OAC_FV_TEST_FAKE_H
#define OAC_FV_TEST_FAKE_H

#include <atomic>
#include <list>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>

#include "api.h"

namespace oac { namespace fv {

class fake_flight_vars : public flight_vars
{
public:

   static variable_group VAR_GROUP;

   fake_flight_vars()
   {
      _runner.swap(
               boost::thread(
                  std::bind(&fake_flight_vars::loop, this)));
   }

   ~fake_flight_vars()
   {
      _must_run = false;
      if (_runner.joinable())
         _runner.join();
   }

   virtual subscription_id subscribe(
         const variable_id& var,
         const var_update_handler& handler) throw (unknown_variable_error)
   {
      if (get_var_group(var) != VAR_GROUP)
         BOOST_THROW_EXCEPTION(unknown_variable_group_error());
      subscription subs =
      {
         make_subscription_id(),
         var,
         handler,
         3
      };
      boost::lock_guard<boost::mutex> guard(_mutex);
      _subscriptions.push_back(subs);
      return subs.id;
   }

   virtual void unsubscribe(const subscription_id& id)
   {
      boost::lock_guard<boost::mutex> guard(_mutex);
      auto subs = std::find_if(
               _subscriptions.begin(),
               _subscriptions.end(),
               [id](const subscription& s)
      {
         return s.id == id;
      });
      if (subs != _subscriptions.end())
         _subscriptions.erase(subs);
   }

private:

   struct subscription
   {
      subscription_id id;
      variable_id var;
      var_update_handler handler;
      int ntimes;
   };

   typedef std::list<subscription> subscription_list;

   std::atomic_bool _must_run;
   subscription_list _subscriptions;
   boost::mutex _mutex;
   boost::thread _runner;

   void loop()
   {
      _must_run = true;
      while (_must_run)
      {
         boost::this_thread::sleep_for(boost::chrono::milliseconds(250));
         _mutex.lock();
         for (auto& subs : _subscriptions)
         {
            if (subs.ntimes > 0)
            {
               subs.handler(subs.var, random_var_value());
               subs.ntimes--;
            }
         }
         for (auto it =  _subscriptions.begin(), end = _subscriptions.end();
              it != end;)
         {
            if (it->ntimes <= 0)
               it = _subscriptions.erase(it);
            else
               it++;
         };
         // std::cerr << "Subscriptions removed" << std::endl;
         _mutex.unlock();
      }
   }

   variable_value random_var_value()
   {
      return variable_value::from_dword(std::rand());
   }
};

variable_group fake_flight_vars::VAR_GROUP("testing");

}} // namespace oac::fv

#endif
