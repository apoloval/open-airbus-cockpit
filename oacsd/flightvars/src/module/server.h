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

#ifndef OAC_FV_SERVER_H
#define OAC_FV_SERVER_H

#include <memory>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <liboac/lang-utils.h>

namespace oac { namespace fv {

class flight_vars;

class flight_vars_server :
      public std::enable_shared_from_this<flight_vars_server>
{
public:

   static const int DEFAULT_PORT;

   flight_vars_server(const ptr<flight_vars>& delegate = nullptr,
                      int port = DEFAULT_PORT);

   void run();

private:

   class connection : public std::enable_shared_from_this<connection>
   {
   public:

      connection(boost::asio::io_service& io_service,
                 const ptr<flight_vars>& delegate);

      inline boost::asio::ip::tcp::socket& get_socket()
      { return _socket; }

      void start();

   private:

      boost::asio::ip::tcp::socket _socket;
      ptr<flight_vars> _delegate;
   };

   ptr<flight_vars> _delegate;
   boost::asio::io_service _io_service;
   boost::asio::ip::tcp::acceptor _acceptor;

   void accept_connection();

   void on_connection_accepted(const ptr<connection>& conn);
};

}} // namespace oac::fv

#endif
