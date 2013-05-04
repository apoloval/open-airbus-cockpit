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
   auto buff = make_ptr(new fixed_buffer(512));
   auto os = buffer::make_output_stream(buff);
   auto is = buffer::make_input_stream(buff);
   stream::write_as_string(
            *os, "The quick brown fox jumps over the lazy dog\n");
   BOOST_CHECK_EQUAL(
            "The quick brown fox jumps over the lazy dog",
            stream::read_line(*is));
}

BOOST_AUTO_TEST_CASE(ShouldReadSeveralLines)
{
   auto buff = make_ptr(new fixed_buffer(512));
   auto os = buffer::make_output_stream(buff);
   auto is = buffer::make_input_stream(buff);
   stream::write_as_string(*os, "The quick brown\n");
   stream::write_as_string(*os, "fox jumps over\n");
   stream::write_as_string(*os, "the lazy dog\n");
   BOOST_CHECK_EQUAL("The quick brown", stream::read_line(*is));
   BOOST_CHECK_EQUAL("fox jumps over", stream::read_line(*is));
   BOOST_CHECK_EQUAL("the lazy dog", stream::read_line(*is));
}

BOOST_AUTO_TEST_CASE(ShouldReadEmptyLineOnEmptyBuffer)
{
   auto buff = make_ptr(new fixed_buffer(0));
   auto is = buffer::make_input_stream(buff);
   BOOST_CHECK_EQUAL("", stream::read_line(*is));
}

BOOST_AUTO_TEST_CASE(ShouldReadPendingLineUntilEndOfFile)
{
   auto buff = make_ptr(new fixed_buffer(15));
   auto os = buffer::make_output_stream(buff);
   auto is = buffer::make_input_stream(buff);
   stream::write_as_string(*os, "The quick brown");
   BOOST_CHECK_EQUAL("The quick brown", stream::read_line(*is));
}

BOOST_AUTO_TEST_CASE(ShouldReadLineWithMultipleChunks)
{
   auto buff = make_ptr(new fixed_buffer(256));
   auto os = buffer::make_output_stream(buff);
   auto is = buffer::make_input_stream(buff);
   stream::write_as_string(*os, "The quick brown fox jumps over the lazy dog, ");
   stream::write_as_string(*os, "The quick brown fox jumps over the lazy dog\n");
   BOOST_CHECK_EQUAL(
            "The quick brown fox jumps over the lazy dog, "
            "The quick brown fox jumps over the lazy dog",
            stream::read_line(*is));
}

BOOST_AUTO_TEST_SUITE_END()
