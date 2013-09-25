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

#ifndef OAC_FV_PROTO_BINARY_H
#define OAC_FV_PROTO_BINARY_H

#include <liboac/endian.h>

#include <flightvars/proto/messages.h>

namespace oac { namespace fv { namespace proto {

inline std::uint16_t msg_type_to_code(message_type msg_type)
{
   return std::uint16_t(msg_type) + 0x700;
}

inline message_type code_to_msg_type(std::uint16_t code)
{
   return message_type(code - 0x700);
}

inline std::uint8_t var_type_to_code(variable_type var_type)
{
   return std::uint8_t(var_type);
}

inline variable_type code_to_var_type(std::uint8_t code)
{
   return variable_type(code);
}

struct binary_message_serializer
{

   template <typename OutputStream>
   static void
   write_msg_begin(
         OutputStream& output,
         message_type msg_type)
   throw (io_exception)
   {
      stream::write_as(
               output,
               native_to_big<std::uint16_t>(msg_type_to_code(msg_type)));
   }

   template <typename OutputStream>
   static void
   write_msg_end(
         OutputStream& output)
   throw (io_exception)
   {
      stream::write_as(output, native_to_big<std::uint16_t>(0x0d0a));
   }

   template <typename OutputStream>
   static void
   write_string_value(
         OutputStream& output,
         const std::string& value)
   throw (io_exception)
   {
      stream::write_as(output, native_to_big<std::uint16_t>(value.length()));
      stream::write_as_string(output, value);
   }

   template <typename OutputStream>
   static void
   write_uint8_value(
         OutputStream& output,
         std::uint8_t value)
   throw (io_exception)
   {
      stream::write_as(output, value);
   }

   template <typename OutputStream>
   static void
   write_uint16_value(
         OutputStream& output,
         std::uint16_t value)
   throw (io_exception)
   {
      stream::write_as(output, native_to_big<std::uint16_t>(value));
   }

   template <typename OutputStream>
   static void
   write_uint32_value(
         OutputStream& output,
         std::uint32_t value)
   throw (io_exception)
   {
      stream::write_as(output, native_to_big<std::uint32_t>(value));
   }

   template <typename OutputStream>
   static void
   write_float_value(
         OutputStream& output,
         float value)
   throw (io_exception)
   {
      // Break the number info binary significant and integral exponent for 2.
      // Then, write the normalized significant and exponent as 32-bits values.
      int exp = 0;
      auto sig = std::frexp(value, &exp);
      std::uint32_t nsig = 2 * std::uint32_t((sig - 0.5f) * UINT32_MAX);
      stream::write_as(output, native_to_big<std::uint32_t>(nsig));
      stream::write_as(output, native_to_big<std::uint32_t>(exp));
   }
};

struct binary_message_deserializer
{
   template <typename InputStream>
   static message_type
   read_msg_begin(
         InputStream& input)
   throw (protocol_exception, io_exception)
   {
      return code_to_msg_type(
               big_to_native(stream::read_as<std::uint16_t>(input)));
   }

   template <typename InputStream>
   static void
   read_msg_end(InputStream& input)
   throw (protocol_exception, io_exception)
   {
      auto eol = native_to_big(stream::read_as<uint16_t>(input));
      if (eol != 0x0d0a)
         OAC_THROW_EXCEPTION(invalid_termination_mark(eol));
   }

   template <typename InputStream>
   static std::string
   read_string_value(InputStream& input)
   throw (protocol_exception, io_exception)
   {
      auto str_len = big_to_native(stream::read_as<std::uint16_t>(input));
      return stream::read_as_string(input, str_len);
   }

   template <typename InputStream>
   static std::uint16_t
   read_uint16_value(InputStream& input)
   throw (protocol_exception, io_exception)
   {
      return big_to_native(stream::read_as<std::uint16_t>(input));
   }

   template <typename InputStream>
   static std::uint8_t
   read_uint8_value(InputStream& input)
   throw (protocol_exception, io_exception)
   {
      return stream::read_as<std::uint8_t>(input);
   }

   template <typename InputStream>
   static std::uint32_t
   read_uint32_value(
         InputStream& input)
   throw (protocol_exception, io_exception)
   {
      return big_to_native(stream::read_as<std::uint32_t>(input));
   }

   template <typename InputStream>
   static float
   read_float_value(
         InputStream& input)
   throw (protocol_exception, io_exception)
   {
      // The stream contains the normalized binary significant and the exponent
      // for 2. We extract them and convert again into float.
      auto nsig = big_to_native(stream::read_as<std::uint32_t>(input));
      auto exp = big_to_native(stream::read_as<std::uint32_t>(input));
      float sig = nsig * 0.5f / UINT32_MAX + 0.5f;
      return std::ldexp(sig, exp);
   }
};

}}} // namespace oac::fv::proto

#endif
