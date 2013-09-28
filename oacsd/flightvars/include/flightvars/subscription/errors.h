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
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OAC_FV_SUBSCRIPTION_ERRORS_H
#define OAC_FV_SUBSCRIPTION_ERRORS_H

#include <liboac/exception.h>

#include <flightvars/subscription/types.h>
#include <flightvars/var.h>

namespace oac { namespace fv { namespace subs {

OAC_DECL_ABSTRACT_EXCEPTION(mapping_exception);

/**
 * An unknown variable was referenced by input.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(no_such_variable_error, mapping_exception,
   (
      "no such variable %s found in subscription mapper",
      var_id.to_string()
   ),
   (var_id, variable_id));

/**
 * An unknown subscription was referenced by input.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(no_such_subscription_error, mapping_exception,
   ("no such subscription ID %d found in subscription mapper", subs_id),
   (subs_id, subscription_id));

/**
 * An already existing variable was provided as input.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(variable_already_exists_error, mapping_exception,
   (
      "variable %s already exists in subscription mapper",
      var_id.to_string()
   ),
   (var_id, variable_id));

/**
 * An already existing subscription was provided as input.
 */
OAC_DECL_EXCEPTION_WITH_PARAMS(
      subscription_already_exists_error,
      mapping_exception,
   ("subscription ID %d already exists in subscription mapper", subs_id),
   (subs_id, subscription_id));

}}} // namespace oac::fv::subs

#endif
