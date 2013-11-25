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

#include "broker.h"

namespace oac { namespace fv { namespace mqtt {

mosquitto_process_runner::mosquitto_process_runner()
 : oac::logger_component("mosquitto_process_runner"),
   _command_line(L"C:\\Program Files (x86)\\mosquitto\\mosquitto.exe")
{
   ZeroMemory(&_proc_info, sizeof(_proc_info));
}

void
mosquitto_process_runner::run_broker()
{
   // config_process_permissions();

   log_info("running Mosquitto broker in a subprocess... ");
   STARTUPINFO startup_info;
   ZeroMemory(&startup_info, sizeof(startup_info));
   ZeroMemory(&_proc_info, sizeof(_proc_info));
   auto cmd = const_cast<wchar_t*>(_command_line.c_str());

   auto success = CreateProcess(
         NULL,                // lpApplicationName
         cmd,                 // lpCommandLine
         NULL,                // lpProcessAttributes
         NULL,                // lpThreadAttributes
         FALSE,               // bInheritHandles
         CREATE_NO_WINDOW,    // dwCreationFlags
         NULL,                // lpEnvironment
         NULL,                // lpCurrentDirectory
         &startup_info,       // lpStartupInfo
         &_proc_info);        // lpProcessInformation
   if (!success)
   {
      auto ec = GetLastError();
      log_error(
            "cannot execute Mosquitto broker as subprocess: return code %d",
            ec);
      OAC_THROW_EXCEPTION(broker_run_error(ec));
   }
   log_info("Mosquitto broker up and running");
}

void
mosquitto_process_runner::shutdown_broker()
{
   log_info("Shutting down Mosquitto broker... ");
   if (!_proc_info.hProcess)
   {
      log_warn("cannot shut down Mosquitto broker: no such process");
      OAC_THROW_EXCEPTION(no_such_broker_error());
   }
   if (
         !TerminateProcess(_proc_info.hProcess, 0) ||
         WaitForSingleObject(_proc_info.hProcess, INFINITE) == WAIT_FAILED ||
         !CloseHandle(_proc_info.hProcess) ||
         !CloseHandle(_proc_info.hThread))
   {
      auto ec = GetLastError();
      log_error(
            "cannot terminate Mosquitto broker subprocess: return code %d",
            ec);
      OAC_THROW_EXCEPTION(broker_shutdown_error(ec));
   }
   log_info("Mosquitto broker shut down successfully");
}

}}} // namespace oac::fv::mqtt
