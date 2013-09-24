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

#include <liboac/fsuipc/client.h>

namespace oac { namespace fsuipc {

void
dummy_user_adapter::process_read_requests()
{
   for (auto& req : _read_requests)
   {
      *req.value = read_value_from_buffer(
               req.offset.address, req.offset.length);
   }
   _read_requests.clear();
}

void
dummy_user_adapter::process_write_requests()
{
   for (auto& req : _write_requests)
   {
      write_value_to_buffer(
               req.offset.address,
               req.offset.length,
               req.offset.value);
   }
   _write_requests.clear();
}

offset_value
dummy_user_adapter::read_value_from_buffer(
      offset_address addr,
      offset_length len)
{
   offset_value value = 0;
   switch (len)
   {
      case OFFSET_LEN_DWORD:
         value += _buffer[addr+3] << 24;
         value += _buffer[addr+2] << 16;
      case OFFSET_LEN_WORD:
         value += _buffer[addr+1] << 8;
      case OFFSET_LEN_BYTE:
         value += _buffer[addr];
   }
   return value;
}

void
dummy_user_adapter::write_value_to_buffer(
      offset_address addr,
      offset_length len,
      offset_value val)
{
   switch (len)
   {
      case OFFSET_LEN_DWORD:
         _buffer[addr+3] = val >> 24;
         _buffer[addr+2] = val >> 16;
      case OFFSET_LEN_WORD:
         _buffer[addr+1] = val >> 8;
      case OFFSET_LEN_BYTE:
         _buffer[addr] = val;
   }
}

}} // namespace oac::fsuipc
