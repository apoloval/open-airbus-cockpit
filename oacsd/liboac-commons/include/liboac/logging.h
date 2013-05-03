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

#ifndef OAC_LOGGING_H
#define OAC_LOGGING_H

#include <string>

#include <Boost/format.hpp>

#include "exception.h"
#include "stream.h"

namespace oac { 

enum log_level
{
	INFO,
   WARN,
	FAIL,
};

class logger
{
public:

   inline static void set_main(const ptr<logger>& logger)
   { _main = logger; }

   inline static ptr<logger> get_main()
   { return _main; }

   inline logger(const log_level& level, const ptr<output_stream>& output)
      : _output(output), _level(level) {}

   /**
    * Log a message to indicated log level.
    */
   void log(log_level level, const std::string& msg);

private:

   ptr<output_stream> _output;
   log_level _level;

   static ptr<logger> _main;
};

/**
 * Log a message to indicated log level in main logger. If no main logger
 * was registered, nothing is done.
 */
void log(log_level level, const std::string& msg);

/**
 * Convenient function for logging Boost format objects in main logger.
 * If no main logger was registered, nothing is done.
 */
void log(log_level level, const boost::format& fmt);

}; // namespace oac

#endif
