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

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <list>
#include <string>

#include <flightvars/var.h>
#include <liboac/exception.h>
#include <liboac/format.h>

#include "observer.h"

#define PROGRAM_NAME "FlightVarsExplorer"

namespace fv = oac::fv;

enum class action
{
   WATCH,
   SET
};

void
print_help()
{
   std::cerr <<
         oac::format(
               "Usage: %s watch <variable> [<variable>]+",
               PROGRAM_NAME) << std::endl <<
         oac::format(
               "       %s set <variable> <value>",
               PROGRAM_NAME) << std::endl << std::endl <<
         "where," << std::endl <<
         "   <variable> indicates a variable ID expressed as \"<group>-><name>\", using " << std::endl <<
         "              double quotes to avoid misinterpretation of symbol '>' " << std::endl <<
         "   <value>    indicates a variable value expressed as <type>:<value>, where " << std::endl <<
         "              <type> is one of 'bool', 'byte', 'word' or 'dword', and <value>" << std::endl <<
         "              is '1'', 'true'' or 'yes' for boolean, or a number otherwise" <<
         std::endl << std::endl <<
         oac::format("Examples: %s watch \"fsuipc/offset->0x0330:2\"", PROGRAM_NAME) << std::endl <<
         oac::format("          %s set \"fsuipc/offset->0x0330:2\" word:16250", PROGRAM_NAME) << std::endl;
}

void print_error_and_exit(const std::string& error)
{
   std::cerr << oac::format("Error: %s", error) << std::endl << std::endl;
   print_help();
   exit(1);
}

fv::var_observer::variable_id_list
parse_variable_ids(int argc, char* argv[])
{
   fv::var_observer::variable_id_list result;
   for (int i = 2; i < argc; i++)
   {
      result.push_back(fv::variable_id::parse(argv[i]));
   }
   return result;
}

fv::variable_value
parse_variable_value(const std::string& arg)
{
   std::vector<std::string> tokens;
   boost::iter_split(tokens, arg, boost::algorithm::first_finder(":"));
   if (tokens.size() != 2)
   {
      print_error_and_exit(
            oac::format("cannot parse variable value in '%s'", arg));
   }

   auto& type = tokens[0];
   auto& value = tokens[1];
   std::transform(type.begin(), type.end(), type.begin(), std::tolower);
   std::transform(value.begin(), value.end(), value.begin(), std::tolower);
   if (type == "bool")
      return fv::variable_value::from_bool(
            value == "1" || value == "true" || value == "yes");
   else if (type == "byte")
      return fv::variable_value::from_byte(
            boost::lexical_cast<std::uint8_t>(value));
   else if (type == "word")
      return fv::variable_value::from_word(
            boost::lexical_cast<std::uint16_t>(value));
   else if (type == "dword")
      return fv::variable_value::from_dword(
            boost::lexical_cast<std::uint32_t>(value));
   else
      print_error_and_exit(
            oac::format("invalid variable value type '%s'", type));
   exit(1); // never reached
}

action
parse_action(int argc, char* argv[])
{
   if (argc < 2)
   {
      print_error_and_exit("invalid argument count");
   }
   std::string act(argv[1]);
   std::transform(act.begin(), act.end(), act.begin(), std::tolower);
   if (act == "watch")
   {
      if (argc < 3)
         print_error_and_exit("invalid argument count for 'watch' action");
      return action::WATCH;
   }
   else if (act == "set")
   {
      if (argc < 4)
         print_error_and_exit("invalid argument count for 'set' action");
      return action::SET;
   }
   else
      print_error_and_exit(oac::format("unknown action '%s'", act));
   exit(1); // never reached
}


int
main(int argc, char* argv[])
{
   try
   {
      auto act = parse_action(argc, argv);
      switch (act)
      {
         case action::WATCH:
         {
            auto vars = parse_variable_ids(argc, argv);
            fv::var_observer obs(vars);
            while (true)
            {
               std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            return 0;
         }
         case action::SET:
         {
            auto var_id = fv::variable_id::parse(argv[2]);
            auto var_value = parse_variable_value(argv[3]);
            fv::var_observer obs(var_id);
            std::cerr <<
                  oac::format(
                        "Setting %s with value %s",
                        var_id.to_string(),
                        var_value.to_string()) <<
                  std::endl;
            obs.set_value(var_id, var_value);
            std::cerr << "Value set successfully" << std::endl;
            return 0;
            break;
         }
      }
   }
   catch (fv::variable_id::parse_error& e)
   {
      std::cerr <<
            "Error while parsing program arguments: " <<
            e.message() <<
            std::endl;
   }
   catch (oac::exception& e)
   {
      std::cerr << "Unexpected exception thrown" << std::endl;
      std::cerr << e.report() << std::endl << std::endl;
   }

   print_help();
   return 1;
}
