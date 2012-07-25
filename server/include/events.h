/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2012 Alvaro Polo
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

#ifndef OAC_SERVER_EVENTS_H
#define OAC_SERVER_EVENTS_H

#include <list>

namespace oac { namespace server {

class EventReceiver {};
   
class EventSender
{
public:

   struct AbstractEvent
   {
      virtual ~AbstractEvent() {}
   };
   
   inline virtual ~EventSender()
   {
      for (EventSubscriberList::iterator it = _eventSubscribers.begin(),
           end = _eventSubscribers.end(); it != end; it++)
      {
         delete *it;
      }
      _eventSubscribers.clear();
   }
   
   template <typename Target, typename Event>
   void subscribe(Target* target, void (Target::* callback) (const Event&))
   {
      AbstractSubscriber* subscriber = 
            new EventSubscriber<Target, Event>(target, callback);
      _eventSubscribers.push_back(subscriber);
   }

protected:

   template <typename Event>
   void sendEvent(const Event& ev)
   {
      for (EventSubscriberList::iterator it = _eventSubscribers.begin(),
           end = _eventSubscribers.end(); it != end; it++)
      {
         (*it)->invoke(ev);
      }
   }

private:

   struct AbstractSubscriber
   {
      virtual void invoke(const AbstractEvent*) = 0;
   };

   template <typename Target, typename Event>
   struct EventSubscriber : AbstractSubscriber
   {
      Target* _target;
      void (Target::* _callback)(const Event&);
      
      EventSubscriber(Target* target, 
                      void (Target::* callback)(const Event&)) :
         _target(target), _callback(callback)
      {}
      
      virtual void invoke(const AbstractEvent* ev)
      {
         const Event* narrowed = dynamic_cast<const Event*>(ev);
         if (narrowed)
         {
            (_target->*_callback)(*narrowed);
         }
      }
   };
   
   typedef std::list<AbstractSubscriber*> EventSubscriberList;
   
   EventSubscriberList _eventSubscribers;

};

#define DECL_EVENT(name, fielddecl...) \
   struct name : ::oac::server::EventSender::AbstractEvent { fielddecl; }; \

}}; // namespace oac::server

#endif
