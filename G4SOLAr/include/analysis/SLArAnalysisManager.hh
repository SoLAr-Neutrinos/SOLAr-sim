/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
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
#include "event/SLArMCTruth.hh"
#include "event/SLArEventAnode.hh"
#include "event/SLArEventSuperCellArray.hh"
#include "event/SLArGenRecords.hh"

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
    G4bool LoadPDSCfg(SLArCfgSystemSuperCell&  pdsCfg );
    G4bool LoadAnodeCfg(SLArCfgAnode&  pixCfg );
    G4bool FillTree();
    G4bool FillGenTree(); 
    void   SetOutputPath(G4String path);
    void   SetOutputName(G4String filename);
    inline void EnableMCTruthOutput(const bool enable) {fEnableMCTruthOutput = enable;}
    inline void EnableEventAnodeOutput(const bool enable) {fEnableEventAnodeOutput = enable;}
    inline void EnableEventPDSOutput(const bool enable) {fEnableEventPDSOutput = enable;}
    inline bool IsMCTruthOutputEnabled() const {return fEnableMCTruthOutput;}
    inline bool IsAnodeOutputEnabled() const {return fEnableEventAnodeOutput;}
    inline bool IsPDSOutputEnabled() const {return fEnableEventPDSOutput;}
    void   WriteSysCfg();
    bool   IsPathValid(G4String path);
    template<typename T> 
      inline int WriteVariable(G4String name, T val) {
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
    int WriteArray(G4String name, G4int size, G4double* val); 
    int WriteCfgFile(G4String name, const char* path); 
    int WriteCfg(G4String name, const char* cfg); 
    int WriteCrossSection(SLArXSecDumpSpec xsec_spec); 

    // Access and I/O methods
    backtracker::SLArBacktrackerManager* GetBacktrackerManager(const G4String sys);
    backtracker::SLArBacktrackerManager* GetBacktrackerManager(const backtracker::EBkTrkReadoutSystem isys);
    void SetupBacktrackerRecords(); 
    inline TTree* GetEventTree() const {return  fEventTree;}
    inline TTree* GetGenRecordsTree() const {return  fGenTree;}

    inline TFile* GetFile() const {return   fRootFile;}
    inline SLArCfgSystemSuperCell& GetPDSCfg() {return  fPDSysCfg;}
    inline std::map<int, SLArCfgAnode>& GetAnodeCfg() {return fAnodeCfg;}
    inline SLArCfgAnode& GetAnodeCfgByTPC(const int tpc_id) {
      if ( fAnodeCfg.count(tpc_id) ) return fAnodeCfg[tpc_id];
      else {
        char error_msg[100]; 
        std::fprintf(stderr, "No Anode associated with TPC id %i found in register. abort.\n\n", tpc_id);
        exit(EXIT_FAILURE);
      }
    }
    inline SLArCfgAnode& GetAnodeCfgByID(const int anode_id) {
      for (auto& anode_cfg : fAnodeCfg) {
        if (anode_cfg.second.GetIdx() == anode_id) {
          return anode_cfg.second;
        }
      }
      std::fprintf(stderr, "No anode with id %i found in register. abort.\n\n", anode_id); 
      exit(EXIT_FAILURE);
    }
    inline const std::map<G4String, G4double>& GetPhysicsBiasingMap() {return fBiasing;}
    inline const std::vector<SLArXSecDumpSpec>& GetXSecDumpVector() {return fXSecDump;}
    inline void SetEventNumber(Int_t event_number) {
      fEventNumber = event_number;
      fListMCPrimary.SetEventNumber(event_number);
      fListEventAnode.SetEventNumber(event_number);
      fListEventPDS.SetEventNumber(event_number);
      fListGenRecords.SetEventNumber(event_number);
    }
    inline Int_t GetEventNumber() const {return fEventNumber;}
    inline SLArMCTruth& GetMCTruth()  {return fListMCPrimary;}
    inline SLArListEventAnode& GetEventAnode() {return fListEventAnode;}
    inline SLArListEventPDS& GetEventPDS() {return fListEventPDS;}
    inline SLArGenRecordsVector& GetGenRecords() {return fListGenRecords;}
    G4bool Save();
    inline void ResetEvent() {
      fListMCPrimary.Reset();
      fListEventAnode.Reset();
      fListEventPDS.Reset();
      fListGenRecords.Reset();
      fEventNumber = -1;
    }

    // mock fake access
    G4bool FakeAccess();
    void RegisterPhyicsBiasing(G4String particle_name, G4double biasing_factor);
    void RegisterXSecDump(const SLArXSecDumpSpec xsec_dump); 
    inline void SetXSecEmin (const double emin) {fXSecEmin = emin;}
    inline void SetXSecEmax (const double emax) {fXSecEmax = emax;}
    inline void SetXSecNPoints (const int npoints) {fXSecNPoints = npoints;}
    inline double GetXSEMin () const {return fXSecEmin;}
    inline double GetXSEMax () const {return fXSecEmax;}
    inline int GetXSNPoints () const {return fXSecNPoints;}
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
    G4double fXSecEmin = 0.01;
    G4double fXSecEmax = 20;
    G4int fXSecNPoints = 1000;

    TFile* fRootFile = {};
    TTree* fEventTree = {}; 
    TTree* fGenTree = {};
    bool   fEnableMCTruthOutput = true; 
    bool   fEnableEventAnodeOutput = true;
    bool   fEnableEventPDSOutput = true;
    bool   fEnableGenTreeOutput = true;
    Int_t  fEventNumber = 0;
    SLArMCTruth fListMCPrimary;
    SLArGenRecordsVector fListGenRecords; 
    SLArListEventPDS fListEventPDS;
    SLArListEventAnode fListEventAnode;

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

