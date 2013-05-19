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

   static ptr_type create() { return new T(); }

   template <typename T1>
   static ptr_type create (T1& v1)
   { return std::make_shared<T>(v1); }

   template <typename T1, typename T2>
   static ptr_type create (T1& v1, T2& v2)
   { return std::make_shared<T>(v1, v2); }

   template <typename T1, typename T2, typename T3>
   static ptr_type create (T1& v1, T2& v2, T3& v3)
   { return std::make_shared<T>(v1, v2, v3); }

   template <typename T1, typename T2, typename T3, typename T4>
   static ptr_type create (T1& v1, T2& v2, T3& v3, T4& v4)
   { return std::make_shared<T>(v1, v2, v3, v4); }

   template <typename T1, typename T2, typename T3, typename T4, typename T5>
   static ptr_type create (T1& v1, T2& v2, T3& v3, T4& v4, T5& v5)
   { return std::make_shared<T>(v1, v2, v3, v4, v5); }
};

/**
 * Maybe-monad template class. This template provides an object that wraps
 * a determined value that maybe or not. In other words, it extends the value
 * space of the type passed as argument with 'nothing'.
 */
template <typename T>
class maybe
{
public:

   static const maybe NOTHING;

   OAC_DECL_ERROR(NothingError, illegal_state_error);

   /**
    * Create a new maybe object set to 'nothing'.
    */
   inline maybe() : _nothing(true) {}

   /**
    * Create a new maybe object set to 'just value'
    */
   inline maybe(const T& value) : _value(value), _nothing(false) {}

   /**
    * Return true if this maybe evaluates to 'nothing'.
    */
   inline bool is_nothing() const
   { return _nothing; }

   /**
    * Return true if this maybe evaluates to 'just value'.
    */
   inline bool is_just() const
   { return !_nothing; }

   /**
    * Obtain the value of this maybe object. If it evaluates to 'nothing',
    * a IllegalStateException is thrown.
    */
   inline const T& get() const throw (NothingError)
   { 
      if (this->is_nothing())
         BOOST_THROW_EXCEPTION(NothingError());
      return _value;
   }

   /**
    * Obtain the value of this maybe object. If it evaluates to 'nothing',
    * a IllegalStateException is thrown.
    */
   inline T& get() throw (NothingError)
   { 
      if (this->is_nothing())
         BOOST_THROW_EXCEPTION(NothingError());
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
   inline const T& operator * () const throw (NothingError)
   { return this->get(); }

   /**
    * Return the value of the maybe, or throw IllegalStateException if 'nothing'
    */
   inline T& operator * () throw (NothingError)
   { return this->get(); }

private:

   T _value;
   bool _nothing;
};

template <typename T>
const maybe<T> maybe<T>::NOTHING;

/**
 * Bind operator for maybe. This operator works as bind operation of monadic
 * maybe type. It produces Maybe<T2> from Maybe<T1 >> function<Maybe<T2>(T1)>.
 */
template <typename T, typename F>
auto operator >> (const maybe<T>& m, const F& f) -> decltype(f(m.get()))
{
   typedef decltype(f(m.get())) R;
   return m.is_just() ? R(f(m.get())) : R();
}

/**
 * Convenient operator for binding the maybe value to a variable. It returns
 * true if maybe is just T, or false if maybe is nothing.
 */
template <typename T>
bool operator >> (const maybe<T>&m, T& t)
{
   auto proceed = m.is_just();
   if (proceed)
      t = m.get();
   return proceed;
}

} // namespace oac

#endif
