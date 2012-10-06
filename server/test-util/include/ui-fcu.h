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

#ifndef OAC_TESTUTIL_UI_FCU_H
#define OAC_TESTUTIL_UI_FCU_H

#include <QtGui/QGroupBox>
#include <QtGui/QLCDNumber>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

#include <devices.h>

namespace oac { namespace testutil {

class FCUTestWindow : public QWidget
{
   Q_OBJECT

public:

   FCUTestWindow(FlightControlUnit* fcu, QWidget* parent = nullptr);

private:

   FlightControlUnit* _fcu;

   QLCDNumber* _speedLCDNumber;
   QSpinBox* _speedSpinBox;
   QRadioButton* _speedManagedRadioButton;
   QRadioButton* _speedSelectedRadioButton;
   QGroupBox* _speedDisplayGroupBox;
   QRadioButton* _speedKnotsRadioButton;
   QRadioButton* _speedMachRadioButton;

   void createSpeedWidgets(QLayout* rootLayout);

private slots:

   void onSpeedManaged();
   void onSpeedSelected();
   void onSpeedValueChanged(int newValue);
};

}}; // namespace oac::testutil

#endif
