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

#ifndef OAC_STREAM_ADAPTERS_H
#define OAC_STREAM_ADAPTERS_H

#include <liboac/exception.h>
#include <liboac/io.h>

namespace oac { namespace stream {

/**
 * An input stream which adapts a Boost ASIO SyncReadStream object to
 * conform InputStream concept.
 */
template <typename SyncReadStream>
class sync_read_stream_adapter
{
public:

   typedef SyncReadStream adapted_type;
   typedef std::shared_ptr<adapted_type> adapted_ptr;

   inline sync_read_stream_adapter(const adapted_ptr& adapted)
      : _adapted(adapted)
   {}

   inline size_t read(void* dest, size_t count)
   {
      try
      { return _adapted->read_some(boost::asio::buffer(dest, count)); }
      catch (boost::system::system_error& e)
      {
         auto& ec = e.code();
         if (ec == boost::asio::error::eof)
            return 0;
         else
            OAC_THROW_EXCEPTION(io::boost_asio_error(ec));
      }
   }

private:

  adapted_ptr _adapted;
};

/**
 * An output stream which adapts a Boost ASIO SyncWriteStream object to
 * conform OutputStream concept.
 */
template <typename SyncWriteStream>
class sync_write_stream_adapter
{
public:

   typedef SyncWriteStream adapted_type;
   typedef std::shared_ptr<adapted_type> adapted_ptr;

   inline sync_write_stream_adapter(const adapted_ptr& adapted)
      : _adapted(adapted)
   {}

   inline std::size_t write(const void* src, size_t count)
   {
      try { return _adapted->write_some(boost::asio::buffer(src, count)); }
      catch (boost::system::system_error& e)
      {
         auto& ec = e.code();
         if (e.code() == boost::asio::error::eof)
            return 0;
         else
            OAC_THROW_EXCEPTION(io::boost_asio_error(ec));
      }
   }

   inline void flush()
   { /* Boost doesn't require flushing the bytes. */ }

private:

  adapted_ptr _adapted;
};

/**
 * Create a pointer to a new adapter compliant with InputStream concept from a
 * Boost SyncReadStream object.
 */
template <typename SyncReadStream>
std::shared_ptr<sync_write_stream_adapter<SyncReadStream>>
make_input_adapter(
      const std::shared_ptr<SyncReadStream>& s)
{ return new sync_read_stream_adapter<SyncReadStream>(s); }

/**
 * Create a pointer to a new adapter compliant with OutputStream concept from a
 * Boost SyncWriteStream object.
 */
template <typename SyncWriteStream>
std::shared_ptr<sync_write_stream_adapter<SyncWriteStream>>
make_output_adapter(
  const std::shared_ptr<SyncWriteStream>& s)
{ return new sync_write_stream_adapter<SyncWriteStream>(s); }

}} // namespace oac::stream

#endif
