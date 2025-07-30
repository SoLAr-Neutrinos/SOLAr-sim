/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetSuperCell
 * @created     : martedì mag 24, 2022 11:41:01 CEST
 */

#ifndef SLARDETSUPERCELL_HH

#define SLARDETSUPERCELL_HH

#include "detector/SLArBaseDetModule.hh"
#include "G4LogicalSkinSurface.hh"

class SLArDetSuperCell : public SLArBaseDetModule
{

public:
  SLArDetSuperCell            ();
  SLArDetSuperCell            (const SLArDetSuperCell &detSuperCell);
  ~SLArDetSuperCell ();
  
  void          SetPhotoDetPos(EPhotoDetPosition kPos);
  void          SetPerfectQE(G4bool kQE);

  void          BuildMaterial(G4String materials_db);
  void          BuildDefalutGeoParMap();
  G4LogicalSkinSurface* BuildLogicalSkinSurface(); 
  void          BuildSuperCell();
  void          BuildLightGuide();
  void          BuildCoating();
  void          BuildWLSCoating();
  void          ReadParTable();
  void          SetVisAttributes();

  EPhotoDetPosition  GetSuperCellPos();
  SLArBaseDetModule* GetCoating();
  SLArMaterial* GetCoatingMaterial();
  G4LogicalSkinSurface* GetSiPMLgSkin() {return fSkinSurface;}

  G4double GetTotalHeight();
  G4double GetSize();
  virtual void Init(const rapidjson::Value&) override {}

protected:

private:
  /* data */
  G4double  fhTot;
  G4double  fSize;

  EPhotoDetPosition  fPos;
  G4bool             fPerfectQE;

  SLArBaseDetModule* fLightGuide;
  SLArBaseDetModule* fCoating; 
  SLArBaseDetModule* fWLSCoating;

  SLArMaterial* fMatSuperCell;
  SLArMaterial* fMatLightGuide;
  SLArMaterial* fMatCoating; 
  SLArMaterial* fMatWLSCoating;

  G4LogicalSkinSurface* fSkinSurface;
};

#endif /* end of include guard SLARDETSUPERCELL_HH */

