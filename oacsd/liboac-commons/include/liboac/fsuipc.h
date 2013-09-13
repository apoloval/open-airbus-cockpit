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

#ifndef OAC_FSUIPC_H
#define OAC_FSUIPC_H

#pragma warning( disable : 4290 )

#include <unordered_map>
#include <unordered_set>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Boost/format.hpp>
#include <fsuipc/fsuipc_user.h>

#include "buffer.h"
#include "exception.h"
#include "logging.h"

#ifndef LOCAL_FSUIPC_BUFFER_SIZE
#define LOCAL_FSUIPC_BUFFER_SIZE (65*1024) // 1KB more for headers
#endif

namespace oac {

/**
 * An exception caused by an unexpected FSUIPC error.
 */
OAC_EXCEPTION_BEGIN(fsuipc_error, io_exception)
   OAC_EXCEPTION_FIELD(error_code, int)
   OAC_EXCEPTION_FIELD(error_message, std::string)
   OAC_EXCEPTION_MSG(
      "FSUIPC library returned an error code %d(%s)",
      error_code,
      error_message)
OAC_EXCEPTION_END()

/**
 * FSUIPC class. This class encapsulates the access to FSUIPC module. It 
 * implements convenient wrappers to read from and write to FSUIPC offsets.
 */
class local_fsuipc :
      public linear_stream_buffer_base<local_fsuipc>
{
public:

   typedef std::shared_ptr<local_fsuipc> ptr_type;

   typedef DWORD Offset;

   class factory 
   {
   public:

      typedef local_fsuipc value_type;

      local_fsuipc* create_fsuipc() throw (fsuipc_error)
      { return new local_fsuipc(); }
   };

   typedef factory factory_type;
   typedef std::shared_ptr<factory_type> factory_ptr;

   local_fsuipc();

   virtual DWORD capacity() const
   { return 0xffff; }

   void read(void* dst, std::uint32_t offset, std::size_t length) const
         throw (buffer::index_out_of_bounds, io_exception);

   void write(const void* src, std::uint32_t offset, std::size_t length)
         throw (buffer::index_out_of_bounds, io_exception);

   template <typename OutputStream>
   void read_to(OutputStream& dst,
                std::uint32_t offset,
                std::size_t length) const
   throw (buffer::index_out_of_bounds, io_exception)
   {
      linear_buffer tmp(length);
      tmp.copy(*this, offset, 0, length);
      tmp.read(dst, 0, length);
   }

   template <typename InputStream>
   std::size_t write_from(InputStream& src,
                          std::uint32_t offset,
                          std::size_t length)
   throw (buffer::index_out_of_bounds, io_exception)
   {
      linear_buffer tmp(length);
      auto nbytes = tmp.write(src, 0, length);
      this->copy(tmp, 0, offset, length);
      return nbytes;
   }

   template <typename Buffer>
   void copy(
         const Buffer& src,
         DWORD src_offset,
         DWORD dst_offset,
         DWORD length)
   throw (buffer::index_out_of_bounds, io_exception)
   {
      BYTE tmp[1024];
      auto sos = src_offset;
      auto dos = dst_offset;
      while (length)
      {
         auto l = length > 1024 ? 1024 : length;
         src.read(tmp, sos, l);
         write(tmp, dos, l);
         dos += l;
         sos += l;
         length -= l;
      }
   }
};

typedef local_fsuipc::ptr_type local_fsuipc_ptr;

/**
 * The address of a FSUIPC offset.
 */
typedef std::uint16_t fsuipc_offset_address;

/**
 * The length of a FSUIPC offset.
 */
enum fsuipc_offset_length
{
   OFFSET_LEN_BYTE   = 1,
   OFFSET_LEN_WORD   = 2,
   OFFSET_LEN_DWORD  = 4
};

/**
 * The value of a FSUIPC offset. It actually have storage enought to store
 * a value of a OFFSET_LEN_DWORD offset. For any other offset length, only
 * the corresponding bytes should be interpreted.
 */
typedef std::uint32_t fsuipc_offset_value;

/**
 * A FSUIPC offset, comprising its address and length.
 */
struct fsuipc_offset
{
   /**
    * A hash type for fsuipc_offset. This allow fsuipc_offset to be used
    * in hash-based STL collections.
    */
   struct hash
   {
      std::size_t operator()(const fsuipc_offset& offset) const
      { return offset.address * 4 + offset.length; }
   };

   fsuipc_offset_address address;
   fsuipc_offset_length length;

   fsuipc_offset(
         fsuipc_offset_address addr,
         fsuipc_offset_length len)
      : address(addr), length(len)
   {}

   bool operator == (const fsuipc_offset& offset) const
   { return address == offset.address && length == offset.length; }
};

/**
 * A FSUIPC valued object, comprising the offset itself with the corresponding
 * value.
 */
struct fsuipc_valued_offset : fsuipc_offset
{
   fsuipc_offset_value value;

   fsuipc_valued_offset(
         const fsuipc_offset& offset,
         fsuipc_offset_value val)
      : fsuipc_offset(offset), value(val)
   {}

   fsuipc_valued_offset(
         fsuipc_offset_address addr,
         fsuipc_offset_length len,
         fsuipc_offset_value val)
      : fsuipc_offset(addr, len), value(val)
   {}
};

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
 *   void read(fsuipc_valued_offset&);
 *
 *   void write(const fsuipc_valued_offset&)
 *
 *   void process()
 *
 * read() function schedules a read operation for the given valued offset.
 * This function stores internally a pointer to the field 'value' of
 * fsuipc_valued_offset struct. When process() is invoked, the value is updated
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
 * Any object able to behave as a collection of fsuipc_offset objects. It must
 * provide the following members.
 *
 *   InputIterator begin();
 *
 *   InputIterator end();
 *
 * Where InputIterator iterates over fsuipc_offset objects.
 */

/**
 * @concept FsuipcValuedOffsetEvaluator
 *
 * Any object that may be invoked with a fsuipc_valued_offset object as
 * argument.
 */

/**
 * @concept FsuipcValuedOffsetCollection
 *
 * Any object able to behave as a collection of fsuipc_valued_offset objects.
 * It must provide the following members.
 *
 *   InputIterator begin();
 *
 *   InputIterator end();
 *
 * Where InputIterator iterates over fsuipc_valued_offset objects.
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

      std::list<fsuipc_valued_offset> values;
      for (auto& offset : offsets)
      {
         values.push_back(fsuipc_valued_offset(offset, 0));
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
 * A class capable to observe updates on FSUIPC using a FSUIPC client. The user
 * code may register the offsets it wants to observe using the function
 * start_observing(). After that, each time the check_for_updates() function is
 * invoked, a evaluation function will be called for each offset which value
 * has changed. The check_for_updates() function may be bound to a
 * ticks_observer to have a regular observation of FSUIPC offsets.
 */
template <typename FsuipcUserAdapter,
          typename FsuipcValuedOffsetEvaluator =
               std::function<void(const fsuipc_valued_offset&)>>
class fsuipc_update_observer
{
public:

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
   fsuipc_update_observer(
            const update_evaluator_type& update_eval = update_evaluator_type(),
            const client_type& client = client_type())
      : _client(client),
        _update_eval(update_eval)
   {}

   const client_type& get_client() const
   { return _client; }

   client_type& get_client()
   { return _client; }

   /**
    * Start observing the given offset. This requires the observer to read
    * the current value of the offset in order to detect updates in further
    * calls to check_for_updates(). Due to this, if you have several offsets
    * to observe consider invoking start_observing with a collection rather
    * than invoke it several times with a single offset.
    *
    * @param offset The offset that must be observed from now on
    */
   void start_observing(const fsuipc_offset& offset)
   {
      start_observing(std::list<fsuipc_offset>(1, offset));
   }

   /**
    * Start observing the given offsets.
    *
    * @param offsets The offsets that must be observed from now on
    */
   template <typename FsuipcOffsetCollection>
   void start_observing(const FsuipcOffsetCollection& offsets)
   {
      for (auto& offset : offsets)
         _offsets.insert(offset);
      _client.query(offsets, [this](const fsuipc_valued_offset& vo)
      {
         _values[vo] = vo.value;
      });
   }

   /**
    * Stop observing the given offset. If the offset was not observed,
    * nothing is done.
    *
    * @param offset The offset that must not be observed anymore
    */
   void stop_observing(const fsuipc_offset& offset)
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
      _client.query(_offsets, [this](const fsuipc_valued_offset& val)
      {
         auto cached_val = _values.find(val);
         if ((cached_val != _values.end()) && (cached_val->second != val.value))
         {
            _values[val] = val.value;
            _update_eval(val);
         }
      });
   }

private:

   client_type _client;
   std::unordered_set<fsuipc_offset, fsuipc_offset::hash> _offsets;
   std::unordered_map<
         fsuipc_offset,
         fsuipc_offset_value,
         fsuipc_offset::hash> _values;
   update_evaluator_type _update_eval;
};

/**
 * A FsuipcUserAdapter which wraps the local FSUIPC functions from
 * ModuleUser.lib
 */
class local_fsuipc_user_adapter : public logger_component
{
public:

   local_fsuipc_user_adapter() throw (fsuipc_error);

   local_fsuipc_user_adapter(const local_fsuipc_user_adapter& adapter);

   ~local_fsuipc_user_adapter();

   void read(fsuipc_valued_offset& valued_offset)
   throw (fsuipc_error);

   void write(const fsuipc_valued_offset& valued_offset)
   throw (fsuipc_error);

   void process() throw (fsuipc_error);

private:

   static std::uint32_t _instance_count;
   static std::uint8_t _buffer[LOCAL_FSUIPC_BUFFER_SIZE];
};

/**
 * A dummy FsuipcUserAdapter which reads and writes from a raw buffer
 * passed as argument to its constructor.
 */
class dummy_fsuipc_user_adapter
{
public:

   void read(fsuipc_valued_offset& valued_offset)
   {
      read_request req = { valued_offset, &valued_offset.value };
      _read_requests.push_back(req);
   }

   void write(const fsuipc_valued_offset& valued_offset)
   {
      write_request req = { valued_offset };
      _write_requests.push_back(req);
   }

   void process()
   {
      process_write_requests();
      process_read_requests();
   }

   fsuipc_offset_value read_value_from_buffer(
         fsuipc_offset_address addr,
         fsuipc_offset_length len);

   void write_value_to_buffer(
         fsuipc_offset_address addr,
         fsuipc_offset_length len,
         fsuipc_offset_value val);

private:

   struct read_request
   {
      fsuipc_offset offset;
      fsuipc_offset_value* value;
   };

   struct write_request
   {
      fsuipc_valued_offset offset;
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
typedef fsuipc_client<local_fsuipc_user_adapter> local_fsuipc_client;

} // namespace oac

#endif
