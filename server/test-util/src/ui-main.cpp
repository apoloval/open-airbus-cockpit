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

#include <Qt/QInputDialog.h>
#include <Qt/QMessageBox.h>

#include "ui-main.h"

namespace oac { namespace testutil {

MainWindow::MainWindow(MainController* ctrl)
   : QWidget(nullptr, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint),
     _ctrl(ctrl)
{
   if (!ctrl)
      _ctrl = new MainController();

   this->setWindowTitle(tr("Open Airbus Cockpit - Test Utility"));

   _rootLayout = new QVBoxLayout(this);
   this->setLayout(_rootLayout);
   _rootLayout->setMargin(10);
   _rootLayout->setSpacing(10);

   // Flight control unit
   _fcuGroupBox = new QGroupBox(this);
   _rootLayout->addWidget(_fcuGroupBox);
   _fcuGroupBox->setTitle(tr("Flight Control Unit"));
   auto fcuLayout = new QHBoxLayout(this);
   _fcuGroupBox->setLayout(fcuLayout);

   auto fcuStatusLabel = new QLabel(tr("Status:"));
   fcuLayout->addWidget(fcuStatusLabel);
   _fcuStatusValueLabel = new QLabel(tr("disconnected"));
   _fcuStatusValueLabel->setStyleSheet(tr("QLabel {color: red}"));
   _fcuStatusValueLabel->setFixedSize(100, 25);
   fcuLayout->addWidget(_fcuStatusValueLabel);

   _fcuConnectionButton = new QPushButton(tr("Connect"), this);
   fcuLayout->addWidget(_fcuConnectionButton);
   this->connect(_fcuConnectionButton, SIGNAL(clicked()),
                 this, SLOT(onFcuConnectionButtonPressed()));

   _fcuTestButton = new QPushButton(tr("Test"), this);
   fcuLayout->addWidget(_fcuTestButton);
   _fcuTestButton->setDisabled(true);
}

bool
MainWindow::selectSerialDevice(QString &selected)
{
   SerialDeviceInfoArray devInfos;
   SerialDeviceManager::getDefault().listSerialDevices(devInfos);

   if (devInfos.empty()){
      QMessageBox msgBox;
      msgBox.setWindowTitle(tr("No device found"));
      msgBox.setText("No serial device was found. Please check that "
                     "Arduino drivers are successfully installed.");
      msgBox.exec();
      return false;
   }

   QStringList items;
   for (auto it = devInfos.begin(), end = devInfos.end(); it != end; ++it) {
      items << QString::fromStdWString(it->name);
   }
   bool ok;
   selected = QInputDialog::getItem(this, tr("Select a serial port"),
                                    tr("Serial port:"), items, 0, false, &ok);
   return ok;
}

void
MainWindow::onFcuConnectionButtonPressed()
{
   auto devInfo = _ctrl->fcuDevInfo();
   if (!devInfo.connected)
   {
      QString selectedDev;
      if (this->selectSerialDevice(selectedDev))
      {
         _ctrl->connectFcu(selectedDev.toStdWString());
         _fcuStatusValueLabel->setText(tr("connected to ") + selectedDev);
         _fcuStatusValueLabel->setStyleSheet(tr("QLabel {color: green}"));
         _fcuConnectionButton->setText(tr("Disconnect"));
         _fcuTestButton->setEnabled(true);
      }
   }
   else
   {
      _ctrl->disconnectFcu();
      _fcuStatusValueLabel->setText(tr("disconnected"));
      _fcuStatusValueLabel->setStyleSheet(tr("QLabel {color: red}"));
      _fcuConnectionButton->setText(tr("Connect"));
      _fcuTestButton->setDisabled(true);
   }
}

}}; // namespace oac::testutil
