// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file SLAr/include/SLArPrimaryGeneratorAction.hh
/// \brief Definition of the SLArPrimaryGeneratorAction class
//
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef SLArPrimaryGeneratorAction_h
#define SLArPrimaryGeneratorAction_h 1

#include <G4VUserPrimaryGeneratorAction.hh>
#include <SLArBaseGenerator.hh>
#include <G4VPhysicalVolume.hh>
#include <globals.hh>

#include <rapidjson/document.h>

class G4Event;
namespace gen {
  class SLArBulkVertexGenerator;
  class SLArBoxSurfaceVertexGenerator;
  class SLArPrimaryGeneratorMessenger;
  class SLArPGunGeneratorAction; 
  class SLArPBombGeneratorAction; 
  //class SLArBackgroundGeneratorAction;
  class SLArExternalGeneratorAction;
  class SLArGENIEGeneratorAction;//--JM


  namespace bxdecay0_g4 {
    class SLArDecay0GeneratorAction;
  }
  namespace marley {
    class SLArMarleyGeneratorAction;
  }
#ifdef SLAR_CRY
  namespace cry {
    class SLArCRYGeneratorAction;
  }
#endif 

#ifdef SLAR_RADSRC
  namespace radsrc {
    class SLArRadSrcGeneratorAction;
  }
#endif

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
  class SLArPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
  {

    public:
      SLArPrimaryGeneratorAction();
      SLArPrimaryGeneratorAction(const rapidjson::Value& config); 
      SLArPrimaryGeneratorAction(const G4String config_file_path); 
      ~SLArPrimaryGeneratorAction();

      virtual void GeneratePrimaries(G4Event*);
      void AddGenerator(const rapidjson::Value& generatorConfig);
      void Configure(const rapidjson::Value& config); 
      void Configure(const G4String config_file_path); 

      inline bool DoTraceOptPhotons() {return fDoTraceOptPhotons;}
      inline bool DoDriftElectrons() {return fDoDriftElectrons;}
      inline void SetTraceOptPhotons(bool do_trace) {fDoTraceOptPhotons = do_trace;}
      inline void SetDriftElectrons(bool do_drift) {fDoDriftElectrons = do_drift;}

      //inline G4String GetMarleyConf() {return fMarleyCfg;}
      //inline EDirectionMode GetDirectionMode() {return fDirectionMode;}
      //inline void SetDirectionMode(EDirectionMode kMode) {fDirectionMode = kMode;}
      //inline EGenerator GetGeneratorIndex() const {return fGeneratorEnum;}
 
      //inline void SetGenerator(EGenerator gen) {fGeneratorEnum = gen;}
      inline G4VUserPrimaryGeneratorAction* GetGenerator(const G4String& label) {
        if (fGeneratorActions.find(label) != fGeneratorActions.end()) {
          return fGeneratorActions[label];
        } 
        return nullptr;
      }
      inline std::map<G4String, SLArBaseGenerator*>& GetGenerators() {return fGeneratorActions;}
      //inline G4ThreeVector GetSourcePosition() const {return fGenPosition;}
      //inline void SetSourcePosition(G4ThreeVector pos) {fGenPosition = pos;}
      //inline G4ThreeVector GetGunDirection() const {return fGenDirection;}
      //inline void SetEventDirection(G4ThreeVector dir) {fGenDirection = dir;}
      //void ConfigBulkVertexGen(G4String vol);
      //void ConfigBoxSurfaceVertexGen(G4String vol); 
      //void SetBackgroundConf(G4String background_conf); 
      //void SetExternalConf(G4String externals_conf); 
      //void SetMarleyConf(G4String marley_conf); 

      //void SetGunEnergy(const G4double ekin); 
      //void SetGunParticle(const G4String particle_name); 
      //void SetGunNumberOfParticles(const G4int n_particles);

      //void SetGENIEEvntID(G4Event *ev, G4int evntID); //--JM
      //void SetGENIEEvntExt(G4int evntID); //--JM
      //void SetGENIEFile(G4String filename); //--JM

#ifdef SLAR_CRY
      void AppendCRYInput(const G4String); 
#endif

      void SetVerboseLevel( G4int verbose) { 
        fVerbose = verbose; 
        for (auto& gen : fGeneratorActions) {
          gen.second->SetVerboseLevel( fVerbose ); 
        }
      }
      inline G4int GetVerboseLevel() const {return fVerbose;}

    private:
      std::map<G4String, SLArBaseGenerator*> fGeneratorActions; 

      SLArPrimaryGeneratorMessenger* fGunMessenger;

      //SLArBulkVertexGenerator* fBulkGenerator;
      //SLArBoxSurfaceVertexGenerator* fBoxGenerator; 

      //EDirectionMode fDirectionMode; 
      //G4String       fVolumeName;
      //G4String       fMarleyCfg; 

      //EGenerator     fGeneratorEnum;
      //G4ThreeVector  fGenPosition;
      //G4ThreeVector  fGenDirection;
      //G4double       fGenEnergy; 
      //G4int          fGenNumberOfParticles;
      //bool           fIncludeBackground; 
      //G4String       fBackgoundModelCfg;

      G4bool fDoDriftElectrons;
      G4bool fDoTraceOptPhotons;

      //G4int fGENIEEvntNum;
      //G4String fGENIEFile;

      //G4String fCRYCfg; 
      G4int fVerbose;

      void Reset(); 

      friend class gen::SLArPrimaryGeneratorMessenger;
  };
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif /*SLArPrimaryGeneratorAction_h*/
