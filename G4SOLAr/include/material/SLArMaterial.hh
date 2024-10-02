/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArMaterialInfo.hh
 * @created     : lunedì ago 08, 2022 17:45:44 CEST
 */

#ifndef SLARMATERIALINFO_H

#define SLARMATERIALINFO_H

#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4OpWLS.hh"
#include "G4OpticalSurface.hh"
#include "rapidjson/document.h"

class SLArMaterial {
  public:
    SLArMaterial();
    SLArMaterial(const SLArMaterial &mat);
    SLArMaterial(G4String matID);
    ~SLArMaterial();

    void                BuildMaterialFromDB(const G4String db_file, G4String mat_id = "");
    G4Material*         GetMaterial();
    G4MaterialPropertiesTable* GetMaterialPropTable(); 
    G4String            GetMaterialID();
    void                SetMaterialID(G4String matID) {fMaterialID = matID;}
    G4OpticalSurface*   GetMaterialOpticalSurf() {return fOpticalSurf;}

    static G4Material*  FindInMaterialTable(const char* mname); 

  protected:
    G4Material*         ParseMaterial(const rapidjson::Value& jmaterial);
    G4Material*         ParseMaterialDB(G4String mat_id); 
    void                ParseSurfaceProperties(const rapidjson::Value& jptable);
    void                ParseMPT(const rapidjson::Value& jptable, 
        G4MaterialPropertiesTable* mpt, G4Material* mat = nullptr);
    G4double            ParseUnit(const rapidjson::Value& junit); 
    G4Material*         BuildFromNist(const rapidjson::Value& jptable); 
    G4Material*         BuildFromAtoms(const rapidjson::Value& jptable); 
    G4Material*         BuildFromMixture(const rapidjson::Value& jptable); 
    //G4Material*         fPEN = nullptr;
  private:
  
    
    void CreateMaterials();
    
    G4String            fDBFile     ; 
    G4String            fMaterialID ;
    G4Material*         fMaterial   ;
    G4OpticalSurface*   fOpticalSurf;
    //G4NistManager* fNistMan = nullptr;
    //G4Material* fMatPEN = nullptr;
};


#endif /* end of include guard SLArMATERIAL_H */

