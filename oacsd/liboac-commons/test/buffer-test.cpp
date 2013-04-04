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

void FillBuffer(Buffer& buff, DWORD from_offset)
{
   for (DWORD i = 0; i < buff.capacity(); i += sizeof(DWORD))
   {
      buff.writeAs<DWORD>(from_offset + i, i / sizeof(DWORD));
   }
}

void BufferWriteTest(
      const Buffer::Factory& fact, DWORD offset)
{
   Ptr<Buffer> buff(fact.createBuffer());
   DWORD d = 600;
   buff->write(&d, offset, sizeof(DWORD));
}

void BufferReadTest(
      const Buffer::Factory& fact, DWORD offset)
{
   Ptr<Buffer> buff(fact.createBuffer());
   DWORD d = 600;
   buff->read(&d, offset, sizeof(DWORD));
}

void BufferWriteReadTest(
      const Buffer::Factory& fact, DWORD offset)
{
   Ptr<Buffer> buff(fact.createBuffer());
   DWORD d1 = 600, d2;
   buff->write(&d1, offset, sizeof(DWORD));
   buff->read(&d2, offset, sizeof(DWORD));
   BOOST_CHECK_EQUAL(d1, d2);
}

void BufferWriteAsReadAsTest(
      const Buffer::Factory& fact, DWORD offset)
{
   Ptr<Buffer> buff(fact.createBuffer());
   buff->writeAs<DWORD>(offset, 600);
   BOOST_CHECK_EQUAL(600, buff->readAs<DWORD>(offset));
}

void BufferCopyTest(
      const Buffer::Factory& fact0,
      const Buffer::Factory& fact1,
      DWORD fill_from_offset,
      DWORD offset0,
      DWORD offset1,
      DWORD length)
throw (Buffer::OutOfBoundsError)
{
   Ptr<Buffer> buff0(fact0.createBuffer());
   Ptr<Buffer> buff1(fact1.createBuffer());

   FillBuffer(*buff0, fill_from_offset);
   buff1->copy(*buff0, offset0, offset1, length);
   for (DWORD i = 0; i < length; i += sizeof(DWORD))
      BOOST_CHECK_EQUAL(i / 4, buff1->readAs<DWORD>(offset1 + i));
}

void DoubleBufferTest(DWORD* seq, unsigned int seql, bool expect_mod)
{
   DoubleBuffer buff(new FixedBuffer(12), new FixedBuffer(12));
   for (unsigned int i = 0; i < seql; i++)
   {
      buff.writeAs<DWORD>(0, seq[i]);
      buff.swap();
   }
   BOOST_CHECK(expect_mod == buff.isModified<sizeof(DWORD)>(0));
}

BOOST_AUTO_TEST_SUITE(FixedBufferTestSuite);

BOOST_AUTO_TEST_CASE(ShouldCreate)
{
   FixedBuffer buff(12);
   BOOST_CHECK_EQUAL(12, buff.capacity());
}

BOOST_AUTO_TEST_CASE(ShouldCreateFromFactory)
{
   Ptr<Buffer::Factory> fact(new FixedBuffer::Factory(12));
   Ptr<Buffer> buff(fact->createBuffer());
   BOOST_CHECK_EQUAL(12, buff->capacity());
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnFirstPosition)
{
   BufferWriteReadTest(FixedBuffer::Factory(12), 0);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnMiddlePosition)
{
   BufferWriteReadTest(FixedBuffer::Factory(12), 4);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnLastPosition)
{
   BufferWriteReadTest(FixedBuffer::Factory(12), 8);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadAs)
{
   BufferWriteAsReadAsTest(FixedBuffer::Factory(12), 4);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteAfterLastPosition)
{
   BOOST_CHECK_THROW(
         BufferWriteTest(FixedBuffer::Factory(12), 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadAfterLastPosition)
{
   BOOST_CHECK_THROW(
         BufferReadTest(FixedBuffer::Factory(12), 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldCopyOnSameOffsets)
{
   BufferCopyTest(FixedBuffer::Factory(12), FixedBuffer::Factory(12),
                  0, 0, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldCopyOnDifferentOffsets)
{
   BufferCopyTest(FixedBuffer::Factory(12), FixedBuffer::Factory(24),
                  0, 0, 12, 12);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(FixedBuffer::Factory(12), FixedBuffer::Factory(12),
                        0, 16, 0, 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(FixedBuffer::Factory(12), FixedBuffer::Factory(12),
                        0, 0, 16, 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongLength)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(FixedBuffer::Factory(12), FixedBuffer::Factory(12),
                        0, 0, 0, 24),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_SUITE_END();




BOOST_AUTO_TEST_SUITE(ShiftedBufferTestSuite);

BOOST_AUTO_TEST_CASE(ShouldCreate)
{
   Ptr<Buffer> inner(FixedBuffer::Factory(12).createBuffer());
   ShiftedBuffer outer(inner, 1024);
   BOOST_CHECK_EQUAL(12, outer.capacity());
}

BOOST_AUTO_TEST_CASE(ShouldCreateFromFactory)
{
   Ptr<Buffer::Factory> fixed_buff_fact(new FixedBuffer::Factory(12));
   Ptr<Buffer::Factory> shifted_buff_fact(
            new ShiftedBuffer::Factory(fixed_buff_fact, 1024));
   Ptr<Buffer> outer(shifted_buff_fact->createBuffer());
   BOOST_CHECK_EQUAL(12, outer->capacity());
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnFirstPosition)
{
   BufferWriteReadTest(
         ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
         1024);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnLastPosition)
{
   BufferWriteReadTest(
         ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
         1032);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadBeforeFirstPosition)
{
   BOOST_CHECK_THROW(
         BufferReadTest(
               ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
               512),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteBeforeFirstPosition)
{
   BOOST_CHECK_THROW(
         BufferWriteTest(
               ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
               512),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadAfterLastPosition)
{
   BOOST_CHECK_THROW(
         BufferReadTest(
               ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
               1036),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteAfterLastPosition)
{
   BOOST_CHECK_THROW(
         BufferWriteTest(
               ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
               1036),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldCopyFromShiftedBuffer)
{
   BufferCopyTest(
         ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
         FixedBuffer::Factory(12),
         1024, 1024, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldCopyToShiftedBuffer)
{
   BufferCopyTest(
         FixedBuffer::Factory(12),
         ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
         0, 0, 1024, 12);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromShiftedBufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
               FixedBuffer::Factory(12),
               1024, 512, 0, 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromShiftedBufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
               FixedBuffer::Factory(12),
               1024, 1024, 16, 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromShiftedBufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
               FixedBuffer::Factory(12),
               1024, 1024, 0, 24),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToShiftedBufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               FixedBuffer::Factory(12),
               ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
               0, 16, 1024, 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToShiftedBufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               FixedBuffer::Factory(12),
               ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
               0, 0, 512, 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToShiftedBufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               FixedBuffer::Factory(12),
               ShiftedBuffer::Factory(new FixedBuffer::Factory(12), 1024),
               0, 0, 1024, 24),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_SUITE_END();




BOOST_AUTO_TEST_SUITE(DoubleBufferTestSuite)

BOOST_AUTO_TEST_CASE(ShouldCreate)
{
   DoubleBuffer buff(new FixedBuffer(12), new FixedBuffer(12));
   BOOST_CHECK_EQUAL(12, buff.capacity());
}

BOOST_AUTO_TEST_CASE(ShouldCreateFromFactory)
{
   DoubleBuffer::Factory fact(new FixedBuffer::Factory(12));
   Ptr<Buffer> buff(fact.createBuffer());
   BOOST_CHECK_EQUAL(12, buff->capacity());
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnFirstPosition)
{
   BufferWriteReadTest(
         DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
         0);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnLastPosition)
{
   BufferWriteReadTest(
         DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
         8);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadAfterLastPosition)
{
   BOOST_CHECK_THROW(
         BufferReadTest(
               DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
               12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteAfterLastPosition)
{
   BOOST_CHECK_THROW(
         BufferWriteTest(
               DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
               12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldCopyFromDoubleBuffer)
{
   BufferCopyTest(
         DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
         FixedBuffer::Factory(12),
         0, 0, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldCopyToDoubleBuffer)
{
   BufferCopyTest(
         FixedBuffer::Factory(12),
         DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
         0, 0, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromDoubleBufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
               FixedBuffer::Factory(12),
               0, 12, 0, 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromDoubleBufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
               FixedBuffer::Factory(12),
               0, 0, 12, 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromDoubleBufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
               FixedBuffer::Factory(12),
               0, 0, 0, 24),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToDoubleBufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               FixedBuffer::Factory(12),
               DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
               0, 12, 0, 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToDoubleBufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               FixedBuffer::Factory(12),
               DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
               0, 0, 12, 12),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToDoubleBufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         BufferCopyTest(
               FixedBuffer::Factory(12),
               DoubleBuffer::Factory(new FixedBuffer::Factory(12)),
               0, 0, 0, 24),
         Buffer::OutOfBoundsError);
}

BOOST_AUTO_TEST_CASE(ShouldDetectModificationOnChange)
{
   DWORD data[] = { 600, 601 };
   DoubleBufferTest(data, 2, true);
}

BOOST_AUTO_TEST_CASE(ShouldNotDetectModificationOnSameValueTwice)
{
   DWORD data[] = { 600, 600 };
   DoubleBufferTest(data, 2, false);
}

BOOST_AUTO_TEST_CASE(ShouldDetectModificationOnSameFirstAndLastValue)
{
   DWORD data[] = { 600, 601, 602, 601, 600 };
   DoubleBufferTest(data, 5, true);
}

BOOST_AUTO_TEST_CASE(ShouldNotDetectModificationOnSameLastTwoValues)
{
   DWORD data[] = { 600, 601, 602, 601, 601 };
   DoubleBufferTest(data, 5, false);
}

BOOST_AUTO_TEST_SUITE_END();
