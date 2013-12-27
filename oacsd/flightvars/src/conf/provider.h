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

#ifndef OAC_FV_CONF_PROVIDER_H
#define OAC_FV_CONF_PROVIDER_H

#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/property_tree/ptree.hpp>

#include "conf/settings.h"

namespace oac { namespace fv { namespace conf {

OAC_DECL_ABSTRACT_EXCEPTION(config_exception);

OAC_DECL_EXCEPTION_WITH_PARAMS(illegal_property_value, config_exception,
   ("invalid value '%s' for config property %s", value, key),
   (key, std::string),
   (value, std::string)
);

OAC_DECL_EXCEPTION_WITH_PARAMS(invalid_config_format, config_exception,
   ("invalid format of config file %s: expected %s format",
         file.string(), expected_format),
   (file, boost::filesystem::path),
   (expected_format, std::string)
);

/**
 * Load settings from a Boost Property Tree object.
 */
void bpt_load_settings(
      const boost::property_tree::ptree& props,
      flightvars_settings& settings)
throw (config_exception);

/** A property tree loader for JSON files. */
struct bpt_json_config_loader
{
   void load_from_file(
         const boost::filesystem::path& file,
         boost::property_tree::ptree& pt);
};

/** A property tree loader for XML files. */
struct bpt_xml_config_loader
{
   void load_from_file(
         const boost::filesystem::path& file,
         boost::property_tree::ptree& pt);
};

/**
 * A property tree loader that attemps to detect file format and use the
 * appropriate loader.
 */
struct bpt_auto_config_loader
{
   void load_from_file(
         const boost::filesystem::path& file,
         boost::property_tree::ptree& pt);
};

class bpt_config_provider
{
public:

   bpt_config_provider(
         const boost::filesystem::path& config_dir)
    : _config_dir(config_dir)
   {}

   void load_settings(
         flightvars_settings& settings)
   throw (config_exception);

private:

   boost::filesystem::path _config_dir;
};

}}} // namespace oac::fv::conf

#endif
