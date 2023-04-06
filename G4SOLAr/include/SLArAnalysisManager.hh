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

#include "TFile.h"
#include "TTree.h"

#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgBaseSystem.hh"
#include "config/SLArCfgMegaTile.hh"
#include "config/SLArCfgSuperCellArray.hh"
#include "event/SLArMCEvent.hh"

#include "SLArAnalysisManagerMsgr.hh"

#include "G4ToolsAnalysisManager.hh"
#include "globals.hh"


class SLArAnalysisManager 
{
  public:
    SLArAnalysisManager(G4bool isMaster);
    ~SLArAnalysisManager();

    // static methods
    static SLArAnalysisManager* Instance();
    static G4bool IsInstance();

    G4bool CreateFileStructure();
    G4bool LoadPDSCfg         (SLArCfgSystemSuperCell*  pdsCfg );
    G4bool LoadAnodeCfg       (SLArCfgAnode*  pixCfg );
    G4bool FillEvTree         ();
    void   SetOutputPath      (G4String path);
    void   SetOutputName      (G4String filename);
    void   WriteSysCfg        ();
    bool   IsPathValid        (G4String path);
    int    WriteVariable      (G4String name, G4double val); 
    int    WriteArray         (G4String name, G4int size, G4double* val); 
    int    WriteCfgFile       (G4String name, const char* path); 
    int    WriteCfg           (G4String name, const char* cfg); 

    // Access and I/O methods
    TTree* GetTree() const {return  fEventTree;}
    TFile* GetFile() const {return   fRootFile;}
    SLArCfgSystemSuperCell* GetPDSCfg() {return  fPDSysCfg;}
    std::map<int, SLArCfgAnode*>& GetAnodeCfg() {return fAnodeCfg;}
    inline SLArCfgAnode* GetAnodeCfg(int id) {
      SLArCfgAnode* anodeCfg = nullptr;
      if ( fAnodeCfg.count(id) ) anodeCfg = fAnodeCfg[id];
      return anodeCfg;}

    SLArMCEvent* GetEvent()  {return    fMCEvent;}
    G4bool Save ();

    // mock fake access
    G4bool FakeAccess();

    SLArAnalysisManagerMsgr* fAnaMsgr;

  protected:
    // virtual functions (overriden in MPI implementation)

  private:
    // static data members
    static SLArAnalysisManager*               fgMasterInstance;
    static G4ThreadLocal SLArAnalysisManager* fgInstance;    

    // data members 
    G4bool   fIsMaster;
    G4String fOutputPath;
    G4String fOutputFileName;

    TFile*              fRootFile;
    TTree*              fEventTree;
    SLArMCEvent*        fMCEvent;

    SLArCfgSystemSuperCell* fPDSysCfg;
    std::map<int, SLArCfgAnode*> fAnodeCfg;
};

#endif /* end of include guard SLArANALYSISMANAGER_HH */

