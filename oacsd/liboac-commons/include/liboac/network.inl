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
      network::tcp_port port)
throw (boost_asio_error)
{
   using namespace boost::asio;
   try
   {
      ip::tcp::endpoint ep(ip::tcp::v4(), port);
      acceptor.open(ep.protocol());
      acceptor.bind(ep);
      acceptor.listen();
   } catch (boost::system::system_error& e)
   {
      OAC_THROW_EXCEPTION(boost_asio_error().with_error_code(e.code()));
   }
}

inline void
connect(
      const network::hostname& remote_host,
      network::tcp_port remote_port,
      boost::asio::io_service& io_srv,
      boost::asio::ip::tcp::socket& socket)
throw (boost_asio_error)
{
   using namespace boost::asio;
   try
   {
      ip::tcp::resolver resolver(io_srv);
      ip::tcp::resolver::query query(
            remote_host,
            boost::lexical_cast<std::string>(remote_port));
      boost::asio::connect(socket, resolver.resolve(query));
   }
   catch (boost::system::system_error& e)
   {
      OAC_THROW_EXCEPTION(boost_asio_error().with_error_code(e.code()));
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
   catch (boost_asio_error& e)
   {
      OAC_THROW_EXCEPTION(network::bind_error()
            .with_port(port)
            .with_cause(e));
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
tcp_client::tcp_client(
      const network::hostname& hostname,
      network::tcp_port port)
throw (network::connection_refused)
{
   try
   {
      _connection = new tcp_connection();
      connect(
            hostname,
            port,
            _connection->io_service(),
            _connection->socket());
   }
   catch (boost_asio_error& e)
   {
      OAC_THROW_EXCEPTION(network::connection_refused()
            .with_remote_host(hostname)
            .with_remote_port(port)
            .with_cause(e));
   }
}

inline tcp_connection&
tcp_client::connection()
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

template <typename StreamBuffer>
std::future<std::size_t>
async_tcp_connection::read(StreamBuffer& buff)
{
   auto promise = std::make_shared<std::promise<std::size_t>>();
   auto fut = promise->get_future();

   read(
         buff,
         std::bind(
               &async_tcp_connection::on_io_completed_with_promise,
               promise,
               std::placeholders::_1,
               std::placeholders::_2));
   return fut;
}

template <typename StreamBuffer, typename WriteHandler>
void
async_tcp_connection::write(
      StreamBuffer& buff,
      WriteHandler handler)
{
   buffer::async_write_some(*_socket, buff, handler);
}

template <typename StreamBuffer>
std::future<std::size_t>
async_tcp_connection::write(StreamBuffer& buff)
{
   auto promise = std::make_shared<std::promise<std::size_t>>();
   auto fut = promise->get_future();

   write(
         buff,
         std::bind(
               &async_tcp_connection::on_io_completed_with_promise,
               promise,
               std::placeholders::_1,
               std::placeholders::_2));
   return fut;
}

inline
void
async_tcp_connection::on_io_completed_with_promise(
      const std::shared_ptr<std::promise<std::size_t>>& promise,
      const boost::system::error_code& ec,
      std::size_t bytes_read)
{
   try
   {
      if (!!ec)
         OAC_THROW_EXCEPTION(boost_asio_error()
               .with_error_code(ec));
      promise->set_value(bytes_read);
   }
   catch (boost_asio_error& e)
   {
      promise->set_exception(std::make_exception_ptr(e));
   }
}



inline
async_tcp_server::async_tcp_server(
      network::tcp_port port,
      const connection_handler& handler,
      const std::shared_ptr<boost::asio::io_service>& io_srv =
            std::make_shared<boost::asio::io_service>(),
      const network::error_handler& ehandler = network::error_handler())
throw (network::bind_error)
   : _acceptor(*io_srv),
     _handler(handler),
     _io_service(io_srv),
     _ehandler(ehandler)
{
   try
   {
      bind_port(_acceptor, port);
   }
   catch (boost_asio_error& e)
   {
      OAC_THROW_EXCEPTION(network::bind_error()
            .with_port(port)
            .with_cause(e));
   }
   start_accept();
}

inline const boost::asio::io_service&
async_tcp_server::io_service() const
{ return *_io_service; }

inline boost::asio::io_service&
async_tcp_server::io_service()
{ return *_io_service; }

inline void
async_tcp_server::start_accept()
{
   auto conn = async_tcp_connection::create(*_io_service, _ehandler);
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



inline
async_tcp_client::async_tcp_client(
      const network::hostname& hostname,
      network::tcp_port port,
      const std::shared_ptr<boost::asio::io_service>& io_srv)
throw (network::connection_refused)
   : _io_service(io_srv),
     _connection(*io_srv)
{
   using namespace boost::asio;

   try
   {
      connect(hostname, port, *_io_service, _connection.socket());
   }
   catch (boost_asio_error& e)
   {
      OAC_THROW_EXCEPTION(network::connection_refused()
            .with_remote_host(hostname)
            .with_remote_port(port)
            .with_cause(e));
   }
}



namespace network {

template <typename Worker>
ptr<tcp_server<thread_worker<Worker, ptr<tcp_connection>>>> make_tcp_server(
      network::tcp_port port, const Worker& worker)
{
   return new tcp_server<thread_worker<Worker, ptr<tcp_connection>>>(
         port, worker);
}

} // namespace network

} // namespace oac

#endif
