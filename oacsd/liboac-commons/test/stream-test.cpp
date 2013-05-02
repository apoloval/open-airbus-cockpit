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

BOOST_AUTO_TEST_SUITE(readerTest)

BOOST_AUTO_TEST_CASE(ShouldReadLine)
{
   ptr<buffer> buff = new fixed_buffer(512);
   ptr<output_stream> os = new buffer_output_stream(buff);
   ptr<input_stream> is = new buffer_input_stream(buff);
   ptr<reader> rd = new reader(is);
   os->write_as<std::string>("The quick brown fox jumps over the lazy dog\n");
   BOOST_CHECK_EQUAL(
            "The quick brown fox jumps over the lazy dog",
            rd->readLine());
}

BOOST_AUTO_TEST_CASE(ShouldReadSeveralLines)
{
   ptr<buffer> buff = new fixed_buffer(512);
   ptr<output_stream> os = new buffer_output_stream(buff);
   ptr<input_stream> is = new buffer_input_stream(buff);
   ptr<reader> rd = new reader(is);
   os->write_as<std::string>("The quick brown\n");
   os->write_as<std::string>("fox jumps over\n");
   os->write_as<std::string>("the lazy dog\n");
   BOOST_CHECK_EQUAL("The quick brown", rd->readLine());
   BOOST_CHECK_EQUAL("fox jumps over", rd->readLine());
   BOOST_CHECK_EQUAL("the lazy dog", rd->readLine());
}

BOOST_AUTO_TEST_CASE(ShouldReadEmptyLineOnEmptyBuffer)
{
   ptr<buffer> buff = new fixed_buffer(0);
   ptr<input_stream> is = new buffer_input_stream(buff);
   ptr<reader> rd = new reader(is);
   BOOST_CHECK_EQUAL("", rd->readLine());
}

BOOST_AUTO_TEST_CASE(ShouldReadPedingLineUntilEndOfFile)
{
   ptr<buffer> buff = new fixed_buffer(15);
   ptr<output_stream> os = new buffer_output_stream(buff);
   ptr<input_stream> is = new buffer_input_stream(buff);
   ptr<reader> rd = new reader(is);
   os->write_as<std::string>("The quick brown");
   BOOST_CHECK_EQUAL("The quick brown", rd->readLine());
}

BOOST_AUTO_TEST_CASE(ShouldReadLineWithMultipleChunks)
{
   ptr<buffer> buff = new fixed_buffer(256);
   ptr<output_stream> os = new buffer_output_stream(buff);
   ptr<input_stream> is = new buffer_input_stream(buff);
   ptr<reader> rd = new reader(is);
   os->write_as<std::string>("The quick brown fox jumps over the lazy dog, ");
   os->write_as<std::string>("The quick brown fox jumps over the lazy dog\n");
   BOOST_CHECK_EQUAL(
            "The quick brown fox jumps over the lazy dog, "
            "The quick brown fox jumps over the lazy dog",
            rd->readLine());
}


BOOST_AUTO_TEST_SUITE_END()
