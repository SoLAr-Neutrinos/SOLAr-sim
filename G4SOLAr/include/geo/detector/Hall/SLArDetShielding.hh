/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetShielding.hh
 * @created     : Thursday Oct 30, 2025 11:10:03 CET
 */

#ifndef SLARDETSHIELDING_HH

#define SLARDETSHIELDING_HH

#include "G4Box.hh"
#include "geo/SLArGeoUtils.hh"
#include "detector/SLArBaseDetModule.hh"
#include "detector/SLArPlaneParameterisation.hpp"
#include <map>

class SLArDetShielding : public SLArBaseDetModule {
  public:
    struct SLArShieldingLayer{
      public:
        inline SLArShieldingLayer() {} 
        inline SLArShieldingLayer(
            G4String layer_name, G4double thickness,
            G4String material_name, G4int importance = 1.0) 
          : fName(layer_name), fThickness(thickness),
          fMaterialName(material_name), fImportance(importance) {};

        G4String  fName = {};
        G4double  fThickness = {};
        G4ThreeVector fDisplacement = {}; 
        G4String fMaterialName = {};
        G4int fImportance = 1;
        G4Material* fMaterial = nullptr;
        SLArBaseDetModule* fModule = nullptr; 
    };


    SLArDetShielding();
    ~SLArDetShielding();

    void AddLayer(
        G4String layer_name, G4double thickness,
        G4String material_name, G4int importance = 1.0);
    virtual void Init(const rapidjson::Value&) override;
    void BuildShielding();
    void BuildMaterials(G4String db_file);

    geo::EBoxFace GetFace() const { return fFace; };
    std::map<unsigned int, SLArShieldingLayer>& GetShieldingLayers() { return fShieldingLayers; };
    const std::map<unsigned int, SLArShieldingLayer>& GetShieldingLayers() const { return fShieldingLayers; };
    void SetVisAttributes(); 


  private: 
    SLArMaterial fBaseMaterial;
    geo::EBoxFace fFace = geo::kYminus;
    std::map<unsigned int, SLArShieldingLayer> fShieldingLayers = {};
    std::vector<SLArPlaneParameterisation*> fParameterisation; 
};



#endif /* end of include guard SLARDETSHIELDING_HH */

