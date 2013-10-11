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

#ifndef OAC_FV_CLIENT_CONNECTION_STATE_H
#define OAC_FV_CLIENT_CONNECTION_STATE_H

#include <future>
#include <list>

namespace oac { namespace fv { namespace client {

class connection_state
{
public:

   connection_state(bool connected = true) : _connected(connected) {}

   bool is_connected() const
   { return _connected; }

   std::future<void> disconnection()
   {
      std::lock_guard<std::mutex> lock(_mtx);
      if (is_connected())
      {
         auto p = _promises.emplace(_promises.begin());
         return p->get_future();
      }
      else
      {
         disconnection_promise p;
         if (_error)
            p.set_exception(_error);
         else
            p.set_value();
         return p.get_future();
      }
   }

   void disconnect()
   {
      std::lock_guard<std::mutex> lock(_mtx);
      if (is_connected())
      {
         _connected = false;
         for (auto& p : _promises)
            p.set_value();
         _promises.clear();
      }
   }

   template <typename Exception>
   void disconnect_with_error(const Exception& e)
   {
      std::lock_guard<std::mutex> lock(_mtx);
      if (is_connected())
      {
         _connected = false;
         _error = std::make_exception_ptr(e);
         for (auto& p : _promises)
            p.set_exception(_error);
         _promises.clear();
      }
   }

private:

   typedef std::promise<void> disconnection_promise;
   typedef std::list<disconnection_promise> disconnection_promise_list;

   std::mutex _mtx;
   bool _connected;
   std::exception_ptr _error;
   disconnection_promise_list _promises;

};

}}} // namespace oac::fv::client

#endif
