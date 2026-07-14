/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArDetAnodeAssembly.hh
 * @created     Tue Mar 21, 2023 11:54:25 CET
 */

#ifndef SLARDETANODEASSEMBLY_HH

#define SLARDETANODEASSEMBLY_HH

#include <map>
#include "detector/SLArBaseDetModule.hh"

class SLArDetReadoutTileAssembly; 
class SLArCfgAnode; 

class SLArDetAnodeAssembly : public SLArBaseDetModule {
  public: 
    SLArDetAnodeAssembly(); 
    ~SLArDetAnodeAssembly(); 

    SLArCfgAnode BuildAnodeConfig(); 
    void BuildMaterial(G4String materials_db); 
    void BuildAnodeAssembly(SLArDetReadoutTileAssembly*); 

    inline const G4ThreeVector& GetNormal() {return fNormal;}
    inline const G4ThreeVector& GetPosition() {return fPosition;}
    inline G4RotationMatrix* GetRotation() {return fRotation;}
    inline SLArBaseDetModule* GetTileAssemblyRow() {return fAnodeRow;}
    inline G4String GetTileAssemblyModel() {return fTileAssemblyModel;}
    inline G4int GetTPCID() {return fTPCID;}

    virtual void Init(const rapidjson::Value&) override; 

  private: 
    G4int fTPCID;
    SLArMaterial* fMatAnode; 
    SLArBaseDetModule* fAnodeRow;
    G4ThreeVector fPosition; 
    G4ThreeVector fNormal; 
    G4RotationMatrix* fRotation; 
    G4String fTileAssemblyModel;
    void SetupAnodePlaneAxes( SLArCfgAnode& );
}; 


#endif /* end of include guard SLARDETANODEASSEMBLY_HH */

