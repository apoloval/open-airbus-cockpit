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
#include "pointer.h"

namespace oac {

class SimConnectClient 
{
public:

   DECL_RUNTIME_ERROR(ConnectionException);

   typedef std::string EventName;
   typedef std::string DataName;
   typedef std::string DataUnits;

   class DataDefinition
   {
   public:

      inline DataDefinition(const SimConnectClient& cli, 
            SIMCONNECT_DATA_DEFINITION_ID id) :
         _handle(cli._handle), _id(id)
      {}

      DataDefinition& add(const DataName& name, const DataUnits& units,
               SIMCONNECT_DATATYPE data_type = SIMCONNECT_DATATYPE_FLOAT64)
            throw (InvalidInputException);

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

      void submit() throw (InvalidInputException);

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

      void submit(void* data);

   private:

      HANDLE _handle;
      SIMCONNECT_DATA_DEFINITION_ID _data_def;
      SIMCONNECT_OBJECT_ID _object;
      SIMCONNECT_DATA_SET_FLAG _flags;
      DWORD _count;
      DWORD _element_size;
   };

   static const EventName SYSTEM_EVENT_1SEC;
   static const EventName SYSTEM_EVENT_4SEC;
   static const EventName SYSTEM_EVENT_6HZ;
   static const EventName SYSTEM_EVENT_FRAME;

   SimConnectClient(const std::string& name) throw (ConnectionException);

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

   void registerOnNullCallback(
         const std::function<void(const SIMCONNECT_RECV& msg)>& callback);

   void registerOnExceptionCallback(
         const std::function<void(
               const SIMCONNECT_RECV_EXCEPTION& msg)>& callback);

   void registerOnOpenCallback(
         const std::function<void(const SIMCONNECT_RECV_OPEN& msg)>& callback);

   void registerOnQuitCallback(
         const std::function<void(const SIMCONNECT_RECV& msg)>& callback);

   void registerOnEventCallback(
         const std::function<void(const SIMCONNECT_RECV_EVENT& msg)>& callback);

   void registerOnEventObjectAddRemoveCallback(
         const std::function<void(
               const SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE& msg)>& callback);

   void registerOnEventFilenameCallback(
         const std::function<void(
               const SIMCONNECT_RECV_EVENT_FILENAME& msg)>& callback);

   void registerOnEventFrameCallback(
         const std::function<void(
               const SIMCONNECT_RECV_EVENT_FRAME& msg)>& callback);

   void registerOnSimObjectDataCallback(
         const std::function<void(
               const SIMCONNECT_RECV_SIMOBJECT_DATA& msg)>& callback);

   void registerOnSimObjectDataByTypeCallback(
         const std::function<void(
               const SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE& msg)>& callback);

   void registerOnWeatherObservationCallback(
         const std::function<void(
               const SIMCONNECT_RECV_WEATHER_OBSERVATION& msg)>& callback);

   void registerOnCloudStateCallback(
         const std::function<void(
               const SIMCONNECT_RECV_CLOUD_STATE& msg)>& callback);

   void registerOnAssignedObjectIDCallback(
         const std::function<void(
               const SIMCONNECT_RECV_ASSIGNED_OBJECT_ID& msg)>& callback);

   void registerOnReservedKeyCallback(
         const std::function<void(
               const SIMCONNECT_RECV_RESERVED_KEY & msg)>& callback);

   void registerOnCustomActionCallback(
         const std::function<void(
               const SIMCONNECT_RECV_CUSTOM_ACTION& msg)>& callback);

   void registerOnSystemStateCallback(
         const std::function<void(
               const SIMCONNECT_RECV_SYSTEM_STATE& msg)>& callback);

   void registerOnClientDataCallback(
         const std::function<void(
               const SIMCONNECT_RECV_CLIENT_DATA& msg)>& callback);

   void subscribeToSystemEvent(
         const EventName& event_name,
         SIMCONNECT_CLIENT_EVENT_ID event_id = 0) throw (InvalidInputException);

   DataDefinition newDataDefinition();

   DataPullRequest newDataPullRequest(const DataDefinition& data_def);

   DataPushRequest newDataPushRequest(const DataDefinition& data_def);

private:

   class AbstractMessageReceiver
   {
   public:
      virtual void sendMessage(SIMCONNECT_RECV* msg) = 0;
      virtual ~AbstractMessageReceiver() {}
   };

   template <typename Message>
   class MessageReceiver : public AbstractMessageReceiver
   {
   public:

      inline MessageReceiver(const std::function<void(const Message&)>& callback) :
         _callback(callback)
      {}

      virtual void sendMessage(SIMCONNECT_RECV* msg)
      {
         auto narrowed_msg = (Message*)(msg);
         _callback(*narrowed_msg);
      }

   private:
      std::function<void(const Message&)> _callback;
   };

   template <typename Message>
   void registerCallback(
         const std::function<void(const Message&)>& callback,
         SIMCONNECT_RECV_ID message_type)
   {
      if (_msg_receivers.size() <= std::size_t(message_type))
         _msg_receivers.resize(message_type + 1);
      _msg_receivers[message_type] = new MessageReceiver<Message>(callback);
   }

   std::string _name;
   HANDLE _handle;
   std::vector<Ptr<AbstractMessageReceiver>> _msg_receivers;
};

}; // namespace oac

#endif
