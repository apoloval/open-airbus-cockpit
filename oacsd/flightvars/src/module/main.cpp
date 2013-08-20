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

// Must be included first to avoid including issues in Boost::asio
#include "server.h"

#include <Windows.h>

#include <liboac/filesystem.h>
#include <liboac/logging.h>
#include <liboac/timing.h>

#include "core.h"
#include "fsuipc.h"

#define LOG_FILE "C:\\Windows\\Temp\\FlightVars.log"

using namespace oac;
using namespace oac::fv;

namespace {

std::shared_ptr<boost::asio::io_service> io_srv;
std::shared_ptr<simconnect_tick_observer> tick_obs;
std::shared_ptr<flight_vars_server> server;
boost::thread srv_thread;

}

struct flight_vars_component_launcher : logger_component
{

   flight_vars_component_launcher()
      : logger_component("flight_vars_component_launcher")
   {}

   void
   start_io_service()
   {
      if (!io_srv)
         io_srv = std::make_shared<boost::asio::io_service>();
   }

   void
   start_tick_observer()
   {
      if (!tick_obs)
      {
         try
         {
            log_info("Initializing tick observer via SimConnect");
            tick_obs = std::make_shared<simconnect_tick_observer>();
            log_info("Tick observer successfully initialized");
         }
         catch (error& e)
         {
            log(
                  log_level::FAIL,
                  "Unexpected error while initializing tick observer: %s",
                  boost::diagnostic_information(e));
         }
      }
   }

   bool
   start_fsuipc()
   {
      try
      {
         log_info("Initializing FSUIPC FlightVars object");
         auto fsuipc = std::make_shared<local_fsuipc_flight_vars>();
               tick_obs->register_handler(
                     std::bind(
                           &local_fsuipc_flight_vars::check_for_updates,
                           fsuipc));

         flight_vars_core::instance()->register_group_master(
               local_fsuipc_flight_vars::VAR_GROUP,
               fsuipc);

         log_info("FSUIPC FlightVars object successfully initialized");
         return true;
      }
      catch (error& e)
      {
         log(
               log_level::FAIL,
               "Unexpected error while initializing FSUIPC "
               "Flight Vars object: %s",
               boost::diagnostic_information(e));
         return false;
      }
   }

   void
   start_server()
   {
      if (!server)
      {
         try
         {
            log_info("Initializing FlightVars TCP server");

            auto core = flight_vars_core::instance();
            server = std::make_shared<flight_vars_server>(
                  core,
                  flight_vars_server::DEFAULT_PORT,
                  io_srv);

            srv_thread = boost::thread([this]() {
               for (;;)
               {
                  try
                  {
                     io_srv->run();
                     break; // run terminates, exit normally
                  }
                  catch (error& e)
                  {
                     log(
                           log_level::FAIL,
                           "Unexpected error while running the IO service: %s",
                           boost::diagnostic_information(e));
                  }
               }
            });

            log_info("FlightVars TCP server successfully initialized");
         } catch (error& e)
         {
            log(
                  log_level::FAIL,
                  "Unexpected error: %s",
                  boost::diagnostic_information(e));
         }
      }
   }

   void
   stop_server()
   {
      log_info("Stopping FlightVars server");
      io_srv->stop();
      srv_thread.join();
      log_info("FlightVars server stopped");
   }
};

void __stdcall DLLStart(void)
{
   try
   {
      file log_file(LOG_FILE);
      set_main_logger(make_logger(log_level::INFO, log_file.append()));

      flight_vars_component_launcher launcher;

      launcher.start_io_service();
      launcher.start_tick_observer();
      launcher.start_fsuipc();
      launcher.start_server();
   }
   catch (std::exception& e)
   {
      log(
            "DLLStart",
            log_level::FAIL,
            str(boost::format("Unexpected error: %s") % e.what()));
   }
   catch (...)
   {
      log(
            "DLLStart",
            log_level::FAIL,
            "Unexpected error: unknown exception thrown");
   }
}

void __stdcall DLLStop(void)
{
   flight_vars_component_launcher launcher;

   launcher.stop_server();
   close_main_logger();
}
