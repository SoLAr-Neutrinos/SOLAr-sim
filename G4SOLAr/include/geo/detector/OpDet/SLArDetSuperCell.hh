/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetSuperCell.hh
 * @created     : Tuesday May 24, 2022 11:41:01 CEST
 */

#ifndef SLARDETSUPERCELL_HH

#define SLARDETSUPERCELL_HH

#include "detector/OpDet/SLArOpticalDetector.hh"

class SLArDetSuperCell : public SLArOpticalDetector
{

public:
  inline SLArDetSuperCell() : SLArOpticalDetector() {  
    fOpDetType = EOpDetType::kSuperCell;
    fOpDetName = "SuperCell";
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
  }

  ~SLArDetSuperCell();
  
  void BuildMaterial(G4String materials_db) override;
  void BuildOpticalDetector() override;
  void BuildLightGuide();
  void BuildCoating();
  G4LogicalSkinSurface* BuildLogicalSkinSurface();
  void SetVisAttributes(const int& level = 1) override;

  inline SLArBaseDetModule* GetCoating() { return fCoating; }
  inline SLArMaterial* GetCoatingMaterial() { return fMatCoating; }

  inline G4double GetTotalHeight() { return fhTot; }
  inline G4double GetSize() { return fSize; }
  virtual void Init(const rapidjson::Value&) override {}

protected:

private:
  G4double  fhTot;
  G4double  fSize;

  SLArBaseDetModule* fLightGuide = {};
  SLArBaseDetModule* fCoating = {}; 

  SLArMaterial* fMatSuperCell = {};
  SLArMaterial* fMatLightGuide = {};
  SLArMaterial* fMatCoating = {}; 
};

#endif /* end of include guard SLARDETSUPERCELL_HH */

