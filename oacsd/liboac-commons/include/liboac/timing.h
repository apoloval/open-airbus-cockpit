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

   std::list<on_tick_handler> _on_tick_handlers;
};

/**
 * A dummy implementation for a tick observer.
 */
class dummy_tick_observer : public tick_observer_base
{
public:

   void tick()
   {
      for (auto& handler : _on_tick_handlers)
         handler();
   }
};

} // namespace oac

#endif
