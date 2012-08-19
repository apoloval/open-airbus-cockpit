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

#ifndef OAC_SERVER_PLATFORM_H
#define OAC_SERVER_PLATFORM_H

/* Constant definitions for platform properties. */
#define OAC_ARCH_32            1
#define OAC_ARCH_64            2

#define OAC_PLATFORM_WINDOWS   1
#define OAC_PLATFORM_LINUX     2
#define OAC_PLATFORM_OSX       3

#define OAC_COMPILER_GCC       1
#define OAC_COMPILER_LLVM      2
#define OAC_COMPILER_MSVC      3
#define OAC_COMPILER_CYGWIN    4
#define OAC_COMPILER_MINGW     5

#define OAC_BIG_ENDIAN         1
#define OAC_LITTLE_ENDIAN      2

/* Check the platform architecture. */
#if defined(__x86_64__) || defined(_M_X64)   || defined(__powerpc64__) || \
    defined(__alpha__)  || defined(__ia64__) || defined(__s390__)      || \
    defined(__s390x__)
#  define OAC_ARCH_TYPE OAC_ARCH_64
#else
#  define OAC_ARCH_TYPE OAC_ARCH_32
#endif

/* Check the platform OS. */
#if defined( __WIN32__ ) || defined( _WIN32 ) || defined( _WIN64 )
#  define OAC_PLATFORM OAC_PLATFORM_WINDOWS
#elif defined( __APPLE_CC__) || (defined(__APPLE__) && defined(__MACH__))
#  define OAC_PLATFORM OAC_PLATFORM_OSX
#else
#  define OAC_PLATFORM OAC_PLATFORM_LINUX
#endif

/* Check the compiler. */
#if defined(__GNUC__)
#  define OAC_COMPILER OAC_COMPILER_GCC
#elif defined(__llvm__) || defined(__clang__)
#  #define OAC_COMPILER OAC_COMPILER_LLVM
#elif defined(_MSC_VER)
#  define OAC_COMPILER OAC_COMPILER_MSVC
#elif defined(__CYGWIN__)
#  #define OAC_COMPILER OAC_COMPILER_CYGWIN
#elif defined(__MINGW32__)
#  #define OAC_COMPILER OAC_COMPILER_MINGW
#else
#  #error cannot determine C++ compiler vendor
#endif

/* Check the endianness. */
#if defined(__i386__) || defined(_M_IX86)    || defined(_X86_)     || \
    defined(__ia64__) || defined(_M_IA64)    || defined(__amd64__) || \
    defined(__amd64)  || defined(__x86_64__) || defined(__x86_64)  || \
    defined(_M_X64)
#  define OAC_ENDIANNESS OAC_LITTLE_ENDIAN
#elif defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__) || \
      defined(__ppc__) || defined(_M_PPC)
#  define OAC_ENDIANNESS OAC_BIG_ENDIAN
#endif

/* Define the symbol export macros. */
#if OAC_PLATFORM == OAC_PLATFORM_WINDOWS
#  ifdef OAC_LIB_BUILD
#     if OAC_COMPILER == OAC_COMPILER_CYGWIN || \
         OAC_COMPILER == OAC_COMPILER_MINGW
#        define OAC_EXPORT __attribute__((dllexport))
#        define OAC_LOCAL
#     else
#        define OAC_EXPORT __declspec(dllexport)
#        define OAC_LOCAL
#     endif
#  else
#     if OAC_COMPILER == OAC_COMPILER_CYGWIN || \
         OAC_COMPILER == OAC_COMPILER_MINGW
#        define OAC_EXPORT __attribute__((dllimport))
#        define OAC_LOCAL
#     else
#        define OAC_EXPORT __declspec(dllimport)
#        define OAC_LOCAL
#     endif
#  endif
#else // non-windows
#  define OAC_EXPORT __attribute__ ((visibility ("default")))
#  define OAC_LOCAL  __attribute__ ((visibility ("hidden")))
#endif

// Integer formats of fixed bit width
#if OAC_ARCH_TYPE == OAC_ARCH_32
typedef unsigned long      UInt32;
typedef long               Int32;
#else
typedef unsigned int       UInt32;
typedef int                Int32;
#endif
typedef unsigned short     UInt16;
typedef short              Int16;
typedef unsigned char      UInt8;
typedef char               Int8;


// Special case: 64-bit types
#if OAC_COMPILER == OAC_COMPILER_MSVC
typedef unsigned __int64   UInt64;
typedef __int64            Int64;
#else
typedef unsigned long long UInt64;
typedef long long          Int64;
#endif

// Disable warnings for using throw() clause in MSVC
#if OAC_COMPILER == OAC_COMPILER_MSVC
#pragma warning( disable : 4290 )
#endif

#endif
