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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "buffer.h"
#include "stream.h"

using namespace oac;

BOOST_AUTO_TEST_SUITE(ReaderTest)

BOOST_AUTO_TEST_CASE(ShouldReadLine)
{
   Ptr<Buffer> buff = new FixedBuffer(512);
   Ptr<OutputStream> os = new BufferOutputStream(buff);
   Ptr<InputStream> is = new BufferInputStream(buff);
   Ptr<Reader> reader = new Reader(is);
   os->writeAs<std::string>("The quick brown fox jumps over the lazy dog\n");
   BOOST_CHECK_EQUAL(
            "The quick brown fox jumps over the lazy dog",
            reader->readLine());
}

BOOST_AUTO_TEST_CASE(ShouldReadSeveralLines)
{
   Ptr<Buffer> buff = new FixedBuffer(512);
   Ptr<OutputStream> os = new BufferOutputStream(buff);
   Ptr<InputStream> is = new BufferInputStream(buff);
   Ptr<Reader> reader = new Reader(is);
   os->writeAs<std::string>("The quick brown\n");
   os->writeAs<std::string>("fox jumps over\n");
   os->writeAs<std::string>("the lazy dog\n");
   BOOST_CHECK_EQUAL("The quick brown", reader->readLine());
   BOOST_CHECK_EQUAL("fox jumps over", reader->readLine());
   BOOST_CHECK_EQUAL("the lazy dog", reader->readLine());
}

BOOST_AUTO_TEST_CASE(ShouldReadEmptyLineOnEmptyBuffer)
{
   Ptr<Buffer> buff = new FixedBuffer(0);
   Ptr<InputStream> is = new BufferInputStream(buff);
   Ptr<Reader> reader = new Reader(is);
   BOOST_CHECK_EQUAL("", reader->readLine());
}

BOOST_AUTO_TEST_CASE(ShouldReadPedingLineUntilEndOfFile)
{
   Ptr<Buffer> buff = new FixedBuffer(15);
   Ptr<OutputStream> os = new BufferOutputStream(buff);
   Ptr<InputStream> is = new BufferInputStream(buff);
   Ptr<Reader> reader = new Reader(is);
   os->writeAs<std::string>("The quick brown");
   BOOST_CHECK_EQUAL("The quick brown", reader->readLine());
}

BOOST_AUTO_TEST_CASE(ShouldReadLineWithMultipleChunks)
{
   Ptr<Buffer> buff = new FixedBuffer(256);
   Ptr<OutputStream> os = new BufferOutputStream(buff);
   Ptr<InputStream> is = new BufferInputStream(buff);
   Ptr<Reader> reader = new Reader(is);
   os->writeAs<std::string>("The quick brown fox jumps over the lazy dog, ");
   os->writeAs<std::string>("The quick brown fox jumps over the lazy dog\n");
   BOOST_CHECK_EQUAL(
            "The quick brown fox jumps over the lazy dog, "
            "The quick brown fox jumps over the lazy dog",
            reader->readLine());
}


BOOST_AUTO_TEST_SUITE_END()
