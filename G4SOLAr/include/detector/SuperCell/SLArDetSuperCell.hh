/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetSuperCell
 * @created     : martedì mag 24, 2022 11:41:01 CEST
 */

#ifndef SLARDETSUPERCELL_HH

#define SLARDETSUPERCELL_HH

#include "detector/SLArBaseDetModule.hh"
#include "material/SLArMaterialInfo.h"

#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4SystemOfUnits.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

class SLArDetSuperCell : public SLArBaseDetModule
{

public:
  SLArDetSuperCell            ();
  SLArDetSuperCell            (const SLArDetSuperCell &detSuperCell);
  ~SLArDetSuperCell ();
  
  void          SetPhotoDetPos(EPhotoDetPosition kPos);
  void          SetPerfectQE(G4bool kQE);

  void          BuildMaterial();
  void          BuildDefalutGeoParMap();
  void          BuildSuperCell();
  void          BuildLightGuide();
  void          BuildCoating();
  void          ReadParTable();
  void          ResetSuperCellGeometry();
  void          SetVisAttributes();

  EPhotoDetPosition    GetSuperCellPos();
  SLArBaseDetModule*   GetCoating();
  SLArMaterialInfo*    GetCoatingMaterial();

  G4double           GetTotalHeight();
  G4double           GetSize();

protected:

private:
  /* data */
  G4double                fhTot;
  G4double                fSize;

  EPhotoDetPosition  fPos;
  G4bool             fPerfectQE;

  SLArBaseDetModule* fLightGuide;
  SLArBaseDetModule* fCoating; 

  SLArMaterialInfo*  fMatSuperCell;
  SLArMaterialInfo*  fMatLightGuide;
  SLArMaterialInfo*  fMatCoating; 
};

#endif /* end of include guard SLARDETSUPERCELL_HH */

