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
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OAC_FV_PROTO_TYPES_H
#define OAC_FV_PROTO_TYPES_H

#include <cstdint>
#include <string>

#ifndef FLIGHTVARS_PROTOCOL_VERSION
#define FLIGHTVARS_PROTOCOL_VERSION 0x0100
#endif

namespace oac { namespace fv { namespace proto {

/**
 * A 16-bits number indicating the version of the protocol.
 */
typedef std::uint16_t protocol_version;

/**
 * The name of a peer that communicates using the protocol.
 */
typedef std::string peer_name;

/**
 * An enumeration for the different types or messages that comprise the
 * protocol.
 */
enum class message_type
{
   BEGIN_SESSION,
   END_SESSION,
   SUBSCRIPTION_REQ,
   SUBSCRIPTION_REP,
   UNSUBSCRIPTION_REQ,
   UNSUBSCRIPTION_REP,
   VAR_UPDATE
};

/**
 * Convert a message type enum value into a string object.
 */
inline
std::string
message_type_to_string(
      message_type msg_type)
{
   switch (msg_type)
   {
      case message_type::BEGIN_SESSION:
         return "begin session message";
      case message_type::END_SESSION:
         return "end session message";
      case message_type::SUBSCRIPTION_REQ:
         return "subscription request message";
      case message_type::SUBSCRIPTION_REP:
         return "subscription reply message";
      case message_type::UNSUBSCRIPTION_REQ:
         return "unsubscription request message";
      case message_type::UNSUBSCRIPTION_REP:
         return "unsubscription reply message";
      case message_type::VAR_UPDATE:
         return "variable update message";
      default:
         OAC_THROW_EXCEPTION(enum_out_of_range_error<message_type>(msg_type));
   }
}

}}} // namespace oac::fv::proto

#endif
