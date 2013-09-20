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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OAC_NETWORK_SERVER_INL
#define OAC_NETWORK_SERVER_INL

#include <liboac/network/asio_utils.h>
#include <liboac/network/server.h>

namespace oac { namespace network {

template <typename Worker>
tcp_server<Worker>::tcp_server(
      network::tcp_port port,
      const Worker& worker)
throw (network::bind_error)
   : _worker(worker),
     _acceptor(_io_service)
{
   try
   {
      bind_port(_acceptor, port);
   }
   catch (const io::boost_asio_error& e)
   {
      OAC_THROW_EXCEPTION(network::bind_error(port, e));
   }
}

template <typename Worker>
void
tcp_server<Worker>::run()
{
   _io_service.reset();
   start_accept();
   _is_started.notify_all();
   _io_service.run();
}

template <typename Worker>
void
tcp_server<Worker>::run_in_background()
{
   _bg_server = boost::thread([this]() { run(); });
   {
      boost::mutex mutex;
      boost::unique_lock<boost::mutex> lock(mutex);
      _is_started.wait(lock);
   }
}

template <typename Worker>
void
tcp_server<Worker>::stop()
{
   _io_service.stop();
   _bg_server.join(); // if not in background, returns immediately
}

template <typename Worker>
void
tcp_server<Worker>::start_accept()
{
   if (_io_service.stopped())
      return;
   auto conn = std::make_shared<tcp_connection>();
   _acceptor.async_accept(
            conn->socket(), std::bind(&tcp_server::on_accept, this, conn));
}

template <typename Worker>
void
tcp_server<Worker>::on_accept(const tcp_connection_ptr& conn)
{
   _worker(conn);
   start_accept();
}

template <typename Worker>
std::shared_ptr<tcp_server<thread_worker<Worker, tcp_connection_ptr>>>
make_tcp_server(
      network::tcp_port port, const Worker& worker)
{
   return std::make_shared<
         tcp_server<thread_worker<Worker, tcp_connection_ptr>>>(
               port, worker);
}

}} // namespace oac::network

#endif
