/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetectorConstruction.hh
 * @created     Wed Nov 16, 2022 09:42:24 CET
 */

#ifndef SLArDetectorConstruction_h
#define SLArDetectorConstruction_h 

#include "detector/TPC/SLArDetTPC.hh"
#include "detector/TPC/SLArDetCryostat.hh"
#include "detector/TPC/SLArDetCathode.hh"
#include "detector/SuperCell/SLArDetSuperCell.hh"
#include "detector/SuperCell/SLArDetSuperCellArray.hh"
#include "detector/Anode/SLArDetReadoutTile.hh"
#include "detector/Anode/SLArDetReadoutTileAssembly.hh"
#include "detector/Anode/SLArDetAnodeAssembly.hh"

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
    void ConstructTarget(); 
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
    //! Return TPC with given id
    SLArDetTPC* GetDetTPC(G4int tpcid);

    //! Build SuperCell object and place the SuperCells according to the given configuration
    void BuildAndPlaceSuperCells();
    //! Build the ReadoutTile object and the place the MegaTiles according to the given configuration
    void BuildAndPlaceAnode();
    //! Get the World's logical volume
    G4LogicalVolume*                GetLogicWorld();
    G4VPhysicalVolume*              GetPhysicalWorld() {return fWorldPhys;} 
    std::vector<G4VPhysicalVolume*>&GetVecSuperCellPV();
    //!  Return the geometry configuration file
    G4String                        GetGeometryCfgFile() {return fGeometryCfgFile;}
    //!  Return the material configuration file
    G4String                        GetMaterialCfgFile() {return fMaterialDBFile;}
    void                            DumpSuperCellMap(G4String path = "");
    //! Construct scorers in the cryostat layers for neutron shielding studies
    void                            ConstructCryostatScorer(); 
    //! Set anode visualization attributes 
    void                            SetAnodeVisAttributes(const int depth = 0); 

  private:
    //! Detector description initilization
    void Init();
    G4String fGeometryCfgFile; //!< Geometry configuration file
    G4String fMaterialDBFile;  //!< Material table file
    //! vector of visualization attributes
    std::vector<G4VisAttributes*>   fVisAttributes; 

    //! TPC detector object (cryostat + LAr target)
    SLArBaseDetModule* fDetector;
    SLArDetCryostat* fCryostat; 
    std::map<int, SLArDetTPC*> fTPC;
    std::map<int, SLArDetCathode*> fCathode; 

    SLArGeoInfo fWorldGeoPars;//!< World volume geometry parameters
    SLArGeoInfo fCavernGeoPars; //!< Cavern volume geometry attributes
    SLArDetSuperCell* fSuperCell; //!< SuperCell detector object
    std::map<int, SLArDetSuperCellArray*> fSCArray;
    SLArDetReadoutTile* fReadoutTile; //!< ReadoutTile detector object
    std::map<int, SLArDetAnodeAssembly*> fAnodes; 
    std::map<G4String, SLArDetReadoutTileAssembly*> fReadoutMegaTile; 

    G4LogicalVolume* fWorldLog; //!< World logical volume
    G4VPhysicalVolume* fWorldPhys; //!< World physical volume
    G4VPhysicalVolume* fCavernPhys;//!< Cavern physical volume
    std::vector<G4VPhysicalVolume*> fSuperCellsPV;
    G4String GetFirstChar(G4String line);
    
    //! Construct Cavern
    void ConstructCavern(); 
    //! Parse the description of the supercell detector system
    void InitSuperCell(const rapidjson::Value&); 
    //! Parse the description of the SC PDS
    void InitPDS(const rapidjson::Value&);
    //! Parse the description of the ReadoutTile detector system
    void InitReadoutTile(const rapidjson::Value&); 
    //! Parse the description of the ReadoutTile detector system
    void InitAnode(const rapidjson::Value&);
    //! Parse the description of the TPC volumes
    void InitTPC(const rapidjson::Value&); 
    //! Parse the description of the cathode elements
    void InitCathode(const rapidjson::Value&); 
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif /*SLArDetectorConstruction_h*/
