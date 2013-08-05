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

#ifndef OAC_TESTUTIL_UIMAIN_H
#define OAC_TESTUTIL_UIMAIN_H

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

#include "ctrl-main.h"
#include "ui-fcu.h"

namespace oac { namespace testutil {

class MainWindow : public QWidget
{
    Q_OBJECT

public:

    MainWindow(MainController* ctrl = nullptr);

private:

    MainController* _ctrl;

    QVBoxLayout* _rootLayout;

    QGroupBox*   _fcuGroupBox;
    QLabel*      _fcuStatusValueLabel;
    QPushButton* _fcuConnectionButton;
    QPushButton* _fcuTestButton;

    FCUTestWindow* _fcuTestWindow;

    bool selectSerialDevice(QString& selected);

private slots:

    void onFcuConnectionButtonPressed();
    void onTestFcuButtonPressed();
    void onTestFcuWindowDestroyed();

};

}}; // namespace oac::testutil

#endif
