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
      auto receiver = static_cast<simconnect_client*>(pContext);
      receiver->on_message(pData, cbData);
   } catch (std::exception& ex) {
      log("SimConnect-DispatchMessage", WARN, ex.what());
   }
}

void OnOpenIgnore(simconnect_client& client, const SIMCONNECT_RECV_OPEN& msg)
{}

}; // anonymous namespace

simconnect_client::data_definition&
simconnect_client::data_definition::add(
      const data_name& name,
      const data_units& units,
      SIMCONNECT_DATATYPE data_type)
throw (data_definition_error)
{ 
   auto result = SimConnect_AddToDataDefinition(
         _handle, _id, name.c_str(), units.c_str(), data_type); 
   if (result != S_OK)
      OAC_THROW_EXCEPTION(data_definition_error()
            .with_name(name)
            .with_units(units));
   return *this;
}

void
simconnect_client::data_pull_request::submit()
throw (data_request_error)
{
   auto result = SimConnect_RequestDataOnSimObject(
         _handle, _id, _data_def, _object, _period, 
         _flags, _origin, _interval, _limit);
   if (result != S_OK)
      OAC_THROW_EXCEPTION(data_request_error()
            .with_data_def(_data_def));
}

void
simconnect_client::data_push_request::submit(void* data)
throw (data_request_error)
{
   auto result = SimConnect_SetDataOnSimObject(
         _handle, _data_def, _object, _flags, _count, _element_size, data);
   if (result != S_OK)
      OAC_THROW_EXCEPTION(data_request_error()
            .with_data_def(_data_def));
}

simconnect_client::client_event::client_event(
      const simconnect_client& cli,
      const event_name& ev_name,
      SIMCONNECT_CLIENT_EVENT_ID id)
throw (event_error) :
   _handle(cli._handle), _id(id), _object(SIMCONNECT_OBJECT_ID_USER),
   _group(SIMCONNECT_GROUP_PRIORITY_HIGHEST), 
   _flags(SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY)
{
   auto result = SimConnect_MapClientEventToSimEvent(
         _handle, _id, ev_name.c_str());
   if (result != S_OK)
      OAC_THROW_EXCEPTION(event_error()
            .with_event(_id));
}

simconnect_client::client_event::client_event(
      const simconnect_client& cli,
      SIMCONNECT_CLIENT_EVENT_ID id)
throw (event_error) :
   _handle(cli._handle), _id(id), _object(SIMCONNECT_OBJECT_ID_USER),
   _group(SIMCONNECT_GROUP_PRIORITY_HIGHEST), 
   _flags(SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY)
{
   auto result = SimConnect_MapClientEventToSimEvent(_handle, _id);
   if (result != S_OK)
      OAC_THROW_EXCEPTION(event_error()
            .with_event(_id));
}

void 
simconnect_client::client_event::transmit(DWORD value)
throw (event_error)
{
   auto result = SimConnect_TransmitClientEvent(
         _handle, _object, _id, value, _group, _flags);
   if (result != S_OK)
      OAC_THROW_EXCEPTION(event_error()
            .with_event(_id));
}

simconnect_client::event_transmitter::event_transmitter(
      const simconnect_client::event_name& event_name) :
   _client(new simconnect_client("Event Transmitter")),
   _event(_client->new_client_event(event_name))
{}


const simconnect_client::event_name
simconnect_client::SYSTEM_EVENT_1SEC("1sec");

const simconnect_client::event_name
simconnect_client::SYSTEM_EVENT_4SEC("4sec");

const simconnect_client::event_name
simconnect_client::SYSTEM_EVENT_6HZ("6hz");

const simconnect_client::event_name
simconnect_client::SYSTEM_EVENT_FRAME("Frame");

const simconnect_client::event_name
simconnect_client::SYSTEM_EVENT_AIRCRAFT_LOADED("AircraftLoaded");

const simconnect_client::event_name
simconnect_client::SYSTEM_EVENT_FLIGHT_LOADED("FlightLoaded");

simconnect_client::simconnect_client(
      const std::string& name)
throw (server_unavailable_error)
   : logger_component("simconnect_client"),
     _name(name)
{ 
   this->open(); 
   this->register_on_open_callback(OnOpenIgnore);
}

simconnect_client::simconnect_client(
      const std::string& name,
      const on_open_callback& open_callback)
throw (server_unavailable_error)
   : logger_component("simconnect_client"),
     _name(name)
{
   this->open();
   this->register_on_open_callback(open_callback);
}

simconnect_client::~simconnect_client()
{
   if (SimConnect_Close(_handle) != S_OK)
      log(WARN, str(boost::format(
            "something failed while disconnecting %s from SimConnect")
            % _name));
}

void
simconnect_client::dispatch_message()
{ SimConnect_CallDispatch(_handle, DispatchMessage, this); }

void
simconnect_client::on_message(SIMCONNECT_RECV* msg, DWORD msg_len)
{
   try 
   {
      auto msg_id = msg->dwID;

      auto callback = this->receiver(SIMCONNECT_RECV_ID(msg_id));
      if (callback)
         callback->send_message(*this, msg);
      else
         log(WARN, str(boost::format(
               "cannot deliver message with ID %d: no callback registered") %
               msg_id));
   }
   catch (const std::exception& ex)
   {
      log(WARN, str(boost::format("cannot deliver SimConnect message: %s") %
            ex.what()));
   }
}

void 
simconnect_client::register_on_null_callback(const on_null_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV>(callback, SIMCONNECT_RECV_ID_NULL);
}

void 
simconnect_client::register_on_exception_callback(
      const on_exception_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_EXCEPTION>(
         callback, SIMCONNECT_RECV_ID_EXCEPTION);
}

void 
simconnect_client::register_on_open_callback(const on_open_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_OPEN>(
      callback, SIMCONNECT_RECV_ID_OPEN);
}

void 
simconnect_client::register_on_quit_callback(const on_quit_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV>(
      callback, SIMCONNECT_RECV_ID_QUIT);
}

void 
simconnect_client::register_on_event_callback(const on_event_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_EVENT>(
      callback, SIMCONNECT_RECV_ID_EVENT);
}

void 
simconnect_client::register_on_event_object_add_remove_callback(
      const on_event_object_add_remove_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE>(
      callback, SIMCONNECT_RECV_ID_EVENT_OBJECT_ADDREMOVE);
}

void 
simconnect_client::register_on_event_filename_callback(
      const on_event_filename_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_EVENT_FILENAME>(
      callback, SIMCONNECT_RECV_ID_EVENT_FILENAME);
}

void 
simconnect_client::register_on_event_frame_callback(
      const on_event_frame_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_EVENT_FRAME>(
      callback, SIMCONNECT_RECV_ID_EVENT_FRAME);
}

void 
simconnect_client::register_on_simobject_data_callback(
      const on_simobject_data_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_SIMOBJECT_DATA>(
      callback, SIMCONNECT_RECV_ID_SIMOBJECT_DATA);
}

void 
simconnect_client::register_on_simobject_data_by_type_callback(
      const on_simobject_data_by_type_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE>(
      callback, SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE);
}

void 
simconnect_client::register_on_weather_observation_callback(
      const on_weather_observation_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_WEATHER_OBSERVATION>(
      callback, SIMCONNECT_RECV_ID_CLOUD_STATE);
}

void 
simconnect_client::register_on_cloud_state_callback(
      const on_cloud_state_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_CLOUD_STATE>(
      callback, SIMCONNECT_RECV_ID_WEATHER_OBSERVATION);
}

void 
simconnect_client::register_on_assigned_object_id_callback(
      const on_assigned_object_id_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_ASSIGNED_OBJECT_ID>(
      callback, SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID);
}

void 
simconnect_client::register_on_reserved_key_callback(
      const on_reserved_key_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_RESERVED_KEY>(
      callback, SIMCONNECT_RECV_ID_RESERVED_KEY);
}

void 
simconnect_client::register_on_custom_action_callback(
      const on_custom_action_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_CUSTOM_ACTION>(
      callback, SIMCONNECT_RECV_ID_CUSTOM_ACTION);
}

void 
simconnect_client::register_on_system_state_callback(
      const on_system_state_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_SYSTEM_STATE>(
      callback, SIMCONNECT_RECV_ID_SYSTEM_STATE);
}

void 
simconnect_client::register_on_client_data_callback(
      const on_client_data_callback& callback)
{
   this->register_callback<SIMCONNECT_RECV_CLIENT_DATA>(
      callback, SIMCONNECT_RECV_ID_CLIENT_DATA);
}

void
simconnect_client::subscribe_to_system_event(
      const event_name& event_name,
      SIMCONNECT_CLIENT_EVENT_ID event_id)
throw (unknown_event_name_error)
{
   auto result = SimConnect_SubscribeToSystemEvent(
         _handle, event_id, event_name.c_str());
   if (result != S_OK)
      OAC_THROW_EXCEPTION(unknown_event_name_error()
            .with_event(event_name));
}

simconnect_client::data_definition
simconnect_client::new_data_definition()
{
   static SIMCONNECT_DATA_DEFINITION_ID next_id = 0xffff0000;
   return data_definition(*this, next_id++);
}

simconnect_client::data_pull_request
simconnect_client::new_data_pull_request(const data_definition& data_def)
{
   static SIMCONNECT_DATA_REQUEST_ID next_id = 0xffff0000;
   return data_pull_request(*this, data_def, next_id++);
}

simconnect_client::data_push_request
simconnect_client::new_data_push_request(const data_definition& data_def)
{ return data_push_request(*this, data_def); }

simconnect_client::client_event
simconnect_client::new_client_event(const event_name& event_name)
{ 
   static SIMCONNECT_CLIENT_EVENT_ID next_id = 0xffff0000;
   return client_event(*this, event_name, next_id++);
}

void
simconnect_client::open()
throw (server_unavailable_error)
{
   if (SimConnect_Open(&_handle, _name.c_str(), NULL, 0, 0, 0) != S_OK)
      OAC_THROW_EXCEPTION(server_unavailable_error());
   if (SimConnect_CallDispatch(_handle, DispatchMessage, this) != S_OK)
      OAC_THROW_EXCEPTION(server_unavailable_error());
}

ptr<simconnect_client::abstract_message_receiver>&
simconnect_client::receiver(SIMCONNECT_RECV_ID message_type)
{
   if (_msg_receivers.size() <= std::size_t(message_type))
         _msg_receivers.resize(message_type + 1);
   return _msg_receivers[message_type];
}

}; // namespace oac
