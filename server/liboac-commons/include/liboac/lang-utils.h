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
class Ptr : public std::shared_ptr<T>
{
public:

   inline Ptr() : std::shared_ptr<T>() {}
   inline Ptr(const Ptr& ptr) : std::shared_ptr<T>(ptr) {}
   inline Ptr(const shared_ptr<T>& ptr) : std::shared_ptr<T>(ptr) {}
   inline Ptr(T* t) : std::shared_ptr<T>(std::shared_ptr<T>(t)) {}
};

/**
 * Maybe-monad template class. This template provides an object that wraps
 * a determined value that maybe or not. In other words, it extends the value
 * space of the type passed as argument with 'nothing'.
 */
template <typename T>
class Maybe
{
public:

   static const Maybe NOTHING;

   /**
    * Create a new maybe object set to 'nothing'.
    */
   inline Maybe() : _nothing(true) {}

   /**
    * Create a new maybe object set to 'just value'
    */
   inline Maybe(const T& value) : _value(value), _nothing(false) {}

   /**
    * Return true if this maybe evaluates to 'nothing'.
    */
   inline bool isNothing() const
   { return _nothing; }

   /**
    * Return true if this maybe evaluates to 'just value'.
    */
   inline bool isJust() const
   { return !_nothing; }

   /**
    * Obtain the value of this maybe object. If it evaluates to 'nothing',
    * a IllegalStateException is thrown.
    */
   inline const T& get() const throw (IllegalStateException)
   { 
      this->isJustOrThrow("get value from maybe");
      return _value;
   }

   /**
    * Obtain the value of this maybe object. If it evaluates to 'nothing',
    * a IllegalStateException is thrown.
    */
   inline T& get() throw (IllegalStateException)
   { 
      this->isJustOrThrow("get value from maybe");
      return _value;
   }

   /**
    * Set the value of this maybe to 'just value'.
    */
   inline void set(const T& value)
   { _value = value; _nothing = false; }

   /**
    * Return the value of the maybe, or throw IllegalStateException if 'nothing'
    */
   inline const T& operator * () const throw (IllegalStateException)
   { return this->get(); }

   /**
    * Return the value of the maybe, or throw IllegalStateException if 'nothing'
    */
   inline T& operator * () throw (IllegalStateException)
   { return this->get(); }

private:

   inline void isJustOrThrow(const std::string& action) const
   throw (IllegalStateException)
   {
      if (this->isNothing())
         throw IllegalStateException(boost::format(
               "cannot %s: evaluates to nothing") % action);      
   }

   T _value;
   bool _nothing;
};

template <typename T>
const Maybe<T> Maybe<T>::NOTHING;

/**
 * Bind operator for maybe. This operator works as bind operation of monadic
 * maybe type. It produces Maybe<T2> from Maybe<T1 >> function<Maybe<T2>(T1)>.
 */
template <typename T, typename F>
auto operator >> (const Maybe<T>& m, const F& f) -> decltype(f(m.get()))
{
   typedef decltype(f(m.get())) R;
   return m.isJust() ? R(f(m.get())) : R();
}

/**
 * Convenient operator for binding the maybe value to a variable. It returns
 * true if maybe is just T, or false if maybe is nothing.
 */
template <typename T>
bool operator >> (const Maybe<T>&m, T& t)
{
   auto proceed = m.isJust();
   if (proceed)
      t = m.get();
   return proceed;
}

} // namespace oac

#endif
