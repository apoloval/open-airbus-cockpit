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

#ifndef OAC_SERVER_CONFIG_H
#define OAC_SERVER_CONFIG_H

#include <KarenCore/platform.h>

#ifdef LIBOAC_BUILD
   #if KAREN_COMPILER == KAREN_COMPILER_CYGWIN || \
       KAREN_COMPILER == KAREN_COMPILER_MINGW || \
       KAREN_COMPILER == KAREN_COMPILER_CLANG
      #define LIBOAC_EXPORT __attribute__((dllexport))
   #else
      #define LIBOAC_EXPORT __declspec(dllexport)
   #endif
#else
   #if KAREN_COMPILER == KAREN_COMPILER_CYGWIN || \
       KAREN_COMPILER == KAREN_COMPILER_MINGW || \
       KAREN_COMPILER == KAREN_COMPILER_CLANG
      #define LIBOAC_EXPORT __attribute__((dllimport))
   #else
      #define LIBOAC_EXPORT __declspec(dllimport)
   #endif
#endif

#endif
