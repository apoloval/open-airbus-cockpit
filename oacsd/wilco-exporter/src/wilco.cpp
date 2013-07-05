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

#include <cmath>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <boost/algorithm/string.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/format.hpp>
#include <liboac/fsuipc.h>
#include <liboac/logging.h>
#include <SimConnect.h>

#include "wilco.h"
#include "wilco-efis.h"
#include "wilco-fcu.h"
#include "wilco-internal.h"

namespace oac { namespace we {

namespace {

const std::string AIRCRAFT_NAME[] =
{
   "Airbus A320 CFM",      // A320_CFM
};

class wilco_cockpit_impl : public wilco_cockpit, public dll_inspector
{
public:

   wilco_cockpit_impl(const aircraft& ac)
   throw (invalid_input_error) :
         dll_inspector(dll_info::for_aircraft(ac),
                      load_dll_for_aircraft(ac)),
         _aircraft(ac)
   {
      auto& dll_info = dll_info::for_aircraft(ac);
      auto dll_instance = this->get_dll_instance();
      _efis_ctrl_panel = new efis_control_panel_impl(dll_info, dll_instance);
      _fcu = new flight_control_unit_impl(dll_info, dll_instance);
   }

   virtual ~wilco_cockpit_impl()
   {
      free_dll(this->get_dll_instance());
   }

   virtual const aircraft& get_aircraft() const
   { return _aircraft; }

   virtual gpu_switch get_gpu_switch() const
   {
      auto hp = this->get_data_object<wilco_headpanel*>(VADDR_HEAD_PANEL);
      auto gpu = this->get_data_object<wilco_gpu*>(VADDR_GPU);
      if (hp && hp->gpu_energy)
         return GPU_ON;
      if (gpu && gpu->vtable->isGpuAvailable(gpu))
         return GPU_AVAILABLE;
      return GPU_OFF;
   }

   virtual void get_apu_switches(apu_switches& sw) const
   {
      auto apu = this->get_data_object<wilco_apu*>(VADDR_APU);
      sw.master = apu->master_switch ? APU_MASTER_ON : APU_MASTER_OFF;

      if (!apu->unused_08 && apu->unused_0c == 0x40590000)
         sw.start = APU_START_AVAILABLE;
      else if (apu->master_switch && apu->unused_0c > 0)
         sw.start = APU_START_ON;
      else
         sw.start = APU_START_OFF;
   }

   virtual sd_page_button get_sd_page_button() const
   {
      auto pedestal = this->get_data_object<wilco_pedestal*>(VADDR_PEDESTAL);
      return (sd_page_button) pedestal->sd_page_selected;
   }

   virtual const flight_control_unit& get_flight_control_unit() const 
   { return *_fcu; }

   virtual flight_control_unit& get_flight_control_unit() 
   { return *_fcu; }

   virtual const efis_control_panel& get_efis_control_panel() const
   { return *_efis_ctrl_panel; }

   virtual efis_control_panel& get_efis_control_panel()
   { return *_efis_ctrl_panel; }

   virtual void debug() const
   {
      auto data = (DWORD*) this->get_actual_address(0x1012AA40);
      track_changes_on_memory(data, 4*sizeof(DWORD));
   }

private:

   inline static binary_switch tobinary_switch(bool expr)
   { return expr ? SWITCHED_ON : SWITCHED_OFF; }

   inline static binary_switch tobinary_switch(DWORD expr)
   { return tobinary_switch(expr > 0); }

   aircraft _aircraft;
   ptr<efis_control_panel> _efis_ctrl_panel;
   ptr<flight_control_unit> _fcu;
};

aircraft_type
resolve_aircraft_type_from_title(const aircraft_title& title)
throw (aircraft::invalid_title)
{
   if (boost::contains(title, "Feelthere"))
   {
      if (boost::contains(title, "A320 CFM"))
         return aircraft_type::A320_CFM;
   }
   BOOST_THROW_EXCEPTION(aircraft::invalid_title() << aircraft::title_info(title));
}

} // anonymous namespace

aircraft::aircraft(const aircraft_type& type) :
      type(type), title(AIRCRAFT_NAME[type]) {}


aircraft::aircraft(const aircraft_title& title)
throw (invalid_title) :
      type(resolve_aircraft_type_from_title(title)), title(title) {}

wilco_cockpit*
wilco_cockpit::new_cockpit(const aircraft& aircraft)
{
   return new wilco_cockpit_impl(aircraft);
}

}}; // namespace oac::we
