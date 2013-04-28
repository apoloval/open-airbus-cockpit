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

class FileInputStream : public InputStream {
public:

   typedef std::string Mode;

   static Mode OPEN_READ;

   DECL_ERROR(OpenError, IOError);

   FileInputStream(const Path& path, const Mode& mode)
      : _fd(_fsopen(path.string().c_str(), mode.c_str(), _SH_DENYWR))
   {
      if (!_fd)
         THROW_ERROR(OpenError());
   }

   inline virtual ~FileInputStream()
   { fclose(_fd); }

   virtual DWORD read(void* buffer, DWORD count) throw (ReadError)
   { return fread(buffer, 1, count, _fd); }

private:

   FILE* _fd;
};

FileInputStream::Mode FileInputStream::OPEN_READ("r");

class FileOutputStream : public OutputStream {
public:

   typedef std::string Mode;

   DECL_ERROR(OpenError, IOError);

   static Mode OPEN_APPEND;
   static Mode OPEN_WRITE;


   FileOutputStream(const Path& path, const Mode& mode)
      : _fd(_fsopen(path.string().c_str(), mode.c_str(), _SH_DENYWR))
   {
      if (!_fd)
         THROW_ERROR(OpenError());
   }

   inline virtual ~FileOutputStream()
   { fclose(_fd); }

   virtual void write(const void* buffer, DWORD count)
   throw (WriteError)
   { fwrite(buffer, 1, count, _fd); }

private:

   FILE* _fd;
};

FileOutputStream::Mode FileOutputStream::OPEN_APPEND("a");
FileOutputStream::Mode FileOutputStream::OPEN_WRITE("w");


} // anonymous namespace

File
File::makeTemp()
{ return File(boost::filesystem::unique_path()); }

bool
File::exists() const
{ return boost::filesystem::exists(_path); }

bool
File::isRegularFile() const
{ return boost::filesystem::is_regular_file(_path); }

bool
File::isDirectory() const
{ return boost::filesystem::is_directory(_path); }

Ptr<InputStream>
File::read() const
{ return new FileInputStream(_path, FileInputStream::OPEN_READ); }

Ptr<OutputStream>
File::append() const
{ return new FileOutputStream(_path, FileOutputStream::OPEN_APPEND); }

Ptr<OutputStream>
File::write() const
{ return new FileOutputStream(_path, FileOutputStream::OPEN_WRITE); }

} // namespace oac
