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

#ifndef OAC_FV_CORE_DOMAIN_H
#define OAC_FV_CORE_DOMAIN_H

#include <boost/filesystem/path.hpp>

#include "mqtt/client.h"

namespace oac { namespace fv { namespace core {

OAC_DECL_ABSTRACT_EXCEPTION(domain_init_exception);

OAC_DECL_EXCEPTION_WITH_PARAMS(invalid_domain_exports, domain_init_exception,
   ("invalid exports for domain %s read from file %s",
         domain_name, exports_file.string()),
   (domain_name, std::string),
   (exports_file, boost::filesystem::path)
);

class domain_controller
{
public:

   domain_controller(const mqtt::client_ptr& mqtt_client)
    : _mqtt { mqtt_client }
   {}

protected:

   /** Retrieve a reference to MQTT client. */
   mqtt::client& mqtt() { return *_mqtt; }

private:

   mqtt::client_ptr _mqtt;
};

using domain_controller_ptr = std::shared_ptr<domain_controller>;

}}}

#endif
