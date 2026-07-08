/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArMaterialsInfo.hh
 * @created     : Monday Jul 06, 2026 15:40:34 CEST
 */

#ifndef SLARMATERIALSINFO_HH

#define SLARMATERIALSINFO_HH

#include <iostream>
#include <iomanip>
#include <map>

#include "G4String.hh"
#include "G4Exception.hh"

#include "core/SLArDebugUtils.hh"

class SLArMaterialsInfo {
  public:
    SLArMaterialsInfo() = default;
    inline SLArMaterialsInfo(const SLArMaterialsInfo &mat)
    {
      for (const auto& p : mat.fMaterialsCatalog) {
        fMaterialsCatalog.insert(p);
      }
    }
    inline ~SLArMaterialsInfo() {
      fMaterialsCatalog.clear();
    }

    inline void RegisterMaterial(const G4String& module, const G4String& mat) 
    {
      fMaterialsCatalog.insert(std::make_pair(module, mat));
    }
    inline void RegisterMaterial(std::pair<G4String, G4String> p) 
    {
      fMaterialsCatalog.insert(p);
    }
    inline void SetMaterial(const G4String& module, const G4String& mat) 
    {
      fMaterialsCatalog[module] = mat;
    };
    inline void SetMaterial(std::pair<G4String, G4String> p)
    {
      fMaterialsCatalog[p.first] = p.second;
    }
    inline bool Contains(G4String str) const 
    {
      return fMaterialsCatalog.find(str) != fMaterialsCatalog.end();
    }
    inline void DumpMaterialsCatalog() const {
      const size_t keyWidth = 20;   // Width for the module name
      const size_t valueWidth = 50; // Width for the material name
      printf("Materials module catalog:\n"); 
      for (const auto& [module, material] : fMaterialsCatalog)
      {
        std::cout
          << std::left
          << std::setfill('.')
          << std::setw(keyWidth) << module
          << " | "
          << std::setw(valueWidth) << material
          << '\n';
      }

      // Restore the default fill character if desired
      std::cout << std::setfill(' ');

    }
    inline G4String GetMaterial(const G4String& str) const 
    {
      auto it = fMaterialsCatalog.find(str);
      if (it != fMaterialsCatalog.end()) {
        return const_cast<G4String&>(it->second); // Return the material name
      } 

      G4ExceptionDescription ed; 
      ed << "Module " << str << " not found in the materials catalog.";
      G4Exception("SLArMaterialInfo::GetMaterial", "SLArMaterialInfo001", FatalException, ed);
      return ""; 
    }

    inline bool ReadFromJSON(const rapidjson::Value::ConstArray& jmats) {
      debug::require_json_object_in_array(jmats, "module", rapidjson::kStringType, "base_material");
      for (const auto& entry : jmats) {
        debug::require_json_type(entry, rapidjson::kObjectType);
        debug::require_json_member(entry, "module");
        debug::require_json_member(entry, "material");
        RegisterMaterial(entry["module"].GetString(), entry["material"].GetString());
      }
      return true;
    }

    inline bool ReadFromJSON(const rapidjson::Value& jconf) 
    {
      debug::require_json_member(jconf, {"materials", "base_material"});

      if (jconf.HasMember("materials")) {
        debug::require_json_type(jconf["materials"], rapidjson::kArrayType);
        return ReadFromJSON(jconf["materials"].GetArray());
      }
      else if (jconf.HasMember("base_material")) {
        debug::require_json_type(jconf["base_material"], rapidjson::kStringType);
        RegisterMaterial("base_material", jconf["base_material"].GetString());
        return true;
      }

      G4ExceptionDescription ed;
      ed << "Missing required JSON member: 'materials' or 'base_material'.";
      G4Exception("SLArMaterialsInfo::ReadFromJSON", "SLArMaterialsInfo001", FatalException, ed);

      return false;
    }

  private: 
    std::map<G4String, G4String> fMaterialsCatalog; 

};

#endif /* end of include guard SLARMATERIALSINFO_HH */

