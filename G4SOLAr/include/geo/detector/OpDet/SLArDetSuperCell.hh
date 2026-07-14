/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArDetSuperCell.hh
 * @created     : Tuesday May 24, 2022 11:41:01 CEST
 */

#ifndef SLARDETSUPERCELL_HH

#define SLARDETSUPERCELL_HH

#include "detector/OpDet/SLArOpticalDetector.hh"
#include "G4LogicalBorderSurface.hh"
#include <G4Exception.hh>

class SLArDetSuperCell : public SLArOpticalDetector
{

public:
  inline SLArDetSuperCell() : 
    SLArOpticalDetector()  {  
    fOpDetType = EOpDetType::kSuperCell;
    fOpDetTypeName = "SuperCell";
  }

  inline SLArDetSuperCell(const G4String& name) : SLArOpticalDetector(name) {
    fOpDetType = EOpDetType::kSuperCell;
  }

  inline SLArDetSuperCell(const SLArDetSuperCell &detSuperCell) : SLArOpticalDetector(detSuperCell)
  {
    fOpDetType = EOpDetType::kSuperCell;
    fMatSuperCell = new SLArMaterial(*detSuperCell.fMatSuperCell); 
    fMatLightGuide = new SLArMaterial(*detSuperCell.fMatLightGuide);
    fMatCoating   = new SLArMaterial(*detSuperCell.fMatCoating);
    fMatWLSCoating = new SLArMaterial(*detSuperCell.fMatWLSCoating);
  }

  ~SLArDetSuperCell();
  
  void BuildMaterials(G4String materials_db = "") override;
  void BuildOpticalDetector() override;
  void BuildLightGuide();
  void BuildCoating();
  void BuildWLSCoating();
  G4LogicalSkinSurface* BuildLogicalSkinSurface();
  G4LogicalBorderSurface* BuildWLSLogicalBorderSurface();
  void SetVisAttributes(const int& level = 1) override;

  inline SLArBaseDetModule* GetCoating() { return fCoating; }
  inline SLArMaterial* GetCoatingMaterial() { return fMatCoating; }

  inline G4double GetTotalHeight() { return fhTot; }
  inline G4double GetSize() { return fSize; }

  inline void Init(const rapidjson::Value& jconf) override {
    SLArOpticalDetector::Init(jconf); 

    fGeoInfo->ReadFromJSON(jconf["dimensions"].GetArray());
    fMatInfo.ReadFromJSON(jconf);
/*
 *
 *    if (jconf.HasMember("coating_material")) {
 *      fMatCoatingName = jconf["coating_material"].GetString(); 
 *    }
 *    else {
 *      G4ExceptionDescription ed;
 *      ed << "SLArDetSuperCell::Init(): No coating material specified in JSON configuration! Defaulting to PTP_sensitive.";
 *      G4Exception("SLArDetSuperCell::Init()", "ConfigError001", JustWarning, ed);
 *      fMatCoatingName = "PTP_sensitive"; 
 *    }
 */
    return;
  }

protected:

private:
  G4double  fhTot = {};
  G4double  fSize = {};

  G4String  fMatCoatingName = {};

  SLArBaseDetModule* fLightGuide = {};
  SLArBaseDetModule* fCoating = {}; 
  SLArBaseDetModule* fWLSCoating = {};

  SLArMaterial* fMatSuperCell = {};
  SLArMaterial* fMatLightGuide = {};
  SLArMaterial* fMatCoating = {}; 
  SLArMaterial* fMatWLSCoating = {};
};

#endif /* end of include guard SLARDETSUPERCELL_HH */

