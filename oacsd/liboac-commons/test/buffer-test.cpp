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

#include <string>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include "buffer.h"

using namespace oac;

void fill_buffer(buffer& buff, DWORD from_offset)
{
   for (DWORD i = 0; i < buff.capacity(); i += sizeof(DWORD))
   {
      if (i + sizeof(DWORD) <= buff.capacity())
         buff.write_as<DWORD>(from_offset + i, i / sizeof(DWORD));
   }
}

void buffer_write_test(
      const buffer::factory& fact, DWORD offset)
{
   ptr<buffer> buff(fact.create_buffer());
   DWORD d = 600;
   buff->write(&d, offset, sizeof(DWORD));
}

void buffer_read_test(
      const buffer::factory& fact, DWORD offset)
{
   ptr<buffer> buff(fact.create_buffer());
   DWORD d = 600;
   buff->read(&d, offset, sizeof(DWORD));
}

void buffer_write_read_test(
      const buffer::factory& fact, DWORD offset)
{
   ptr<buffer> buff(fact.create_buffer());
   DWORD d1 = 600, d2;
   buff->write(&d1, offset, sizeof(DWORD));
   buff->read(&d2, offset, sizeof(DWORD));
   BOOST_CHECK_EQUAL(d1, d2);
}

void buffer_write_as_read_as_test(
      const buffer::factory& fact, DWORD offset)
{
   ptr<buffer> buff(fact.create_buffer());
   buff->write_as<DWORD>(offset, 600);
   BOOST_CHECK_EQUAL(600, buff->read_as<DWORD>(offset));
}

void buffer_copy_test(
      const buffer::factory& fact0,
      const buffer::factory& fact1,
      DWORD fill_from_offset,
      DWORD offset0,
      DWORD offset1,
      DWORD length)
throw (buffer::out_of_bounds_error)
{
   ptr<buffer> buff0(fact0.create_buffer());
   ptr<buffer> buff1(fact1.create_buffer());

   fill_buffer(*buff0, fill_from_offset);
   buff1->copy(*buff0, offset0, offset1, length);
   for (DWORD i = 0; i < length; i += sizeof(DWORD))
      BOOST_CHECK_EQUAL(i / 4, buff1->read_as<DWORD>(offset1 + i));
}

void double_buffer_test(DWORD* seq, unsigned int seql, bool expect_mod)
{
   double_buffer buff(new fixed_buffer(12), new fixed_buffer(12));
   for (unsigned int i = 0; i < seql; i++)
   {
      buff.write_as<DWORD>(0, seq[i]);
      buff.swap();
   }
   BOOST_CHECK(expect_mod == buff.is_modified<sizeof(DWORD)>(0));
}

template <typename BufferType>
ptr<BufferType> prepare_buffer_stream(DWORD length)
{
   auto b = new fixed_buffer(length);
   fill_buffer(*b, 0);
   return new BufferType(b);
}

BOOST_AUTO_TEST_SUITE(fixed_bufferTestSuite);

BOOST_AUTO_TEST_CASE(ShouldCreate)
{
   fixed_buffer buff(12);
   BOOST_CHECK_EQUAL(12, buff.capacity());
}

BOOST_AUTO_TEST_CASE(ShouldCreateFromFactory)
{
   ptr<buffer::factory> fact(new fixed_buffer::factory(12));
   ptr<buffer> buff(fact->create_buffer());
   BOOST_CHECK_EQUAL(12, buff->capacity());
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnFirstPosition)
{
   buffer_write_read_test(fixed_buffer::factory(12), 0);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnMiddlePosition)
{
   buffer_write_read_test(fixed_buffer::factory(12), 4);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnLastPosition)
{
   buffer_write_read_test(fixed_buffer::factory(12), 8);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndread_as)
{
   buffer_write_as_read_as_test(fixed_buffer::factory(12), 4);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_write_test(fixed_buffer::factory(12), 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_read_test(fixed_buffer::factory(12), 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldCopyOnSameOffsets)
{
   buffer_copy_test(fixed_buffer::factory(12), fixed_buffer::factory(12),
                  0, 0, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldCopyOnDifferentOffsets)
{
   buffer_copy_test(fixed_buffer::factory(12), fixed_buffer::factory(24),
                  0, 0, 12, 12);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(fixed_buffer::factory(12), fixed_buffer::factory(12),
                        0, 16, 0, 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(fixed_buffer::factory(12), fixed_buffer::factory(12),
                        0, 0, 16, 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongLength)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(fixed_buffer::factory(12), fixed_buffer::factory(12),
                        0, 0, 0, 24),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_SUITE_END();




BOOST_AUTO_TEST_SUITE(shifted_bufferTestSuite);

BOOST_AUTO_TEST_CASE(ShouldCreate)
{
   ptr<buffer> inner(fixed_buffer::factory(12).create_buffer());
   shifted_buffer outer(inner, 1024);
   BOOST_CHECK_EQUAL(12, outer.capacity());
}

BOOST_AUTO_TEST_CASE(ShouldCreateFromFactory)
{
   ptr<buffer::factory> fixed_buff_fact(new fixed_buffer::factory(12));
   ptr<buffer::factory> shifted_buff_fact(
            new shifted_buffer::factory(fixed_buff_fact, 1024));
   ptr<buffer> outer(shifted_buff_fact->create_buffer());
   BOOST_CHECK_EQUAL(12, outer->capacity());
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnFirstPosition)
{
   buffer_write_read_test(
         shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
         1024);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnLastPosition)
{
   buffer_write_read_test(
         shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
         1032);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadBeforeFirstPosition)
{
   BOOST_CHECK_THROW(
         buffer_read_test(
               shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
               512),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteBeforeFirstPosition)
{
   BOOST_CHECK_THROW(
         buffer_write_test(
               shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
               512),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_read_test(
               shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
               1036),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_write_test(
               shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
               1036),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldCopyFromshifted_buffer)
{
   buffer_copy_test(
         shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
         fixed_buffer::factory(12),
         1024, 1024, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldCopyToshifted_buffer)
{
   buffer_copy_test(
         fixed_buffer::factory(12),
         shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
         0, 0, 1024, 12);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromshifted_bufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
               fixed_buffer::factory(12),
               1024, 512, 0, 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromshifted_bufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
               fixed_buffer::factory(12),
               1024, 1024, 16, 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromshifted_bufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
               fixed_buffer::factory(12),
               1024, 1024, 0, 24),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToshifted_bufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               fixed_buffer::factory(12),
               shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
               0, 16, 1024, 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToshifted_bufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               fixed_buffer::factory(12),
               shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
               0, 0, 512, 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToshifted_bufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               fixed_buffer::factory(12),
               shifted_buffer::factory(new fixed_buffer::factory(12), 1024),
               0, 0, 1024, 24),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_SUITE_END();




BOOST_AUTO_TEST_SUITE(double_buffer_testSuite)

BOOST_AUTO_TEST_CASE(ShouldCreate)
{
   double_buffer buff(new fixed_buffer(12), new fixed_buffer(12));
   BOOST_CHECK_EQUAL(12, buff.capacity());
}

BOOST_AUTO_TEST_CASE(ShouldCreateFromFactory)
{
   double_buffer::factory fact(new fixed_buffer::factory(12));
   ptr<buffer> buff(fact.create_buffer());
   BOOST_CHECK_EQUAL(12, buff->capacity());
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnFirstPosition)
{
   buffer_write_read_test(
         double_buffer::factory(new fixed_buffer::factory(12)),
         0);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnLastPosition)
{
   buffer_write_read_test(
         double_buffer::factory(new fixed_buffer::factory(12)),
         8);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_read_test(
               double_buffer::factory(new fixed_buffer::factory(12)),
               12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_write_test(
               double_buffer::factory(new fixed_buffer::factory(12)),
               12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldCopyFromdouble_buffer)
{
   buffer_copy_test(
         double_buffer::factory(new fixed_buffer::factory(12)),
         fixed_buffer::factory(12),
         0, 0, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldCopyTodouble_buffer)
{
   buffer_copy_test(
         fixed_buffer::factory(12),
         double_buffer::factory(new fixed_buffer::factory(12)),
         0, 0, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromdouble_bufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               double_buffer::factory(new fixed_buffer::factory(12)),
               fixed_buffer::factory(12),
               0, 12, 0, 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromdouble_bufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               double_buffer::factory(new fixed_buffer::factory(12)),
               fixed_buffer::factory(12),
               0, 0, 12, 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromdouble_bufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               double_buffer::factory(new fixed_buffer::factory(12)),
               fixed_buffer::factory(12),
               0, 0, 0, 24),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyTodouble_bufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               fixed_buffer::factory(12),
               double_buffer::factory(new fixed_buffer::factory(12)),
               0, 12, 0, 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyTodouble_bufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               fixed_buffer::factory(12),
               double_buffer::factory(new fixed_buffer::factory(12)),
               0, 0, 12, 12),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyTodouble_bufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               fixed_buffer::factory(12),
               double_buffer::factory(new fixed_buffer::factory(12)),
               0, 0, 0, 24),
         buffer::out_of_bounds_error);
}

BOOST_AUTO_TEST_CASE(ShouldDetectModificationOnChange)
{
   DWORD data[] = { 600, 601 };
   double_buffer_test(data, 2, true);
}

BOOST_AUTO_TEST_CASE(ShouldNotDetectModificationOnSameValueTwice)
{
   DWORD data[] = { 600, 600 };
   double_buffer_test(data, 2, false);
}

BOOST_AUTO_TEST_CASE(ShouldDetectModificationOnSameFirstAndLastValue)
{
   DWORD data[] = { 600, 601, 602, 601, 600 };
   double_buffer_test(data, 5, true);
}

BOOST_AUTO_TEST_CASE(ShouldNotDetectModificationOnSameLastTwoValues)
{
   DWORD data[] = { 600, 601, 602, 601, 601 };
   double_buffer_test(data, 5, false);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(BufferStreams)

BOOST_AUTO_TEST_CASE(ShouldReadFrombuffer_input_stream)
{
   auto s = prepare_buffer_stream<buffer_input_stream>(16);
   BOOST_CHECK_EQUAL(0, s->read_as<DWORD>());
   BOOST_CHECK_EQUAL(1, s->read_as<DWORD>());
   BOOST_CHECK_EQUAL(2, s->read_as<DWORD>());
   BOOST_CHECK_EQUAL(3, s->read_as<DWORD>());
}

BOOST_AUTO_TEST_CASE(ShouldReadAndDetectNoMoreBytesAvailable)
{
   auto s = prepare_buffer_stream<buffer_input_stream>(2);
   BYTE buf[4];
   BOOST_CHECK_EQUAL(2, s->read(buf, 4));
   BOOST_CHECK_EQUAL(0, s->read(buf, 4));
}


BOOST_AUTO_TEST_CASE(ShouldFailOnread_asWhenNoEnoughBytes)
{
   auto s = prepare_buffer_stream<buffer_input_stream>(2);
   BOOST_CHECK_THROW(s->read_as<DWORD>(), input_stream::eof_error);
}

BOOST_AUTO_TEST_CASE(ShouldWriteTobuffer_output_stream)
{
   auto s = prepare_buffer_stream<buffer_output_stream>(16);
   s->write_as<DWORD>(100);
   s->write_as<DWORD>(101);
   s->write_as<DWORD>(102);
   s->write_as<DWORD>(103);
   auto b = s->get_buffer();
   BOOST_CHECK_EQUAL(100, b->read_as<DWORD>(0));
   BOOST_CHECK_EQUAL(101, b->read_as<DWORD>(4));
   BOOST_CHECK_EQUAL(102, b->read_as<DWORD>(8));
   BOOST_CHECK_EQUAL(103, b->read_as<DWORD>(12));
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteWhenNoEnoughCapacity)
{
   auto s = prepare_buffer_stream<buffer_output_stream>(2);
   BOOST_CHECK_THROW(s->write_as<DWORD>(100),
                     buffer_output_stream::capacity_exhausted_error);
}

BOOST_AUTO_TEST_SUITE_END()
