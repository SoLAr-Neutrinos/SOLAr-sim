/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArDetReadoutTileAssemblyAssembly.hh
 * @created     : Mon Jul 18, 2022 11:09:54 CEST
 */

#ifndef SLArDetReadoutTileAssemblyASSEMBLY_HH

#define SLArDetReadoutTileAssemblyASSEMBLY_HH

#include "detector/SLArBaseDetModule.hh"

class SLArDetReadoutTile;

class SLArDetReadoutTileAssembly : public SLArBaseDetModule 
{       
  public:
    SLArDetReadoutTileAssembly(); 
    SLArDetReadoutTileAssembly(const SLArDetReadoutTileAssembly& detReadoutPlane); 
    ~SLArDetReadoutTileAssembly(); 

    void BuildMaterial(G4String materials_db); 
    void BuildDefalutGeoParMap(); 
    void BuildReadoutPlane(SLArDetReadoutTile*); 
    SLArBaseDetModule* GetTileRow() {return fTileRow;}
    virtual void Init(const rapidjson::Value&) override {}
    void SetVisAttributes(const int depth = 0);
  protected:

  private: 
    SLArMaterial* fMatReadoutPlane; 
    SLArBaseDetModule* fTileRow;
    void BuildTileRow(SLArDetReadoutTile*);
};

#endif /* end of include guard SLArDetReadoutTileAssembly_HH */

