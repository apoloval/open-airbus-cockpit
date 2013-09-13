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

#ifndef OAC_COCKPIT_FSUIPC_H
#define OAC_COCKPIT_FSUIPC_H

#include "buffer.h"
#include "cockpit.h"
#include "fsuipc.h"

namespace oac {

class fsuipc_cockpit_back : public cockpit_back
{
public:

   inline fsuipc_cockpit_back(
         const local_fsuipc::factory_ptr& fsuipc_fact) :
      _fsuipc_fact(fsuipc_fact)
   {}

   virtual void sync_up() throw (sync_error);
   virtual void sync_down() throw (sync_error);
   virtual flight_control_unit& get_flight_control_unit() throw (sync_error);
   virtual efis_control_panel& get_efis_control_panel() throw (sync_error);

private:

   double_buffer<shifted_buffer<linear_buffer>>::ptr_type _buffer;
   local_fsuipc::factory_ptr _fsuipc_fact;
   local_fsuipc_ptr _fsuipc;
   flight_control_unit_ptr _fcu;
   efis_control_panel_ptr _efis_ctrl_panel;

   void init_buffer();
   void init_fsuipc() throw (sync_error);
   void init_fcu() throw (sync_error);
   void init_efis_control_panel() throw (sync_error);
   bool is_sync() const;
};

typedef std::shared_ptr<fsuipc_cockpit_back> fsuipc_cockpit_back_ptr;

} // namespace oac

#endif
