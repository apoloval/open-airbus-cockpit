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

#ifndef OAC_TIME_H
#define OAC_TIME_H

#include <list>
#include <mutex>

#include <boost/asio.hpp>

#include <liboac/simconn.h>

namespace oac {

/**
 * @concept TickObserver
 *
 * An object able to observe time ticks. The objects satisfying this concept
 * are able to observe time ticks and allow to register handlers to be invoked
 * every time a time tick is observed. A time tick is considered any elapsed
 * period of time not specified by the concept.
 *
 * It must provide the following members.
 *
 *   template <typename OnTickHandler>
 *   void register_handler(const OnTickHandler&);
 *
 * Where OnTickHandler is any function which receives no argument and return
 * no value. register_handler() is used to register a tick handler. Tick
 * handlers cannot be unregistered unless the TickObserver object is destroyed.
 */

/**
 * Base class for tick observers to maintain a list of handlers.
 */
class tick_observer_base
{
public:

   typedef std::function<void(void)> on_tick_handler;

   template <typename OnTickHandler>
   void register_handler(const OnTickHandler& handler)
   {
      _on_tick_handlers.push_back(handler);
   }

protected:

   void notify_all()
   {
      for (auto& handler : _on_tick_handlers)
         handler();
   }

private:

   std::list<on_tick_handler> _on_tick_handlers;
};

/**
 * Base class for tick observers that potentially may register and notify
 * handlers under concurrenty connditions.
 */
class concurrent_tick_observer_base : private tick_observer_base
{
public:

   template <typename OnTickHandler>
   void register_handler(const OnTickHandler& handler)
   {
      std::lock_guard<std::recursive_mutex> lock(_mutex);
      tick_observer_base::register_handler(handler);
   }

protected:

   void notify_all()
   {
      std::lock_guard<std::recursive_mutex> lock(_mutex);
      tick_observer_base::notify_all();
   }

private:

   std::recursive_mutex _mutex;
};

/**
 * A dummy implementation for a tick observer.
 */
class dummy_tick_observer : public tick_observer_base
{
public:

   void tick()
   { notify_all(); }
};

/**
 * A tick observer supported by SimConnect. This observer is bounded to
 * SimConnect's time events, so for each tick notified by SimConnect, the
 * tick observers are in turn notified.
 */
class simconnect_tick_observer : public concurrent_tick_observer_base
{
public:

   /**
    * An exception caused by an initialization error.
    */
   OAC_DECL_EXCEPTION(
         simconnect_error,
         oac::exception,
         "an unexpected error occurs while observing ticks via SimConnect");

   /**
    * Create a new SimConnect observer for given event.
    *
    * @param sc_event The SimConnect event whose notification will be
    *                 considered as a tick.
    */
   simconnect_tick_observer(
         const simconnect_client::event_name& sc_event =
               simconnect_client::SYSTEM_EVENT_6HZ)
   throw (simconnect_error);

   /**
    * Invoke the dispatch message function of SimConnect client. This
    * is used to dispatch any pending message from SimConnect when running
    * as a stand-alone process. For FS/Prepar3D plugins, it is not necessary.
    */
   void dispatch() throw (simconnect_error);

private:

   std::unique_ptr<simconnect_client> _simconnect;

   void on_event(
         simconnect_client& client,
         const SIMCONNECT_RECV_EVENT& msg);
};

/**
 * An adapter which allows a tick observer to dispatch the notifications via
 * an ASIO IO service object. This class provides a TickObserver compliant
 * interface. It wraps any other TickObserver object (its delegate) along
 * a shared Boost IO service. Any handler registered into the adapter will
 * be registered in the delegate in a way that the tick handler is dispatched
 * using the Boost IO service provided in the constructor.
 */
template <typename TickObserver>
class asio_tick_observer_adapter
{
public:

   typedef std::function<void(void)> on_tick_handler;

   asio_tick_observer_adapter(
         const std::shared_ptr<boost::asio::io_service>& io_srv,
         const TickObserver& delegate = TickObserver())
      : _io_service(io_srv)
   {}

   template <typename OnTickHandler>
   void register_handler(const OnTickHandler& handler)
   {
      _delegate.register_handler([this, handler]() {
         _io_service->post(handler);
      });
   }

   const TickObserver& delegate() const { return _delegate; }

   TickObserver& delegate() { return _delegate; }

private:

   std::shared_ptr<boost::asio::io_service> _io_service;
   TickObserver _delegate;
};

} // namespace oac

#endif
