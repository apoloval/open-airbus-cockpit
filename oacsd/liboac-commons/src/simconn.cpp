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

#include <Boost/format.hpp>

#include "logging.h"
#include "simconn.h"

namespace oac {

namespace {

void CALLBACK DispatchMessage(
      SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
   try 
   {
      auto receiver = static_cast<SimConnectClient*>(pContext);
      receiver->onMessage(pData, cbData);
   } catch (std::exception& ex) {
      Log(WARN, ex.what());
   }
}

void OnOpenIgnore(SimConnectClient& client, const SIMCONNECT_RECV_OPEN& msg)
{}

}; // anonymous namespace

SimConnectClient::DataDefinition&
SimConnectClient::DataDefinition::add(
      const DataName& name, 
      const DataUnits& units, 
      SIMCONNECT_DATATYPE data_type)
throw (DataDefinitionError)
{ 
   auto result = SimConnect_AddToDataDefinition(
         _handle, _id, name.c_str(), units.c_str(), data_type); 
   if (result != S_OK)
      THROW_ERROR(DataDefinitionError() <<
            DataNameInfo(name) << DataUnitsInfo(units));
   return *this;
}

void
SimConnectClient::DataPullRequest::submit()
throw (DataRequestError)
{
   auto result = SimConnect_RequestDataOnSimObject(
         _handle, _id, _data_def, _object, _period, 
         _flags, _origin, _interval, _limit);
   if (result != S_OK)
      THROW_ERROR(DataRequestError() << DataDefinitionInfo(_data_def));
}

void
SimConnectClient::DataPushRequest::submit(void* data)
throw (DataRequestError)
{
   auto result = SimConnect_SetDataOnSimObject(
         _handle, _data_def, _object, _flags, _count, _element_size, data);
   if (result != S_OK)
      THROW_ERROR(DataRequestError() << DataDefinitionInfo(_data_def));
}

SimConnectClient::ClientEvent::ClientEvent(
      const SimConnectClient& cli,
      const EventName& event_name,
      SIMCONNECT_CLIENT_EVENT_ID id)
throw (EventError) :
   _handle(cli._handle), _id(id), _object(SIMCONNECT_OBJECT_ID_USER),
   _group(SIMCONNECT_GROUP_PRIORITY_HIGHEST), 
   _flags(SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY)
{
   auto result = SimConnect_MapClientEventToSimEvent(
         _handle, _id, event_name.c_str());
   if (result != S_OK)
      THROW_ERROR(EventError() <<
            SimConnectFunctionInfo("SimConnect_MapClientEventToSimEvent") <<
            EventNameInfo(event_name));
}

SimConnectClient::ClientEvent::ClientEvent(
      const SimConnectClient& cli,
      SIMCONNECT_CLIENT_EVENT_ID id)
throw (EventError) :
   _handle(cli._handle), _id(id), _object(SIMCONNECT_OBJECT_ID_USER),
   _group(SIMCONNECT_GROUP_PRIORITY_HIGHEST), 
   _flags(SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY)
{
   auto result = SimConnect_MapClientEventToSimEvent(_handle, _id);
   if (result != S_OK)
      THROW_ERROR(EventError() <<
            SimConnectFunctionInfo("SimConnect_MapClientEventToSimEvent"));
}

void 
SimConnectClient::ClientEvent::transmit(DWORD value)
throw (EventError)
{
   auto result = SimConnect_TransmitClientEvent(
         _handle, _object, _id, value, _group, _flags);
   if (result != S_OK)
      THROW_ERROR(EventError() <<
            SimConnectFunctionInfo("SimConnect_TransmitClientEvent"));
}

SimConnectClient::EventTransmitter::EventTransmitter(
      const SimConnectClient::EventName& event_name) :
   _client(new SimConnectClient("Event Transmitter")),
   _event(_client->newClientEvent(event_name))
{}


const SimConnectClient::EventName
SimConnectClient::SYSTEM_EVENT_1SEC("1sec");

const SimConnectClient::EventName
SimConnectClient::SYSTEM_EVENT_4SEC("4sec");

const SimConnectClient::EventName
SimConnectClient::SYSTEM_EVENT_6HZ("6hz");

const SimConnectClient::EventName
SimConnectClient::SYSTEM_EVENT_FRAME("Frame");

const SimConnectClient::EventName
SimConnectClient::SYSTEM_EVENT_AIRCRAFT_LOADED("AircraftLoaded");

const SimConnectClient::EventName
SimConnectClient::SYSTEM_EVENT_FLIGHT_LOADED("FlightLoaded");

SimConnectClient::SimConnectClient(const std::string& name)
throw (ConnectionError) :
   _name(name)
{ 
   this->open(); 
   this->registerOnOpenCallback(OnOpenIgnore);
}

SimConnectClient::SimConnectClient(const std::string& name,
         const OnOpenCallback& open_callback) 
throw (ConnectionError) :
   _name(name)
{
   this->open();
   this->registerOnOpenCallback(open_callback);
}

SimConnectClient::~SimConnectClient()
{
   if (SimConnect_Close(_handle) != S_OK)
      Log(WARN, str(boost::format(
            "something failed while disconnecting %s from SimConnect")
            % _name));
}

void
SimConnectClient::dispatchMessage()
{ SimConnect_CallDispatch(_handle, DispatchMessage, this); }

void
SimConnectClient::onMessage(SIMCONNECT_RECV* msg, DWORD msg_len)
{
   try 
   {
      auto msg_id = msg->dwID;

      auto callback = this->receiver(SIMCONNECT_RECV_ID(msg_id));
      if (callback)
         callback->sendMessage(*this, msg);
      else
         Log(WARN, str(boost::format(
               "cannot deliver message with ID %d: no callback registered") %
               msg_id));
   }
   catch (const std::exception& ex)
   {
      Log(WARN, str(boost::format("cannot deliver SimConnect message: %s") %
            ex.what()));
   }
}

void 
SimConnectClient::registerOnNullCallback(const OnNullCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV>(callback, SIMCONNECT_RECV_ID_NULL);
}

void 
SimConnectClient::registerOnExceptionCallback(
      const OnExceptionCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_EXCEPTION>(
         callback, SIMCONNECT_RECV_ID_EXCEPTION);
}

void 
SimConnectClient::registerOnOpenCallback(const OnOpenCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_OPEN>(
      callback, SIMCONNECT_RECV_ID_OPEN);
}

void 
SimConnectClient::registerOnQuitCallback(const OnQuitCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV>(
      callback, SIMCONNECT_RECV_ID_QUIT);
}

void 
SimConnectClient::registerOnEventCallback(const OnEventCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_EVENT>(
      callback, SIMCONNECT_RECV_ID_EVENT);
}

void 
SimConnectClient::registerOnEventObjectAddRemoveCallback(
      const OnEventObjectAddRemoveCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE>(
      callback, SIMCONNECT_RECV_ID_EVENT_OBJECT_ADDREMOVE);
}

void 
SimConnectClient::registerOnEventFilenameCallback(
      const OnEventFilenameCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_EVENT_FILENAME>(
      callback, SIMCONNECT_RECV_ID_EVENT_FILENAME);
}

void 
SimConnectClient::registerOnEventFrameCallback(
      const OnEventFrameCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_EVENT_FRAME>(
      callback, SIMCONNECT_RECV_ID_EVENT_FRAME);
}

void 
SimConnectClient::registerOnSimObjectDataCallback(
      const OnSimObjectDataCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_SIMOBJECT_DATA>(
      callback, SIMCONNECT_RECV_ID_SIMOBJECT_DATA);
}

void 
SimConnectClient::registerOnSimObjectDataByTypeCallback(
      const OnSimObjectDataByTypeCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE>(
      callback, SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE);
}

void 
SimConnectClient::registerOnWeatherObservationCallback(
      const OnWeatherObservationCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_WEATHER_OBSERVATION>(
      callback, SIMCONNECT_RECV_ID_CLOUD_STATE);
}

void 
SimConnectClient::registerOnCloudStateCallback(
      const OnCloudStateCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_CLOUD_STATE>(
      callback, SIMCONNECT_RECV_ID_WEATHER_OBSERVATION);
}

void 
SimConnectClient::registerOnAssignedObjectIDCallback(
      const OnAssignedObjectIDCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_ASSIGNED_OBJECT_ID>(
      callback, SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID);
}

void 
SimConnectClient::registerOnReservedKeyCallback(
      const OnReservedKeyCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_RESERVED_KEY>(
      callback, SIMCONNECT_RECV_ID_RESERVED_KEY);
}

void 
SimConnectClient::registerOnCustomActionCallback(
      const OnCustomActionCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_CUSTOM_ACTION>(
      callback, SIMCONNECT_RECV_ID_CUSTOM_ACTION);
}

void 
SimConnectClient::registerOnSystemStateCallback(
      const OnSystemStateCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_SYSTEM_STATE>(
      callback, SIMCONNECT_RECV_ID_SYSTEM_STATE);
}

void 
SimConnectClient::registerOnClientDataCallback(
      const OnClientDataCallback& callback)
{
   this->registerCallback<SIMCONNECT_RECV_CLIENT_DATA>(
      callback, SIMCONNECT_RECV_ID_CLIENT_DATA);
}

void
SimConnectClient::subscribeToSystemEvent(
      const EventName& event_name,
      SIMCONNECT_CLIENT_EVENT_ID event_id)
throw (UnknownEventNameError)
{
   auto result = SimConnect_SubscribeToSystemEvent(
         _handle, event_id, event_name.c_str());
   if (result != S_OK)
      THROW_ERROR(UnknownEventNameError() << EventNameInfo(event_name));
}

SimConnectClient::DataDefinition
SimConnectClient::newDataDefinition()
{
   static SIMCONNECT_DATA_DEFINITION_ID next_id = 0xffff0000;
   return DataDefinition(*this, next_id++);
}

SimConnectClient::DataPullRequest 
SimConnectClient::newDataPullRequest(const DataDefinition& data_def)
{
   static SIMCONNECT_DATA_REQUEST_ID next_id = 0xffff0000;
   return DataPullRequest(*this, data_def, next_id++);
}

SimConnectClient::DataPushRequest 
SimConnectClient::newDataPushRequest(const DataDefinition& data_def)
{ return DataPushRequest(*this, data_def); }

SimConnectClient::ClientEvent
SimConnectClient::newClientEvent(const EventName& event_name)
{ 
   static SIMCONNECT_CLIENT_EVENT_ID next_id = 0xffff0000;
   return ClientEvent(*this, event_name, next_id++);
}

void
SimConnectClient::open()
throw (ConnectionError)
{
   if (SimConnect_Open(&_handle, _name.c_str(), NULL, 0, 0, 0) != S_OK)
      THROW_ERROR(ConnectionError());
   if (SimConnect_CallDispatch(_handle, DispatchMessage, this) != S_OK)
      THROW_ERROR(ConnectionError());
}

Ptr<SimConnectClient::AbstractMessageReceiver>&
SimConnectClient::receiver(SIMCONNECT_RECV_ID message_type)
{
   if (_msg_receivers.size() <= std::size_t(message_type))
         _msg_receivers.resize(message_type + 1);
   return _msg_receivers[message_type];
}

}; // namespace oac
