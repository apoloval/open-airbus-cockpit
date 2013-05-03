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

#ifndef OAC_FV_PROTOCOL_H
#define OAC_FV_PROTOCOL_H

#include <cstdint>
#include <string>

#include <boost/variant.hpp>

#include <liboac/exception.h>
#include <liboac/stream.h>

namespace oac { namespace fv { namespace proto {

typedef std::uint16_t protocol_version;

typedef std::string endpoint_name;

extern const protocol_version CURRENT_PROTOCOL_VERSION;

/**
 * Begin session message. This message is sent by the client when initiates
 * the session and the server as response to that. It indicates the endpoint
 * name and the protocol version it implements.
 */
struct begin_session_message {
   endpoint_name ep_name;
   protocol_version proto_ver;

   inline begin_session_message(
         const endpoint_name& ep_name,
         protocol_version proto_ver = CURRENT_PROTOCOL_VERSION)
      : ep_name(ep_name), proto_ver(proto_ver)
   {}
};

/**
 * This union wraps all kinds of messages into a single one.
 */
typedef boost::variant<
      begin_session_message
> message;

struct message_internals
{
   enum message_type {
      MSG_BEGIN_SESSION = 0x700
   };
};

/**
 * An abstraction for an object able to serialize messages.
 */
class message_serializer
{
public:

   inline virtual ~message_serializer() {}

   /**
    * Serialize given message into given output stream.
    */
   void serialize(const message& msg, output_stream& output);

protected:

   virtual void write_msg_begin(
         output_stream& output,
         message_internals::message_type msg_type) = 0;

   virtual void write_msg_end(output_stream& output) = 0;

   virtual void write_string_value(
         output_stream& output, const std::string& value) = 0;

   virtual void write_uint16_value(
         output_stream& output, std::uint16_t value) = 0;

private:

   struct visitor : public boost::static_visitor<void>
   {
      message_serializer& serializer;
      output_stream& output;

      inline visitor(message_serializer& serializer,
                     output_stream& output)
         : serializer(serializer), output(output)
      {}

      void operator()(const begin_session_message& msg) const;
   };
};

class message_deserializer
{
public:

   /**
    * Thrown when a protocol error is found. Contains:
    *  - expected_input_info, indicating the expected protocol element
    *  - actual_input_info, indicating the actual protocol element found
    */
   OAC_DECL_ERROR(protocol_error, invalid_input_error);

   OAC_DECL_ERROR_INFO(expected_input_info, std::string);
   OAC_DECL_ERROR_INFO(actual_input_info, std::string);

   inline virtual ~message_deserializer() {}

   message deserialize(input_stream& input) throw(protocol_error);

protected:

   virtual message_internals::message_type read_msg_begin(
         input_stream& input) throw (protocol_error) = 0;

   virtual void read_msg_end(
         input_stream& input) throw (protocol_error) = 0;

   virtual std::string read_string_value(
         input_stream& input) throw (protocol_error) = 0;

   virtual std::uint16_t read_uint16_value(
         input_stream& input) throw (protocol_error) = 0;

private:

   begin_session_message deserialize_begin_session(
         input_stream& input) throw(protocol_error);
};

class binary_message_serializer : public message_serializer
{
protected:

   virtual void write_msg_begin(
         output_stream& output,
         message_internals::message_type msg_type);

   virtual void write_msg_end(output_stream& output);

   virtual void write_string_value(
         output_stream& output, const std::string& value);

   virtual void write_uint16_value(
         output_stream& output, std::uint16_t value);
};

class binary_message_deserializer : public message_deserializer
{
protected:

   virtual message_internals::message_type read_msg_begin(
         input_stream& input) throw (protocol_error);

   virtual void read_msg_end(
         input_stream& input) throw (protocol_error);

   virtual std::string read_string_value(
         input_stream& input) throw (protocol_error);

   virtual std::uint16_t read_uint16_value(
         input_stream& input) throw (protocol_error);
};

}}} // namespace oac::fv::proto

#endif
