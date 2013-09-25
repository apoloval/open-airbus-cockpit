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

#ifndef OAC_SIMCONN_H
#define OAC_SIMCONN_H

#include <functional>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <SimConnect.h>

#include <liboac/exception.h>
#include <liboac/logging.h>

namespace oac {

class simconnect_client : public logger_component
{
public:

   typedef std::string event_name;
   typedef std::string data_name;
   typedef std::string data_units;

   OAC_DECL_ABSTRACT_EXCEPTION(invalid_operation_exception);

   OAC_DECL_EXCEPTION(
         server_unavailable_error,
         invalid_operation_exception,
         "SimConnect server is not available");

   OAC_DECL_EXCEPTION_WITH_PARAMS(
         unknown_event_name_error,
         invalid_operation_exception,
      ("no such event name %s available in SimConnect", event),
      (event, event_name));


   OAC_DECL_EXCEPTION_WITH_PARAMS(
         data_definition_error,
         invalid_operation_exception,
      (
         "cannot define data for variable %s in %s in SimConnect",
         name,
         units
      ),
      (name, data_name),
      (units, data_units));

   OAC_DECL_EXCEPTION_WITH_PARAMS(
         data_request_error,
         invalid_operation_exception,
      ("data request %d failed in SimConnect", data_def),
      (data_def, SIMCONNECT_DATA_DEFINITION_ID));

   OAC_DECL_EXCEPTION_WITH_PARAMS(event_error, invalid_operation_exception,
      ("operation for client event %d failed in SimConnect", event),
      (event, SIMCONNECT_CLIENT_EVENT_ID));

   class data_definition
   {
   public:

      inline data_definition(const simconnect_client& cli,
            SIMCONNECT_DATA_DEFINITION_ID id) :
         _handle(cli._handle), _id(id)
      {}

      data_definition& add(const data_name& name, const data_units& units,
               SIMCONNECT_DATATYPE data_type = SIMCONNECT_DATATYPE_FLOAT64)
            throw (data_definition_error);

      inline SIMCONNECT_DATA_DEFINITION_ID id() const
      { return _id; }

   private:

      HANDLE _handle;
      SIMCONNECT_DATA_DEFINITION_ID _id;
   };

   class data_pull_request
   {
   public:

      inline data_pull_request(const simconnect_client& cli,
            const data_definition& data_def,
            SIMCONNECT_DATA_REQUEST_ID id) :
         _data_def(data_def.id()),_handle(cli._handle), _id(id), 
         _object(SIMCONNECT_OBJECT_ID_USER), _period(SIMCONNECT_PERIOD_ONCE),
         _flags(SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT), _origin(0), 
         _interval(0), _limit(0)
      {}

      inline data_pull_request& set_object(SIMCONNECT_OBJECT_ID object)
      { _object = object; return *this; }

      inline data_pull_request& set_period(SIMCONNECT_PERIOD period)
      { _period = period; return *this; }

      inline data_pull_request& set_flags(SIMCONNECT_DATA_REQUEST_FLAG flags)
      { _flags = flags; return *this; }

      inline data_pull_request& set_origin(DWORD origin)
      { _origin = origin; return *this; }

      inline data_pull_request& set_interval(DWORD interval)
      { _interval = interval; return *this; }

      inline data_pull_request& set_limit(DWORD limit)
      { _limit = limit; return *this; }

      void submit() throw (data_request_error);

   private:

      HANDLE _handle;
      SIMCONNECT_DATA_REQUEST_ID _id;
      SIMCONNECT_DATA_DEFINITION_ID _data_def;
      SIMCONNECT_OBJECT_ID _object;
      SIMCONNECT_PERIOD _period;
      SIMCONNECT_DATA_REQUEST_FLAG _flags;
      DWORD _origin;
      DWORD _interval;
      DWORD _limit;
   };

   class data_push_request
   {
   public:

      inline data_push_request(const simconnect_client& cli,
            const data_definition& data_def) :
         _handle(cli._handle), _data_def(data_def.id()),
         _object(SIMCONNECT_OBJECT_ID_USER),
         _flags(SIMCONNECT_DATA_SET_FLAG_DEFAULT), _count(0), 
         _element_size(sizeof(double))
      {}

      inline data_push_request& set_object(SIMCONNECT_OBJECT_ID object)
      { _object = object; return *this; }

      inline data_push_request& set_flags(SIMCONNECT_DATA_SET_FLAG flags)
      { _flags = flags; return *this; }

      inline data_push_request& set_count(DWORD count)
      { _count = count; return *this; }

      inline data_push_request& set_element_size(DWORD element_size)
      { _element_size = element_size; return *this; }

      void submit(void* data) throw (data_request_error);

   private:

      HANDLE _handle;
      SIMCONNECT_DATA_DEFINITION_ID _data_def;
      SIMCONNECT_OBJECT_ID _object;
      SIMCONNECT_DATA_SET_FLAG _flags;
      DWORD _count;
      DWORD _element_size;
   };

   class client_event
   {
   public:

      client_event(const simconnect_client& cli,
                  const event_name& event_name,
                  SIMCONNECT_CLIENT_EVENT_ID id) throw (event_error);

      client_event(const simconnect_client& cli,
                  SIMCONNECT_CLIENT_EVENT_ID id) throw (event_error);

      inline client_event& set_object(SIMCONNECT_OBJECT_ID object)
      { _object = object; return *this; }

      inline client_event& set_group(SIMCONNECT_NOTIFICATION_GROUP_ID group)
      { _group = group; return *this; }

      inline client_event& set_flags(SIMCONNECT_EVENT_FLAG flags)
      { _flags = flags; return *this; }

      inline SIMCONNECT_CLIENT_EVENT_ID id() const
      { return _id; }

      void transmit(DWORD value = 0);

   private:

      HANDLE _handle;
      SIMCONNECT_CLIENT_EVENT_ID _id;
      SIMCONNECT_OBJECT_ID _object;
      SIMCONNECT_NOTIFICATION_GROUP_ID _group;
      SIMCONNECT_EVENT_FLAG _flags;
   };

   template <typename T>
   class variable_watch
   {
   public:

      inline variable_watch() :
         _client("Variable Watch"), _data_def(_client.new_data_definition())
      {
      }

      inline data_definition get_data_definition()
      { return _data_def; }

      inline T get() const
      {      
         bool done = false;
         T data;
         _client.register_on_simobject_data_callback([&data, &done](
            simconnect_client& client, const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)
         {
            memcpy(&data, &msg.dwData, sizeof(T));
            done = true;
         });
         _client.new_data_pull_request(_data_def).submit();
         while (!done)
            _client.dispatch_message();
         return data;
      }

      inline void set(const T& t)
      {
         _client.new_data_push_request(_data_def)
               .set_element_size(sizeof(T))
               .submit((void*) &t);
      }

   protected:

      mutable simconnect_client _client;
      data_definition _data_def;
   };

   class event_transmitter
   {
   public:

      event_transmitter(const simconnect_client::event_name& event_name);

      inline void transmit(DWORD value)
      { _event.transmit(value); }

   private:

      std::shared_ptr<simconnect_client> _client;
      client_event _event;
   };

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV& msg)> on_null_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_EXCEPTION& msg)> on_exception_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_OPEN& msg)> on_open_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV& msg)> on_quit_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_EVENT& msg)> on_event_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE& msg)> 
               on_event_object_add_remove_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_EVENT_FILENAME& msg)> on_event_filename_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_EVENT_FRAME& msg)> on_event_frame_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)> on_simobject_data_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE& msg)> 
               on_simobject_data_by_type_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_WEATHER_OBSERVATION& msg)> 
               on_weather_observation_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_CLOUD_STATE& msg)> on_cloud_state_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_ASSIGNED_OBJECT_ID& msg)> 
               on_assigned_object_id_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_RESERVED_KEY & msg)> on_reserved_key_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_CUSTOM_ACTION& msg)> on_custom_action_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_SYSTEM_STATE& msg)> on_system_state_callback;

   typedef std::function<void(
         simconnect_client& client,
         const SIMCONNECT_RECV_CLIENT_DATA& msg)> on_client_data_callback;

   static const event_name SYSTEM_EVENT_1SEC;
   static const event_name SYSTEM_EVENT_4SEC;
   static const event_name SYSTEM_EVENT_6HZ;
   static const event_name SYSTEM_EVENT_FRAME;
   static const event_name SYSTEM_EVENT_AIRCRAFT_LOADED;
   static const event_name SYSTEM_EVENT_FLIGHT_LOADED;

   simconnect_client(const std::string& name) throw (server_unavailable_error);

   simconnect_client(const std::string& name,
         const on_open_callback& open_callback) throw (server_unavailable_error);

   virtual ~simconnect_client();

   /**
    * Dispatch an incoming message (if any) from the server. When compiled
    * into a DLL plugin, it has no effect. 
    */
   void dispatch_message();

   /**
    * Callback function member for incoming messages from SimConnect. The
    * default implementation dispatches the message to the previously
    * registered callback, if any. It may be overriden if needed, but doing
    * that may break the callback registration mechanism, so do it with care.
    */
   virtual void on_message(SIMCONNECT_RECV* msg, DWORD msg_len);

   void register_on_null_callback(const on_null_callback& callback);

   void register_on_exception_callback(const on_exception_callback& callback);

   void register_on_open_callback(const on_open_callback& callback);

   void register_on_quit_callback(const on_quit_callback& callback);

   void register_on_event_callback(const on_event_callback& callback);

   void register_on_event_object_add_remove_callback(
         const on_event_object_add_remove_callback& callback);

   void register_on_event_filename_callback(const on_event_filename_callback& callback);

   void register_on_event_frame_callback(const on_event_frame_callback& callback);

   void register_on_simobject_data_callback(const on_simobject_data_callback& callback);

   void register_on_simobject_data_by_type_callback(
         const on_simobject_data_by_type_callback& callback);

   void register_on_weather_observation_callback(
         const on_weather_observation_callback& callback);

   void register_on_cloud_state_callback(const on_cloud_state_callback& callback);

   void register_on_assigned_object_id_callback(
         const on_assigned_object_id_callback& callback);

   void register_on_reserved_key_callback(const on_reserved_key_callback& callback);

   void register_on_custom_action_callback(const on_custom_action_callback& callback);

   void register_on_system_state_callback(const on_system_state_callback& callback);

   void register_on_client_data_callback(const on_client_data_callback& callback);

   void subscribe_to_system_event(
         const event_name& event_name,
         SIMCONNECT_CLIENT_EVENT_ID event_id = 0) throw (unknown_event_name_error);

   data_definition new_data_definition();

   data_pull_request new_data_pull_request(const data_definition& data_def);

   data_push_request new_data_push_request(const data_definition& data_def);

   client_event new_client_event(const event_name& event_name);

private:

   class abstract_message_receiver
   {
   public:
      virtual void send_message(
            simconnect_client& client, SIMCONNECT_RECV* msg) = 0;
      virtual ~abstract_message_receiver() {}
   };

   typedef std::shared_ptr<
         abstract_message_receiver> abstract_message_receiver_ptr;

   template <typename Message>
   class message_receiver : public abstract_message_receiver
   {
   public:

      inline message_receiver(const std::function<void(
            simconnect_client&, const Message&)>& callback) :
         _callback(callback)
      {}

      virtual void send_message(simconnect_client& client, SIMCONNECT_RECV* msg)
      {
         auto narrowed_msg = static_cast<Message*>(msg);
         _callback(client, *narrowed_msg);
      }

   private:
      std::function<void(simconnect_client&, const Message&)> _callback;
   };

   template <typename Message>
   void register_callback(
         const std::function<void(simconnect_client&, const Message&)>& callback,
         SIMCONNECT_RECV_ID message_type)
   {
      this->receiver(message_type) =
            std::make_shared<message_receiver<Message>>(callback);
   }

   void open() throw (server_unavailable_error);

   abstract_message_receiver_ptr& receiver(
         SIMCONNECT_RECV_ID message_type);

   std::string _name;
   HANDLE _handle;
   std::vector<abstract_message_receiver_ptr> _msg_receivers;
};

typedef std::shared_ptr<simconnect_client> simconnect_client_ptr;

}; // namespace oac

#endif
