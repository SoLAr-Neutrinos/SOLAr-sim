/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetCryostat.hh
 * @created     Wed Mar 15, 2023 11:57:11 CET
 */

#ifndef SLARDETCRYOSTAT_HH

#define SLARDETCRYOSTAT_HH

#include "detector/SLArBaseDetModule.hh"
#include "G4VPVParameterisation.hh"

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

    G4String  fName;
    G4double  fHalfSizeX;
    G4double  fHalfSizeY; 
    G4double  fHalfSizeZ; 
    G4double  fThickness;
    G4int     fImportance; 

    G4String  fMaterialName;
    G4Material* fMaterial = nullptr;
    SLArBaseDetModule* fModule = nullptr; 
};

typedef std::map<int, SLArCryostatLayer> SLArCryostatStructure; 

class SLArDetCryostat : public SLArBaseDetModule {
  public:
    SLArDetCryostat(); 
    ~SLArDetCryostat(); 

    void BuildCryostat(); 
    void BuildMaterials(G4String); 
    void BuildCryostatStructure(const rapidjson::Value& jcryo);
    SLArBaseDetModule* GetAirflowUnit() {return fAirFlowUnit;}
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
    G4bool fBuildSupport; 
    G4bool fAddNeutronBricks; 
    G4bool fAddFloorAirflow; 
    std::map<G4String, SLArMaterial*> fMaterials;
    std::map<geo::EBoxFace, SLArBaseDetModule*> fSupportStructureFaces;
    std::vector<G4VPhysicalVolume*> fSupportStructureEdges;

    SLArCryostatStructure fCryostatStructure; 
    SLArCryostatStructure fShieldingStructure;

    SLArBaseDetModule* BuildCryostatLayer(
        G4String name, 
        G4double x_, G4double y_, G4double z_, G4double tk_, 
        G4Material* mat);

    SLArBaseDetModule* BuildShieldingLayer(
        G4String name, 
        G4double x_, G4double z_, G4double tk_, 
        G4Material* mat);

    void BuildSupportStructureUnit(); 
    void BuildSupportStructureEdgeUnit(); 
    void BuildAirFlowUnit();
    SLArBaseDetModule* BuildSupportStructure();
    SLArBaseDetModule* BuildSupportStructureFace(geo::EBoxFace kFace); 
    SLArBaseDetModule* BuildSupportStructurePatch(G4double width, G4double len, G4String name); 
    SLArBaseDetModule* BuildSupportStructureEdge(G4double len, G4String name); 
};




#endif /* end of include guard SLARDETCRYOSTAT_HH */

