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

#ifndef OAC_LANG_UTILS_H
#define OAC_LANG_UTILS_H

#include "exception.h"

namespace oac {

/**
 * Pointer template class. This template provides an implementation for a
 * smart pointer based on reference counting (shared object). It just extends
 * std::shared_ptr by providing some convenient constructors not present in
 * the STL type.
 */
template <typename T>
class ptr : public std::shared_ptr<T>
{
public:

   inline ptr() : std::shared_ptr<T>() {}
   inline ptr(const ptr& p) : std::shared_ptr<T>(p) {}
   inline ptr(const shared_ptr<T>& p) : std::shared_ptr<T>(p) {}
   inline ptr(T* t) : std::shared_ptr<T>(std::shared_ptr<T>(t)) {}

   template <typename U>
     inline ptr(const ptr<U>& p)
       : std::shared_ptr<T>(std::static_pointer_cast<T>(p)) {}
};

template <typename T>
inline ptr<T> make_ptr(T* p)
{ return ptr<T>(p); }

template <typename T, typename Pointer = std::shared_ptr<T>>
class shared_by_ptr
{
public:

   typedef Pointer ptr_type;

   virtual ~shared_by_ptr() {}

   static ptr_type create() { return ptr_type(new T()); }

   template <typename T1>
   static ptr_type create(T1&& v1)
   { return ptr_type(new T(std::forward<T1>(v1))); }

   template <typename T1, typename T2>
   static ptr_type create(T1&& v1, T2&& v2)
   { return ptr_type(new T(std::forward<T1>(v1), std::forward<T2>(v2))); }

   template <typename T1, typename T2, typename T3>
   static ptr_type create(T1&& v1, T2&& v2, T3&& v3)
   {
      return ptr_type(new T(
            std::forward<T1>(v1),
            std::forward<T2>(v2),
            std::forward<T3>(v3)));
   }

   template <typename T1, typename T2, typename T3, typename T4>
   static ptr_type create(T1&& v1, T2&& v2, T3&& v3, T4&& v4)
   {
      return ptr_type(new T(
            std::forward<T1>(v1),
            std::forward<T2>(v2),
            std::forward<T3>(v3),
            std::forward<T4>(v4)));
   }

   template <typename T1, typename T2, typename T3, typename T4, typename T5>
   static ptr_type create(T1&& v1, T2&& v2, T3&& v3, T4&& v4, T5&& v5)
   {
      return ptr_type(new T(
            std::forward<T1>(v1),
            std::forward<T2>(v2),
            std::forward<T3>(v3),
            std::forward<T4>(v4),
            std::forward<T5>(v5)));
   }
};

} // namespace oac

#endif
