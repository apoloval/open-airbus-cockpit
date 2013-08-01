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

BOOST_AUTO_TEST_SUITE(FsuipcOffsetMeta)

BOOST_AUTO_TEST_CASE(MustCreateFromHexAddress)
{
   fsuipc_offset_meta meta(make_var_id("fsuipc/offset", "0x200"));
   BOOST_CHECK_EQUAL(0x200, meta.address());
   BOOST_CHECK_EQUAL(1, meta.length());
}

BOOST_AUTO_TEST_CASE(MustCreateFromHexAddressWithDecimalSize)
{
   fsuipc_offset_meta meta(make_var_id("fsuipc/offset", "0x200:2"));
   BOOST_CHECK_EQUAL(0x200, meta.address());
   BOOST_CHECK_EQUAL(2, meta.length());
}

BOOST_AUTO_TEST_CASE(MustCreateFromDecimalAddressWithSize)
{
   fsuipc_offset_meta meta(make_var_id("fsuipc/offset", "512:2"));
   BOOST_CHECK_EQUAL(512, meta.address());
   BOOST_CHECK_EQUAL(2, meta.length());
}

BOOST_AUTO_TEST_CASE(MustCreateFromHexAddressWithHexSize)
{
   fsuipc_offset_meta meta(make_var_id("fsuipc/offset", "0x200:0x2"));
   BOOST_CHECK_EQUAL(0x200, meta.address());
   BOOST_CHECK_EQUAL(2, meta.length());
}

BOOST_AUTO_TEST_CASE(MustCreateFromHexAddressWithTextualByteSize)
{
   fsuipc_offset_meta meta1(make_var_id("fsuipc/offset", "0x200:byte"));
   fsuipc_offset_meta meta2(make_var_id("fsuipc/offset", "0x200:BYTE"));

   BOOST_CHECK_EQUAL(0x200, meta1.address());
   BOOST_CHECK_EQUAL(1, meta1.length());
   BOOST_CHECK_EQUAL(0x200, meta2.address());
   BOOST_CHECK_EQUAL(1, meta2.length());
}

BOOST_AUTO_TEST_CASE(MustCreateFromHexAddressWithTextualWordSize)
{
   fsuipc_offset_meta meta1(make_var_id("fsuipc/offset", "0x200:word"));
   fsuipc_offset_meta meta2(make_var_id("fsuipc/offset", "0x200:WORD"));

   BOOST_CHECK_EQUAL(0x200, meta1.address());
   BOOST_CHECK_EQUAL(2, meta1.length());
   BOOST_CHECK_EQUAL(0x200, meta2.address());
   BOOST_CHECK_EQUAL(2, meta2.length());
}

BOOST_AUTO_TEST_CASE(MustCreateFromHexAddressWithTextualDoubleWordSize)
{
   fsuipc_offset_meta meta1(make_var_id("fsuipc/offset", "0x200:dword"));
   fsuipc_offset_meta meta2(make_var_id("fsuipc/offset", "0x200:DWORD"));

   BOOST_CHECK_EQUAL(0x200, meta1.address());
   BOOST_CHECK_EQUAL(4, meta1.length());
   BOOST_CHECK_EQUAL(0x200, meta2.address());
   BOOST_CHECK_EQUAL(4, meta2.length());
}

BOOST_AUTO_TEST_CASE(MustThrowWhenCreatingFromNoFsuipcVarGroup)
{
   BOOST_CHECK_THROW(
            fsuipc_offset_meta(make_var_id("simconnect/var", "0x200:2")),
            fsuipc::invalid_var_group_error);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenCreatingFromHexBeginningNotANumberVarName)
{
   BOOST_CHECK_THROW(
            fsuipc_offset_meta(make_var_id("fsuipc/offset", "foobar")),
            fsuipc::var_name_syntax_error);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenCreatingFromNonHexBeginningNotANumberVarName)
{
   BOOST_CHECK_THROW(
            fsuipc_offset_meta(make_var_id("fsuipc/offset", "zulu")),
            fsuipc::var_name_syntax_error);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenCreatingFromHexAddressWithInvalidSizeNumber)
{
   BOOST_CHECK_THROW(
            fsuipc_offset_meta(make_var_id("fsuipc/offset", "0x200:3")),
            fsuipc::var_name_syntax_error);
}

BOOST_AUTO_TEST_CASE(MustThrowWhenCreatingFromHexAddressWithNotNumberSize)
{
   BOOST_CHECK_THROW(
            fsuipc_offset_meta(make_var_id("fsuipc/offset", "0x200:foobar")),
            fsuipc::var_name_syntax_error);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(FsuipcOffsetDb)

auto example_var_1 = make_var_id("fsuipc/offset", "0x200:2");
auto last_var_updated = make_var_id("fsuipc/offset", "no name");

auto example_offset_1 = fsuipc_offset_meta(example_var_1);

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
            fsuipc_offset_db::unknown_offset_error);
   BOOST_CHECK_THROW(
            db.get_offset_for_subscription(subs.get_subscription_id()),
            fsuipc_offset_db::unknown_subscription_error);
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
            fsuipc_offset_db::unknown_subscription_error);
   BOOST_CHECK(
            db.get_offset_for_subscription(subs2.get_subscription_id()) ==
            example_offset_1);
}

BOOST_AUTO_TEST_SUITE_END()
