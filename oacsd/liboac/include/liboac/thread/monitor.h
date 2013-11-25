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

#ifndef OAC_THREAD_MONITOR_H
#define OAC_THREAD_MONITOR_H

#include <mutex>

namespace oac { namespace thread {

/**
 * A wrapper utility that offers monitor semantics. This class may be use
 * to wrap any object in order to provide monitor semantics as the following
 * example shows.
 *
 * @code
 * class foobar
 * {
 * public:
 *    int do_something(int number);
 * };
 *
 * thread::monitor<foobar> mfoobar;
 * int i = mfoobar->do_something(12);
 * @endcode
 *
 * In this example, the -> operator attemps to lock the monitor internal mutex.
 * When done, the call to do_something() is executed and after that the monitor
 * unlocks its mutex allowing other threads to invoke operations on foobar
 * object.
 *
 * There are some important restrictions to take into account while using this
 * class.
 *
 * - Each time the -> operator is invoked, the mutex is locked, and it is not
 *   unlocked until the whole expression is fully evaluated.
 *
 *   @code
 *   int i = mfoobar->do_something() + mfoobar->do_something();
 *   @endcode
 *
 *   In this example, monitor is locked, then the function is invoked, then
 *   the monitor is locked again, then the function is invoked one more time,
 *   and finally the monitor is unlocked twice.
 *
 *   This precise example is harmless, since the monitor is using a recursive
 *   mutex to deal with this very circumstance. But a deadlock may be caused
 *   if another monitor is used in the same expression as in:
 *
 *   @code
 *   // Thread A
 *   int i = mon1->value_one() + mon2->value_two();
 *
 *   // Thread B
 *   int i = mon2->value_two() + mon1->value_one();
 *   @endcode
 *
 *   In this case, both threads may be in a deadlock. Please note that monitor
 *   class cannot protect itself from this scenario so it should be avoided.
 */
template <typename T>
class monitor
{
public:

   struct guard
   {
      T* _obj;
      std::recursive_mutex* _mutex;

      guard(T* obj, std::recursive_mutex* mutex) : _obj(obj), _mutex(mutex)
      { _mutex->lock(); }

      ~guard() { _mutex->unlock(); }

      T* operator -> () { return _obj; }
   };

   /**
    * Create a new monitor by wrapping the object passed as argument.
    */
   monitor(const T& obj = T()) : _obj(obj) {}

   /**
    * Member access operator. It returns a guard object that, in turn, locks
    * the internal mutex in its constructor and unlocks it in the destructor.
    * The guard object will provide access to the wrapped object by overriding
    * the member access operator. Since it is not destroyed until the outer
    * expression is fully evaluated, the internal mutex is guaranteed to be
    * locked until then.
    */
   guard operator -> () { return guard(&_obj, &_mutex); }

private:

   T _obj;
   std::recursive_mutex _mutex;
};

}} // namespace oac::thread

#endif
