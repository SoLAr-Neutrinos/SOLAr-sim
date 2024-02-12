/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArMaterial.cc
 * @created     : lunedì ago 08, 2022 17:49:24 CEST
 */

#include "SLArUserPath.hh"
#include "material/SLArMaterial.hh"

#include "rapidjson/document.h"
#include "rapidjson/encodings.h"
#include "rapidjson/allocators.h"
#include "rapidjson/filereadstream.h"

#include "G4UIcommand.hh"
#include "G4NistManager.hh"
#include <cassert>
#include <regex>
#include <iterator>


SLArMaterial::SLArMaterial() : 
  fDBFile(""), fMaterialID(""), fMaterial(nullptr), fOpticalSurf(nullptr)
{}

SLArMaterial::SLArMaterial(const SLArMaterial &mat)
{
  fDBFile        = mat.fDBFile; 
  fMaterialID    = mat.fMaterialID; 
  fMaterial      = mat.fMaterial; 
  fOpticalSurf   = mat.fOpticalSurf; 
}

SLArMaterial::SLArMaterial(G4String matID)
{
  SetMaterialID(matID);
}

SLArMaterial::~SLArMaterial()
{}

G4Material* SLArMaterial::GetMaterial() 
{
  return fMaterial;
}

G4MaterialPropertiesTable* SLArMaterial::GetMaterialPropTable() {
  return fMaterial->GetMaterialPropertiesTable(); 
}

G4String SLArMaterial::GetMaterialID()
{
  return fMaterialID;
}

G4Material* SLArMaterial::FindInMaterialTable(const char* mname) {
  G4Material* mm = nullptr;

  auto mtable = G4Material::GetMaterialTable(); 
  
  for (const auto &m : *mtable) {
    if (std::strcmp(mname, m->GetName()) == 0) {
      printf("SLArMaterial::BuildMaterialFromDB(%s) Material already defined. Using stored object.\n", 
          mname); 
      mm = m; 
      return mm; 
    }
  }

  return mm;
}

G4Material* SLArMaterial::ParseMaterialDB(G4String mat_id) {
  G4Material* material = nullptr; 
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  // open material description file
  FILE* mat_cfg_file = std::fopen(fDBFile, "r");
  char readBuffer[65536];
  rapidjson::FileReadStream is(mat_cfg_file, readBuffer, sizeof(readBuffer));

  rapidjson::Document d;
  d.ParseStream<rapidjson::kParseCommentsFlag>(is);
  assert(d.IsObject());
  assert(d.HasMember("materials")); 
  assert(d["materials"].IsArray()); 


  for (const auto &mats :  d["materials"].GetArray()) {
    assert(mats.HasMember("name"));
    if ( !G4StrUtil::contains(mats["name"].GetString(), mat_id) ) {
      continue;
    } else {
      material = ParseMaterial(mats);
      fclose(mat_cfg_file); 
      return material; 
    }
  }

  printf("SLArMaterial::BuildMaterialFromDB(%s) WARNING:", mat_id.c_str()); 
  printf(" Unable to find %s in material DB (%s)\n",mat_id.c_str(), fDBFile.c_str());
  printf("Falling back to NIST databse\n"); 
  material = G4NistManager::Instance()->FindOrBuildMaterial(mat_id, true); 
  material->SetName(mat_id); 

  fclose(mat_cfg_file);

  return material; 
}

void SLArMaterial::BuildMaterialFromDB(G4String db_file, G4String mat_id) {

  if (mat_id.empty()) mat_id = fMaterialID;
  else SetMaterialID(mat_id);

  fDBFile = db_file; 
   
  if ( (fMaterial = FindInMaterialTable(mat_id)) ) {
    return;  
  } 
  
  fMaterial = ParseMaterialDB(mat_id); 

  return;
}

G4Material* SLArMaterial::BuildFromNist(const rapidjson::Value& jmaterial) {
  auto nistManager = G4NistManager::Instance(); 
  G4Material* material = nullptr; 
  printf("Building %s from NIST database\n", jmaterial["name"].GetString()); 
  material = nistManager->FindOrBuildMaterial(jmaterial["NIST"].GetString(), true); 
  material->SetName(jmaterial["name"].GetString()); 
  return material;
}

G4Material* SLArMaterial::BuildFromAtoms(const rapidjson::Value& jmaterial) {
  auto nistManager = G4NistManager::Instance(); 
  G4Material* material = nullptr; 
  printf("Building %s based on listed atomic components\n", jmaterial["name"].GetString()); 
  assert(jmaterial.HasMember("name"));
  assert(jmaterial.HasMember("density"));
  assert(jmaterial["atomic_components"].IsArray());

  auto components = jmaterial["atomic_components"].GetArray();
  auto density = jmaterial["density"].GetObj(); 
  double vunit = 1.0; 
  if (density.HasMember("unit")) 
    vunit = G4UIcommand::ValueOf(density["unit"].GetString()); 

  if (jmaterial.HasMember("temperature")) {
    auto temperature = jmaterial["temperature"].GetObj();
    G4double tunit = 1.0; 
    if (temperature.HasMember("unit")) 
      tunit = G4UIcommand::ValueOf(temperature["unit"].GetString()); 

    material = new G4Material(
        jmaterial["name"].GetString(), 
        density["val"].GetDouble()*vunit, 
        components.Size(), G4State::kStateLiquid, 
        temperature["val"].GetDouble()*tunit);
  } else {
    material = new G4Material(
        jmaterial["name"].GetString(), 
        density["val"].GetDouble()*vunit, 
        components.Size()); 
  }

  for (const auto& comp : components) {
    G4Element* el = nullptr;
    if ( (el = nistManager->FindOrBuildElement(comp["Z"].GetInt())) ) {}
    else {
      el = new G4Element(
          comp["name"].GetString(), 
          comp["symb"].GetString(),
          comp["Z"].GetDouble(), 
          comp["A"].GetDouble()); 
    } 
    if (comp.HasMember("fraction")) {
      material->AddElement(el, comp["fraction"].GetDouble()); 
    } else if (comp.HasMember("nAtom")) {
      material->AddElement(el, comp["nAtom"].GetInt()); 
    } else {
      printf("SLArMaterial::BuildMaterial() %s weight is not specified\n", 
          comp["name"].GetString());
    }
  }

  return material; 
}

G4Material* SLArMaterial::BuildFromMixture(const rapidjson::Value& jmaterial) {
  G4Material* material = nullptr; 
  printf("Building %s based on listed compounds\n", jmaterial["name"].GetString()); 
  
  assert(jmaterial.HasMember("density"));
  assert(jmaterial["mixture_components"].IsArray()); 

  auto components = jmaterial["mixture_components"].GetArray(); 
  auto density = jmaterial["density"].GetObj(); 
  double vunit = 1.0; 
  if (density.HasMember("unit")) 
    vunit = G4UIcommand::ValueOf(density["unit"].GetString()); 

  material = new G4Material(jmaterial["name"].GetString(), 
      density["val"].GetDouble()*vunit, components.Size()); 

  for (const auto& comp : components) {
    assert(comp.HasMember("name")); 
    assert(comp.HasMember("massFraction")); 
    G4String comp_name = comp["name"].GetString(); 

    G4Material* mat = FindInMaterialTable(comp_name); 
    if (!mat) {
      if (comp.HasMember("NIST")) mat = BuildFromNist( comp );  
      else {
        mat = ParseMaterialDB(comp_name);
      }
    }
    G4double mass_fraction = comp["massFraction"].GetDouble(); 

    material->AddMaterial(mat, mass_fraction); 
  }

  printf("SLArMaterial::BuildFromMixture(%s)\n", jmaterial["name"].GetString());
  printf("Material elements mass fraction breakdown\n");
  for (size_t i = 0; i < material->GetNumberOfElements(); i++) {
    auto elm = material->GetElement(i); 
    double frac = material->GetFractionVector()[i]; 
    printf("[%lu]: %s : %.2f%%\n", i, elm->GetSymbol().c_str(), frac*100);
  }

  return material; 
}

G4Material* SLArMaterial::ParseMaterial(const rapidjson::Value& jmaterial) {
  printf("SLArMaterial::BuildMaterialFromDB(%s)\n", jmaterial["name"].GetString()); 
  G4Material* material = nullptr; 
  if (jmaterial.HasMember("NIST")) {
    material = BuildFromNist(jmaterial); 
  } else if (jmaterial.HasMember("atomic_components")) {
    material = BuildFromAtoms(jmaterial); 
  } else if (jmaterial.HasMember("mixture_components")) {
    material = BuildFromMixture(jmaterial); 
  } else {
    printf("SLArMaterial::ParseMaterial WARNING: No known way to build %s\n", 
        jmaterial["name"].GetString());
  }

  if (jmaterial.HasMember("PropertiesTable")) {
    printf("SLArMaterial::BuildMaterial(%s): Building MaterialPropertiesTable\n", 
        jmaterial["name"].GetString());
    auto ptable = new G4MaterialPropertiesTable(); 
    ParseMPT(jmaterial["PropertiesTable"], ptable, material); 
    material->SetMaterialPropertiesTable(ptable); 
  }

  if (jmaterial.HasMember("SurfaceProperties")) {
    printf("SLArMaterial::BuildMaterial(%s): Building Material Surface Properties\n", 
        jmaterial["name"].GetString());
    ParseSurfaceProperties(jmaterial["SurfaceProperties"]);
  }

  printf("DONE\n");

  return material; 
}

void SLArMaterial::ParseMPT(const rapidjson::Value& jptable, G4MaterialPropertiesTable* mpt, G4Material* mat) {

  assert(jptable.IsArray());
  const auto properties = jptable.GetArray(); 

  for (const auto& p : properties) {
    assert(p.HasMember("property")); 
    assert(p.HasMember("value")); 

    G4String pname = p["property"].GetString(); 
    printf("Parsing %s\n", pname.c_str());

    if (p["value"].IsArray()) {
      assert(p["value"].GetArray().Size() == 2); 
      std::vector<double> vE; vE.reserve(500); 
      std::vector<double> vP; vP.reserve(500); 
      for (const auto &v : p["value"].GetArray()) {
        assert(v.HasMember("var")); 
        assert(v.HasMember("val")); 
        assert(v["val"].IsArray()); 
        G4double vunit = 1.0; 
        if (v.HasMember("unit")) {
          //vunit = G4UIcommand::ValueOf(v["unit"].GetString());
          vunit = ParseUnit(v["unit"]); 
        }
        if (strcmp("Energy", v["var"].GetString()) == 0) {
          for (const auto &val : v["val"].GetArray()) {
            vE.push_back( val.GetDouble()*vunit ); 
          }
        } else {
          for (const auto &val : v["val"].GetArray()) {
            vP.push_back( val.GetDouble()*vunit ); 
          } 
        }
      }
      G4bool is_custom = false; 
      if (p.HasMember("custom")) is_custom = p["custom"].GetBool(); 
      mpt->AddProperty(pname, vE, vP, is_custom); 
    } else {
      G4double punit = 1.0; 
      if (p.HasMember("unit")) {
        //punit = G4UIcommand::ValueOf(p["unit"].GetString()); 
        punit = ParseUnit(p["unit"]); 
      }
      G4double pvalue = p["value"].GetDouble(); 
      G4bool is_custom = false; 
      if (p.HasMember("custom")) is_custom = p["custom"].GetBool(); 


      if (pname == "BIRKSCONSTANT") {
        if (mat) 
          mat->GetIonisation()->SetBirksConstant(pvalue*CLHEP::mm/CLHEP::MeV); 
        else 
          printf("WARNING: Cannot set Birk's constant. No material provided.\n");
      } else {
        mpt->AddConstProperty(pname, pvalue*punit, is_custom);
      }
    }
  }
  return;
}

void SLArMaterial::ParseSurfaceProperties(const rapidjson::Value& jptable) {
  assert(jptable.IsObject());
  auto surfcfg = jptable.GetObject(); 
  assert(surfcfg.HasMember("name"));
  G4String surfName = surfcfg["name"].GetString();
  fOpticalSurf = new G4OpticalSurface(surfcfg["name"].GetString());

  assert(surfcfg.HasMember("model"));
  assert(surfcfg.HasMember("type"));
  assert(surfcfg.HasMember("finished")); 

  G4String model_str = surfcfg["model"].GetString(); 
  G4String type_str  = surfcfg["type"].GetString(); 
  G4String finished_str = surfcfg["finished"].GetString(); 

  if (G4StrUtil::contains(model_str, "unified")) {
    fOpticalSurf->SetModel(G4OpticalSurfaceModel::unified); 
  } else { 
    fOpticalSurf->SetModel(G4OpticalSurfaceModel::glisur); 
  }

  if (G4StrUtil::contains(type_str, "dielectric_metal")) {
    fOpticalSurf->SetType( G4SurfaceType::dielectric_metal );
  } else {
    fOpticalSurf->SetType( G4SurfaceType::dielectric_dielectric );
  }

  if (G4StrUtil::contains(finished_str, "polished")) {
    fOpticalSurf->SetFinish( G4OpticalSurfaceFinish::polished );
  } else { 
    fOpticalSurf->SetFinish( G4OpticalSurfaceFinish::ground ); 
  }

  if (surfcfg.HasMember("polished")) {
    fOpticalSurf->SetPolish( surfcfg["polished"].GetDouble() );
  }


  if (surfcfg.HasMember("PropertiesTable")) {
    G4MaterialPropertiesTable* srf_ptable = new G4MaterialPropertiesTable();
    ParseMPT(surfcfg["PropertiesTable"], srf_ptable);
    fOpticalSurf->SetMaterialPropertiesTable(srf_ptable);
  }

  return;
}

G4double SLArMaterial::ParseUnit(const rapidjson::Value& junit) {
  G4double vunit = 1.0; 
  assert(junit.IsString()); 
  G4String sunit = junit.GetString(); 

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
      printf("Divide %s\n", unit_match.c_str());
      vunit /= G4UIcommand::ValueOf(unit_match);  
    } else {
      printf("Multiply %s\n", unit_match.c_str());
      vunit *= G4UIcommand::ValueOf(unit_match); 
    }
  }

  //printf("vunit is %g\n", vunit);

  return vunit; 
}

