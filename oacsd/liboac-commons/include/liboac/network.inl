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

#ifndef OAC_NETWORK_INL
#define OAC_NETWORK_INL

#include "network.h"

namespace oac {

namespace {

inline void
bind_port(
      boost::asio::ip::tcp::acceptor& acceptor,
      std::uint16_t port)
throw (network::bind_error)
{
   using namespace boost::asio;
   try
   {
      ip::tcp::endpoint ep(ip::tcp::v4(), port);
      acceptor.open(ep.protocol());
      acceptor.bind(ep);
      acceptor.listen();
   } catch (boost::system::system_error& se)
   {
      BOOST_THROW_EXCEPTION(
               network::bind_error() <<
               boost_system_error_info(se) <<
               message_info(se.what()));
   }
}

} // anonymous namespace

inline
tcp_connection::tcp_connection()
   : _socket(new boost::asio::ip::tcp::socket(_io_service))
{}

inline boost::asio::io_service&
tcp_connection::io_service()
{ return _io_service; }

inline boost::asio::ip::tcp::socket&
tcp_connection::socket()
{ return *_socket; }

inline ptr<tcp_connection::input_stream>
tcp_connection::input()
{ return new input_stream(_socket); }

inline ptr<tcp_connection::output_stream>
tcp_connection::output()
{ return new output_stream(_socket); }



template <typename Worker>
tcp_server<Worker>::tcp_server(
      std::uint16_t port,
      const Worker& worker)
throw (network::bind_error)
   : _worker(worker),
     _acceptor(_io_service)
{
   bind_port(_acceptor, port);
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
   auto conn = make_ptr(new tcp_connection());
   _acceptor.async_accept(
            conn->socket(), std::bind(&tcp_server::on_accept, this, conn));
}

template <typename Worker>
void
tcp_server<Worker>::on_accept(const ptr<tcp_connection>& conn)
{
   _worker(conn);
   start_accept();
}



inline
tcp_client::tcp_client(const std::string& hostname, std::uint16_t port)
{
   using namespace boost::asio;

   _connection = new tcp_connection();
   ip::tcp::resolver resolver(_connection->io_service());
   ip::tcp::resolver::query query(
         hostname, boost::lexical_cast<std::string>(port));
   connect(_connection->socket(), resolver.resolve(query));
}

inline tcp_connection&
tcp_client::connection() throw (network::connection_closed_error)
{ return *_connection; }

inline ptr<tcp_connection::input_stream>
tcp_client::input()
{ return _connection->input(); }

inline ptr<tcp_connection::output_stream>
tcp_client::output()
{ return _connection->output(); }



inline
async_tcp_connection::async_tcp_connection(
      boost::asio::io_service& io_service,
      const network::error_handler& ehandler)
   : _socket(new socket_type(io_service))
{}

inline
async_tcp_connection::async_tcp_connection(
      boost::asio::io_service& io_service)
   : _socket(new socket_type(io_service))
{}

inline boost::asio::ip::tcp::socket&
async_tcp_connection::socket()
{ return *_socket; }

template <typename StreamBuffer, typename ReadHandler>
void
async_tcp_connection::read(
      StreamBuffer& buff,
      ReadHandler handler)
{
   buffer::async_read_some(*_socket, buff, handler);
}

template <typename StreamBuffer, typename WriteHandler>
void
async_tcp_connection::write(
      StreamBuffer& buff,
      WriteHandler handler)
{
   buffer::async_write_some(*_socket, buff, handler);
}



template <typename ConnectionHandler, typename ErrorHandler>
async_tcp_server::async_tcp_server(
      std::uint16_t port,
      const ConnectionHandler& handler,
      const ErrorHandler& ehandler)
throw (network::bind_error)
   : _acceptor(_io_service),
     _handler(handler),
     _ehandler(ehandler)
{
   bind_port(_acceptor, port);
}

template <typename ConnectionHandler>
async_tcp_server::async_tcp_server(
      std::uint16_t port,
      const ConnectionHandler& handler)
throw (network::bind_error)
   : _acceptor(_io_service),
     _handler(handler)
{
   bind_port(_acceptor, port);
}

inline
async_tcp_server::~async_tcp_server()
{ stop(); }

/**
 * Run the server. It uses the current thread to execute the accept loop.
 * It stops running after a call to stop().
 */
inline void
async_tcp_server::run()
{
   _io_service.reset();
   start_accept();
   _is_started.notify_all();
   _io_service.run();
}

/**
 * Run the server in background. It creates a background thread which
 * executes the accept loop. It returns the control once the thread is
 * sucessfuly created and accept loop is started. Any immediate subsequent
 * call to stop shall work.
 */
inline void
async_tcp_server::run_in_background()
{
   _bg_server = boost::thread([this]() { run(); });
   {
      boost::mutex mutex;
      boost::unique_lock<boost::mutex> lock(mutex);
      _is_started.wait(lock);
   }
}

/**
 * Stop the server. If running in background, it waits for the background
 * thread to finish before returning.
 */
inline void
async_tcp_server::stop()
{
   _io_service.stop();
   if (boost::this_thread::get_id() != _bg_server.get_id() &&
       _bg_server.joinable())
      _bg_server.join();
}

inline void
async_tcp_server::start_accept()
{
   auto conn = async_tcp_connection::create(_io_service, _ehandler);
   _acceptor.async_accept(
            conn->socket(),
            std::bind(&async_tcp_server::on_accept, this, conn));
}

inline void
async_tcp_server::on_accept(
      const async_tcp_connection::ptr_type& conn)
{
   _handler(conn);
   start_accept();
}

namespace network {

template <typename Worker>
ptr<tcp_server<thread_worker<Worker, ptr<tcp_connection>>>> make_tcp_server(
      std::uint16_t port, const Worker& worker)
{
   return new tcp_server<thread_worker<Worker, ptr<tcp_connection>>>(
         port, worker);
}

} // namespace network

} // namespace oac

#endif
