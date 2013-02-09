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

#include "ui-fcu.h"

#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>

namespace oac { namespace testutil {

FCUTestWindow::FCUTestWindow(FlightControlUnit* fcu, QWidget* parent)
   : QWidget(parent, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint),
     _fcu(fcu)
{
   this->setWindowTitle(tr("Flight Control Unit - Test"));
   auto rootLayout = new QHBoxLayout(this);
   this->setLayout(rootLayout);
   rootLayout->setMargin(10);
   rootLayout->setSpacing(10);

   this->createSpeedWidgets(rootLayout);
}

void
FCUTestWindow::createSpeedWidgets(QLayout* rootLayout)
{
   auto speedGroupBox = new QGroupBox(this);
   rootLayout->addWidget(speedGroupBox);
   speedGroupBox->setTitle("Speed");
   auto speedLayout = new QVBoxLayout(this);
   speedGroupBox->setLayout(speedLayout);
   _speedLCDNumber = new QLCDNumber(3, this);
   speedLayout->addWidget(_speedLCDNumber);
   _speedLCDNumber->setFixedHeight(50);
   _speedLCDNumber->display("---");

   _speedSpinBox = new QSpinBox(this);
   speedLayout->addWidget(_speedSpinBox);
   _speedSpinBox->setRange(50, 400);
   this->connect(_speedSpinBox, SIGNAL(valueChanged(int)),
                 _speedLCDNumber, SLOT(display(int)));
   this->connect(_speedSpinBox, SIGNAL(valueChanged(int)),
                 this, SLOT(onSpeedValueChanged(int)));
   _speedSpinBox->setEnabled(false);
   _speedSpinBox->setSpecialValueText("---");

   auto modeGroupBox = new QGroupBox(tr("Mode"), this);
   speedLayout->addWidget(modeGroupBox);
   modeGroupBox->setLayout(new QVBoxLayout(this));
   _speedManagedRadioButton = new QRadioButton(tr("Managed"), modeGroupBox);
   modeGroupBox->layout()->addWidget(_speedManagedRadioButton);
   _speedManagedRadioButton->setChecked(true);
   this->connect(_speedManagedRadioButton, SIGNAL(clicked()),
                 this, SLOT(onSpeedManaged()));
   _speedSelectedRadioButton = new QRadioButton(tr("Selected"), modeGroupBox);
   modeGroupBox->layout()->addWidget(_speedSelectedRadioButton);
   this->connect(_speedSelectedRadioButton, SIGNAL(clicked()),
                 this, SLOT(onSpeedSelected()));

   _speedDisplayGroupBox = new QGroupBox(tr("Display"), this);
   speedLayout->addWidget(_speedDisplayGroupBox);
   _speedDisplayGroupBox->setEnabled(false);
   _speedDisplayGroupBox->setLayout(new QVBoxLayout(this));
   _speedKnotsRadioButton = new QRadioButton(tr("Knots"), _speedDisplayGroupBox);
   _speedDisplayGroupBox->layout()->addWidget(_speedKnotsRadioButton);
   _speedKnotsRadioButton->setChecked(true);
   _speedMachRadioButton = new QRadioButton(tr("Mach"), _speedDisplayGroupBox);
   _speedDisplayGroupBox->layout()->addWidget(_speedMachRadioButton);
}

void
FCUTestWindow::onSpeedManaged()
{
   _fcu->setSpeedMode(FlightControlUnit::PARAM_MANAGED);

   _speedSpinBox->setValue(_speedSpinBox->minimum());
   _speedSpinBox->setEnabled(false);
   _speedLCDNumber->display("---");

   _speedDisplayGroupBox->setEnabled(false);
}

void
FCUTestWindow::onSpeedSelected()
{
   _fcu->setSpeedMode(FlightControlUnit::PARAM_SELECTED);

   auto speed = _fcu->speedValue();
   auto speedValue = (_fcu->speedUnits() == Speed::UNITS_KT)
         ? speed.asKnots() : speed.asMach();
   _speedSpinBox->setValue(speedValue);
   _speedSpinBox->setEnabled(true);

   _speedDisplayGroupBox->setEnabled(true);
}

void
FCUTestWindow::onSpeedValueChanged(int newValue)
{
   if (newValue > _speedSpinBox->minimum())
   {
      _fcu->setSpeedValue((_fcu->speedUnits() == Speed::UNITS_KT)
                          ? Speed(newValue)
                          : Speed(newValue / 100.0f, Speed::UNITS_MACH));
   }
}

}}; // namespace oac::testutil
