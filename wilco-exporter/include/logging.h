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

#include <exception>
#include <string>

#include <Boost/format.hpp>

namespace oac { 

enum LogLevel
{
	INFO,
   WARN,
	FAIL,
};

/**
 * Initialize the logger to output at given filename. 
 */
void InitLogger(const std::string& output_filename);

/**
 * Close current logger if any.
 */
void CloseLogger();

/**
 * Log a message to indicated log level. If logger was not initialized,
 * nothing is done.
 */
void Log(LogLevel level, const std::string& msg);

/**
 * Convenient function for logging Boost format objects. 
 */
void Log(LogLevel level, const boost::format& fmt);

/**
 * Log a message encapsulated by given exception and throw this latter. 
 */
void LogAndThrow(LogLevel level, const std::exception& excp);

}; // namespace oac

#endif
