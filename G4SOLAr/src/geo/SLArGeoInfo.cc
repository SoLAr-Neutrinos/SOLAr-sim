/**
 * @author      : guff (guff@guff-gssi)
 * @file        : SLArGeoInfo
 * @created     : giovedì ago 01, 2019 10:32:45 CEST
 */

#include "SLArGeoInfo.hh"
#include "SLArUnit.hpp"
#include <utility>
#include <regex>

SLArGeoInfo::SLArGeoInfo() {}

SLArGeoInfo::SLArGeoInfo(const SLArGeoInfo &geo)
{
  fGeoPar = geo.fGeoPar;
}

SLArGeoInfo::~SLArGeoInfo()
{
  G4cout << "Clearing SLArGeoInfo map..." << G4endl;
  fGeoPar.clear();
  //fMatPar.clear();  
}

void SLArGeoInfo::RegisterGeoPar( G4String str, G4double val)
{
  fGeoPar.insert( std::pair<G4String, G4double>
      (str, val) );
  return;
}

void SLArGeoInfo::RegisterGeoPar(std::pair<G4String, G4double> p)
{
  fGeoPar.insert( p );
  return;
}

void SLArGeoInfo::SetGeoPar(G4String str, G4double val) 
{
  if (Contains(str))
    fGeoPar.find(str)->second = val;
  else 
    RegisterGeoPar(str, val);

  return;
}

void SLArGeoInfo::SetGeoPar(std::pair<G4String, G4double> p)
{
  if ( Contains(p.first) )
    fGeoPar.find(p.first)->second = p.second;
  else 
  {
    std::pair<G4String, G4double> p_copy = 
      std::make_pair(p.first, p.second);
    RegisterGeoPar( p_copy );
  }
  return;
}


bool SLArGeoInfo::Contains(G4String str)
{
  int n = 0;
  bool out = false;
  n = fGeoPar.count( str );
  if ( n > 0) out = true;
  else        out = false;

  return out;
}

G4double SLArGeoInfo::GetGeoPar(G4String str)
{
  G4double out = 0;
  if (!Contains(str))
  {
    const G4String err_msg = "SLArGeoInfo::GetGeoPar() ERROR: Par " 
                    + str + " not found in parameter map";
    G4Exception("SLArGeoInfo::GetGeoPar()", "GeoInfo000", FatalException, err_msg);
  }
  else {
    out = fGeoPar.find(str)->second;
  }

  return out;
}

std::pair<G4String, G4double> SLArGeoInfo::GetGeoPair(G4String str)
{
  std::pair<G4String,G4double> out;
  if (!Contains(str))
  {
    const G4String err_msg = "SLArGeoInfo::GetGeoPair() ERROR: Par " 
                    + str + " not found in parameter map";
    G4Exception("SLArGeoInfo::GetGeoPair()", "GeoInfo001", FatalException, err_msg);
  }
  else {
    out = *fGeoPar.find(str);
  }

  return out;
}

void SLArGeoInfo::DumpParMap()
{
  G4cout << "SLArGeoInfo::DumpParMap()" << G4endl;
  G4cout << "- - - - - - - - - - - - - - - - - - - -" << G4endl;
  for (auto const& it : fGeoPar)
  {
    G4cout << "- " << it.first << ":\t" << it.second << G4endl;
  }
  G4cout << "- - - - - - - - - - - - - - - - - - - -\n" << G4endl;
   
}

bool SLArGeoInfo::ReadFromJSON(const rapidjson::Value::ConstArray& dim) {
  for (const auto &xx : dim) {
    const auto entry = xx.GetObj(); 
    const char* name = entry["name"].GetString();
    G4double val = unit::ParseJsonVal(entry); 
    RegisterGeoPar(name, val); 
  }
  return true;
}

bool SLArGeoInfo::ReadFromJSON(const rapidjson::Value::ConstObject& obj, const char* prefix) {
  if (obj.HasMember("xyz")) {
    auto jxyz = obj["xyz"].GetArray(); 
    assert(jxyz.Size() == 3);
    G4double vunit = unit::GetJSONunit(obj); 
    
    G4String suff[3] = {"_x", "_y", "_z"}; 
    int j=0; 
    for (const auto &jval : jxyz) {
      G4double val  = jval.GetDouble() * vunit; 
      G4String name = prefix + suff[j];  
      RegisterGeoPar(name, val); 
      ++j;
    }
    return true; 
  }
  else if (obj.HasMember("name") && obj.HasMember("val")) {
    G4double val = unit::ParseJsonVal(obj); 
    RegisterGeoPar(obj["name"].GetString(), val); 
  }

  return true; 
}
