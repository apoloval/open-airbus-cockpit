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

#ifndef OAC_POINTER_H
#define OAC_POINTER_H

namespace oac {

template <typename T>
class Ptr : public std::shared_ptr<T>
{
public:

   inline Ptr() : std::shared_ptr<T>() {}
   inline Ptr(const Ptr& ptr) : std::shared_ptr<T>(ptr) {}
   inline Ptr(const shared_ptr<T>& ptr) : std::shared_ptr<T>(ptr) {}
   inline Ptr(T* t) : std::shared_ptr<T>(std::shared_ptr<T>(t)) {}
};

}; // namespace oac

#endif
