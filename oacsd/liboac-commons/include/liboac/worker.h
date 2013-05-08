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

#ifndef OAC_WORKER_H
#define OAC_WORKER_H

#include <boost/thread.hpp>

namespace oac {

template <typename Worker, typename WorkUnit>
class thread_worker
{
public:

   inline thread_worker(const Worker& worker) : _worker(worker) {}

   void operator() (const WorkUnit& work_unit)
   { work(work_unit); }

   void work(const WorkUnit& work_unit)
   { boost::thread([this, work_unit]() { _worker(work_unit); }); }

private:

   Worker _worker;
};

namespace worker {

template <typename Worker, typename WorkUnit>
thread_worker<Worker, WorkUnit> make_thread_worker(
      const Worker& worker)
{ return thread_worker<Worker, WorkUnit>(worker); }

} // namespace worker

}

#endif // OAC_WORKER_H
