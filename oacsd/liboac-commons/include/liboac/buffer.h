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

#ifndef OAC_BUFFER_H
#define OAC_BUFFER_H

#include "buffer/double.h"
#include "buffer/errors.h"
#include "buffer/functions.h"
#include "buffer/linear.h"
#include "buffer/ring.h"
#include "buffer/shifted.h"

namespace oac {

/**
 * @concept Buffer
 *
 * A class that may be used as data storage, allowing reading from and
 * writing to bytes. Buffer concept extends the InputStream and OutputStream
 * concepts, so it implements their members plus:
 *
 * typedef std::shared_ptr<Buffer> ptr_type;
 *
 * typedef BufferFactory factory_type;
 *
 * typedef std::shared_ptr<factory_type> factory_ptr;
 *
 * std::size_t Buffer::capacity() const;
 *
 * void Buffer::read(void* dst, std::uint32_t offset, std::size_t length) const
 *       throw (out_of_bounds_error);
 *
 * template <typename OutputStream>
 * void Buffer::read_to(
 *    OutputStream& dst, std::uint32_t offset, std::size_t length) const
 *       throw (out_of_bounds_error, read_error, stream::write_error);
 *
 * void Buffer::write(const void* src, std::uint32_t offset, std::size_t length)
 *       throw (out_of_bounds_error, write_error);
 *
 * template <typename InputStream>
 * std::size_t Buffer::write_from(
 *       InputStream& src, std::uint32_t offset, std::size_t length)
 *       throw (out_of_bounds_error, stream::read_error, write_error);
 *
 * template <typename Buffer>
 * void copy(const Buffer& src, std::uint32_t src_offset,
 *           std::uint32_t dst_offset, std::size_t length)
 *    throw (out_of_bounds_error, read_error, write_error);
 */

/**
 * @concept BufferFactory
 *
 * A class that may be used to create buffer objects under a common interface.
 * It provides the following members, where T is the type of buffer objects
 * it creates.
 *
 * typedef T value_type;
 *
 * T* create_buffer();
 *
 */

/**
 * @concept StreamBuffer
 *
 * A class able to store data as Buffer but allows stream-like read and write
 * operations. It conforms the Buffer and Stream concepts, with the following
 * additional operations.
 *
 * std::size_t available_for_read() const;
 *
 * std::size_t available_for_write() const;
 *
 * boost::optional<std::uint32_t> mark() const;
 *
 * void set_mark();
 *
 * void unset_mark();
 *
 * void reset();
 *
 * template <typename AsyncReadStream,
 *           typename AsyncReadHandler>
 * void async_write_some_from(AsyncReadStream& stream, ReadHandler handler);
 *
 * template <typename AsyncWriteStream,
 *           typename AsyncWriteHandler>
 * void async_read_some_to(AsyncWriteStream& stream, WriteHandler handler);
 *
 */

/**
 * @concept AsyncReadHandler
 *
 * An object able to be invoked with an `attempt<std::size_t>` object
 * indicating the resulting bytes read.
 */

/**
 * @concept AsyncWriteHandler
 *
 * An object able to be invoked with an `attempt<std::size_t>` object
 * indicating the resulting bytes written.
 */

} // namespace oac

#endif
