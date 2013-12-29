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

#include "fsuipc/domain.h"
#include "mqtt/client_dummy.h"

using namespace oac;
using namespace oac::fv;

BOOST_AUTO_TEST_SUITE(FsuipcOffsetsDomainTest)

struct let_test
{

   let_test()
   {
      _mqtt = std::make_shared<mqtt_client>();
   }

   let_test& with_exports(const oac::fsuipc::offset& o)
   {
      _exports.push_back(o);
      return *this;
   }

   template <typename... T>
   let_test& with_exports(const oac::fsuipc::offset& o, T... t)
   {
      _exports.push_back(o);
      return with_exports(t...);
   }

   let_test& create_exports_file()
   {
      auto file = format(
            "C:\\Windows\\Temp\\domain-test_%d.json", std::abs(std::rand()));
      boost::property_tree::ptree pt;
      for (auto& exp : _exports)
      {
         boost::property_tree::ptree entry;
         entry.put("address", exp.address);
         entry.put("length", static_cast<int>(exp.length));
         pt.push_back(std::make_pair("", entry));
      }
      boost::property_tree::write_json(file, pt);
      _settings.exports_file = file;
      return *this;
   }

   let_test& init_domain()
   {
      _fsuipc_adapter = oac::fsuipc::make_dummy_user_adapter();
      _dom = fv::fsuipc::make_domain(_settings, _mqtt, _fsuipc_adapter);
      return *this;
   }

   let_test& on_offset_change(const oac::fsuipc::valued_offset& o)
   {
      _fsuipc_adapter->write_value_to_buffer(o.address, o.length, o.value);
      return *this;
   }

   let_test& observe_for_changes()
   {
      _dom->fsuipc_observer().check_for_updates();
      return *this;
   }

   template <typename T>
   let_test& on_message_received(
         const mqtt::topic& tpc,
         const T& value)
   {
      _mqtt->publish_as(tpc, value, mqtt::qos_level::LEVEL_0);
      return *this;
   }

   let_test& assert_subscription(
         const mqtt::topic_pattern& tpc,
         const mqtt::qos_level& qos)
   {
      const auto& subs = _mqtt->subscriptions();
      BOOST_CHECK_MESSAGE(
            std::any_of(
                  subs.begin(),
                  subs.end(),
                  [tpc, qos](const mqtt::dummy_client::subscription& s)
                  {
                     return s.pattern == tpc && s.qos == qos;
                  }),
            "there is no registered subscription for " <<
                  tpc.to_string() << " on " <<
                  mqtt::qos_level_conversions::to_string(qos));
      return *this;
   }

   let_test& assert_observing(const oac::fsuipc::offset& o)
   {
      BOOST_CHECK_MESSAGE(
            _dom->fsuipc_observer().is_observing(o),
            format("domain is not observing %x:%d", o.address, o.length));
      return *this;
   }

   template <typename T>
   let_test& assert_message_sent(
         const mqtt::topic& tpc,
         const T& value)
   {
      const auto& msgs = _mqtt->messages();
      BOOST_CHECK_MESSAGE(
            std::any_of(
                  msgs.begin(),
                  msgs.end(),
                  [tpc, value](const mqtt::raw_message& msg)
                  {
                     return msg.to_typed<T>().data == value;
                  }),
            format("no message was received for topic %s with expected value",
                  tpc.to_string()));
      return *this;
   }

   let_test& assert_offset_changed(
         const oac::fsuipc::valued_offset& o)
   {
      auto value = _fsuipc_adapter->read_value_from_buffer(o.address, o.length);
      BOOST_CHECK_MESSAGE(
            value == o.value,
            format("unexpected value in offset 0x%x", o.address));
      return *this;
   }

private:

   using mqtt_client = oac::fv::mqtt::dummy_client;
   using mqtt_client_ptr = std::shared_ptr<mqtt_client>;
   using offset_list = std::list<oac::fsuipc::offset>;
   using fsuipc_user_adapter = oac::fsuipc::dummy_user_adapter;
   using fsuipc_client = oac::fsuipc::fsuipc_client<fsuipc_user_adapter>;
   using domain = oac::fv::fsuipc::domain<fsuipc_user_adapter>;
   using domain_ptr = std::shared_ptr<domain>;

   mqtt_client_ptr _mqtt;
   offset_list _exports;
   conf::domain_settings _settings;
   oac::fsuipc::dummy_user_adapter_ptr _fsuipc_adapter;
   domain_ptr _dom;
};

BOOST_AUTO_TEST_CASE(MustInitFromSettings)
{
   let_test()
         .with_exports(
               oac::fsuipc::offset(0x1000, oac::fsuipc::OFFSET_LEN_BYTE),
               oac::fsuipc::offset(0x2000, oac::fsuipc::OFFSET_LEN_WORD),
               oac::fsuipc::offset(0x3000, oac::fsuipc::OFFSET_LEN_DWORD))
         .create_exports_file()
         .init_domain()
         .assert_subscription(
               "/flightvars/fsuipc/offset/+",
               mqtt::qos_level::LEVEL_0)
         .assert_observing(
               oac::fsuipc::offset(0x1000, oac::fsuipc::OFFSET_LEN_BYTE))
         .assert_observing(
               oac::fsuipc::offset(0x2000, oac::fsuipc::OFFSET_LEN_WORD))
         .assert_observing(
               oac::fsuipc::offset(0x3000, oac::fsuipc::OFFSET_LEN_DWORD));
}

BOOST_AUTO_TEST_CASE(MustSendMessageOnOffsetChange)
{
   let_test()
         .with_exports(
               oac::fsuipc::offset(0x1000, oac::fsuipc::OFFSET_LEN_BYTE))
         .create_exports_file()
         .init_domain()
         .on_offset_change(
               oac::fsuipc::valued_offset(
                     0x1000, oac::fsuipc::OFFSET_LEN_BYTE, 150))
         .observe_for_changes()
         .assert_message_sent("/flightvars/fsuipc/offset/1000:1", 150);
}

BOOST_AUTO_TEST_CASE(MustChangeOffsetOnMessageReceived)
{
   let_test()
         .with_exports(
               oac::fsuipc::offset(0x1000, oac::fsuipc::OFFSET_LEN_BYTE))
         .create_exports_file()
         .init_domain()
         .on_message_received("/flightvars/fsuipc/offset/1000:1", 150)
         .assert_offset_changed(
               oac::fsuipc::valued_offset(
                     0x1000, oac::fsuipc::OFFSET_LEN_BYTE, 150));
}

BOOST_AUTO_TEST_SUITE_END()
