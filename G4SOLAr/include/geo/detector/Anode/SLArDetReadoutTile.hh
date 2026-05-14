/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetReadoutTile.hh
 * @created     : giovedì lug 14, 2022 09:34:31 CEST
 */

#ifndef SLARDETREADOUTTILE_HH

#define SLARDETREADOUTTILE_HH

#include "geo/detector/SLArBaseDetModule.hh"
#include "geo/detector/OpDet/SLArDetSiPM.hh"
#include "G4LogicalSkinSurface.hh"

class TH2Poly; 

class SLArDetReadoutTile : public SLArBaseDetModule
{

struct SUnitCellComponent {
  G4String fName; 
  SLArBaseDetModule* fMod; 
  G4ThreeVector fPos;
  G4int fCopyNo;

  SUnitCellComponent(G4String, G4int, SLArBaseDetModule*, G4ThreeVector); 
}; 

struct SUnitCellPixelArea {
  G4String fName; 
  std::vector<G4ThreeVector> fEdges; 

  SUnitCellPixelArea(G4String); 
}; 

public:
  SLArDetReadoutTile();
  SLArDetReadoutTile(const SLArDetReadoutTile &detReadoutTile);
  ~SLArDetReadoutTile();
  
  void BuildMaterial(G4String materials_db);
  void BuildComponentsDefinition(const rapidjson::Value&); 
  void BuildUnitCellPixMap(const rapidjson::Value&); 
  void BuildUnitCellStructure(const rapidjson::Value&); 
  void BuildReadoutTile();
  void BuildPCB();
  TH2Poly* BuildTileChgPixelMap(const G4ThreeVector& xAxis, const G4ThreeVector& yAxis, 
      const G4ThreeVector* _shift = nullptr, const G4RotationMatrix* _rot = nullptr); 
  void BuildChargePix();
  void BuildUnitCell(); 

  void SetVisAttributes(const int depth = 0);

  SLArBaseDetModule* GetUnitCell() {return fUnitCell;}
  SLArBaseDetModule* GetChargePixel() {return fChargePix;}
  SLArDetSiPM* GetSiPM() {return fSiPM;}
  virtual void Init(const rapidjson::Value&) override {}
  const std::vector<SUnitCellComponent>& GetUnitCellStructure() {return fCellStructure;}
  const std::vector<SUnitCellPixelArea>& GetUnitCellPixelMap() {return fCellPixelMap;}
  G4int GetNumberOfCellRows() const; 
  G4int GetNumberOfCellCols() const;
protected:

private:
  G4double fhTot = {};
  G4double fSize = {};

  SLArBaseDetModule* fBasePCB = {};
  SLArBaseDetModule* fBaseTile = {};
  SLArBaseDetModule* fChargePix = {};
  SLArDetSiPM* fSiPM = {};
  SLArBaseDetModule* fUnitCell = {}; 

  G4String fMaterialDBPath = {};
  SLArMaterial*  fMatReadoutTile = {}; 
  SLArMaterial*  fMatPCB = {};
  SLArMaterial*  fMatCopper = {}; 
  SLArMaterial*  fMatChargePix = {};

  std::vector<SUnitCellComponent> fCellStructure; 
  std::vector<SUnitCellPixelArea> fCellPixelMap; 

  friend class SLArDetReadoutPlane;

public: 
};

inline SLArDetReadoutTile::SUnitCellComponent::SUnitCellComponent(
    G4String name, G4int copyNo, SLArBaseDetModule* mod, G4ThreeVector pos) {
 fName = name; 
 fCopyNo = copyNo; 
 fMod = mod; 
 fPos = pos; 
}

inline SLArDetReadoutTile::SUnitCellPixelArea::SUnitCellPixelArea(G4String name) 
  : fName(name) {}

#endif /* end of include guard SLARDETREADOUTTILE_HH */

