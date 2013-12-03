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

#include <Windows.h>

#include <boost/test/auto_unit_test.hpp>

#include <liboac/filesystem.h>

using namespace oac;

BOOST_AUTO_TEST_SUITE(fileMeta)

BOOST_AUTO_TEST_CASE(ShouldIndicateExistence)
{
   file f("C:\\Windows\\Notepad.exe");
   BOOST_CHECK(f.exists());
}

BOOST_AUTO_TEST_CASE(ShouldIndicateUnexistence)
{
   file f("C:\\Windows\\Foobar.exe");
   BOOST_CHECK(!f.exists());
}

BOOST_AUTO_TEST_CASE(ShouldIndicateRegularfile)
{
   file f("C:\\Windows\\Notepad.exe");
   BOOST_CHECK(f.is_regular_file());
   BOOST_CHECK(!f.is_directory());
}

BOOST_AUTO_TEST_CASE(ShouldIndicateDirectory)
{
   file f("C:\\Windows");
   BOOST_CHECK(!f.is_regular_file());
   BOOST_CHECK(f.is_directory());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(fileIO)

BOOST_AUTO_TEST_CASE(ShouldWriteAndRead)
{
   file f(file::makeTemp());
   auto output = f.append();
   stream::write_as<DWORD>(*output, 100);
   stream::write_as<DWORD>(*output, 200);
   stream::write_as<DWORD>(*output, 300);
   output.reset(); // stream close

   auto input = f.read();
   BOOST_CHECK_EQUAL(100, stream::read_as<DWORD>(*input));
   BOOST_CHECK_EQUAL(200, stream::read_as<DWORD>(*input));
   BOOST_CHECK_EQUAL(300, stream::read_as<DWORD>(*input));
}

BOOST_AUTO_TEST_CASE(ShouldNotReadBehindEndOffile)
{
   file f(file::makeTemp());
   auto output = f.append();
   stream::write_as<DWORD>(*output, 100);
   stream::write_as<DWORD>(*output, 200);
   stream::write_as<DWORD>(*output, 300);
   output.reset(); // stream close

   auto input = f.read();
   BYTE buff[4];
   stream::read_as<DWORD>(*input);
   stream::read_as<DWORD>(*input);
   stream::read_as<DWORD>(*input);
   BOOST_CHECK_EQUAL(0, input->read(buff, 4));
}

BOOST_AUTO_TEST_SUITE_END()
