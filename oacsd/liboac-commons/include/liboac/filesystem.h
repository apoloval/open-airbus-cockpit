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

#ifndef OAC_FILESYSTEM_H
#define OAC_FILESYSTEM_H

#include <boost/filesystem.hpp>

#include "stream.h"

namespace oac {

typedef boost::filesystem::path Path;

class file {
public:

   OAC_DECL_ERROR(open_error, invalid_input_error);
   OAC_DECL_ERROR(not_found_error, open_error);

   static file makeTemp();

   inline file(const Path& path) : _path(path) {}

   bool exists() const;

   bool is_regular_file() const;

   bool is_directory() const;

   ptr<input_stream> read() const;

   ptr<output_stream> append() const;

   ptr<output_stream> write() const;

private:

   Path _path;
};

} // namespace oac

#endif
