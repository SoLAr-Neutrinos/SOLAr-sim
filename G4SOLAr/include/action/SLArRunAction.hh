/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArRunAction
 * @created     : venerdì nov 04, 2022 09:26:25 CET
 */

#ifndef SLArRunAction_h
#define SLArRunAction_h 1

#include "physics/SLArElectronDrift.hh"
#include "physics/SLArFastLightSim.hh"
#include "physics/SLArFastLightSimMessenger.hh"
#include "SLArRandomExtra.hh"

#include "G4UserRunAction.hh"
#include "G4Transform3D.hh"
#include "globals.hh"

class SLArEventAction;
class G4LogicalVolume;

class G4Run;

/// Run action class

class SLArRunAction : public G4UserRunAction
{
  public:
    SLArRunAction();
    virtual ~SLArRunAction();

    virtual G4Run* GenerateRun();
    virtual void   BeginOfRunAction(const G4Run*);
    virtual void   EndOfRunAction(const G4Run*);
    inline SLArElectronDrift* GetElectronDrift() {return fElectronDrift;}
    inline G4String GetG4MacroFile() const {return fG4MacroFile;}
    inline void SetG4MacroFile(const G4String file_path) {fG4MacroFile = file_path;}
    inline void RegisterExtScorerLV(G4LogicalVolume* lv) {fExtScorerLV.push_back(lv);}
    inline SLArRandom* GetTRandomInterface() {return fTRandomInterface;}
    inline const G4Transform3D& GetTransformWorld2Det() const {return fTransformWorld2Det;}
    void SetFastLightSimConfig(const G4String& configPath);
    void SetFastLightSimulatorType(const G4String& type);
    void EnableFastLightSim(G4bool enable);
    G4String GetFastLightSimConfigPath() const { return fFLSConfigPath; }
    G4bool IsFastLightSimEnabled() const { return fFastLightSimEnabled; }
    SLArFastLightSimDispatcher* GetFastLightSimDispatcher() const { return fFastLightSimDispatcher.get(); }

  private:
    void InitializeFastLightSim();

    G4String fG4MacroFile = {}; 
    SLArEventAction* fEventAction = {};
    SLArElectronDrift* fElectronDrift = {}; 
    SLArRandom* fTRandomInterface = {};
    G4Transform3D fTransformWorld2Det;

    std::vector<G4String> fSDName;  
    std::vector<G4LogicalVolume*> fExtScorerLV; 

    std::unique_ptr<SLArFastLightSimDispatcher> fFastLightSimDispatcher;
    std::unique_ptr<SLArFastLightSimMessenger> fFastLightSimMessenger;
    
    G4String fFLSConfigPath;
    G4bool fFastLightSimEnabled;

};


#endif
