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

#include <cstdio>

#include "filesystem.h"

namespace oac {

namespace {

class file_input_stream : public input_stream {
public:

   typedef std::string mode;

   static mode OPEN_READ;

   OAC_DECL_ERROR(open_error, io_error);

   file_input_stream(const Path& path, const mode& mode)
      : _fd(_fsopen(path.string().c_str(), mode.c_str(), _SH_DENYWR))
   {
      if (!_fd)
         BOOST_THROW_EXCEPTION(open_error());
   }

   inline virtual ~file_input_stream()
   { fclose(_fd); }

   virtual DWORD read(void* buffer, DWORD count) throw (read_error)
   { return fread(buffer, 1, count, _fd); }

private:

   FILE* _fd;
};

file_input_stream::mode file_input_stream::OPEN_READ("r");

class file_output_stream : public output_stream {
public:

   typedef std::string mode;

   OAC_DECL_ERROR(open_error, io_error);

   static mode OPEN_APPEND;
   static mode OPEN_WRITE;


   file_output_stream(const Path& path, const mode& mode)
      : _fd(_fsopen(path.string().c_str(), mode.c_str(), _SH_DENYWR))
   {
      if (!_fd)
         BOOST_THROW_EXCEPTION(open_error());
   }

   inline virtual ~file_output_stream()
   {
      fflush(_fd);
      fclose(_fd);
   }

   virtual void write(const void* buffer, DWORD count)
   throw (write_error)
   { fwrite(buffer, 1, count, _fd); }

   virtual void flush()
   { fflush(_fd); }

private:

   FILE* _fd;
};

file_output_stream::mode file_output_stream::OPEN_APPEND("a");
file_output_stream::mode file_output_stream::OPEN_WRITE("w");


} // anonymous namespace

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

ptr<input_stream>
file::read() const
{ return new file_input_stream(_path, file_input_stream::OPEN_READ); }

ptr<output_stream>
file::append() const
{ return new file_output_stream(_path, file_output_stream::OPEN_APPEND); }

ptr<output_stream>
file::write() const
{ return new file_output_stream(_path, file_output_stream::OPEN_WRITE); }

} // namespace oac
