/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetFieldCage.hh
 * @created     : Tuesday Apr 28, 2026 14:24:37 CEST
 */

#ifndef SLARDETFIELDCAGE_HH

#define SLARDETFIELDCAGE_HH

#include "detector/SLArBaseDetModule.hh"
#include "G4ThreeVector.hh"

class SLArDetFieldCage : public SLArBaseDetModule {
  public:
    SLArDetFieldCage();
    ~SLArDetFieldCage();

    /**
     * Resolve all geometry parameters.
     * Explicit JSON values take priority; tpcGeoInfo supplies defaults.
     * Must be called before Build().
     *
     * @param jconf       "field_cage" JSON node
     * @param shape       shape of the parent TPC
     * @param driftDir    electron drift direction unit vector
     * @param tpcGeoInfo  parent TPC's GeoInfo (for default dimensions)
     */
    virtual void Init(const rapidjson::Value& jconf) override;

    /**
     * Build solids, logical volumes, and parameterised placement.
     *
     * @param matConductor  material for the conducting electrodes
     * @param matFill       material filling the cage envelope (typically LAr)
     */
    void Build();

    inline void BuildMaterial(const G4String& db_file = "") 
    {
      fMatFill = new SLArMaterial();
      fMatFill->SetMaterialID(fMatInfo.GetMaterial("base_material"));
      fMatFill->BuildMaterialFromDB(db_file);

      fMatConductor = new SLArMaterial();
      fMatConductor->SetMaterialID(fMatInfo.GetMaterial("conductor_material"));
      fMatConductor->BuildMaterialFromDB(db_file);
    }

    geo::EGeoShape        GetShape()    const { return fShape; }
    const G4ThreeVector&  GetShift()    const { return fShift; }
    void SetVisAttributes(G4bool visible);

  private:
    geo::EGeoShape    fShape;
    G4ThreeVector     fShift;        // offset from TPC centre at placement
    G4ThreeVector     fDriftDir;     // cached from Init(), used in Build()

    SLArMaterial* fMatConductor;  // material for the conducting electrodes
    SLArMaterial* fMatFill;       // material filling the cage envelope (typically LAr)

    void BuildBox();
    void BuildTub();

    // Helper: find drift and transverse axis indices
    void ResolveDriftAxes(int& iDrift, int& iTrans1, int& iTrans2) const;
};



#endif /* end of include guard SLARDETFIELDCAGE_HH */

