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

#include <string>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include <boost/thread.hpp>

#include "buffer.h"
#include "fsuipc.h"

using namespace oac;

BOOST_AUTO_TEST_SUITE(FsuipcClient)

/**
 * DSL for testing. You might ignore it and check out the test cases instead.
 */
struct let_test
{
   std::uint8_t _buffer[0xffff];
   fsuipc_client<dummy_fsuipc_user_adapter> _cli;
   std::list<fsuipc_valued_offset> _update_inputs;
   std::list<fsuipc_offset> _query_inputs;
   std::list<fsuipc_valued_offset> _query_results;

   let_test& with_offset(
         fsuipc_offset_address addr,
         fsuipc_offset_length len,
         fsuipc_offset_value val)
   {
      _cli.user_adapter().write_value_to_buffer(addr, len, val);
      return *this;
   }

   let_test& with_update_input(
         fsuipc_offset_address addr,
         fsuipc_offset_length len,
         fsuipc_offset_value val)
   {
      _update_inputs.push_back(
               fsuipc_valued_offset(fsuipc_offset(addr, len), val));
      return *this;
   }

   let_test& update()
   {
      _cli.update(_update_inputs);
      return *this;
   }

   let_test& must_have_offset(
         fsuipc_offset_address addr,
         fsuipc_offset_length len,
         fsuipc_offset_value val)
   {
      BOOST_CHECK_EQUAL(
               val,
               _cli.user_adapter().read_value_from_buffer(addr, len));
      return *this;
   }

   let_test& with_query_input(
         fsuipc_offset_address addr,
         fsuipc_offset_length len)
   {
      _query_inputs.push_back(fsuipc_offset(addr, len));
      return *this;
   }

   let_test& query()
   {
      _cli.query(_query_inputs, [this](const fsuipc_valued_offset& value)
      {
         _query_results.push_back(value);
      });
      return *this;
   }

   let_test& results_count_is(std::size_t count)
   {
      BOOST_CHECK_EQUAL(count, _query_results.size());
      return *this;
   }

   let_test& must_return(
         fsuipc_offset_address addr,
         fsuipc_offset_length len,
         fsuipc_offset_value val)
   {
      BOOST_CHECK(
               std::any_of(
                  _query_results.begin(),
                  _query_results.end(),
                  [addr, len, val](const fsuipc_valued_offset& item)
      {
         return item.address == addr &&
               item.length == len &&
               item.value == val;
      }));
      return *this;
   }

   let_test& and_then()
   {
      _update_inputs.clear();
      _query_inputs.clear();
      _query_results.clear();
      return *this;
   }
};

BOOST_AUTO_TEST_CASE(MustQueryAfterCleanConstruction)
{
   let_test()
         .with_offset(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .with_offset(0x704, OFFSET_LEN_WORD, 0x0506)
         .with_query_input(0x700, OFFSET_LEN_DWORD)
         .with_query_input(0x704, OFFSET_LEN_WORD)
         .query()
         .results_count_is(2)
         .must_return(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .must_return(0x704, OFFSET_LEN_WORD, 0x0506);
}

BOOST_AUTO_TEST_CASE(MustQueryAfterOtherQuery)
{
   let_test()
         .with_query_input(0x700, OFFSET_LEN_DWORD)
         .with_query_input(0x704, OFFSET_LEN_WORD)
         .query()
         .and_then()
         .with_offset(0x706, OFFSET_LEN_BYTE, 0xe0)
         .with_offset(0x707, OFFSET_LEN_BYTE, 0xf0)
         .with_offset(0x708, OFFSET_LEN_DWORD, 0x81828384)
         .with_query_input(0x706, OFFSET_LEN_BYTE)
         .with_query_input(0x707, OFFSET_LEN_BYTE)
         .with_query_input(0x708, OFFSET_LEN_DWORD)
         .query()
         .results_count_is(3)
         .must_return(0x706, OFFSET_LEN_BYTE, 0xe0)
         .must_return(0x707, OFFSET_LEN_BYTE, 0xf0)
         .must_return(0x708, OFFSET_LEN_DWORD, 0x81828384);
}

BOOST_AUTO_TEST_CASE(MustUpdateAfterCleanConstruction)
{
   let_test()
         .with_update_input(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .with_update_input(0x704, OFFSET_LEN_WORD, 0x0506)
         .update()
         .must_have_offset(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .must_have_offset(0x704, OFFSET_LEN_WORD, 0x0506);
}

BOOST_AUTO_TEST_CASE(MustQueryAfterUpdate)
{
   let_test()
         .with_update_input(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .with_update_input(0x704, OFFSET_LEN_WORD, 0x0506)
         .update()
         .and_then()
         .with_query_input(0x700, OFFSET_LEN_DWORD)
         .with_query_input(0x704, OFFSET_LEN_WORD)
         .query()
         .results_count_is(2)
         .must_return(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .must_return(0x704, OFFSET_LEN_WORD, 0x0506);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(FsuipcUpdateObserver)

struct let_test
{
   let_test()
      : _observer(
           std::bind(
              &let_test::on_offset_update,
              this,
              std::placeholders::_1))
   {}

   let_test& then_offset_changes(
            fsuipc_offset_address addr,
            fsuipc_offset_length len,
            fsuipc_offset_value val)
   {
      _observer.get_client().user_adapter().write_value_to_buffer(
               addr, len, val);
      return *this;
   }

   let_test& observe(
            fsuipc_offset_address addr,
            fsuipc_offset_length len)
   {
      _observer.start_observing(fsuipc_offset(addr, len));
      return *this;
   }

   let_test& unobserve(
            fsuipc_offset_address addr,
            fsuipc_offset_length len)
   {
      _observer.stop_observing(fsuipc_offset(addr, len));
      return *this;
   }

   let_test& check_for_updates()
   {
      _observer.check_for_updates();
      return *this;
   }

   let_test& assert_updates_count(std::size_t count)
   {
      BOOST_CHECK_EQUAL(count, _updates.size());
      return *this;
   }

   let_test& assert_update(
            fsuipc_offset_address addr,
            fsuipc_offset_length len,
            fsuipc_offset_value val)
   {
      BOOST_CHECK(
               std::any_of(
                  _updates.begin(),
                  _updates.end(),
                  [addr, len, val](const fsuipc_valued_offset& vo)
      {
         return vo.address == addr && vo.length == len && vo.value == val;
      }));
      return *this;
   }

   let_test& and_then()
   {
      _updates.clear();
      return *this;
   }

private:

   fsuipc_update_observer<dummy_fsuipc_user_adapter> _observer;
   std::list<fsuipc_valued_offset> _updates;

   void on_offset_update(const fsuipc_valued_offset& update)
   {
      _updates.push_back(update);
   }
};

BOOST_AUTO_TEST_CASE(MustObserveNoChangeIfOffsetIsNotUpdated)
{
   let_test()
         .observe(0x700, OFFSET_LEN_DWORD)
         .observe(0x704, OFFSET_LEN_WORD)
         .check_for_updates()
         .assert_updates_count(0);
}


BOOST_AUTO_TEST_CASE(MustObserveOffsetUpdateAfterOffsetIsChanged)
{
   let_test()
         .observe(0x700, OFFSET_LEN_DWORD)
         .observe(0x704, OFFSET_LEN_WORD)
         .check_for_updates()
         .assert_updates_count(0)
         .then_offset_changes(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .check_for_updates()
         .assert_updates_count(1)
         .assert_update(0x700, OFFSET_LEN_DWORD, 0x01020304);
}

BOOST_AUTO_TEST_CASE(MustNotObserveOffsetUpdateAfterOffsetIsChangedBySameValue)
{
   let_test()
         .observe(0x700, OFFSET_LEN_DWORD)
         .then_offset_changes(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .check_for_updates()
         .and_then()
         .then_offset_changes(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .check_for_updates()
         .assert_updates_count(0);
}

BOOST_AUTO_TEST_CASE(MustNotObserveOffsetUpdateAfterStoppingObserving)
{
   let_test()
         .observe(0x700, OFFSET_LEN_DWORD)
         .observe(0x704, OFFSET_LEN_WORD)
         .then_offset_changes(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .check_for_updates()
         .and_then()
         .unobserve(0x700, OFFSET_LEN_DWORD)
         .unobserve(0x704, OFFSET_LEN_WORD)
         .then_offset_changes(0x700, OFFSET_LEN_DWORD, 0x05060708)
         .then_offset_changes(0x700, OFFSET_LEN_DWORD, 0x090c)
         .check_for_updates()
         .assert_updates_count(0);
}

BOOST_AUTO_TEST_CASE(MustNotObserveOffsetUpdateAfterReobserving)
{
   let_test()
         .observe(0x700, OFFSET_LEN_DWORD)
         .then_offset_changes(0x700, OFFSET_LEN_DWORD, 0x01020304)
         .check_for_updates()
         .and_then()
         .unobserve(0x700, OFFSET_LEN_DWORD)
         .then_offset_changes(0x700, OFFSET_LEN_DWORD, 0x05060708)
         .observe(0x700, OFFSET_LEN_DWORD)
         .then_offset_changes(0x700, OFFSET_LEN_DWORD, 0x05060708)
         .check_for_updates()
         .assert_updates_count(0);
}

BOOST_AUTO_TEST_SUITE_END()
