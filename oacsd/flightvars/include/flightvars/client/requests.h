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

#ifndef OAC_FV_CLIENT_REQUESTS_H
#define OAC_FV_CLIENT_REQUESTS_H

#include <future>
#include <list>
#include <unordered_map>

#include <liboac/exception.h>
#include <liboac/logging.h>

#include "api.h"
#include "client/errors.h"

namespace oac { namespace fv { namespace client {

template <typename RetVal>
class request
{
public:

   request() {}

   void set_result(const RetVal& val)
   {
      _promise.set_value(val);
   }

   template <typename Exception>
   void set_error(const Exception& e)
   {
      try { throw e; }
      catch (...)
      { _promise.set_exception(std::current_exception()); }
   }

   template <typename Rep, typename Period>
   RetVal get_result(
         const std::chrono::duration<Rep, Period>& timeout)
   throw (request_timeout_error)
   {
      auto fut = _promise.get_future();
      auto state = fut.wait_for(timeout);
      if (state == std::future_status::timeout)
         OAC_THROW_EXCEPTION(request_timeout_error());
      return fut.get();
   }

private:

   std::promise<RetVal> _promise;

   request(const request<RetVal>&);

   request& operator = (const request& req);
};

template <>
class request<void>
{
public:

   request() {}

   void set_result()
   {
      _promise.set_value();
   }

   template <typename Exception>
   void set_error(const Exception& e)
   {
      try { throw e; }
      catch (...)
      { _promise.set_exception(std::current_exception()); }
   }

   template <typename Rep, typename Period>
   void get_result(
         const std::chrono::duration<Rep, Period>& timeout)
   {
      auto fut = _promise.get_future();
      auto state = fut.wait_for(timeout);
      if (state == std::future_status::timeout)
         OAC_THROW_EXCEPTION(request_timeout_error());
      fut.get();
   }

private:

   std::promise<void> _promise;

   request(const request<void>&);
};

/**
 * A subscription request created by the client interface in order to be
 * processed by the connection manager.
 */
class subscription_request : public request<subscription_id>
{
public:

   subscription_request(
         const variable_id& var_id,
         const flight_vars::var_update_handler& handler)
      : _var_id(var_id),
        _handler(handler)
   {}

   const variable_id& var_id() const
   { return _var_id; }

   const flight_vars::var_update_handler& handler() const
   { return _handler; }

private:

   variable_id _var_id;
   flight_vars::var_update_handler _handler;
};

typedef std::shared_ptr<subscription_request> subscription_request_ptr;

/**
 * An unsubscription request created by the client interface in order to be
 * processed by the connection manager.
 */
class unsubscription_request : public request<void>
{
public:

   unsubscription_request(
         subscription_id subs_id)
      : _virtual_subs_id(subs_id)
   {}

   subscription_id master_subs_id() const
   { return _master_subs_id; }

   void update_master_subs_id(subscription_id id)
   { _master_subs_id = id; }

   subscription_id virtual_subs_id() const
   { return _virtual_subs_id; }

private:

   subscription_id _master_subs_id;
   subscription_id _virtual_subs_id;
};

typedef std::shared_ptr<unsubscription_request> unsubscription_request_ptr;

/**
 * A request of variable update.
 */
class variable_update_request : public request<void>
{
public:

   variable_update_request(
         subscription_id subs_id,
         const variable_value& var_value)
      : _virtual_subs_id(subs_id),
        _var_value(var_value)
   {}

   subscription_id virtual_subs_id() const
   { return _virtual_subs_id; }

   const variable_value& var_value() const
   { return _var_value; }

private:

   subscription_id _virtual_subs_id;
   variable_value _var_value;
};

typedef std::shared_ptr<variable_update_request> variable_update_request_ptr;

/**
 * A pool of requests maintained by the connection manager to track the
 * pending actions over the connection.
 */
class request_pool : public logger_component
{
public:

   typedef std::list<subscription_request_ptr> subscription_request_list;

   typedef std::list<unsubscription_request_ptr> unsubscription_request_list;

   request_pool() : logger_component("client-request-pool") {}

   void insert(const subscription_request_ptr& req)
   {
      auto& lst = _subs_reqs[req->var_id()];
      lst.push_back(req);
   }

   subscription_request_list pop_subscription_requests(
         const variable_id& var_id)
   {
      return std::move(_subs_reqs[var_id]);
   }

   void insert(const unsubscription_request_ptr& req)
   {
      auto& lst = _unsubs_reqs[req->master_subs_id()];
      lst.push_back(req);
   }

   unsubscription_request_list pop_unsubscription_requests(
         subscription_id subs_id)
   {
      return std::move(_unsubs_reqs[subs_id]);
   }

   /**
    * Propagate the given error along every request found in this pool.
    */
   template <typename Exception>
   void propagate_error(const Exception& e)
   {
      propagate_error(_subs_reqs, e);
      propagate_error(_unsubs_reqs, e);
   }

private:

   typedef std::unordered_map<
         variable_id,
         subscription_request_list,
         variable_id_hash> subscription_requests_map;

   typedef std::unordered_map<
         subscription_id,
         unsubscription_request_list> unsubscription_requests_map;

   subscription_requests_map _subs_reqs;
   unsubscription_requests_map _unsubs_reqs;

   template <typename RequestMap, typename Exception>
   void propagate_error(
         RequestMap& map,
         const Exception& e)
   {
      for (auto& lst : map)
         for (auto& req : lst.second)
            req->set_error(e);
      map.clear();
   }
};

}}} // namespace oac::fv::client

#endif
