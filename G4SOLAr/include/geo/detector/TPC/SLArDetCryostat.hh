/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetCryostat.hh
 * @created     Wed Mar 15, 2023 11:57:11 CET
 */

#ifndef SLARDETCRYOSTAT_HH

#define SLARDETCRYOSTAT_HH

#include "detector/SLArBaseDetModule.hh"
#include "G4VPVParameterisation.hh"
#include <G4Types.hh>

struct SLArCryostatLayer{
  public:
    SLArCryostatLayer(); 
    SLArCryostatLayer(
        G4String   model_name, 
        G4double*  halfSize,  
        G4double   thickness,
        G4String   material_name,
        G4int      importance = 1);
    SLArCryostatLayer(
        G4String   model_name, 
        G4double   thickness,
        G4String   material_name, 
        G4int      importance);

    ~SLArCryostatLayer() {}

    G4String  fName = {};
    G4double  fHalfSizeX = {};
    G4double  fHalfSizeY = {}; 
    G4double  fHalfSizeZ = {}; 
    G4double  fRadius = {}; 
    G4double  fHalfLength = {};
    G4double  fThickness = {};
    G4int     fImportance = 1; 

    G4String  fMaterialName = {};
    G4Material* fMaterial = nullptr;
    SLArBaseDetModule* fModule = nullptr; 
};

typedef std::map<int, SLArCryostatLayer> SLArCryostatStructure; 

enum class ECryostatShape { kBox = 0, kCylinder = 1 };
static const std::map<ECryostatShape, G4String> CryostatShapeName = {
  {ECryostatShape::kBox, "box"},
  {ECryostatShape::kCylinder, "cylinder"}
};

static ECryostatShape get_cryostat_shape_code(const G4String& shape_str) {
  for (const auto& kv : CryostatShapeName) {
    if (kv.second == shape_str) return kv.first;
  }
  throw std::runtime_error("Invalid cryostat shape string: " + shape_str);
}

static G4String get_cryostat_shape_str(ECryostatShape shape_code) {
  auto it = CryostatShapeName.find(shape_code);
  if (it != CryostatShapeName.end()) return it->second;
  throw std::runtime_error("Invalid cryostat shape code: " + std::to_string((int)shape_code));
}

class SLArDetCryostat : public SLArBaseDetModule {
  public:
    SLArDetCryostat(); 
    ~SLArDetCryostat(); 

    void BuildCryostat(); 
    void BuildMaterials(G4String); 
    void BuildCryostatStructure(const rapidjson::Value& jcryo);
    SLArBaseDetModule* GetAirflowUnit() {return fAirFlowUnit;}
    ECryostatShape GetShape() const {return fShape;}
    SLArCryostatStructure& GetCryostatStructure() {return fCryostatStructure;}
    SLArCryostatStructure& GetShieldingStructure() {return fShieldingStructure;}
    inline std::map<geo::EBoxFace, SLArBaseDetModule*>& GetCryostatSupportStructureFaces() {return fSupportStructureFaces;}
    inline std::vector<G4VPhysicalVolume*>& GetCryostatSupportStructureEdges() {return fSupportStructureEdges;}
    inline SLArBaseDetModule* GetSupportStructure() {return fSupportStructure;}
    inline SLArBaseDetModule* GetWaffleUnit() {return fWaffleUnit;}
    inline SLArBaseDetModule* GetWaffleCornerUnit() {return fWaffleEdgeUnit;}
    virtual void Init(const rapidjson::Value&) override {}
    bool HasSupportStructure() const {return fBuildSupport;}
    void SetWorldMaterial(SLArMaterial* mat) {fMatWorld = mat;}
    inline void SetSupportStructureVisibility(bool visible) {fSupportStructureVisibility = visible;}
    void SetVisAttributes();
    G4bool HasAirFlow() const {return fAddFloorAirflow;}
    

  private: 
    SLArMaterial* fMatWorld = {}; 
    SLArMaterial* fMatWaffle = {}; 
    SLArMaterial* fMatBrick = {}; 
    SLArBaseDetModule* fWaffleUnit = {};
    SLArBaseDetModule* fWaffleEdgeUnit = {};
    SLArBaseDetModule* fAirFlowUnit = {}; 
    SLArBaseDetModule* fSupportStructure = {};
    ECryostatShape fShape = ECryostatShape::kBox;
    G4bool fBuildSupport; 
    G4bool fAddNeutronBricks; 
    G4bool fAddFloorAirflow; 
    G4bool fSupportStructureVisibility;
    std::map<G4String, SLArMaterial*> fMaterials;
    std::map<geo::EBoxFace, SLArBaseDetModule*> fSupportStructureFaces;
    std::vector<G4VPhysicalVolume*> fSupportStructureEdges;

    SLArCryostatStructure fCryostatStructure; 
    SLArCryostatStructure fShieldingStructure;

    SLArBaseDetModule* BuildCryostatBoxLayer(
        G4String name, 
        G4double x_, G4double y_, G4double z_, G4double tk_, 
        G4Material* mat);

    SLArBaseDetModule* BuildCryostatTubLayer(
        G4String name,
        G4double r_, G4double hz_, G4double tk_,
        G4Material* mat);

    SLArBaseDetModule* BuildShieldingLayer(
        G4String name, 
        G4double x_, G4double z_, G4double tk_, 
        G4Material* mat);

    void BuildCryostatBoxStructure(const rapidjson::Value& jcryo);
    void BuildCryostatTubStructure(const rapidjson::Value& jcryo);
    void BuildSupportStructureUnit(); 
    void BuildSupportStructureEdgeUnit(); 
    void BuildAirFlowUnit();
    SLArBaseDetModule* BuildSupportStructure();
    SLArBaseDetModule* BuildSupportStructureFace(geo::EBoxFace kFace); 
    SLArBaseDetModule* BuildSupportStructurePatch(G4double width, G4double len, G4String name); 
    SLArBaseDetModule* BuildSupportStructureEdge(G4double len, G4String name); 
};




#endif /* end of include guard SLARDETCRYOSTAT_HH */

