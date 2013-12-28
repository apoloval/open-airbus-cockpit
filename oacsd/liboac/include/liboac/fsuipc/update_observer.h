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

#ifndef OAC_FSUIPC_UPDATE_OBSERVER_H
#define OAC_FSUIPC_UPDATE_OBSERVER_H

#include <functional>
#include <unordered_map>
#include <unordered_set>

#include <liboac/fsuipc/client.h>
#include <liboac/fsuipc/offset.h>

namespace oac { namespace fsuipc {

/**
 * A class capable to observe updates on FSUIPC using a FSUIPC client. The user
 * code may register the offsets it wants to observe using the function
 * start_observing(). After that, each time the check_for_updates() function is
 * invoked, a evaluation function will be called for each offset which value
 * has changed. The check_for_updates() function may be bound to a
 * ticks_observer to have a regular observation of FSUIPC offsets.
 */
template <typename FsuipcUserAdapter,
          typename FsuipcValuedOffsetEvaluator =
               std::function<void(const valued_offset&)>>
class update_observer
{
public:

   using client_type = fsuipc_client<FsuipcUserAdapter>;
   using client_ptr_type = fsuipc_client_ptr<FsuipcUserAdapter>;

   typedef fsuipc_client<FsuipcUserAdapter> client_type;
   typedef FsuipcValuedOffsetEvaluator update_evaluator_type;

   /**
    * Create a new observer for given evaluation function and FSUIPC client.
    *
    * @param update_eval The evaluation function to be invoked when
    *                    check_for_updates() function detects changes in
    *                    offsets being observed
    * @param client      The FSUIPC client to be used for observing
    */
   update_observer(
            const client_ptr_type& client,
            const update_evaluator_type& update_eval = update_evaluator_type())
      : _client(client),
        _update_eval(update_eval)
   {}

   /**
    * Start observing the given offset. This requires the observer to read
    * the current value of the offset in order to detect updates in further
    * calls to check_for_updates(). Due to this, if you have several offsets
    * to observe consider invoking start_observing with a collection rather
    * than invoke it several times with a single offset.
    *
    * @param offset The offset that must be observed from now on
    */
   void start_observing(const offset& o)
   {
      start_observing(std::list<offset>(1, o));
   }

   /**
    * Start observing the given offsets.
    *
    * @param offsets The offsets that must be observed from now on
    */
   template <typename FsuipcOffsetCollection>
   void start_observing(const FsuipcOffsetCollection& offsets)
   {
      for (auto& offset : offsets) {
         _offsets.insert(offset);      
         _pending_welcomes.insert(offset);
      }
      _client->query(offsets, [this](const valued_offset& val)
      {
         _values[val] = val.value;
      });
   }

   /**
    * Stop observing the given offset. If the offset was not observed,
    * nothing is done.
    *
    * @param offset The offset that must not be observed anymore
    */
   void stop_observing(const offset& offset)
   {
      _offsets.erase(offset);
   }

   /**
    * Check for any updates in the offsets being observed. For each offset to
    * be observed (previously indicated via start_observing() function), it
    * queries the client for its current value. The offset is considered
    * updated based on the following criteria.
    *
    *   -  If this is the first time the offset is being checked for updates:
    * the value obtained when the offset began to be observed
    *
    *   - Otherwise: the value of the last call to check_for_updates()
    *
    * If the offset is considered updated, the evaluation function passed to
    * the constructor is invoked with the new offset value as argument.
    */
   void check_for_updates()
   {
      _client->query(_offsets, [this](const valued_offset& val)
      {
         auto cached_val = _values.find(val);
         auto pending_welcome = _pending_welcomes.find(val);

         if ((pending_welcome != _pending_welcomes.end()) ||
             (cached_val->second != val.value))
         {
            _values[val] = val.value;
            _update_eval(val);
         }
      });
      _pending_welcomes.clear();
   }

private:

   typedef std::unordered_set<offset, offset::hash> offset_set;
   typedef std::unordered_map<
         offset, offset_value, offset::hash> offset_value_map;

   client_ptr_type _client;
   offset_set _offsets;
   offset_set _pending_welcomes;
   offset_value_map _values;
   update_evaluator_type _update_eval;
};

using dummy_update_observer = update_observer<dummy_user_adapter>;
using local_update_observer = update_observer<local_user_adapter>;

}} // namespace oac::fsuipc

#endif
