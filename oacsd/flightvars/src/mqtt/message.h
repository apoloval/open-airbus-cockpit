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

#ifndef OAC_FV_MQTT_MESSAGE_H
#define OAC_FV_MQTT_MESSAGE_H

#include <liboac/buffer/fixed.h>
#include <liboac/exception.h>

#include "mqtt/topic.h"
#include "mqtt/qos.h"

namespace oac { namespace fv { namespace mqtt {

/** A MQTT message. */
template <typename Data>
struct message
{
   /** The topic the message was sent to. */
   const topic tpc;

   /** The message data. */
   const Data data;

   /** The QoS level the message was sent. */
   const qos_level qos;

   /** The retain flag of the message. */
   const bool retain;
};

/**
 * A special form of message where data is represented by a fixed buffer.
 *
 * The fixed buffer is guarranted to contain exactly the same capacity as
 * the size of the message data.
 */
template <>
struct message<buffer::fixed_buffer>
{
   OAC_DECL_EXCEPTION_WITH_PARAMS(conversion_error, oac::exception,
      ("cannot convert buffered message with %d bytes "
            "to a message data of %d bytes", actual_size, expected_size),
      (expected_size, std::size_t),
      (actual_size, std::size_t)
   );

   /** The topic the message was sent to. */
   topic tpc;

   /** The message data. */
   buffer::fixed_buffer data;

   /** The QoS level the message was sent. */
   qos_level qos;

   /** The retain flag of the message. */
   bool retain;

   message(
         const topic& tpc,
         const void* data,
         std::size_t data_len,
         const qos_level& qos,
         bool retain)
    : tpc { tpc },
      data { data_len },
      qos { qos },
      retain { retain }
   {
      this->data.write(data, 0, data_len);
   }

   template <typename Data>
   message<Data> convert_to() const
   throw (conversion_error)
   {
      auto expected_data_len = sizeof(Data);
      auto actual_data_len = data.capacity();
      if (expected_data_len == actual_data_len)
         return message<Data>
         { tpc, *static_cast<const Data*>(data.data()), qos, retain };
      else
         OAC_THROW_EXCEPTION(
               conversion_error(expected_data_len, actual_data_len));
   }
};

using buffered_message = message<buffer::fixed_buffer>;

}}} // namespace oac::fv::mqtt

#endif
