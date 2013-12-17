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

#ifndef OAC_THREAD_CHANNEL_H
#define OAC_THREAD_CHANNEL_H

#include <chrono>
#include <condition_variable>
#include <list>
#include <mutex>

#include <liboac/exception.h>

namespace oac { namespace thread {

OAC_DECL_EXCEPTION(channel_timeout_error, oac::exception,
      "timeout while receiving a message from channel");

/**
 * A synchronization object based on a channel. This object may be use to
 * synchronize different threads by following the channel abstraction. One
 * thread is able to write messages to the channel, while another thread
 * can read them. If there is no thread receiving a message, the message
 * is queued until a new reader arrives. If there is no message to read, the
 * reader will block until a new message is available or the reception
 * times out, what happens first.
 */
template <typename Message>
class channel
{
public:

   /**
    * A predicate on a message object. This predicate type is used by readers
    * to filter the messages. For any new message in the channel, the reader
    * will evaluate it using the predicate. If true, the message is returned
    * as result of the read operation. Otherwise, the message is
    */
   typedef std::function<bool(const Message&)> message_predicate;

   static bool any_message_pred(const Message&) { return true; }

   channel() : _timeout(std::chrono::microseconds::max()) {}

   channel(const channel& chan) = delete;

   channel& operator = (const channel& chan) = delete;

   /**
    * Write the given message into this channel.
    */
   channel& operator << (const Message& msg)
   {
      write(msg);
      return *this;
   }

   /**
    * Read a messsage from this channel. This operation invokes read(), so
    * the same considerations should be assumed.
    */
   channel& operator >> (Message& msg)
   {
      msg = read();
      return *this;
   }

   template <typename Rep, typename Period>
   void set_timeout(const std::chrono::duration<Rep,Period>& timeout)
   {
      std::lock_guard<std::mutex> lock(_mutex);
      _timeout = std::chrono::duration_cast<std::chrono::microseconds>(timeout);
   }

   /**
    * Write a message to the channel. If there is no reader waiting,
    * the message is queued until a new reader is able to receive it.
    * Otherwise, a reader that was blocked waiting for a message is awaken,
    * so it can read the message.
    */
   void write(const Message& msg)
   {
      std::lock_guard<std::mutex> lock(_mutex);
      _messages.push_back(msg);
      _new_msg.notify_all();
   }

   /**
    * Read a messsage from this channel. This operation invokes read_for()
    * with the channel timeout value. By default, it is set to infinity, but
    * it can be overriden using the set_timeout() function. The message
    * predicate is used to filter those messages that are not to be read.
    */
   Message read(const message_predicate& msg_pred = any_message_pred)
   { return read_for(_timeout, msg_pred); }

   /**
    * Read a message before given timeout. If there is any queued message,
    * it will be returned. Otherwise, the caller thread will be blocked until
    * a new message is written or the given timeout, what happens first. The
    * message predicate is used to filter those messages that are not to be
    * read.
    */
   template <typename Rep, typename Period>
   Message read_for(
         const std::chrono::duration<Rep,Period>& timeout,
         const message_predicate& msg_pred = any_message_pred)
   {
      using namespace std::chrono;
      std::unique_lock<std::mutex> lock { _mutex };
      auto sleep_for = timeout;
      while (true)
      {
         auto msg = std::find_if(_messages.begin(), _messages.end(), msg_pred);
         if (msg != _messages.end())
         {
            auto result = *msg;
            _messages.erase(msg);
            lock.unlock();
            return result;
         }
         else
         {
            auto t1 = steady_clock::now();
            if (_new_msg.wait_for(lock, sleep_for) == std::cv_status::timeout)
               OAC_THROW_EXCEPTION(channel_timeout_error());
            auto t2 = steady_clock::now();
            sleep_for -= duration_cast<decltype(sleep_for)>(t2 - t1);
         }
      }
   }

private:

   typedef std::list<Message> message_queue;

   message_queue _messages;
   std::mutex _mutex;
   std::condition_variable _new_msg;
   std::chrono::microseconds _timeout;
};

}} // namespace oac::thread

#endif
