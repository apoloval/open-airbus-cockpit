/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012 Alvaro Polo
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

#include <Windows.h>

#include <SimConnect.h>

#include "exception.h"
#include "lang-utils.h"

namespace oac {

class SimConnectClient 
{
public:

   typedef std::string EventName;
   typedef std::string DataName;
   typedef std::string DataUnits;

   DECL_ERROR(UnknownEventNameError, InvalidInputError);
   DECL_ERROR_INFO(EventNameInfo, EventName);

   DECL_ERROR(DataDefinitionError, InvalidInputError);
   DECL_ERROR_INFO(DataNameInfo, DataName);
   DECL_ERROR_INFO(DataUnitsInfo, DataUnits);

   DECL_ERROR(DataRequestError, IOError);
   DECL_ERROR_INFO(DataDefinitionInfo, SIMCONNECT_DATA_DEFINITION_ID);

   DECL_ERROR(EventError, IOError);
   DECL_ERROR_INFO(SimConnectFunctionInfo, std::string);


   class DataDefinition
   {
   public:

      inline DataDefinition(const SimConnectClient& cli, 
            SIMCONNECT_DATA_DEFINITION_ID id) :
         _handle(cli._handle), _id(id)
      {}

      DataDefinition& add(const DataName& name, const DataUnits& units,
               SIMCONNECT_DATATYPE data_type = SIMCONNECT_DATATYPE_FLOAT64)
            throw (DataDefinitionError);

      inline SIMCONNECT_DATA_DEFINITION_ID id() const
      { return _id; }

   private:

      HANDLE _handle;
      SIMCONNECT_DATA_DEFINITION_ID _id;
   };

   class DataPullRequest
   {
   public:

      inline DataPullRequest(const SimConnectClient& cli,
            const DataDefinition& data_def,
            SIMCONNECT_DATA_REQUEST_ID id) :
         _data_def(data_def.id()),_handle(cli._handle), _id(id), 
         _object(SIMCONNECT_OBJECT_ID_USER), _period(SIMCONNECT_PERIOD_ONCE), 
         _flags(SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT), _origin(0), 
         _interval(0), _limit(0)
      {}

      inline DataPullRequest& setObject(SIMCONNECT_OBJECT_ID object)
      { _object = object; return *this; }

      inline DataPullRequest& setPeriod(SIMCONNECT_PERIOD period)
      { _period = period; return *this; }

      inline DataPullRequest& setFlags(SIMCONNECT_DATA_REQUEST_FLAG flags)
      { _flags = flags; return *this; }

      inline DataPullRequest& setOrigin(DWORD origin)
      { _origin = origin; return *this; }

      inline DataPullRequest& setInterval(DWORD interval)
      { _interval = interval; return *this; }

      inline DataPullRequest& setLimit(DWORD limit)
      { _limit = limit; return *this; }

      void submit() throw (DataRequestError);

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

   class DataPushRequest
   {
   public:

      inline DataPushRequest(const SimConnectClient& cli,
            const DataDefinition& data_def) :
         _handle(cli._handle), _data_def(data_def.id()),
         _object(SIMCONNECT_OBJECT_ID_USER),
         _flags(SIMCONNECT_DATA_SET_FLAG_DEFAULT), _count(0), 
         _element_size(sizeof(double))
      {}

      inline DataPushRequest& setObject(SIMCONNECT_OBJECT_ID object)
      { _object = object; return *this; }

      inline DataPushRequest& setFlags(SIMCONNECT_DATA_SET_FLAG flags)
      { _flags = flags; return *this; }

      inline DataPushRequest& setCount(DWORD count)
      { _count = count; return *this; }

      inline DataPushRequest& setElementSize(DWORD element_size)
      { _element_size = element_size; return *this; }

      void submit(void* data) throw (DataRequestError);

   private:

      HANDLE _handle;
      SIMCONNECT_DATA_DEFINITION_ID _data_def;
      SIMCONNECT_OBJECT_ID _object;
      SIMCONNECT_DATA_SET_FLAG _flags;
      DWORD _count;
      DWORD _element_size;
   };

   class ClientEvent
   {
   public:

      ClientEvent(const SimConnectClient& cli,
                  const EventName& event_name,
                  SIMCONNECT_CLIENT_EVENT_ID id) throw (EventError);

      ClientEvent(const SimConnectClient& cli,
                  SIMCONNECT_CLIENT_EVENT_ID id) throw (EventError);

      inline ClientEvent& setObject(SIMCONNECT_OBJECT_ID object)
      { _object = object; return *this; }

      inline ClientEvent& setGroup(SIMCONNECT_NOTIFICATION_GROUP_ID group)
      { _group = group; return *this; }

      inline ClientEvent& setFlags(SIMCONNECT_EVENT_FLAG flags)
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
   class VariableWatch
   {
   public:

      inline VariableWatch() :
         _client("Variable Watch"), _data_def(_client.newDataDefinition())
      {
      }

      inline DataDefinition dataDefinition()
      { return _data_def; }

      inline T get() const
      {      
         bool done = false;
         T data;
         _client.registerOnSimObjectDataCallback([&data, &done](
            SimConnectClient& client, const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)
         {
            memcpy(&data, &msg.dwData, sizeof(T));
            done = true;
         });
         _client.newDataPullRequest(_data_def).submit();
         while (!done)
            _client.dispatchMessage();
         return data;
      }

      inline void set(const T& t)
      {
         _client.newDataPushRequest(_data_def)
               .setElementSize(sizeof(T))
               .submit((void*) &t);
      }

   protected:

      mutable SimConnectClient _client;
      DataDefinition _data_def;
   };

   class EventTransmitter
   {
   public:

      EventTransmitter(const SimConnectClient::EventName& event_name);

      inline void transmit(DWORD value)
      { _event.transmit(value); }

   private:

      Ptr<SimConnectClient> _client;
      ClientEvent _event;
   };

   typedef std::function<void(
         SimConnectClient& client, 
         const SIMCONNECT_RECV& msg)> OnNullCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_EXCEPTION& msg)> OnExceptionCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_OPEN& msg)> OnOpenCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV& msg)> OnQuitCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_EVENT& msg)> OnEventCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE& msg)> 
               OnEventObjectAddRemoveCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_EVENT_FILENAME& msg)> OnEventFilenameCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_EVENT_FRAME& msg)> OnEventFrameCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)> OnSimObjectDataCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE& msg)> 
               OnSimObjectDataByTypeCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_WEATHER_OBSERVATION& msg)> 
               OnWeatherObservationCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_CLOUD_STATE& msg)> OnCloudStateCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_ASSIGNED_OBJECT_ID& msg)> 
               OnAssignedObjectIDCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_RESERVED_KEY & msg)> OnReservedKeyCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_CUSTOM_ACTION& msg)> OnCustomActionCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_SYSTEM_STATE& msg)> OnSystemStateCallback;

   typedef std::function<void(
         SimConnectClient& client,
         const SIMCONNECT_RECV_CLIENT_DATA& msg)> OnClientDataCallback;

   static const EventName SYSTEM_EVENT_1SEC;
   static const EventName SYSTEM_EVENT_4SEC;
   static const EventName SYSTEM_EVENT_6HZ;
   static const EventName SYSTEM_EVENT_FRAME;
   static const EventName SYSTEM_EVENT_AIRCRAFT_LOADED;
   static const EventName SYSTEM_EVENT_FLIGHT_LOADED;

   SimConnectClient(const std::string& name) throw (ConnectionError);

   SimConnectClient(const std::string& name,
         const OnOpenCallback& open_callback) throw (ConnectionError);

   virtual ~SimConnectClient();

   /**
    * Dispatch an incoming message (if any) from the server. When compiled
    * into a DLL plugin, it has no effect. 
    */
   void dispatchMessage();

   /**
    * Callback function member for incoming messages from SimConnect. The
    * default implementation dispatches the message to the previously
    * registered callback, if any. It may be overriden if needed, but doing
    * that may break the callback registration mechanism, so do it with care.
    */
   virtual void onMessage(SIMCONNECT_RECV* msg, DWORD msg_len);

   void registerOnNullCallback(const OnNullCallback& callback);

   void registerOnExceptionCallback(const OnExceptionCallback& callback);

   void registerOnOpenCallback(const OnOpenCallback& callback);

   void registerOnQuitCallback(const OnQuitCallback& callback);

   void registerOnEventCallback(const OnEventCallback& callback);

   void registerOnEventObjectAddRemoveCallback(
         const OnEventObjectAddRemoveCallback& callback);

   void registerOnEventFilenameCallback(const OnEventFilenameCallback& callback);

   void registerOnEventFrameCallback(const OnEventFrameCallback& callback);

   void registerOnSimObjectDataCallback(const OnSimObjectDataCallback& callback);

   void registerOnSimObjectDataByTypeCallback(
         const OnSimObjectDataByTypeCallback& callback);

   void registerOnWeatherObservationCallback(
         const OnWeatherObservationCallback& callback);

   void registerOnCloudStateCallback(const OnCloudStateCallback& callback);

   void registerOnAssignedObjectIDCallback(
         const OnAssignedObjectIDCallback& callback);

   void registerOnReservedKeyCallback(const OnReservedKeyCallback& callback);

   void registerOnCustomActionCallback(const OnCustomActionCallback& callback);

   void registerOnSystemStateCallback(const OnSystemStateCallback& callback);

   void registerOnClientDataCallback(const OnClientDataCallback& callback);

   void subscribeToSystemEvent(
         const EventName& event_name,
         SIMCONNECT_CLIENT_EVENT_ID event_id = 0) throw (UnknownEventNameError);

   DataDefinition newDataDefinition();

   DataPullRequest newDataPullRequest(const DataDefinition& data_def);

   DataPushRequest newDataPushRequest(const DataDefinition& data_def);

   ClientEvent newClientEvent(const EventName& event_name);

private:

   class AbstractMessageReceiver
   {
   public:
      virtual void sendMessage(
            SimConnectClient& client, SIMCONNECT_RECV* msg) = 0;
      virtual ~AbstractMessageReceiver() {}
   };

   template <typename Message>
   class MessageReceiver : public AbstractMessageReceiver
   {
   public:

      inline MessageReceiver(const std::function<void(
            SimConnectClient&, const Message&)>& callback) :
         _callback(callback)
      {}

      virtual void sendMessage(SimConnectClient& client, SIMCONNECT_RECV* msg)
      {
         auto narrowed_msg = static_cast<Message*>(msg);
         _callback(client, *narrowed_msg);
      }

   private:
      std::function<void(SimConnectClient&, const Message&)> _callback;
   };

   template <typename Message>
   void registerCallback(
         const std::function<void(SimConnectClient&, const Message&)>& callback,
         SIMCONNECT_RECV_ID message_type)
   {
      this->receiver(message_type) = new MessageReceiver<Message>(callback);
   }

   void open() throw (ConnectionError);

   Ptr<AbstractMessageReceiver>& receiver(SIMCONNECT_RECV_ID message_type);

   std::string _name;
   HANDLE _handle;
   std::vector<Ptr<AbstractMessageReceiver>> _msg_receivers;
};

}; // namespace oac

#endif
