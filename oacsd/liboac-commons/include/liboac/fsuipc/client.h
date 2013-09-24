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

#ifndef OAC_FSUIPC_CLIENT_H
#define OAC_FSUIPC_CLIENT_H

#include <liboac/fsuipc/local.h>
#include <liboac/fsuipc/offset.h>

namespace oac { namespace fsuipc {

/**
 * @concept FsuipcUserAdapter
 *
 * A class which adapts the FSUIPC user libraries by wrapping its functions.
 * It let's the FSUIPC components of OAC to be decoupled from the concrete
 * library (i.e. ModuleUser.lib or FSUIPC_User.lib) and even from a dummy
 * implementation for mocking.
 *
 * Classes conforming this concept must provide the following members.
 *
 *   void read(valued_offset&);
 *
 *   void write(const valued_offset&)
 *
 *   void process()
 *
 * read() function schedules a read operation for the given valued offset.
 * This function stores internally a pointer to the field 'value' of
 * valued_offset struct. When process() is invoked, the value is updated
 * using such pointer.
 *
 * write() function schedules a write operation for the given valued offset.
 * It shall not maintain any pointer to the value but copy it, so any subsequent
 * change or destruction in the valued offset passed as argument must be
 * irrelevant to the adapter object.
 *
 * process() function will commit the pending read and write actions performed
 * via read() and write() functions.
 */

/**
 * @concept FsuipcOffsetCollection
 *
 * Any object able to behave as a collection of offset objects. It must
 * provide the following members.
 *
 *   InputIterator begin();
 *
 *   InputIterator end();
 *
 * Where InputIterator iterates over offset objects.
 */

/**
 * @concept FsuipcValuedOffsetEvaluator
 *
 * Any object that may be invoked with a valued_offset object as
 * argument.
 */

/**
 * @concept FsuipcValuedOffsetCollection
 *
 * Any object able to behave as a collection of valued_offset objects.
 * It must provide the following members.
 *
 *   InputIterator begin();
 *
 *   InputIterator end();
 *
 * Where InputIterator iterates over valued_offset objects.
 */

/**
 * FSUIPC client. This class provides a convenience wrapping for a FSUIPC
 * client. The FSUIPC basic operations (read, write and process) are decoupled
 * into a FsuipcUserAdapter object passed as argument. The wrapped provides
 * a new interface oriented to massive object reading (query) and massive
 * object writing (update).
 */
template <typename FsuipcUserAdapter>
class fsuipc_client
{
public:

   /**
    * Create a new client over given FSUIPC user adapter.
    *
    * @param user_adapter The user adapter that wraps the FSUIPC API functions.
    *                     By default obtained by its default constructor.
    */
   fsuipc_client(
         const FsuipcUserAdapter& user_adapter = FsuipcUserAdapter())
      : _user_adapter(user_adapter)
   {}

   FsuipcUserAdapter& user_adapter()
   { return _user_adapter; }

   const FsuipcUserAdapter& user_adapter() const
   { return _user_adapter; }

   /**
    * Execute a query on the given offsets and execute the corresponding
    * evaluate function for each of them. Take into consideration that each
    * query will invoke a process() function on the user adapter with the
    * corresponding overhead it means. Consider execute a single query for
    * multiple offsets rather than many queries for single offsets.
    *
    * @param offsets  A collection of offsets whose value is to be queried
    * @param evaluate A evaluation function that will be executed with the
    *                 value of each offset indicated in offsets
    */
   template <typename FsuipcOffsetCollection,
             typename FsuipcValuedOffsetEvaluator>
   void query(
         const FsuipcOffsetCollection& offsets,
         const FsuipcValuedOffsetEvaluator& evaluate)
   {
      if (offsets.empty())
         return;

      std::list<valued_offset> values;
      for (auto& offset : offsets)
      {
         values.push_back(valued_offset(offset, 0));
         auto& valued_offset = values.back();
         _user_adapter.read(valued_offset);
      }
      _user_adapter.process();
      for (auto& val : values)
         evaluate(val);
   }

   /**
    * Update the value for the offsets passed as argument. Take into
    * consideration that each update will invoke a process() function on the
    * user adapter with the corresponding overhead it means. Consider execute
    * a single update for multiple offsets rather than many updates for single
    * offsets.
    *
    * @param valued_offsets The valued offsets that are to be updated
    */
   template <typename FsuipcValuedOffsetCollection>
   void update(
         const FsuipcValuedOffsetCollection& valued_offsets)
   {
      if (valued_offsets.empty())
         return;

      for (auto& val : valued_offsets)
      {
         _user_adapter.write(val);
      }
      _user_adapter.process();
   }

private:

   FsuipcUserAdapter _user_adapter;
};

/**
 * A dummy FsuipcUserAdapter which reads and writes from a raw buffer
 * passed as argument to its constructor.
 */
class dummy_user_adapter
{
public:

   void read(valued_offset& valued_offset)
   {
      read_request req = { valued_offset, &valued_offset.value };
      _read_requests.push_back(req);
   }

   void write(const valued_offset& valued_offset)
   {
      write_request req = { valued_offset };
      _write_requests.push_back(req);
   }

   void process()
   {
      process_write_requests();
      process_read_requests();
   }

   offset_value read_value_from_buffer(
         offset_address addr,
         offset_length len);

   void write_value_to_buffer(
         offset_address addr,
         offset_length len,
         offset_value val);

private:

   struct read_request
   {
      offset offset;
      offset_value* value;
   };

   struct write_request
   {
      valued_offset offset;
   };

   std::uint8_t _buffer[0xffff];
   std::list<read_request> _read_requests;
   std::list<write_request> _write_requests;

   void process_read_requests();

   void process_write_requests();
};

/**
 * A client for local FSUIPC using ModuleUser.lib
 */
typedef fsuipc_client<local_user_adapter> local_fsuipc_client;

}} // namespace oac::fsuipc

#endif
