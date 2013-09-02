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

#ifndef OAC_LOGGING_H
#define OAC_LOGGING_H

#include <string>

#include <Boost/format.hpp>

#include "exception.h"
#include "stream.h"

namespace oac { 

/**
 * The level of severity of the logged entries.
 */
enum class log_level
{
	INFO,
   WARN,
   FAIL,
};

/**
 * The author of each log entry.
 */
typedef std::string log_author;

/**
 * The message reported on each log entry.
 */
typedef std::string log_message;

/**
 * An abstraction for a log entity.
 */
class logger
{
public:

   inline virtual ~logger() {}

   /**
    * Write a new log entry.
    *
    * @param author  The author (typically a program module or class) of the
    *                log entry
    * @param level   The severity level of the log entry
    * @param msg     The message of the log entry
    */
   virtual void log(
         const log_author& author,
         log_level level,
         const log_message& msg) = 0;

protected:

   /**
    * Converts the given log level into a string object.
    */
   const char* level_str(log_level level);

   std::string get_time();
};

/**
 * A logger able to print log messages into an output stream.
 */
template <typename OutputStream>
class output_stream_logger : public logger
{
public:

   /**
    * Create a new logger for given level and output stream.
    *
    * @param level   The minimum level of log entries that will be accepted.
    *                any log entry with a level below this one will be ignored.
    * @param output  The output stream where log messages will be printed
    */
   output_stream_logger(
         const log_level& level,
         const std::shared_ptr<OutputStream>& output)
      : _output(output), _level(level)
   {}

   virtual void log(
         const log_author& author,
         log_level level,
         const log_message& msg)
   {
      if (_level <= level)
      {
         auto line = str(boost::format("[%s] %s <%s> : %s\n") %
                         level_str(level) % get_time() % author % msg);
         stream::write_as_string(*_output, line);
         _output->flush();
      }
   }

private:

   ptr<OutputStream> _output;
   log_level _level;

   static ptr<logger> _main;
};

class logger_component : public logger
{
public:

   logger_component(
         const log_author& author,
         const std::shared_ptr<logger>& parent = nullptr)
      : _author(author),
        _parent(parent)
   {}

   virtual void log(
            const log_author& author,
            log_level level,
            const log_message& msg);

protected:

   /**
    * A convenience function to log a message using the author passed to
    * the logger component upon construction.
    */
   template <typename... Args>
   void log(
      log_level level,
      const char* fmt,
      const Args&... args)
   { log(_author, level, format(fmt, args...)); }

   /**
    * Convenience function for logging INFO entries.
    */
   template <typename... Args>
   void log_info(
         const char* fmt,
         const Args&... args)
   { log(log_level::INFO, fmt, args...); }

   /**
    * Convenience function for logging WARN entries.
    */
   template <typename... Args>
   void log_warn(
         const char* fmt,
         const Args&... args)
   { log(log_level::WARN, fmt, args...); }

   /**
    * Convenience function for logging FAIL entries.
    */
   template <typename... Args>
   void log_fail(
         const char* fmt,
         const Args&... args)
   { log(log_level::FAIL, fmt, args...); }

private:

   log_author _author;
   std::shared_ptr<logger> _parent;
};

/**
 * Create a new logger for given level and output stream.
 */
template <typename OutputStream>
std::shared_ptr<output_stream_logger<OutputStream>> make_logger(
      const log_level& level,
      const std::shared_ptr<OutputStream>& output)
{ return std::make_shared<output_stream_logger<OutputStream>>(level, output); }

/**
 * Set the main abstract logger.
 */
void set_main_logger(const std::shared_ptr<logger>& logger);

/**
 * Close the main logger.
 */
inline void close_main_logger()
{ set_main_logger(nullptr); }

/**
 * Obtain the main abstract logger.
 */
std::shared_ptr<logger> get_main_logger();

/**
 * Log a message to indicated log level in main logger. If no main logger
 * was registered, nothing is done.
 */

/**
 * Create a new log entry with given properties using the main logger. If no
 * main logger was registered, nothing is done.
 *
 * @param author  The author of the log entry
 * @param level   The severity level of the log entry
 * @param msg     The message of the log entry
 */
void log(
      const log_author& author,
      log_level level,
      const log_message& msg);

} // namespace oac

#endif
