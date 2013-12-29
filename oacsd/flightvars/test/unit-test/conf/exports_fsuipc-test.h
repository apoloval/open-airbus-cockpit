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
 * You Must have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/test/auto_unit_test.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <liboac/fsuipc/offset.h>

#include "conf/exports_fsuipc.h"

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_SUITE(ExportsFsuipcTest)

struct let_test
{
   let_test()
    : _file
   {
      format("C:\\Windows\\Temp\\exports_fsuipc-%d.json",
            std::abs(std::rand()))
   }
   {}

   ~let_test()
   {
      boost::filesystem::remove(_file);
   }

   let_test& file_with_hex_addresses(
         const oac::fsuipc::offset_address& addr1,
         const oac::fsuipc::offset_length& len1,
         const oac::fsuipc::offset_address& addr2,
         const oac::fsuipc::offset_length& len2)
   {
      auto text = format("["
            "{\"address\": \"0x%x\", \"length\": %d},"
            "{\"address\": \"0x%x\", \"length\": %d}"
            "]", addr1, len1, addr2, len2);
      boost::filesystem::ofstream os { _file };
      os << text << std::endl;
      os.close();
      return *this;
   }

   let_test& file_with_dec_addresses(
         const oac::fsuipc::offset_address& addr1,
         const oac::fsuipc::offset_length& len1,
         const oac::fsuipc::offset_address& addr2,
         const oac::fsuipc::offset_length& len2)
   {
      auto text = format("["
            "{\"address\": \"%d\", \"length\": %d},"
            "{\"address\": \"%d\", \"length\": %d}"
            "]", addr1, len1, addr2, len2);
      boost::filesystem::ofstream os { _file };
      os << text << std::endl;
      os.close();
      return *this;
   }

   let_test& must_load_exports(
         const oac::fsuipc::offset_address& addr1,
         const oac::fsuipc::offset_length& len1,
         const oac::fsuipc::offset_address& addr2,
         const oac::fsuipc::offset_length& len2)
   {
      conf::domain_settings setts
      {
         conf::domain_type::FSUIPC_OFFSETS,
         "fsuipc-offsets",
         "description",
         true,
         _file,
         boost::property_tree::ptree {}
      };
      std::list<oac::fsuipc::offset> offsets;
      conf::load_exports(setts, offsets);
      BOOST_CHECK_MESSAGE(
            matches(offsets, addr1, len1),
            format("missing offset 0x%x:%d", addr1, len1));
      BOOST_CHECK_MESSAGE(
            matches(offsets, addr2, len2),
            format("missing offset 0x%x:%d", addr2, len2));
      return *this;
   }

private:

   boost::filesystem::path _file;

   bool matches(
         const std::list<oac::fsuipc::offset>& offsets,
         const oac::fsuipc::offset_address& addr,
         const oac::fsuipc::offset_length& len)
   {
      return std::any_of(
            offsets.begin(),
            offsets.end(),
            [addr, len](const oac::fsuipc::offset& o)
            {
               return o.address == addr && o.length == len;
            });
   }
};

BOOST_AUTO_TEST_CASE(MustLoadOffsetsWithHexadecimalAddress)
{
   let_test()
         .file_with_hex_addresses(
               0x1000, oac::fsuipc::OFFSET_LEN_BYTE,
               0x2000, oac::fsuipc::OFFSET_LEN_WORD)
         .must_load_exports(
               0x1000, oac::fsuipc::OFFSET_LEN_BYTE,
               0x2000, oac::fsuipc::OFFSET_LEN_WORD);
}

BOOST_AUTO_TEST_CASE(MustLoadOffsetsWithDecimalAddress)
{
   let_test()
         .file_with_dec_addresses(
               0x1000, oac::fsuipc::OFFSET_LEN_BYTE,
               0x2000, oac::fsuipc::OFFSET_LEN_WORD)
         .must_load_exports(
               0x1000, oac::fsuipc::OFFSET_LEN_BYTE,
               0x2000, oac::fsuipc::OFFSET_LEN_WORD);
}

BOOST_AUTO_TEST_SUITE_END()
