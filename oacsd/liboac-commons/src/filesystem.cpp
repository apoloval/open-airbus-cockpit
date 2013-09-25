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

#include <liboac/filesystem.h>

namespace oac {

file_input_stream::mode file_input_stream::OPEN_READ("r");

file_input_stream_ptr file_input_stream::STDIN(new file_input_stream(stdin));

file_input_stream::file_input_stream(
      const boost::filesystem::path& path,
      const mode& mode)
throw (filesystem::open_error)
   : _fd(_fsopen(path.string().c_str(), mode.c_str(), _SH_DENYWR))
{
   if (!_fd)
      OAC_THROW_EXCEPTION(filesystem::open_error(path));
}

file_input_stream::file_input_stream(
      file_input_stream&& s)
   : _fd(s._fd)
{
   s._fd = nullptr;
}

file_input_stream::~file_input_stream()
{
   if (_fd)
      fclose(_fd);
}

std::size_t
file_input_stream::read(
      void* dest,
      std::size_t count)
throw (io_exception)
{ return fread(dest, 1, count, _fd); }



file_output_stream::mode file_output_stream::OPEN_APPEND("a");
file_output_stream::mode file_output_stream::OPEN_WRITE("w");

file_output_stream_ptr file_output_stream::STDOUT(
      new file_output_stream(stdout));
file_output_stream_ptr file_output_stream::STDERR(
      new file_output_stream(stderr));

file_output_stream::file_output_stream(
      const boost::filesystem::path& path,
      const mode& mode)
throw (filesystem::open_error)
   : _fd(_fsopen(path.string().c_str(), mode.c_str(), _SH_DENYWR))
{
   if (!_fd)
      OAC_THROW_EXCEPTION(filesystem::open_error(path));
}

file_output_stream::file_output_stream(file_output_stream&& s)
   : _fd(s._fd)
{
   s._fd = nullptr;
}

file_output_stream::~file_output_stream()
{
   if (_fd)
   {
      fflush(_fd);
      fclose(_fd);
   }
}

std::size_t
file_output_stream::write(const void* buffer, std::size_t count)
throw (io_exception)
{ return fwrite(buffer, 1, count, _fd); }

void
file_output_stream::flush()
{ fflush(_fd); }



file
file::makeTemp()
{ return file(boost::filesystem::unique_path()); }

bool
file::exists() const
{ return boost::filesystem::exists(_path); }

bool
file::is_regular_file() const
{ return boost::filesystem::is_regular_file(_path); }

bool
file::is_directory() const
{ return boost::filesystem::is_directory(_path); }

file_input_stream_ptr
file::read() const
throw (filesystem::open_error)
{
   return std::make_shared<file_input_stream>(
         _path,
         file_input_stream::OPEN_READ);
}

file_output_stream_ptr
file::append() const
throw (filesystem::open_error)
{
   return std::make_shared<file_output_stream>(
         _path,
         file_output_stream::OPEN_APPEND);
}

file_output_stream_ptr
file::write() const
throw (filesystem::open_error)
{
   return std::make_shared<file_output_stream>(
         _path,
         file_output_stream::OPEN_WRITE);
}

} // namespace oac
