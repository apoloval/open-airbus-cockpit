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

#ifndef OAC_FV_PROTO_ERRORS_H
#define OAC_FV_PROTO_ERRORS_H

#include <cstdint>

#include <liboac/exception.h>

#include <flightvars/proto/types.h>

namespace oac { namespace fv { namespace proto {

/**
 * An exception indicating a protocol error.
 */
OAC_DECL_ABSTRACT_EXCEPTION(protocol_exception);

/**
 * An exception indicating the reception of a unexpected message.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(unexpected_message_error, protocol_exception,
   ("unexpected %s received", message_type_to_string(msg_type)),
   (msg_type, message_type));

/**
 * An exception indicating a invalid variable type code while deserializing.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(invalid_variable_type, protocol_exception,
   ("invalid variable type code 0x%x received", var_code),
   (var_code, std::uint8_t));

/**
 * An exception indicating a invalid message type code while deserializing.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(invalid_message_type, protocol_exception,
   ("invalid message type code 0x%x received", message_code),
   (message_code, std::uint16_t));

/**
 * An exception indicating a invalid message termination mark
 * while deserializing.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(invalid_termination_mark, protocol_exception,
   (
      "invalid termination mark 0x%x received (expected 0x0D0A)",
      termination_mark
   ),
   (termination_mark, std::uint16_t));

}}} // namespace oac::fv::proto

#endif
