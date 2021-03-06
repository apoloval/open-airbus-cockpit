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

#include <string>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include <boost/thread.hpp>

#include <liboac/buffer.h>
#include <liboac/stream.h>

using namespace oac;
using namespace oac::buffer;

template <typename Buffer>
void fill_buffer(Buffer& buff, std::size_t from_offset)
{
   for (std::size_t i = 0; i < buff.capacity(); i += sizeof(std::size_t))
   {
      if (i + sizeof(std::size_t) <= buff.capacity())
         buffer::write_as<std::size_t>(
               buff,
               from_offset + i,
               i / sizeof(std::size_t));
   }
}

template <typename BufferFactory>
void buffer_write_test(
      const std::shared_ptr<BufferFactory>& fact,
      std::size_t offset)
{
   std::shared_ptr<typename BufferFactory::value_type> buff(
         fact->create_buffer());
   std::size_t d = 600;
   buff->write(&d, offset, sizeof(std::size_t));
}

template <typename BufferFactory>
void buffer_read_test(
      const std::shared_ptr<BufferFactory>& fact,
      std::size_t offset)
{
   std::shared_ptr<typename BufferFactory::value_type> buff(
         fact->create_buffer());
   std::size_t d = 600;
   buff->read(&d, offset, sizeof(std::size_t));
}

template <typename BufferFactory>
void buffer_write_read_test(
      const std::shared_ptr<BufferFactory>& fact,
      std::size_t offset)
{
   std::shared_ptr<typename BufferFactory::value_type> buff(
         fact->create_buffer());
   std::size_t d1 = 600, d2;
   buff->write(&d1, offset, sizeof(std::size_t));
   buff->read(&d2, offset, sizeof(std::size_t));
   BOOST_CHECK_EQUAL(d1, d2);
}

template <typename BufferFactory>
void buffer_write_as_read_as_test(
      const std::shared_ptr<BufferFactory>& fact,
      std::size_t offset)
{
   std::shared_ptr<typename BufferFactory::value_type> buff(
         fact->create_buffer());
   buffer::write_as<std::size_t>(*buff, offset, 600);
   BOOST_CHECK_EQUAL(600, buffer::read_as<std::size_t>(*buff, offset));
}

template <typename BufferFactory1, typename BufferFactory2>
void buffer_copy_test(
      const std::shared_ptr<BufferFactory1>& fact0,
      const std::shared_ptr<BufferFactory2>& fact1,
      std::size_t fill_from_offset,
      std::size_t offset0,
      std::size_t offset1,
      std::size_t length)
throw (buffer::index_out_of_bounds)
{
   std::shared_ptr<typename BufferFactory1::value_type> buff0(
         fact0->create_buffer());
   std::shared_ptr<typename BufferFactory2::value_type> buff1(
         fact1->create_buffer());

   fill_buffer(*buff0, fill_from_offset);
   buff1->copy(*buff0, offset0, offset1, length);
   for (std::size_t i = 0; i < length; i += sizeof(std::size_t))
      BOOST_CHECK_EQUAL(
            i / 4,
            buffer::read_as<std::size_t>(*buff1, offset1 + i));
}

void double_buffer_test(std::size_t* seq, unsigned int seql, bool expect_mod)
{
   double_buffer<> buff(
         std::make_shared<linear_buffer>(12),
         std::make_shared<linear_buffer>(12));
   for (unsigned int i = 0; i < seql; i++)
   {
      buffer::write_as<std::size_t>(buff, 0, seq[i]);
      buff.swap();
   }
   BOOST_CHECK(expect_mod == buff.is_modified<sizeof(std::size_t)>(0));
}

linear_buffer_ptr prepare_buffer(std::size_t length)
{
   auto b = std::make_shared<linear_buffer>(length);
   fill_buffer(*b, 0);
   return b;
}

BOOST_AUTO_TEST_SUITE(LinearBufferTestSuite);

BOOST_AUTO_TEST_CASE(ShouldCreate)
{
   linear_buffer buff(12);
   BOOST_CHECK_EQUAL(12, buff.capacity());
}

BOOST_AUTO_TEST_CASE(ShouldCreateFromFactory)
{
   auto fact = std::make_shared<linear_buffer::factory>(12);
   linear_buffer_ptr buff(fact->create_buffer());
   BOOST_CHECK_EQUAL(12, buff->capacity());
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnFirstPosition)
{
   buffer_write_read_test(std::make_shared<linear_buffer::factory>(12), 0);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnMiddlePosition)
{
   buffer_write_read_test(std::make_shared<linear_buffer::factory>(12), 4);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnLastPosition)
{
   buffer_write_read_test(std::make_shared<linear_buffer::factory>(12), 8);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndread_as)
{
   buffer_write_as_read_as_test(
         std::make_shared<linear_buffer::factory>(12), 4);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_write_test(std::make_shared<linear_buffer::factory>(12), 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_read_test(std::make_shared<linear_buffer::factory>(12), 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldCopyOnSameOffsets)
{
   buffer_copy_test(
         std::make_shared<linear_buffer::factory>(12),
         std::make_shared<linear_buffer::factory>(12),
         0, 0, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldCopyOnDifferentOffsets)
{
   buffer_copy_test(
         std::make_shared<linear_buffer::factory>(12),
         std::make_shared<linear_buffer::factory>(24),
         0, 0, 12, 12);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<linear_buffer::factory>(12),
               std::make_shared<linear_buffer::factory>(12),
               0, 16, 0, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<linear_buffer::factory>(12),
               std::make_shared<linear_buffer::factory>(12),
               0, 0, 16, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongLength)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<linear_buffer::factory>(12),
               std::make_shared<linear_buffer::factory>(12),
               0, 0, 0, 24),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadAsStream)
{
   linear_buffer buff(16);
   BOOST_CHECK_EQUAL(0, buff.available_for_read());
   BOOST_CHECK_EQUAL(16, buff.available_for_write());

   stream::write_as<std::uint32_t>(buff, 1000);
   BOOST_CHECK_EQUAL(4, buff.available_for_read());
   BOOST_CHECK_EQUAL(12, buff.available_for_write());

   stream::write_as<std::uint32_t>(buff, 1001);
   BOOST_CHECK_EQUAL(8, buff.available_for_read());
   BOOST_CHECK_EQUAL(8, buff.available_for_write());

   stream::write_as<std::uint32_t>(buff, 1002);
   BOOST_CHECK_EQUAL(12, buff.available_for_read());
   BOOST_CHECK_EQUAL(4, buff.available_for_write());

   stream::write_as<std::uint32_t>(buff, 1003);
   BOOST_CHECK_EQUAL(16, buff.available_for_read());
   BOOST_CHECK_EQUAL(0, buff.available_for_write());
}

BOOST_AUTO_TEST_CASE(ShouldOperateWithMark)
{
   linear_buffer buff(16);

   stream::write_as<std::uint32_t>(buff, 1000);
   stream::write_as<std::uint32_t>(buff, 1001);
   stream::write_as<std::uint32_t>(buff, 1002);
   stream::write_as<std::uint32_t>(buff, 1003);

   buff.set_mark();
   stream::read_as<std::uint32_t>(buff);
   stream::read_as<std::uint32_t>(buff);
   BOOST_CHECK_EQUAL(8, buff.available_for_read());
   BOOST_CHECK_EQUAL(0, buff.available_for_write());

   buff.reset();
   BOOST_CHECK_EQUAL(16, buff.available_for_read());
   BOOST_CHECK_EQUAL(0, buff.available_for_write());
   BOOST_CHECK_EQUAL(1000, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(1001, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(1002, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(1003, stream::read_as<std::uint32_t>(buff));

   BOOST_CHECK_EQUAL(0, buff.available_for_read());
   BOOST_CHECK_EQUAL(0, buff.available_for_write());

   buff.unset_mark();
   BOOST_CHECK_EQUAL(0, buff.available_for_read());
   BOOST_CHECK_EQUAL(0, buff.available_for_write());
}

BOOST_AUTO_TEST_SUITE_END();



BOOST_AUTO_TEST_SUITE(RingBufferTestSuite);

BOOST_AUTO_TEST_CASE(ShouldCreateBuffer)
{
   ring_buffer buff(12);
   BOOST_CHECK_EQUAL(12, buff.capacity());
}

BOOST_AUTO_TEST_CASE(ShouldCreateFromFactory)
{
   auto fact = std::make_shared<ring_buffer::factory>(12);
   ring_buffer_ptr buff(fact->create_buffer());
   BOOST_CHECK_EQUAL(12, buff->capacity());
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnFirstPosition)
{
   buffer_write_read_test(std::make_shared<ring_buffer::factory>(12), 0);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnMiddlePosition)
{
   buffer_write_read_test(std::make_shared<ring_buffer::factory>(12), 4);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnLastPosition)
{
   buffer_write_read_test(std::make_shared<ring_buffer::factory>(12), 8);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndread_as)
{
   buffer_write_as_read_as_test(std::make_shared<ring_buffer::factory>(12), 4);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_write_test(std::make_shared<ring_buffer::factory>(12), 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_read_test(std::make_shared<ring_buffer::factory>(12), 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldCopyOnSameOffsets)
{
   buffer_copy_test(
         std::make_shared<ring_buffer::factory>(12),
         std::make_shared<ring_buffer::factory>(12),
         0, 0, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldCopyOnDifferentOffsets)
{
   buffer_copy_test(
         std::make_shared<ring_buffer::factory>(12),
         std::make_shared<ring_buffer::factory>(24),
         0, 0, 12, 12);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<ring_buffer::factory>(12),
               std::make_shared<ring_buffer::factory>(12),
               0, 16, 0, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<ring_buffer::factory>(12),
               std::make_shared<ring_buffer::factory>(12),
               0, 0, 16, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyWithWrongLength)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<ring_buffer::factory>(12),
               std::make_shared<ring_buffer::factory>(12),
               0, 0, 0, 24),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadAsStream)
{
   ring_buffer buff(16);
   BOOST_CHECK_EQUAL(0, buff.available_for_read());
   BOOST_CHECK_EQUAL(16, buff.available_for_write());

   stream::write_as<std::uint32_t>(buff, 1000);
   BOOST_CHECK_EQUAL(4, buff.available_for_read());
   BOOST_CHECK_EQUAL(12, buff.available_for_write());

   stream::write_as<std::uint32_t>(buff, 1001);
   BOOST_CHECK_EQUAL(8, buff.available_for_read());
   BOOST_CHECK_EQUAL(8, buff.available_for_write());

   stream::write_as<std::uint32_t>(buff, 1002);
   BOOST_CHECK_EQUAL(12, buff.available_for_read());
   BOOST_CHECK_EQUAL(4, buff.available_for_write());

   stream::write_as<std::uint32_t>(buff, 1003);
   BOOST_CHECK_EQUAL(16, buff.available_for_read());
   BOOST_CHECK_EQUAL(0, buff.available_for_write());

   BOOST_CHECK_EQUAL(1000, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(12, buff.available_for_read());
   BOOST_CHECK_EQUAL(4, buff.available_for_write());

   BOOST_CHECK_EQUAL(1001, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(8, buff.available_for_read());
   BOOST_CHECK_EQUAL(8, buff.available_for_write());

   BOOST_CHECK_EQUAL(1002, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(4, buff.available_for_read());
   BOOST_CHECK_EQUAL(12, buff.available_for_write());

   BOOST_CHECK_EQUAL(1003, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(0, buff.available_for_read());
   BOOST_CHECK_EQUAL(16, buff.available_for_write());
}

BOOST_AUTO_TEST_CASE(ShouldReadAndWriteAfterBroken)
{
   ring_buffer buff(16);
   stream::write_as<std::uint32_t>(buff, 1000);
   stream::write_as<std::uint32_t>(buff, 1001);
   stream::write_as<std::uint32_t>(buff, 1002);
   stream::write_as<std::uint32_t>(buff, 1003);
   stream::read_as<std::uint32_t>(buff);
   stream::read_as<std::uint32_t>(buff);
   BOOST_CHECK_EQUAL(8, buff.available_for_read());
   BOOST_CHECK_EQUAL(8, buff.available_for_write());

   stream::write_as<std::uint32_t>(buff, 1004);
   BOOST_CHECK_EQUAL(12, buff.available_for_read());
   BOOST_CHECK_EQUAL(4, buff.available_for_write());

   stream::write_as<std::uint32_t>(buff, 1005);
   BOOST_CHECK_EQUAL(16, buff.available_for_read());
   BOOST_CHECK_EQUAL(0, buff.available_for_write());

   BOOST_CHECK_EQUAL(1002, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(12, buff.available_for_read());
   BOOST_CHECK_EQUAL(4, buff.available_for_write());

   BOOST_CHECK_EQUAL(1003, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(8, buff.available_for_read());
   BOOST_CHECK_EQUAL(8, buff.available_for_write());

   BOOST_CHECK_EQUAL(1004, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(4, buff.available_for_read());
   BOOST_CHECK_EQUAL(12, buff.available_for_write());

   BOOST_CHECK_EQUAL(1005, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(0, buff.available_for_read());
   BOOST_CHECK_EQUAL(16, buff.available_for_write());
}

BOOST_AUTO_TEST_CASE(ShouldReadAndWriteLongFlow)
{
   boost::condition_variable available;
   boost::mutex mutex;
   ring_buffer buff(1024);
   std::size_t top = 1024 * 1024; // 1MiB

   boost::thread writer([&]() {
      std::size_t nwrite = 0;
      std::uint32_t counter = 0;
      while (nwrite < top)
      {
         boost::unique_lock<boost::mutex> lock(mutex);
         while (!buff.available_for_write())
            available.wait(lock);
         stream::write_as<std::uint32_t>(buff, counter);
         available.notify_one();
         counter++;
         nwrite += sizeof(std::uint32_t);
      }
   });

   boost::thread reader([&]() {
      std::size_t nread = 0;
      std::uint32_t counter = 0;
      while (nread < top)
      {
         boost::unique_lock<boost::mutex> lock(mutex);
         while (!buff.available_for_read())
            available.wait(lock);
         BOOST_CHECK_EQUAL(counter, stream::read_as<std::uint32_t>(buff));
         available.notify_one();
         counter++;
         nread += sizeof(std::uint32_t);
      }
   });
   writer.join();
   reader.join();
}

BOOST_AUTO_TEST_CASE(ShouldOperateWithMark)
{
   ring_buffer buff(16);

   stream::write_as<std::uint32_t>(buff, 1000);
   stream::write_as<std::uint32_t>(buff, 1001);
   stream::write_as<std::uint32_t>(buff, 1002);
   stream::write_as<std::uint32_t>(buff, 1003);

   buff.set_mark();
   stream::read_as<std::uint32_t>(buff);
   stream::read_as<std::uint32_t>(buff);
   BOOST_CHECK_EQUAL(8, buff.available_for_read());
   BOOST_CHECK_EQUAL(0, buff.available_for_write());

   buff.reset();
   BOOST_CHECK_EQUAL(16, buff.available_for_read());
   BOOST_CHECK_EQUAL(0, buff.available_for_write());
   BOOST_CHECK_EQUAL(1000, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(1001, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(1002, stream::read_as<std::uint32_t>(buff));
   BOOST_CHECK_EQUAL(1003, stream::read_as<std::uint32_t>(buff));

   BOOST_CHECK_EQUAL(0, buff.available_for_read());
   BOOST_CHECK_EQUAL(0, buff.available_for_write());

   buff.unset_mark();
   BOOST_CHECK_EQUAL(0, buff.available_for_read());
   BOOST_CHECK_EQUAL(16, buff.available_for_write());
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(ShiftedBufferTestSuite)

BOOST_AUTO_TEST_CASE(ShouldCreate)
{
   auto inner = linear_buffer_ptr(linear_buffer::factory(12).create_buffer());
   shifted_buffer<linear_buffer> outer(inner, 1024);
   BOOST_CHECK_EQUAL(12, outer.capacity());
}

BOOST_AUTO_TEST_CASE(ShouldCreateFromFactory)
{
   auto fixed_buff_fact = std::make_shared<linear_buffer::factory>(12);
   auto shifted_buff_fact = buffer::shift_factory(fixed_buff_fact, 1024);
   auto outer = shifted_buff_fact->create_buffer(); // FIXME memory leak
   BOOST_CHECK_EQUAL(12, outer->capacity());
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnFirstPosition)
{
   buffer_write_read_test(
         buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
         1024);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnLastPosition)
{
   buffer_write_read_test(
         buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
         1032);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadBeforeFirstPosition)
{
   BOOST_CHECK_THROW(
         buffer_read_test(
               buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
               512),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteBeforeFirstPosition)
{
   BOOST_CHECK_THROW(
         buffer_write_test(
               buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
               512),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_read_test(
               buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
               1036),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_write_test(
               buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
               1036),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldCopyFromshifted_buffer)
{
   buffer_copy_test(
         buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
         std::make_shared<linear_buffer::factory>(12),
         1024, 1024, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldCopyToshifted_buffer)
{
   buffer_copy_test(
         std::make_shared<linear_buffer::factory>(12),
         buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
         0, 0, 1024, 12);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromshifted_bufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
               std::make_shared<linear_buffer::factory>(12),
               1024, 512, 0, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromshifted_bufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
               std::make_shared<linear_buffer::factory>(12),
               1024, 1024, 16, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromshifted_bufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
               std::make_shared<linear_buffer::factory>(12),
               1024, 1024, 0, 24),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToshifted_bufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<linear_buffer::factory>(12),
               buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
               0, 16, 1024, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToshifted_bufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<linear_buffer::factory>(12),
               buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
               0, 0, 512, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyToshifted_bufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<linear_buffer::factory>(12),
               buffer::shift_factory(std::make_shared<linear_buffer::factory>(12), 1024),
               0, 0, 1024, 24),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_SUITE_END()




BOOST_AUTO_TEST_SUITE(DoubleBufferTestSuite)

BOOST_AUTO_TEST_CASE(ShouldCreate)
{
   auto buff = buffer::dup_buffers<linear_buffer>(
            std::make_shared<linear_buffer>(12),
            std::make_shared<linear_buffer>(12));
   BOOST_CHECK_EQUAL(12, buff->capacity());
}

BOOST_AUTO_TEST_CASE(ShouldCreateFromFactory)
{
   auto fact = buffer::dup_factory(std::make_shared<linear_buffer::factory>(12));
   auto buff = fact->create_buffer();
   BOOST_CHECK_EQUAL(12, buff->capacity());
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnFirstPosition)
{
   buffer_write_read_test(
         buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
         0);
}

BOOST_AUTO_TEST_CASE(ShouldWriteAndReadOnLastPosition)
{
   buffer_write_read_test(
            buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
         8);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnReadAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_read_test(
               buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
               12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnWriteAfterLastPosition)
{
   BOOST_CHECK_THROW(
         buffer_write_test(
               buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
               12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldCopyFromdouble_buffer)
{
   buffer_copy_test(
         buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
         std::make_shared<linear_buffer::factory>(12),
         0, 0, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldCopyTodouble_buffer)
{
   buffer_copy_test(
         std::make_shared<linear_buffer::factory>(12),
         buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
         0, 0, 0, 12);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromdouble_bufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
               std::make_shared<linear_buffer::factory>(12),
               0, 12, 0, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromdouble_bufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
               std::make_shared<linear_buffer::factory>(12),
               0, 0, 12, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyFromdouble_bufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
               std::make_shared<linear_buffer::factory>(12),
               0, 0, 0, 24),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyTodouble_bufferWithWrongSourceOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<linear_buffer::factory>(12),
               buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
               0, 12, 0, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyTodouble_bufferWithWrongDestinationOffset)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<linear_buffer::factory>(12),
               buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
               0, 0, 12, 12),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldFailOnCopyTodouble_bufferWithWrongLength)
{
   BOOST_CHECK_THROW(
         buffer_copy_test(
               std::make_shared<linear_buffer::factory>(12),
               buffer::dup_factory(std::make_shared<linear_buffer::factory>(12)),
               0, 0, 0, 24),
         buffer::index_out_of_bounds);
}

BOOST_AUTO_TEST_CASE(ShouldDetectModificationOnChange)
{
   std::size_t data[] = { 600, 601 };
   double_buffer_test(data, 2, true);
}

BOOST_AUTO_TEST_CASE(ShouldNotDetectModificationOnSameValueTwice)
{
   std::size_t data[] = { 600, 600 };
   double_buffer_test(data, 2, false);
}

BOOST_AUTO_TEST_CASE(ShouldDetectModificationOnSameFirstAndLastValue)
{
   std::size_t data[] = { 600, 601, 602, 601, 600 };
   double_buffer_test(data, 5, true);
}

BOOST_AUTO_TEST_CASE(ShouldNotDetectModificationOnSameLastTwoValues)
{
   std::size_t data[] = { 600, 601, 602, 601, 601 };
   double_buffer_test(data, 5, false);
}

BOOST_AUTO_TEST_SUITE_END()
