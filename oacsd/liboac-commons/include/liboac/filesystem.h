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

#ifndef OAC_FILESYSTEM_H
#define OAC_FILESYSTEM_H

#include <cstdio>

#include <boost/filesystem.hpp>

#include "stream.h"

namespace oac {

/**
 * A input stream which read bytes from a file. It conforms InputStream
 * concept.
 */
class file_input_stream
{
public:

   typedef std::string mode;

   static mode OPEN_READ;

   static ptr<file_input_stream> STDIN;

   OAC_DECL_ERROR(open_error, io_error);

   inline file_input_stream(FILE* fd) : _fd(fd) {}

   file_input_stream(const boost::filesystem::path& path, const mode& mode);

   file_input_stream(file_input_stream&& s);

   ~file_input_stream();

   std::size_t read(void* dest,
                    std::size_t count) throw (stream::read_error);

private:

   FILE* _fd;
};

/**
 * An output stream which writes bytes to a file. It conforms OutputStream
 * concept.
 */
class file_output_stream
{
public:

   typedef std::string mode;

   OAC_DECL_ERROR(open_error, io_error);

   static mode OPEN_APPEND;
   static mode OPEN_WRITE;

   static ptr<file_output_stream> STDOUT;
   static ptr<file_output_stream> STDERR;

   inline file_output_stream(FILE* fd) : _fd(fd) {}

   file_output_stream(const boost::filesystem::path& path, const mode& mode);

   file_output_stream(file_output_stream&& s);

   ~file_output_stream();

   std::size_t write(
         const void* buffer,
         std::size_t count) throw (stream::write_error);

   void flush();

private:

   FILE* _fd;
};

class file {
public:

   OAC_DECL_ERROR(open_error, invalid_input_error);
   OAC_DECL_ERROR(not_found_error, open_error);

   static file makeTemp();

   inline file(const boost::filesystem::path& path) : _path(path) {}

   bool exists() const;

   bool is_regular_file() const;

   bool is_directory() const;

   ptr<file_input_stream> read() const;

   ptr<file_output_stream> append() const;

   ptr<file_output_stream> write() const;

private:

   boost::filesystem::path _path;
};

} // namespace oac

#endif
