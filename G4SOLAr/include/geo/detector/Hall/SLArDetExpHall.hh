/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArDetExpHall.hh
 * @created     : Tuesday Nov 12, 2024 16:03:06 CET
 */

#ifndef SLARDETEXPHALL_HH

#define SLARDETEXPHALL_HH

#include "G4BooleanSolid.hh"
#include "G4Box.hh"

#include "detector/SLArBaseDetModule.hh"

class SLArDetExpHall : public SLArBaseDetModule {
  public: 
    enum EExpHallLayout {kBox = 0, kBarrelVault = 1};
    static const EExpHallLayout GetExpHallLayout(const G4String& layout_name) {
      if (layout_name == "box") return kBox;
      if (layout_name == "barrel_vault") return kBarrelVault;
      return kBox;
    }
    static const G4String GetExpHallLayoutName(const EExpHallLayout& layout) {
      if (layout == kBox) return "box";
      if (layout == kBarrelVault) return "barrel_vault";
      return "box";
    }

    struct SLArWallLayer{
      public:
        SLArWallLayer(); 
        SLArWallLayer(
            G4String   layer_name, 
            G4double   thickness,
            G4String   material_name);

        inline ~SLArWallLayer() {
          if (fModule) delete fModule;
        } 

        G4String  fName = {};
        SLArMaterial fMaterial = {};
        G4double  fThickness = {};
        G4ThreeVector fDisplacement = {}; 
        SLArBaseDetModule* fModule = nullptr; 
    };


    SLArDetExpHall();
    ~SLArDetExpHall();

    virtual void Init(const rapidjson::Value&) override;
    void BuildExpHall();
    void BuildMaterials(G4String db_file);
    void BuildLayers(const G4VPhysicalVolume* world_pv);
    const G4double GetBoxShift() const {return fHallBoxShift;}
    inline const G4ThreeVector GetBoxCenter() const {
      G4double hall_shift = GetSolidDisplacement(static_cast<G4BooleanSolid*>(fModSV));
      G4ThreeVector box_center(0, fHallBoxShift + hall_shift, 0);
      return (fModPV->GetObjectTranslation() + box_center);
    }
    inline const G4ThreeVector GetBoxHalfSize() const
    {
      return G4ThreeVector(
          0.5*fGeoInfo->GetGeoPar("inner_size_x"), 
          0.5*fGeoInfo->GetGeoPar("inner_size_y"), 
          0.5*fGeoInfo->GetGeoPar("inner_size_z"));
    }
    inline const G4ThreeVector GetBoxSize() const
    {
      return G4ThreeVector(
          fGeoInfo->GetGeoPar("inner_size_x"), 
          fGeoInfo->GetGeoPar("inner_size_y"), 
          fGeoInfo->GetGeoPar("inner_size_z"));
    }
    const EExpHallLayout& GetLayout() const {return fLayout;}
    G4double GetWallThickness() const {
      G4double thickness = 0;
      for (auto& layer : fWallLayers) {
        thickness += layer.second.fThickness;
      }
      return thickness;
    }

  private:
    EExpHallLayout fLayout;
    SLArMaterial fBaseMaterial;
    G4double fHallBoxShift = 0;
    std::map<uint16_t, SLArWallLayer> fWallLayers;

    G4VSolid* BuildLayerSolid(const double& d, SLArWallLayer* layer = nullptr);
    G4double GetSolidDisplacement(const G4BooleanSolid* solid) const;
    G4double GetSolidDisplacement(G4BooleanSolid* solid);
};



#endif /* end of include guard SLARDETEXPHALL_HH */

