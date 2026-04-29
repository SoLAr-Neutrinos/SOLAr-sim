/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDebugUtils
 * @created     : Thursday Oct 30, 2025 15:07:23 CET
 */

#ifndef SLARDEBUGUTILS_HH

#define SLARDEBUGUTILS_HH

#include <type_traits>

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

  template<typename T>
    T get_json_value(const rapidjson::Value& val);

  template<>
    inline int get_json_value<int>(const rapidjson::Value& val) { return val.GetInt(); }

  template<>
    inline double get_json_value<double>(const rapidjson::Value& val) { return val.GetDouble(); }

  template<>
    inline bool get_json_value<bool>(const rapidjson::Value& val) { return val.GetBool(); }

  template<>
    inline std::string get_json_value<std::string>(const rapidjson::Value& val) { return val.GetString(); }

  // Helper to convert value to string for error message
  template<typename T>
    std::string to_string_for_error(const T& value) {
      using Decayed = std::decay_t<T>;

      if constexpr (std::is_same<Decayed, std::string>::value)
        return value;
      else if constexpr (std::is_convertible<Decayed, const char*>::value)
        return std::string(value);
      else
        return std::to_string(value);
    }

  template <typename T>
    inline void require_json_object_in_array(
        const rapidjson::Value& arr,
        const char* member_name,
        rapidjson::Type expected_type,
        const T& expected_value
        )
    {
      using Decayed = std::decay_t<T>;
      using ValueType = std::conditional_t<
        std::is_same_v<Decayed, const char*> ||
        std::is_same_v<Decayed, char*>,
        std::string,
        Decayed>;

      for (const auto& entry : arr.GetArray()) {
        if (entry.IsObject() && entry.HasMember(member_name)) {
          if (entry[member_name].GetType() != expected_type) {
            G4String err_msg = G4String("JSON member \"") + member_name +
              "\" has unexpected type. Expected: " + JsonTypeName(expected_type) +
              ", got: " + JsonTypeName(entry[member_name].GetType());
            G4Exception("debug::require_json_object_in_array", "JsonDebug000", FatalException, err_msg);
            return;
          }

          try {
            ValueType value = get_json_value<ValueType>(entry[member_name]);
            if (value == expected_value) return;
          } catch (...) {
            G4String err_msg = G4String("Failed to extract value for member \"") + member_name + "\"";
            G4Exception("debug::require_json_object_in_array", "JsonDebug001", FatalException, err_msg);
            return;
          }
        }
      }

      G4String err_msg = G4String("No JSON object in array has member \"") + member_name +
        "\" with expected value: " + to_string_for_error(expected_value);
      G4Exception("debug::require_json_object_in_array", "JsonDebug000", FatalException, err_msg);
      return;
    }

  template<> void inline require_json_object_in_array<std::string>(
      const rapidjson::Value& arr,
      const char* member_name,
      rapidjson::Type expected_type,
      const std::string& expected_value
      );
  template<> void inline require_json_object_in_array<double>(
      const rapidjson::Value& arr,
      const char* member_name,
      rapidjson::Type expected_type,
      const double& expected_value
      );
  template<> void inline require_json_object_in_array<int>(
      const rapidjson::Value& arr,
      const char* member_name,
      rapidjson::Type expected_type,
      const int& expected_value
      );
  template<> void inline require_json_object_in_array<bool>(
      const rapidjson::Value& arr,
      const char* member_name,
      rapidjson::Type expected_type,
      const bool& expected_value
      );


  inline void require_json_member(
      const rapidjson::Value& obj,
      const char* member_name)
  {
    if (obj.HasMember(member_name) == false) {
      G4String err_msg = G4String("Missing required JSON member: ") + member_name;
      G4Exception("debug::require_json_member", "JsonDebug001", FatalException, err_msg);
    }
  }

  inline void require_json_array(
      const rapidjson::Value& obj,
      rapidjson::SizeType expected_size = 0)
  {
    if (obj.IsArray() == false) {
      G4String err_msg = G4String("Expected JSON value to be an array, got: ") + JsonTypeName(obj.GetType());
      G4Exception("debug::require_json_array", "JsonDebug001", FatalException, err_msg);
    }
    if (expected_size > 0 && obj.Size() != expected_size) {
      G4String err_msg = G4String("Expected JSON array of size ") + std::to_string(expected_size) +
        ", got size: " + std::to_string(obj.Size());
      G4Exception("debug::require_json_array", "JsonDebug002", FatalException, err_msg);
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

  inline bool check_json_value_field(const rapidjson::Value& j, const G4String& field) {
    if (j.HasMember("val") == false) {
      G4String msg = "SLArGPSDirectionGenerator::Config ERROR: ";
      msg += "Field \"" + field + "\" must have a \"val\" field\n";
      G4Exception("debug::check_json_value_field", "JsonDebug003", FatalException, msg);
    }

    const auto& jval = j["val"];
    if (jval.IsNumber() == false && jval.IsArray() == false) {
      G4String msg = "SLArGPSDirectionGenerator::CheckJSONFieldIsSValue ERROR: ";
      msg += "Field \"val\" must be a number or an array\n";
      G4Exception("debug::check_json_value_field", "JsonDebug004", FatalException, msg);
    }

    if (j.HasMember("unit")) {
      debug::require_json_type(j["unit"], rapidjson::kStringType);
    }
    return true;
  }

}

#endif /* end of include guard SLARDEBUGUTILS_HH */

