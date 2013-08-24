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

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <liboac/buffer.h>

#include "fsuipc.h"

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_SUITE(VariableIdToFsuipcOffsetConversion)

BOOST_AUTO_TEST_CASE(MustConvertVarNameWithHexName)
{
   auto offset = to_fsuipc_offset(
            make_var_id("fsuipc/offset", "0x200"));
   BOOST_CHECK_EQUAL(0x200, offset.address);
   BOOST_CHECK_EQUAL(1, offset.length);
}

BOOST_AUTO_TEST_CASE(MustConvertVarNameWithHexAddressWithDecimalSize)
{
   auto offset = to_fsuipc_offset(
            make_var_id("fsuipc/offset", "0x200:2"));
   BOOST_CHECK_EQUAL(0x200, offset.address);
   BOOST_CHECK_EQUAL(2, offset.length);
}

BOOST_AUTO_TEST_CASE(MustConvertVarNameWithDecimalAddress)
{
   auto offset = to_fsuipc_offset(
            make_var_id("fsuipc/offset", "512"));
   BOOST_CHECK_EQUAL(512, offset.address);
   BOOST_CHECK_EQUAL(1, offset.length);
}

BOOST_AUTO_TEST_CASE(MustConvertVarNameWithDecimalAddressWithDecimalSize)
{
   auto offset = to_fsuipc_offset(
            make_var_id("fsuipc/offset", "512:2"));
   BOOST_CHECK_EQUAL(512, offset.address);
   BOOST_CHECK_EQUAL(2, offset.length);
}

BOOST_AUTO_TEST_CASE(MustConvertVarNameWithWithHexSize)
{
   auto offset = to_fsuipc_offset(
            make_var_id("fsuipc/offset", "0x200:0x2"));
   BOOST_CHECK_EQUAL(0x200, offset.address);
   BOOST_CHECK_EQUAL(2, offset.length);
}

BOOST_AUTO_TEST_CASE(MustConvertVarNameWithTextualByteSize)
{
   auto offset1 = to_fsuipc_offset(
            make_var_id("fsuipc/offset", "0x200:byte"));
   auto offset2 = to_fsuipc_offset(
            make_var_id("fsuipc/offset", "0x200:BYTE"));

   BOOST_CHECK_EQUAL(0x200, offset1.address);
   BOOST_CHECK_EQUAL(1, offset1.length);
   BOOST_CHECK_EQUAL(0x200, offset2.address);
   BOOST_CHECK_EQUAL(1, offset2.length);
}

BOOST_AUTO_TEST_CASE(MustConvertVarNameWithTextualWordSize)
{
   auto offset1 = to_fsuipc_offset(
            make_var_id("fsuipc/offset", "0x200:word"));
   auto offset2 = to_fsuipc_offset(
            make_var_id("fsuipc/offset", "0x200:WORD"));

   BOOST_CHECK_EQUAL(0x200, offset1.address);
   BOOST_CHECK_EQUAL(2, offset1.length);
   BOOST_CHECK_EQUAL(0x200, offset2.address);
   BOOST_CHECK_EQUAL(2, offset2.length);
}

BOOST_AUTO_TEST_CASE(MustConvertVarNameWithTextualDoubleWordSize)
{
   auto offset1 = to_fsuipc_offset(
            make_var_id("fsuipc/offset", "0x200:dword"));
   auto offset2 = to_fsuipc_offset(
            make_var_id("fsuipc/offset", "0x200:DWORD"));

   BOOST_CHECK_EQUAL(0x200, offset1.address);
   BOOST_CHECK_EQUAL(4, offset1.length);
   BOOST_CHECK_EQUAL(0x200, offset2.address);
   BOOST_CHECK_EQUAL(4, offset2.length);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenConvertingFromNoFsuipcVarGroup)
{
   BOOST_CHECK_THROW(
            to_fsuipc_offset(make_var_id("simconnect/var", "0x200:2")),
            fsuipc::invalid_var_group_error);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenConvertingVarNameWithTextualNameWithHexPrefix)
{
   BOOST_CHECK_THROW(
            to_fsuipc_offset(make_var_id("fsuipc/offset", "foobar")),
            fsuipc::var_name_syntax_error);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenConvertingVarNameWithTextualNameNonHexPrefix)
{
   BOOST_CHECK_THROW(
            to_fsuipc_offset(make_var_id("fsuipc/offset", "zulu")),
            fsuipc::var_name_syntax_error);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenConvertingVarNameWithHexAddressWithWrongSizeNumber)
{
   BOOST_CHECK_THROW(
            to_fsuipc_offset(make_var_id("fsuipc/offset", "0x200:3")),
            fsuipc::var_name_syntax_error);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenConvertingVarNameWithHexAddressWithWrongTextualSize)
{
   BOOST_CHECK_THROW(
            to_fsuipc_offset(make_var_id("fsuipc/offset", "0x200:foobar")),
            fsuipc::var_name_syntax_error);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(FsuipcValuedOffsetToVariableValueConversion)

BOOST_AUTO_TEST_CASE(MustConvertByteOffset)
{
   auto var_value = to_variable_value(
            fsuipc_valued_offset(
               fsuipc_offset(0x700, OFFSET_LEN_BYTE),
               0x0a));
   BOOST_CHECK_EQUAL(VAR_BYTE, var_value.get_type());
   BOOST_CHECK_EQUAL(0x0a, var_value.as_byte());
}

BOOST_AUTO_TEST_CASE(MustConvertWordOffset)
{
   auto var_value = to_variable_value(
            fsuipc_valued_offset(
               fsuipc_offset(0x700, OFFSET_LEN_WORD),
               0x0a0b));
   BOOST_CHECK_EQUAL(VAR_WORD, var_value.get_type());
   BOOST_CHECK_EQUAL(0x0a0b, var_value.as_word());
}

BOOST_AUTO_TEST_CASE(MustConvertDoubleWordOffset)
{
   auto var_value = to_variable_value(
            fsuipc_valued_offset(
               fsuipc_offset(0x700, OFFSET_LEN_DWORD),
               0x0a0b0c0d));
   BOOST_CHECK_EQUAL(VAR_DWORD, var_value.get_type());
   BOOST_CHECK_EQUAL(0x0a0b0c0d, var_value.as_dword());
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(VariableValueToFsuipcValuedOffsetConversion)

BOOST_AUTO_TEST_CASE(MustConvertBoolVariable)
{
   auto valued_offset1 = to_fsuipc_offset_value(
            variable_value::from_bool(true));
   auto valued_offset2 = to_fsuipc_offset_value(
            variable_value::from_bool(false));
   BOOST_CHECK_EQUAL(1, valued_offset1);
   BOOST_CHECK_EQUAL(0, valued_offset2);
}

BOOST_AUTO_TEST_CASE(MustConvertByteVariable)
{
   auto valued_offset = to_fsuipc_offset_value(
            variable_value::from_byte(0x0a));
   BOOST_CHECK_EQUAL(0x0a, valued_offset);
}

BOOST_AUTO_TEST_CASE(MustConvertWordVariable)
{
   auto valued_offset = to_fsuipc_offset_value(
            variable_value::from_word(0x0a0b));
   BOOST_CHECK_EQUAL(0x0a0b, valued_offset);
}

BOOST_AUTO_TEST_CASE(MustConvertDoubleWordVariable)
{
   auto valued_offset = to_fsuipc_offset_value(
            variable_value::from_dword(0x0a0b0c0d));
   BOOST_CHECK_EQUAL(0x0a0b0c0d, valued_offset);
}

BOOST_AUTO_TEST_CASE(MustConvertFloatVariable)
{
   auto valued_offset = to_fsuipc_offset_value(
            variable_value::from_float(3.1415f));
   BOOST_CHECK_EQUAL(3, valued_offset);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(FsuipcOffsetDb)

auto example_var_1 = make_var_id("fsuipc/offset", "0x200:2");
auto last_var_updated = make_var_id("fsuipc/offset", "no name");

auto example_offset_1 = to_fsuipc_offset(example_var_1);

void update_handler(
      const variable_id& id,
      const variable_value&)
{
   last_var_updated = id;
}

BOOST_AUTO_TEST_CASE(MustInitializeEmpty)
{
   fsuipc_offset_db db;

   BOOST_CHECK(db.get_all_offsets().empty());
}

BOOST_AUTO_TEST_CASE(MustCreateSubscriptionWhenEmpty)
{
   fsuipc_offset_db db;
   auto subs = db.create_subscription(example_var_1, update_handler);

   BOOST_CHECK_EQUAL(
            1,
            db.get_all_offsets().size());
   BOOST_CHECK(example_offset_1 == db.get_all_offsets().front());
   BOOST_CHECK_EQUAL(
            1,
            db.get_subscriptions_for_offset(example_offset_1).size());
   BOOST_CHECK(
            subs == db.get_subscriptions_for_offset(example_offset_1).front());
   BOOST_CHECK(
            db.get_offset_for_subscription(subs.get_subscription_id()) ==
            example_offset_1);
}

BOOST_AUTO_TEST_CASE(MustCreateSubscriptionWhenNotEmpty)
{
   fsuipc_offset_db db;
   auto subs1 = db.create_subscription(example_var_1, update_handler);
   auto subs2 = db.create_subscription(example_var_1, update_handler);

   BOOST_CHECK_EQUAL(
            1,
            db.get_all_offsets().size());
   BOOST_CHECK(example_offset_1 == db.get_all_offsets().front());
   BOOST_CHECK_EQUAL(
            2,
            db.get_subscriptions_for_offset(example_offset_1).size());
   BOOST_CHECK(
            db.get_offset_for_subscription(subs1.get_subscription_id()) ==
            example_offset_1);
   BOOST_CHECK(
            db.get_offset_for_subscription(subs2.get_subscription_id()) ==
            example_offset_1);
}

BOOST_AUTO_TEST_CASE(MustRemoveSubscriptionWhenOffsetOnlyHaveOne)
{
   fsuipc_offset_db db;
   auto subs = db.create_subscription(example_var_1, update_handler);
   db.remove_subscription(subs.get_subscription_id());

   BOOST_CHECK(db.get_all_offsets().empty());
   BOOST_CHECK_THROW(
            db.get_subscriptions_for_offset(example_offset_1),
            fsuipc_offset_db::no_such_offset_error);
   BOOST_CHECK_THROW(
            db.get_offset_for_subscription(subs.get_subscription_id()),
            fsuipc_offset_db::no_such_subscription_error);
}

BOOST_AUTO_TEST_CASE(MustRemoveSubscriptionWhenOffsetHaveSeveral)
{
   fsuipc_offset_db db;
   auto subs1 = db.create_subscription(example_var_1, update_handler);
   auto subs2 = db.create_subscription(example_var_1, update_handler);
   db.remove_subscription(subs1.get_subscription_id());

   BOOST_CHECK_EQUAL(
            1,
            db.get_all_offsets().size());
   BOOST_CHECK(example_offset_1 == db.get_all_offsets().front());
   BOOST_CHECK_EQUAL(
            1,
            db.get_subscriptions_for_offset(example_offset_1).size());
   BOOST_CHECK(
            subs2 == db.get_subscriptions_for_offset(example_offset_1).front());
   BOOST_CHECK_THROW(
            db.get_offset_for_subscription(subs1.get_subscription_id()),
            fsuipc_offset_db::no_such_subscription_error);
   BOOST_CHECK(
            db.get_offset_for_subscription(subs2.get_subscription_id()) ==
            example_offset_1);
}

BOOST_AUTO_TEST_SUITE_END()
