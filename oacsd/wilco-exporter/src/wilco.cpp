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

class WilcoCockpitImpl : public WilcoCockpit, public DllInspector
{
public:

   WilcoCockpitImpl(const Aircraft& aircraft)
   throw (InvalidInputError) :
         DllInspector(DllInfo::forAircraft(aircraft),
                      LoadDLLForAircraft(aircraft)),
         _aircraft(aircraft)
   {
      auto& dll_info = DllInfo::forAircraft(aircraft);
      auto dll_instance = this->getDLLInstance();
      _efis_ctrl_panel = new EFISControlPanelImpl(dll_info, dll_instance);
      _fcu = new FlightControlUnitImpl(dll_info, dll_instance);
   }

   virtual ~WilcoCockpitImpl()
   {
      FreeDLL(this->getDLLInstance());
   }

   virtual const Aircraft& aircraft() const
   { return _aircraft; }

   virtual GPUSwitch getGpuSwitch() const
   {
      auto hp = this->getDataObject<Wilco_HeadPanel*>(VADDR_HEAD_PANEL);
      auto gpu = this->getDataObject<Wilco_GPU*>(VADDR_GPU);
      if (hp && hp->gpu_energy)
         return GPU_ON;
      if (gpu && gpu->vtable->isGpuAvailable(gpu))
         return GPU_AVAILABLE;
      return GPU_OFF;
   }

   virtual void getAPUSwitches(APUSwitches& sw) const
   {
      auto apu = this->getDataObject<Wilco_APU*>(VADDR_APU);
      sw.master = apu->master_switch ? APU_MASTER_ON : APU_MASTER_OFF;

      if (!apu->unused_08 && apu->unused_0c == 0x40590000)
         sw.start = APU_START_AVAILABLE;
      else if (apu->master_switch && apu->unused_0c > 0)
         sw.start = APU_START_ON;
      else
         sw.start = APU_START_OFF;
   }

   virtual SDPageButton getSDPageButton() const
   {
      auto pedestal = this->getDataObject<Wilco_Pedestal*>(VADDR_PEDESTAL);
      return (SDPageButton) pedestal->sd_page_selected;
   }

   virtual const FlightControlUnit& getFlightControlUnit() const 
   { return *_fcu; }

   virtual FlightControlUnit& getFlightControlUnit() 
   { return *_fcu; }

   virtual const EFISControlPanel& getEFISControlPanel() const
   { return *_efis_ctrl_panel; }

   virtual EFISControlPanel& getEFISControlPanel()
   { return *_efis_ctrl_panel; }

   virtual void debug() const
   {
      auto data = (DWORD*) this->getActualAddress(0x1012AA40);
      TrackChangesOnMemory(data, 4*sizeof(DWORD));
   }

private:

   inline static BinarySwitch toBinarySwitch(bool expr)
   { return expr ? SWITCHED_ON : SWITCHED_OFF; }

   inline static BinarySwitch toBinarySwitch(DWORD expr)
   { return toBinarySwitch(expr > 0); }

   Aircraft _aircraft;
   Ptr<EFISControlPanel> _efis_ctrl_panel;
   Ptr<FlightControlUnit> _fcu;
};

Aircraft::Type
ResolveAircraftTypeFromTitle(const Aircraft::Title& title)
throw (Aircraft::InvalidTitle)
{
   if (boost::contains(title, "Feelthere"))
   {
      if (boost::contains(title, "A320 CFM"))
         return Aircraft::A320_CFM;
   }
   THROW_ERROR(Aircraft::InvalidTitle() << Aircraft::TitleInfo(title));
}

} // anonymous namespace

Aircraft::Aircraft(const Type& type) :
      type(type), title(AIRCRAFT_NAME[type]) {}


Aircraft::Aircraft(const Title& title)
throw (InvalidTitle) :
      type(ResolveAircraftTypeFromTitle(title)), title(title) {}

WilcoCockpit*
WilcoCockpit::newCockpit(const Aircraft& aircraft)
{
   return new WilcoCockpitImpl(aircraft);
}

}}; // namespace oac::we
