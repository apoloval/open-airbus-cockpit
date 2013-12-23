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

/** A typed MQTT message. */
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

   /** Convert this typed message into a raw message. */
   message<buffer::fixed_buffer> to_raw() const;
};

/** Make a typed message from its parameters. */
template <typename Data>
message<Data>
make_typed_message(
      const topic& tpc,
      const Data& data,
      const qos_level& qos = qos_level::LEVEL_0,
      bool retain = false)
{
   return { tpc, data, qos, retain };
}

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
   message<Data> to_typed() const
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

using raw_message = message<buffer::fixed_buffer>;

template <typename Data>
message<buffer::fixed_buffer>
message<Data>::to_raw() const
{
   return { tpc, &data, sizeof(data), qos, retain };
}

/** An object able to publish MQTT messages. */
class message_publisher
{
public:

   /** Publish a MQTT message. */
   virtual void publish(const raw_message& msg) = 0;

   template <typename Data>
   void publish_as(const message<Data>& msg)
   {
      publish(msg.to_raw());
   }

   /**
    * Publish a new message from an object on given topic.
    */
   template <typename T>
   void publish_as(
         const topic& tpc,
         const T& data,
         const qos_level& qos = qos_level::LEVEL_0,
         bool retain = false)
   {
      publish_as(make_typed_message(tpc, data, qos, retain));
   }

};

using message_publisher_ptr = std::shared_ptr<message_publisher>;

}}} // namespace oac::fv::mqtt

#endif
