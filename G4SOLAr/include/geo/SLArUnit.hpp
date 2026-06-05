/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArUnit.hpp
 * @created     : Thursday Apr 11, 2024 14:36:14 CEST
 */

#ifndef SLARUNIT_HPP

#define SLARUNIT_HPP
#include "core/SLArDebugUtils.hh"

#include <regex>
#include <G4UIcommand.hh>
#include <rapidjson/document.h>

namespace unit {
   //double SLArGeoInfo::Unit2Val(const char* unit) {
  //return G4UIcommand::ValueOf( unit );
  //}
  static inline double Unit2Val(const char* cunit) {
    G4double vunit = 1.0; 
    G4String sunit = cunit;
    // Removes whitespaces
    sunit.erase(std::remove_if(sunit.begin(), sunit.end(), ::isspace), sunit.end());

    std::regex rgx_unit( "((^|\\*|/)\\w+)" );
    auto unit_begin = std::sregex_iterator(sunit.begin(), sunit.end(), rgx_unit); 
    auto unit_end   = std::sregex_iterator(); 

    for (std::sregex_iterator i = unit_begin; i!=unit_end; ++i) {
      std::smatch match = *(i); 
      G4String unit_match = match.str(); 
      char front = unit_match.front(); 
      if (front == '*') {
        unit_match.erase(0, 1); 
        //printf("Multiply %s\n", unit_match.c_str());
        vunit *= G4UIcommand::ValueOf(unit_match); 
      } else if (front == '/') {
        unit_match.erase(0, 1); 
        //printf("Divide %s\n", unit_match.c_str());
        vunit /= G4UIcommand::ValueOf(unit_match);  
      } else {
        //printf("Multiply %s\n", unit_match.c_str());
        vunit *= G4UIcommand::ValueOf(unit_match);
      }
    }

    //printf("geo::Unit2Val(%s): %g\n", cunit, vunit); 
    return vunit; 
  }
  
  static inline double Unit2Val(const rapidjson::Value& junit) {
    G4double vunit = 1.0; 
    assert(junit.IsString()); 
    vunit = Unit2Val(junit.GetString()); 
    return vunit; 
  }
  
  static inline double GetJSONunit (const rapidjson::Value& obj) {
    if (!obj.HasMember("unit")) return 1.0; 
    return Unit2Val(obj["unit"].GetString()); 
  }

  static inline double ParseJsonVal(const rapidjson::Value& jval) {
    debug::require_json_type(jval, rapidjson::kObjectType);
    debug::require_json_member(jval, {"val", "value"});
    const char* val_key = (jval.HasMember("val")) ? "val" : "value";
    G4double vunit = GetJSONunit(jval); 
    return jval[val_key].GetDouble() * vunit; 
  } 
}


#endif /* end of include guard SLARUNIT_HPP */

