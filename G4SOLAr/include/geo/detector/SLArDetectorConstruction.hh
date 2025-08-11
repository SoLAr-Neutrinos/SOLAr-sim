/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetectorConstruction.hh
 * @created     Wed Nov 16, 2022 09:42:24 CET
 */

#ifndef SLArDetectorConstruction_h
#define SLArDetectorConstruction_h 

#include "detector/Hall/SLArDetExpHall.hh"
#include "detector/TPC/SLArDetTPC.hh"
#include "detector/TPC/SLArDetCryostat.hh"
#include "detector/TPC/SLArDetCathode.hh"
#include "detector/SuperCell/SLArDetSuperCell.hh"
#include "detector/SuperCell/SLArDetSuperCellArray.hh"
#include "detector/Anode/SLArDetReadoutTile.hh"
#include "detector/Anode/SLArDetReadoutTileAssembly.hh"
#include "detector/Anode/SLArDetAnodeAssembly.hh"
#include "physics/LiquidArgon/SLArLArProperties.hh"

#include "SLArAnalysisManagerMsgr.hh"

#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4MaterialPropertyVector.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4VisAttributes.hh"

#include "G4IStore.hh"
#include "G4VIStore.hh"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class SLArCfgSuperCellArray;

/**
 * @brief SoLAr detector construction class
 *
 * This class constructs the SoLAr detector building its individual 
 * components (cryostat, target, readout system) and placing them 
 * in the world volume. 
 */
class SLArDetectorConstruction : public G4VUserDetectorConstruction
{
  friend class SLArAnalysisManagerMsgr;

  public:
    //! Constructor
    SLArDetectorConstruction(G4String, G4String);
    //! Destructor
    virtual ~SLArDetectorConstruction();

  public:
    
    //! Construct world and place detectors
    virtual G4VPhysicalVolume* Construct();
    //! Construct Target
    void ConstructTarget(const rapidjson::Value& d); 
    //! Construct Cathode
    void ConstructCathode();
    //! Construct Cryostat
    void ConstructCryostat(); 
    //! Construct Sensitive Detectors and cryostat scorers
    virtual void ConstructSDandField();
    //! Construct virtual pixelization of the anode readout system
    void ConstructAnodeMap(); 
    G4VIStore* CreateImportanceStore();
    //! Return SLArDetectorConstruction::fTPCs map
    inline std::map<G4int, SLArDetTPC*>& GetDetTPCs() {return fTPC;}
    //! Return ReadoutTile detector object
    inline SLArDetReadoutTile* GetReadoutTile(const G4String& tile_model_name) {
      auto it = fReadoutTile.find(tile_model_name);
      if (it != fReadoutTile.end()) {
        return it->second;
      } else {
        return nullptr;
      }
    }
    //! Return ReadoutTile detector object
    inline SLArDetReadoutTile* GetReadoutTile(const G4String& tile_model_name) const {
      auto it = fReadoutTile.find(tile_model_name);
      if (it != fReadoutTile.end()) {
        return it->second;
      } else {
        return nullptr;
      }
    }
    //! Return Readout tile model map
    inline std::map<G4String, SLArDetReadoutTile*>& GetReadoutTileMap() {return fReadoutTile;}
    //! Return readout tile model map
    inline const std::map<G4String, SLArDetReadoutTile*>& GetReadoutTileMap() const {return fReadoutTile;}
    //! Return TPC with given id
    SLArDetTPC* GetDetTPC(G4int tpcid);
    //! Build SuperCell object and place the SuperCells according to the given configuration
    void BuildAndPlaceSuperCells();
    //! Build the ReadoutTile object and the place the MegaTiles according to the given configuration
    void BuildAndPlaceAnode();
    //! Get the World's logical volume
    G4LogicalVolume*                GetLogicWorld();
    //! Get the World's physical volume
    inline G4VPhysicalVolume*              GetPhysicalWorld() {return fWorldPhys;} 
    //! Get the vector containing the SuperCell Physical Volumes
    inline std::vector<G4VPhysicalVolume*>&GetVecSuperCellPV() {return fSuperCellsPV;}
    //! Get the vector containing the Physical Volumes of volumes set as ExtScorer
    inline std::vector<G4VPhysicalVolume*>&GetVecExtScorerPV() {return fExtScorerPV;}
    //! Get the LAr target volume
    inline SLArBaseDetModule* GetLArTargetVolume() {return fDetector;}
    //! Get the LAr target volume 
    inline const SLArBaseDetModule* GetLArTargetVolume() const {return fDetector;}
    //!  Return the geometry configuration file
    G4String                        GetGeometryCfgFile() {return fGeometryCfgFile;}
    //!  Return the material configuration file
    G4String                        GetMaterialCfgFile() {return fMaterialDBFile;}
    //!  Return Liquid Argon Properties
    SLArLArProperties&              GetLArProperties() {return fLArProperties;}
    //!  Return Liquid Argon Properties
    const SLArLArProperties&        GetLArProperties() const {return fLArProperties;}
    void                            DumpSuperCellMap(G4String path = "");
    //! Construct scorers in the cryostat layers for neutron shielding studies
    void                            ConstructCryostatScorer(); 
    //! Set anode visualization attributes 
    void                            SetAnodeVisAttributes(const int depth = 0); 
    //! Add External Scorer Volume
    void                            AddExternalScorer(const G4String phys_volume_name, const G4String alias);

  private:
    //! Detector description initilization
    void Init();
    G4String fGeometryCfgFile; //!< Geometry configuration file
    G4String fMaterialDBFile;  //!< Material table file
    SLArLArProperties fLArProperties; //!< Liquid Argon Properties
    //! vector of visualization attributes
    std::vector<G4VisAttributes*>   fVisAttributes; 

    //! TPC detector object (cryostat + LAr target)
    SLArBaseDetModule* fDetector;
    SLArDetCryostat* fCryostat; 
    std::map<int, SLArDetTPC*> fTPC;
    std::map<int, SLArDetCathode*> fCathode; 

    SLArGeoInfo fWorldGeoPars;//!< World volume geometry parameters
    SLArGeoInfo fCavernGeoPars; //!< Cavern volume geometry attributes
    SLArDetExpHall* fExpHall; //!< Experimental Hall detector object
    SLArDetSuperCell* fSuperCell; //!< SuperCell detector object
    std::map<int, SLArDetSuperCellArray*> fSCArray;
    std::map<G4String,SLArDetReadoutTile*> fReadoutTile; //!< ReadoutTile detector object map
    std::map<int, SLArDetAnodeAssembly*> fAnodes; 
    std::map<G4String, SLArDetReadoutTileAssembly*> fReadoutMegaTile; 

    G4LogicalVolume* fWorldLog; //!< World logical volume
    G4VPhysicalVolume* fWorldPhys; //!< World physical volume
    std::vector<G4VPhysicalVolume*> fSuperCellsPV;
    std::vector<G4VPhysicalVolume*> fExtScorerPV;
    G4String GetFirstChar(G4String line);
    
    //! Construct Experimental Hall
    void ConstructExperimentalHall();
    //! Parse the description of the experimental hall
    void InitExpHall(const rapidjson::Value&);
    //! Parse the description of the supercell detector system
    void InitSuperCell(const rapidjson::Value&); 
    //! Parse the description of the SC PDS
    void InitPDS(const rapidjson::Value&);
    //! Parse the description of the ReadoutTile detector system
    void InitReadoutTiles(const rapidjson::Value&); 
    //! Parse the description of the ReadoutTile detector system
    void InitAnode(const rapidjson::Value&);
    //! Parse the description of the TPC volumes
    void InitTPC(const rapidjson::Value&); 
    //! Parse the description of the cathode elements
    void InitCathode(const rapidjson::Value&); 
    //! Setup the readout tile detector element
    void SetupReadoutTile(const rapidjson::Value& jtile);
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif /*SLArDetectorConstruction_h*/
