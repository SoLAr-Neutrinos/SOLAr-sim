/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArAnalysisManager.hh
 * @created     Wed Feb 12, 2020 15:03:53 CET
 * @brief       Custom SoLAr-sim Analysis Manager
 *
 * Custom analysis manager reimplemented from 
 * G4RootAnalysisManager
 */

#ifndef SLArANALYSISMANAGER_HH

#define SLArANALYSISMANAGER_HH

#include <cstdio>
#include <stdexcept>
#include "TFile.h"
#include "TTree.h"
#include "TParameter.h"

#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgBaseSystem.hh"
#include "config/SLArCfgMegaTile.hh"
#include "config/SLArCfgSuperCellArray.hh"
#include "event/SLArMCEvent.hh"

#include "SLArBacktrackerManager.hh"
#include "SLArAnalysisManagerMsgr.hh"

#include "G4ToolsAnalysisManager.hh"
#include "globals.hh"


class SLArAnalysisManager 
{
  public:
    struct SLArXSecDumpSpec {
      G4String particle_name; 
      G4String process_name; 
      G4String material_name;
      G4bool   log_span;

      SLArXSecDumpSpec(); 
      SLArXSecDumpSpec(const G4String& par, const G4String& proc, const G4String& mat, const bool& do_log = false);
    };

    SLArAnalysisManager(G4bool isMaster);
    ~SLArAnalysisManager();

    // static methods
    static SLArAnalysisManager* Instance();
    static G4bool IsInstance();

    inline void SetSeed( const G4long myseed ) {fSeed = myseed;}
    inline G4long GetSeed() const {return fSeed;}

    void   ConstructBacktracker(const G4String readout_system); 
    void   ConstructBacktracker(const backtracker::EBkTrkReadoutSystem isys); 
    G4bool CreateEventStructure();
    G4bool CreateFileStructure();
    G4bool LoadPDSCfg         (SLArCfgSystemSuperCell&  pdsCfg );
    G4bool LoadAnodeCfg       (SLArCfgAnode&  pixCfg );
    G4bool FillEvTree         ();
    void   SetOutputPath      (G4String path);
    void   SetOutputName      (G4String filename);
    void   WriteSysCfg        ();
    bool   IsPathValid        (G4String path);
    template<typename T> 
      inline int WriteVariable (G4String name, T val) {
        if (!fRootFile) {
          printf("SLArAnalysisManager::WriteVariable WARNING ");
          printf("rootfile not present yet. Cannot write %s variable.\n", 
              name.c_str());
          return 666;
        } 

        TParameter<T> var(name, val); 
        fRootFile->cd(); 
        int status = var.Write(); 
        return status; 
    }
    int    WriteArray         (G4String name, G4int size, G4double* val); 
    int    WriteCfgFile       (G4String name, const char* path); 
    int    WriteCfg           (G4String name, const char* cfg); 
    int    WriteCrossSection  (SLArXSecDumpSpec xsec_spec); 

    // Access and I/O methods
    backtracker::SLArBacktrackerManager* GetBacktrackerManager(const G4String sys);
    backtracker::SLArBacktrackerManager* GetBacktrackerManager(const backtracker::EBkTrkReadoutSystem isys);
    void SetupBacktrackerRecords(); 
    TTree* GetTree() const {return  fEventTree;}
    TFile* GetFile() const {return   fRootFile;}
    SLArCfgSystemSuperCell& GetPDSCfg() {return  fPDSysCfg;}
    std::map<int, SLArCfgAnode>& GetAnodeCfg() {return fAnodeCfg;}
    inline SLArCfgAnode& GetAnodeCfg(int id) {
      if ( fAnodeCfg.count(id) ) return fAnodeCfg[id];
      else {
        char error_msg[100]; 
        std::sprintf(error_msg, "No Anode with id %i found in register. abort.\n\n", id);
        throw std::runtime_error(error_msg); 
      }
    }
    inline const std::map<G4String, G4double>& GetPhysicsBiasingMap() {return fBiasing;}
    inline const std::vector<SLArXSecDumpSpec>& GetXSecDumpVector() {return fXSecDump;}
    SLArMCEvent& GetEvent()  {return fMCEvent;}
    G4bool Save ();

    // mock fake access
    G4bool FakeAccess();
    void RegisterPhyicsBiasing(G4String particle_name, G4double biasing_factor);
    void RegisterXSecDump(const SLArXSecDumpSpec xsec_dump); 
    inline void SetStoreTrajectoryFull(const bool store_trj_pts) {fTrajectoryFull = store_trj_pts;} 
    inline G4bool StoreTrajectoryFull() const {return fTrajectoryFull;}

    SLArAnalysisManagerMsgr* fAnaMsgr;
#ifdef SLAR_EXTERNAL
    void SetupExternalsTree(); 
    inline TTree* GetExternalsTree() {return fExternalsTree;}
    inline SLArEventTrajectoryLite& GetExternalRecord() {return fExternalRecord;}
#endif 

  protected:
    // virtual functions (overriden in MPI implementation)

  private:
    // static data members
    static SLArAnalysisManager*               fgMasterInstance;
    static G4ThreadLocal SLArAnalysisManager* fgInstance;    

    // data members 
    G4bool   fIsMaster;
    G4long   fSeed; 
    G4String fOutputPath;
    G4String fOutputFileName;
    G4bool   fTrajectoryFull;
    std::map<G4String, G4double> fBiasing; 
    std::vector<SLArXSecDumpSpec> fXSecDump;

    TFile* fRootFile;
    TTree* fEventTree;
    SLArMCEvent  fMCEvent;
#ifdef SLAR_EXTERNAL
    SLArEventTrajectoryLite fExternalRecord;
    TTree* fExternalsTree;
#endif 

    backtracker::SLArBacktrackerManager* fSuperCellBacktrackerManager;
    backtracker::SLArBacktrackerManager* fVUVSiPMBacktrackerManager;
    backtracker::SLArBacktrackerManager* fChargeBacktrackerManager;

    SLArCfgSystemSuperCell fPDSysCfg;
    std::map<int, SLArCfgAnode> fAnodeCfg;
};

#endif /* end of include guard SLArANALYSISMANAGER_HH */

