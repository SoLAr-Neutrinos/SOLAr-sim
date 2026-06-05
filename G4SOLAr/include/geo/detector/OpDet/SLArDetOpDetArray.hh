/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArDetOpDetArray.hh
 * @created     Fri Mar 24, 2023 15:57:35 CET
 */

#ifndef SLARDETSUPERCELLARRAY_HH

#define SLARDETSUPERCELLARRAY_HH

#include "detector/SLArBaseDetModule.hh"
#include "detector/OpDet/SLArDetSuperCell.hh"
#include "detector/OpDet/SLArOpticalDetector.hh"
#include "detector/SLArPlaneParameterisation.hpp"

class SLArCfgSuperCellArray;

class SLArDetOpDetArray : public SLArBaseDetModule {
  public:
    /// Selects how optical detectors are placed inside the array volume.
    /// kParameterised: uses G4PVParameterised driven by SLArPlaneParameterisation
    ///                 objects read from the "replication_data" JSON field.
    /// kExplicit:      places each detector individually via G4PVPlacement at
    ///                 coordinates provided in the "opdet_positions" JSON field.
    enum class EPlacementMode { kParameterised = 0, kExplicit = 1 };
    //! Single entry of the explicit-position list parsed from JSON.
    struct SExplicitOpDetPos {
      G4int         id;       //!< Copy number / detector identifier
      G4ThreeVector position; //!< Position in the array-local frame [Geant4 length units]
    };


    SLArDetOpDetArray(); 
    ~SLArDetOpDetArray(); 
    
    SLArCfgSuperCellArray BuildOpDetArrayCfg(); 
    void BuildMaterial(G4String materials_db); 
    void BuildOpDetArray(SLArOpticalDetector*); 

    const G4ThreeVector& GetNormal() {return fNormal;}
    const G4ThreeVector& GetPosition() {return fPosition;}
    const G4ThreeVector& GetGlbPosition() {return fGlobalPosition;}
    G4RotationMatrix* GetRotation() {return fRotation;}
    G4String GetPhotoDetModel() {return fPhotoDetModel;}
    G4int GetTPCID() {return fTPCID;}

    virtual void SetGlobalPos( const G4ThreeVector glb_pos ) {fGlobalPosition = glb_pos;}
    virtual void Init(const rapidjson::Value&) override; 

  private:
    std::pair<int, G4double> ComputeArrayTrueLength(G4double width, G4double spacing, G4double max_len);
    //! Build the array by placing each optical detector individually at the
    //! coordinates stored in fExplicitPositions.
    void BuildOpDetArrayExplicit(SLArOpticalDetector* opdet);

    //! Build the array using G4PVParameterised, with replication parameters 
    //! stored in SLArPlaneParameterisation objects in fParameterisation.
    void BuildOpDetArrayParameterised(SLArOpticalDetector* opdet);

    //! Fill the SLArCfgSuperCellArray for the parameterised placement mode.
    void FillCfgParameterised(SLArCfgSuperCellArray& arrayCfg) const;

    //! Fill the SLArCfgSuperCellArray for the explicit placement mode.
    void FillCfgExplicit(SLArCfgSuperCellArray& arrayCfg) const;

    G4int fTPCID; 
    SLArMaterial* fMaterialBase;
    SLArBaseDetModule* fOpDetModuleBase; 
    G4ThreeVector fPosition; 
    G4ThreeVector fGlobalPosition; 
    G4ThreeVector fNormal;
    G4RotationMatrix* fRotation;
    G4String fPhotoDetModel;
    EPlacementMode fPlacementMode = EPlacementMode::kParameterised;
    std::vector<SLArPlaneParameterisation*> fParameterisation; 
    std::vector<SLArBaseDetModule*> fSubModules;
    std::vector<SExplicitOpDetPos> fExplicitPositions;

}; 



#endif /* end of include guard SLARDETSUPERCELLARRAY_HH */

