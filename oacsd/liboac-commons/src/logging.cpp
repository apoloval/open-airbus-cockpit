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

#include <ctime>
#include <stdio.h>

#include "logging.h"

namespace oac {

static FILE* LOGGER_OUTPUT = nullptr;

static const char* LEVEL_STR[] = 
{
   "INFO", "WARN", "FAIL"
};

void
InitLogger(const std::string& logFile)
{
   CloseLogger();
	LOGGER_OUTPUT = _fsopen(logFile.c_str(), "a", _SH_DENYWR);
}

void CloseLogger()
{
   if (LOGGER_OUTPUT)
   {
      fclose(LOGGER_OUTPUT);
      LOGGER_OUTPUT = nullptr;
   }
}

void
Log(LogLevel level, const std::string& msg)
{
   if (LOGGER_OUTPUT)
   {
      auto t = time(nullptr);
      char time_buf[26];
      struct tm lt;
      localtime_s(&lt, &t);
      asctime_s(time_buf, &lt);
      time_buf[24] = '\0';
		fprintf(LOGGER_OUTPUT, "[%s] %s : %s\n", 
            LEVEL_STR[level], time_buf, msg.c_str());
      fflush(LOGGER_OUTPUT);
   }
}

void
Log(LogLevel level, const boost::format& fmt)
{ Log(level, str(fmt)); }

void
LogAndThrow(LogLevel level, const Error& e)
{
   Log(level, e.what());
   THROW_ERROR(e);
}

} // namespace oac
