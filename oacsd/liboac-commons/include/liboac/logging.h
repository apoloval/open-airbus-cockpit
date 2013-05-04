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

class abstract_logger
{
public:

   inline virtual ~abstract_logger() {}

   virtual void log(log_level level, const std::string& msg) = 0;

protected:

   const char* level_str(log_level level);

   std::string get_time();
};

template <typename OutputStream>
class logger : public abstract_logger
{
public:

   inline logger(const log_level& level, const ptr<OutputStream>& output)
      : _output(output), _level(level) {}

   /**
    * Log a message to indicated log level.
    */
   inline virtual void log(log_level level, const std::string& msg)
   {
      if (_level <= level)
      {
         auto line = str(boost::format("[%s] %s : %s\n") %
                         level_str(level) % get_time() % msg);
         stream::write_as_string(*_output, line);
         _output->flush();
      }
   }

private:

   ptr<OutputStream> _output;
   log_level _level;

   static ptr<logger> _main;
};

/**
 * Create a new logger for given level and output stream.
 */
template <typename OutputStream>
ptr<logger<OutputStream>> make_logger(
      const log_level& level, const ptr<OutputStream>& output)
{ return new logger<OutputStream>(level, output); }

/**
 * Set the main abstract logger.
 */
void set_main_logger(const ptr<abstract_logger>& logger);

/**
 * Close the main logger.
 */
inline void close_main_logger()
{ set_main_logger(nullptr); }

/**
 * Obtain the main abstract logger.
 */
ptr<abstract_logger> get_main_logger();

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
