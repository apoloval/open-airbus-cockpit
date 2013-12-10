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

#ifndef OAC_BUFFER_ASIO_HANDLER_H
#define OAC_BUFFER_ASIO_HANDLER_H

#include <memory>

#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>

#include "liboac/exception.h"
#include "liboac/io.h"
#include "liboac/network/errors.h"

namespace oac { namespace buffer {

template <typename Handler, typename UpdateIndex>
struct async_io_handler
{
   async_io_handler(
         const Handler& handler,
         const UpdateIndex& update_index)
      : _handler(handler),
        _update_index(update_index)
   {}

   void operator() (
         const boost::system::error_code& ec,
         std::size_t nbytes)
   {
      if (!ec)
      {
         _update_index(nbytes);
         _handler(util::make_success(nbytes));
      }
      else
      {
         auto ecv = ec.value();
         switch (ecv)
         {
            case boost::asio::error::eof:
               _handler(util::make_failure<std::size_t>(
                     OAC_MAKE_EXCEPTION(io::eof_error())));
               break;
            case boost::asio::error::connection_aborted:
            case boost::asio::error::connection_reset:
               _handler(util::make_failure<std::size_t>(
                     OAC_MAKE_EXCEPTION(network::connection_reset())));
               break;
            default:
               _handler(util::make_failure<std::size_t>(
                     OAC_MAKE_EXCEPTION(io::boost_asio_error(ec))));
               break;
         }
      }
   }

private:

   Handler _handler;
   UpdateIndex _update_index;
};

template <typename Handler, typename UpdateIndex>
async_io_handler<Handler, UpdateIndex>
make_io_handler(
      const Handler& handler,
      const UpdateIndex& update_index)
{
   return async_io_handler<Handler, UpdateIndex>(handler, update_index);
}

}} // namespace oac::buffer

#endif
