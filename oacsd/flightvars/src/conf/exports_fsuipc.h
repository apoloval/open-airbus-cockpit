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

#ifndef OAC_FV_CONF_EXPORTS_FSUIPC_H
#define OAC_FV_CONF_EXPORTS_FSUIPC_H

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <liboac/fsuipc/offset.h>

#include "conf/provider.h"
#include "core/domain.h"

namespace oac { namespace fv { namespace conf {

class offset_address_translator
{
public:
   using internal_type = std::string;
   using external_type = oac::fsuipc::offset_address;

   boost::optional<external_type> get_value(const internal_type &v)
   {
      oac::fsuipc::offset_address addr;
      std::stringstream ss;

      if (v.substr(0, 2) == "0x")
         ss << std::hex;

      ss << v;
      ss >> addr;
      return addr;
   }

   boost::optional<internal_type> put_value(const external_type &v)
   {
      return format("0x%x", v);
   }
};

template <typename Collection>
void load_exports(
      const conf::domain_settings& dom_setts,
      Collection& exports)
{
   try
   {
      boost::property_tree::ptree pt;
      conf::bpt_auto_config_loader config_loader;
      config_loader.load_from_file(dom_setts.exports_file, pt);

      for (auto& entry : pt)
      {
         auto& offset_desc = entry.second;
         auto addr = offset_desc.get<oac::fsuipc::offset_address>(
               "address", offset_address_translator());
         auto len = static_cast<oac::fsuipc::offset_length>(
               offset_desc.get<int>("length"));
         oac::fsuipc::offset offset(addr, len);

         exports.insert(exports.begin(), offset);
      }
   }
   catch (const conf::config_exception& e)
   {
      OAC_THROW_EXCEPTION(
            core::invalid_domain_exports(
                  dom_setts.name, dom_setts.exports_file, e));
   }
   catch (const boost::property_tree::ptree_error& e)
   {
      OAC_THROW_EXCEPTION(
            core::invalid_domain_exports(
                  dom_setts.name, dom_setts.exports_file, e));
   }
}

}}}

#endif
