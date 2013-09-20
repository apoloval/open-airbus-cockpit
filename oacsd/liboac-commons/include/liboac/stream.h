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

#ifndef OAC_STREAM_H
#define OAC_STREAM_H

#include <liboac/stream/adapters.h>
#include <liboac/stream/functions.h>

namespace oac { namespace stream {

/**
 * @concept InputStream
 *
 * A class which provides a member to read bytes from with the signature:
 *
 * std::size_t InputStream::read(
 *       void* dest, std::size_t count) throw (io_exception);
 *
 * The read() operation is synchronous, so the caller shall be blocked if
 * there is no available data in the stream. When data is available, the bytes
 * are copied into dest buffer. read() may not read all of the requested number
 * of bytes if all them are not available yet. For that, use stream::read_all()
 * function instead. When the stream is closed by the other end, a call to
 * read() shall read the remaining bytes which were not consumed, if any.
 * After that, any subsequent call to read() shall return 0.
 *
 */

/**
 * @concept OutputStream
 *
 * A class which provides a member to write bytes to, with the signature:
 *
 * std::size_t OutputStream::write(
 *       const void* src, std::size_t count) throw (io_exception);
 *
 * void OutputStream::flush();
 *
 * The write() operation is synchronous, so the caller shall be blocked if
 * it is not possible to write any byte yet. When data may be write, the bytes
 * are copied from src buffer. write() may not write all of the requested
 * number of bytes. For that, use stream::write_all() function instead. When
 * the stream is closed by the other end, a call to write() shall return 0.
 *
 * The flush() operation forces the sending of any pending byte that was not
 * sent to the device.
 */

}} // namespace oac::stream

#endif
