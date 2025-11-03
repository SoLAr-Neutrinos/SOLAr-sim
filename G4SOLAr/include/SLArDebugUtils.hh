/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDebugUtils
 * @created     : Thursday Oct 30, 2025 15:07:23 CET
 */

#ifndef SLARDEBUGUTILS_HH

#define SLARDEBUGUTILS_HH

#include "G4Exception.hh"
#include "G4String.hh"
#include "rapidjson/document.h"
#include "rapidjson/allocators.h"

namespace debug {
  inline const char* JsonTypeName(rapidjson::Type type) {
    switch (type) {
      case rapidjson::kNullType:   return "Null";
      case rapidjson::kFalseType:  return "False";
      case rapidjson::kTrueType:   return "True";
      case rapidjson::kObjectType: return "Object";
      case rapidjson::kArrayType:  return "Array";
      case rapidjson::kStringType: return "String";
      case rapidjson::kNumberType: return "Number";
      default:                     return "Unknown";
    }
  }

  inline void require_json_member(
      const rapidjson::Value& obj,
      const char* member_name)
  {
    if (obj.HasMember(member_name) == false) {
      G4String err_msg = G4String("Missing required JSON member: ") + member_name;
      G4Exception("debug::require_json_member", "JsonDebug001", FatalException, err_msg);
    }
  }

  inline void require_json_type( 
      const rapidjson::Value& obj, 
      rapidjson::Type expected_type) 
  {
    if (obj.GetType() != expected_type) {
      G4String err_msg = G4String("Unexpected JSON value type. Expected: ") + 
        JsonTypeName(expected_type) + 
        G4String(", got: ") + 
        JsonTypeName(obj.GetType());
      G4Exception("debug::require_json_type", "JsonDebug002", FatalException, err_msg);
    }
  }
}

#endif /* end of include guard SLARDEBUGUTILS_HH */

