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

#ifndef OAC_FV_MQTT_TOPIC_H
#define OAC_FV_MQTT_TOPIC_H

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <liboac/exception.h>

namespace oac { namespace fv { namespace mqtt {

OAC_DECL_EXCEPTION_WITH_PARAMS(illegal_topic_error, oac::exception,
   ("topic %s is invalid (must not contain '+' or '#')", tpc),
   (tpc, std::string)
);

class topic
{
public:

   topic(const std::string& tpc) : _stringfied(tpc)
   {
      if (!is_valid_topic(tpc))
         OAC_THROW_EXCEPTION(illegal_topic_error(tpc));
   }

   topic(const char* tpc) : topic(std::string(tpc)) {}

   operator const std::string& () const { return _stringfied; }

   std::string to_string() const { return _stringfied; }

   const char* to_c_str() const { return _stringfied.c_str(); }

private:

   std::string _stringfied;

   static bool is_valid_topic(const std::string& tpc)
   {
      return (tpc.find('+') == std::string::npos) &&
             (tpc.find('#') == std::string::npos);
   }
};

OAC_DECL_EXCEPTION_WITH_PARAMS(illegal_topic_pattern_error, oac::exception,
   ("topic pattern %s is invalid: %s", pattern, reason),
   (pattern, std::string),
   (reason, std::string)
);

class topic_pattern
{
public:

   topic_pattern(const std::string& pattern) : _stringfied(pattern)
   {
      auto tokens = split_string(pattern);
      for (std::size_t i = 0; i < tokens.size(); i++)
      {
         auto& token = tokens[i];
         elem e;
         if (token.find('+') != std::string::npos)
         {
            if (token.length() != 1)
               OAC_THROW_EXCEPTION(illegal_topic_pattern_error(
                     pattern, "+ cannot be mixed with other symbols"));
            e.type = elem_type::PLUS_WILDCARD;
         }
         else if (token.find('#') != std::string::npos)
         {
            if (token.length() != 1)
               OAC_THROW_EXCEPTION(illegal_topic_pattern_error(
                     pattern, "# cannot be mixed with other symbols"));
            if (i != tokens.size() - 1)
               OAC_THROW_EXCEPTION(illegal_topic_pattern_error(
                     pattern, "# must be the last element"));
            e.type = elem_type::HASH_WILDCARD;
         }
         else
         {
            e.type = elem_type::LITERAL;
            e.literal = token;
         }
         _elems.push_back(e);
      }
   }

   topic_pattern(const char* pattern) : topic_pattern(std::string(pattern)) {}

   operator const std::string& () const { return _stringfied; }

   std::string to_string() const { return _stringfied; }

   const char* to_c_str() const { return _stringfied.c_str(); }

   bool match(const topic& tpc) const
   {
      auto tokens = split_string(tpc.to_string());
      std::size_t i;
      for (i = 0; i < std::min(tokens.size(), _elems.size()); i++)
      {
         auto& elem = _elems[i];
         auto& token = tokens[i];
         switch (elem.type)
         {
            case elem_type::LITERAL:
               if (token != elem.literal) return false;
               break;
            case elem_type::HASH_WILDCARD:
               return true;
         }
      }
      return _elems.size() == tokens.size();
   }

private:

   enum class elem_type
   {
      LITERAL,
      PLUS_WILDCARD,
      HASH_WILDCARD,
   };

   struct elem
   {
      elem_type type;
      std::string literal;
   };

   std::string _stringfied;
   std::vector<elem> _elems;

   static std::vector<std::string> split_string(
         const std::string& str)
   {
      std::vector<std::string> tokens;
      boost::split(tokens, str, boost::is_any_of("/"));
      return tokens;
   }
};

}}} // namespace oac::fv::mqtt

#endif
