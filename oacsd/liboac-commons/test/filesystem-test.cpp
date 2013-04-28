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

#include <Windows.h>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "filesystem.h"

using namespace oac;

BOOST_AUTO_TEST_SUITE(FileMeta)

BOOST_AUTO_TEST_CASE(ShouldIndicateExistence)
{
   File f("C:\\Windows\\Notepad.exe");
   BOOST_CHECK(f.exists());
}

BOOST_AUTO_TEST_CASE(ShouldIndicateUnexistence)
{
   File f("C:\\Windows\\Foobar.exe");
   BOOST_CHECK(!f.exists());
}

BOOST_AUTO_TEST_CASE(ShouldIndicateRegularFile)
{
   File f("C:\\Windows\\Notepad.exe");
   BOOST_CHECK(f.isRegularFile());
   BOOST_CHECK(!f.isDirectory());
}

BOOST_AUTO_TEST_CASE(ShouldIndicateDirectory)
{
   File f("C:\\Windows");
   BOOST_CHECK(!f.isRegularFile());
   BOOST_CHECK(f.isDirectory());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(FileIO)

BOOST_AUTO_TEST_CASE(ShouldWriteAndRead)
{
   File f(File::makeTemp());
   auto output = f.append();
   output->writeAs<DWORD>(100);
   output->writeAs<DWORD>(200);
   output->writeAs<DWORD>(300);
   output.reset(); // stream close

   auto input = f.read();
   BOOST_CHECK_EQUAL(100, input->readAs<DWORD>());
   BOOST_CHECK_EQUAL(200, input->readAs<DWORD>());
   BOOST_CHECK_EQUAL(300, input->readAs<DWORD>());
}

BOOST_AUTO_TEST_CASE(ShouldNotReadBehindEndOfFile)
{
   File f(File::makeTemp());
   auto output = f.append();
   output->writeAs<DWORD>(100);
   output->writeAs<DWORD>(200);
   output->writeAs<DWORD>(300);
   output.reset(); // stream close

   auto input = f.read();
   BYTE buff[4];
   input->readAs<DWORD>();
   input->readAs<DWORD>();
   input->readAs<DWORD>();
   BOOST_CHECK_EQUAL(0, input->read(buff, 4));
}

BOOST_AUTO_TEST_SUITE_END()
