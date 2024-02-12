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
/// \file SLAr/src/SLArPrimaryGeneratorAction.cc
/// \brief Implementation of the SLArPrimaryGeneratorAction class
//
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
#include "SLArAnalysisManager.hh"

#include "SLArPrimaryGeneratorAction.hh"
#include "SLArPrimaryGeneratorMessenger.hh"
#include "SLArBulkVertexGenerator.hh"
#include "SLArBoxSurfaceVertexGenerator.hh"
#include "SLArPGunGeneratorAction.hh"
#include "SLArPBombGeneratorAction.hh"
#include "SLArMarleyGeneratorAction.hh"
#include "SLArDecay0GeneratorAction.hh"
#include "SLArExternalGeneratorAction.hh"
#include "SLArBackgroundGeneratorAction.hh"
#include "SLArGENIEGeneratorAction.hh"//--JM

#include "Randomize.hh"
#include "SLArRandomExtra.hh"

#include "G4VSolid.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArPrimaryGeneratorAction::SLArPrimaryGeneratorAction()
 : G4VUserPrimaryGeneratorAction(), 
   fGeneratorActions(7, nullptr),//--JM Change 5->6
   fBulkGenerator(0), 
   fVolumeName(""), 
   fGeneratorEnum(kParticleGun), 
   fGunPosition(0, 0, 0),
   fGunDirection(0, 0, 1), 
   fDoTraceOptPhotons(true), 
   fDoDriftElectrons(true), 
   fVerbose(0)
{
  G4int n_particle = 1;
  fGeneratorActions[kParticleGun]= new SLArPGunGeneratorAction(n_particle); 
  fGeneratorActions[kParticleBomb]= new SLArPBombGeneratorAction(n_particle); 
  fGeneratorActions[kMarley]= new marley::SLArMarleyGeneratorAction(); 
  fGeneratorActions[kDecay0]= new bxdecay0_g4::SLArDecay0GeneratorAction(); 
  fGeneratorActions[kBackground] = new SLArBackgroundGeneratorAction(); 
  fGeneratorActions[kExternalGen] = new SLArExternalGeneratorAction(); 
  fGeneratorActions[kGENIE] = new SLArGENIEGeneratorAction();//--JM

  fBulkGenerator = new SLArBulkVertexGenerator(); 
  fBoxGenerator  = new SLArBoxSurfaceVertexGenerator(); 

  //create a messenger for this class
  fGunMessenger = new SLArPrimaryGeneratorMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArPrimaryGeneratorAction::~SLArPrimaryGeneratorAction()
{
  printf("Deleting SLArPrimaryGeneratorAction...\n");
  int igen = 0; 
  for (auto &gen : fGeneratorActions) {
    printf("igen = %i\n", igen);
    if (gen) {
      printf("Deleting gen %i\n", igen);
      if (igen == 0) {
        SLArPGunGeneratorAction* local = (SLArPGunGeneratorAction*)gen;
        delete local;
      }
      else if (igen == 1) {
        SLArPBombGeneratorAction* local = (SLArPBombGeneratorAction*)gen;
        delete local;
      }
      else if (igen == 2) {
        auto local = (bxdecay0_g4::SLArDecay0GeneratorAction*)gen;
        delete local;
      }
      else if (igen == 3) {
        auto local = (marley::SLArMarleyGeneratorAction*)gen;
        delete local;
      }
      else if (igen == 4) {
        auto local = (SLArBackgroundGeneratorAction*)gen;
        delete local;
      }
      //gen = nullptr;
    }
    igen++;
  }
  if (fBulkGenerator) delete fBulkGenerator;
  if (fBoxGenerator) delete fBoxGenerator;
  if (fGunMessenger) delete fGunMessenger;
  printf("DONE\n");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPrimaryGeneratorAction::SetBulkName(G4String vol) {
  fVolumeName = vol; 
  auto volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(fVolumeName); 
  if (volume == nullptr) {
    printf("SLArPrimaryGeneratorAction::SetBulkName(%s) WARNING\n", vol.c_str());
    printf("Unable to find %s in physical volume store.\n", vol.c_str());
  }

  fBulkGenerator->SetBulkLogicalVolume(volume->GetLogicalVolume()); 
  fBulkGenerator->SetSolidTranslation(volume->GetTranslation()); 
  fBulkGenerator->SetSolidRotation(volume->GetRotation()); 
  return;
}

void SLArPrimaryGeneratorAction::SetBoxName(G4String vol) {
  fVolumeName = vol; 
  auto volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(fVolumeName); 
  if (volume == nullptr) {
    printf("SLArPrimaryGeneratorAction::SetBoxName(%s) WARNING\n", vol.c_str());
    printf("Unable to find %s in physical volume store.\n", vol.c_str());
  }

  fBoxGenerator->SetBoxLogicalVolume(volume->GetLogicalVolume()); 
  fBoxGenerator->SetSolidTranslation(volume->GetTranslation()); 
  fBoxGenerator->SetSolidRotation(volume->GetRotation()); 
  return;
}


void SLArPrimaryGeneratorAction::SetGunParticle(const G4String particle_name) 
{
  SLArPGunGeneratorAction* particle_gun = 
    (SLArPGunGeneratorAction*)fGeneratorActions[kParticleGun]; 
  particle_gun->SetParticle(particle_name); 

  SLArPBombGeneratorAction* particle_bomb = 
    (SLArPBombGeneratorAction*)fGeneratorActions[kParticleBomb]; 
  particle_bomb->SetParticle(particle_name); 
}

void SLArPrimaryGeneratorAction::SetGunEnergy(const G4double ekin) 
{
  SLArPGunGeneratorAction* particle_gun = 
    (SLArPGunGeneratorAction*)fGeneratorActions[kParticleGun]; 
  particle_gun->SetParticleKineticEnergy(ekin); 

  SLArPBombGeneratorAction* particle_bomb = 
    (SLArPBombGeneratorAction*)fGeneratorActions[kParticleBomb]; 
  particle_bomb->SetParticleKineticEnergy(ekin); 
}

void SLArPrimaryGeneratorAction::SetGunNumberOfParticles(const G4int n) 
{
  //SLArPGunGeneratorAction* particle_gun = 
    //(SLArPGunGeneratorAction*)fGeneratorActions[kParticleGun]; 
  //particle_gun->SetNumberOfParticles(n); 

  SLArPBombGeneratorAction* particle_bomb = 
    (SLArPBombGeneratorAction*)fGeneratorActions[kParticleBomb]; 
  particle_bomb->SetNumberOfParticles(n); 
}


void SLArPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // Store Primary information id dst
  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();

  if (fDirectionMode == kRandom) {
    fGunDirection = SampleRandomDirection();
  }

  if (fBulkGenerator->GetBulkLogicalVolume()) {
      fBulkGenerator->ShootVertex( fGunPosition ); 
      if (fVerbose) {
        printf("Gun position: %.2f, %.2f, %.2f\n", 
            fGunPosition.x(), fGunPosition.y(), fGunPosition.z()); 
      }
  }  
 
  G4VUserPrimaryGeneratorAction* gen = nullptr; 

  switch (fGeneratorEnum) {
    
    case kDecay0:
      {
        auto decay0_gen = 
          (bxdecay0_g4::SLArDecay0GeneratorAction*)fGeneratorActions[kDecay0]; 
        if (decay0_gen->HasVertexGenerator() == false) {
          decay0_gen->SetVertexGenerator(fBulkGenerator);
        }
        gen = decay0_gen; 
      }
      break;

    case kMarley: 
      {
        marley::SLArMarleyGeneratorAction* marley_gen = 
          (marley::SLArMarleyGeneratorAction*)fGeneratorActions[kMarley]; 
        marley_gen->SetVertexGenerator(fBulkGenerator); 
        marley_gen->SetNuDirection(fGunDirection); 
        gen = marley_gen; 
      }
      break;
    
    case kParticleGun: 
      {
        SLArPGunGeneratorAction* pgun_gen = 
          (SLArPGunGeneratorAction*)fGeneratorActions[kParticleGun]; 
        pgun_gen->SetParticlePosition(fGunPosition); 
        pgun_gen->SetParticleMomentumDirection(fGunDirection); 
        gen = pgun_gen; 
      }
      break;

    case kParticleBomb: 
      {
        SLArPBombGeneratorAction* pbomb_gen = 
          (SLArPBombGeneratorAction*)fGeneratorActions[kParticleBomb]; 
        pbomb_gen->SetParticlePosition(fGunPosition); 
        if (fDirectionMode == kFixed) {
          pbomb_gen->SetParticleMomentumDirection(fGunDirection); 
        }
        else {
          pbomb_gen->SetParticleMomentumDirection( G4ThreeVector(0, 0, 0) );
        }
        gen = pbomb_gen; 
      }
      break;

    case kExternalGen:
      {
        SLArExternalGeneratorAction* ext_gen = 
          (SLArExternalGeneratorAction*)fGeneratorActions[kExternalGen]; 
        ext_gen->SetVertexGenerator(fBoxGenerator); 
        gen = ext_gen;
      }        
      break;

    case kGENIE:
      {
	SLArGENIEGeneratorAction* genie_gen = 
	  (SLArGENIEGeneratorAction*)fGeneratorActions[kGENIE]; 
	genie_gen->SetGENIEEvntExt(fGENIEEvntNum);
	genie_gen->Initialize(fGENIEFile);
	gen = genie_gen;
      }        
      break;//--JM

    default:
      {
        printf("SLArPGunGeneratorAction::GeneratePrimaries() ERROR ");
        printf("Unknown generator option.\n");
        return;
      }
      break;
  };

  if (fIncludeBackground) {
    printf("SLArPrimaryGeneratorAction::GeneratePrimaries: ");
    printf("Using SLArBackgroundGeneratorAction!!\n");
    SLArBackgroundGeneratorAction* bkgGen = 
      (SLArBackgroundGeneratorAction*)fGeneratorActions[kBackground]; 
    bxdecay0_g4::SLArDecay0GeneratorAction* decay0Gen = 
      (bxdecay0_g4::SLArDecay0GeneratorAction*)fGeneratorActions[kDecay0];

    bkgGen->LoadPrimaryGenerator(gen); 
    bkgGen->LoadBackgroundGenerator(decay0Gen); 
    bkgGen->LoadVertexGenerator(fBulkGenerator); 
    bkgGen->PrintBackgroundModel(); 
    gen = bkgGen;
  }

  gen->GeneratePrimaries(anEvent); 
  G4int n = anEvent->GetNumberOfPrimaryVertex(); 

  if (fVerbose) {
    printf("Primary Generator Action produced %i vertex(ices)\n", n); 
  }
  for (int i=0; i<n; i++) {
    //std::unique_ptr<SLArMCPrimaryInfoUniquePtr> tc_primary = std::make_unique<SLArMCPrimaryInfoUniquePtr>();
    SLArMCPrimaryInfo tc_primary;
    G4int np = anEvent->GetPrimaryVertex(i)->GetNumberOfParticle(); 
    if (fVerbose) {
      printf("vertex %i has %i particles at t = %g\n", n, np, 
          anEvent->GetPrimaryVertex(i)->GetT0()); 
    }
    for (int ip = 0; ip<np; ip++) {
      //printf("getting particle %i...\n", ip); 
      auto particle = anEvent->GetPrimaryVertex(i)->GetPrimary(ip); 

      if (!particle->GetParticleDefinition()) {
        tc_primary.SetID  (particle->GetPDGcode()); 
        tc_primary.SetName("Ion");
        //printf("Particle has no definition: PDG ID = %i, setting generic ion\n", particle->GetPDGcode());
      } else {
        tc_primary.SetID  (particle->GetPDGcode());
        tc_primary.SetName(particle->GetParticleDefinition()->GetParticleName());
        //printf("Particle has definition: PDG ID = %i, setting %s\n", 
            //particle->GetPDGcode(), tc_primary.GetName());
      }

      tc_primary.SetTrackID(particle->GetTrackID());
      tc_primary.SetPosition(anEvent->GetPrimaryVertex(i)->GetX0(),
          anEvent->GetPrimaryVertex(i)->GetY0(), 
          anEvent->GetPrimaryVertex(i)->GetZ0());
      tc_primary.SetMomentum(
          particle->GetPx(), particle->GetPy(), particle->GetPz(), 
          particle->GetKineticEnergy());
      tc_primary.SetTime(anEvent->GetPrimaryVertex(i)->GetT0()); 
      

#ifdef SLAR_DEBUG
      printf("Adding particle to primary output list\n"); 
      tc_primary.PrintParticle(); 
      //getchar();
#endif
      SLArAnaMgr->GetEvent()->RegisterPrimary( tc_primary );
    }
  }

  return;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPrimaryGeneratorAction::SetMarleyConf(G4String marley_conf) {
  fMarleyCfg = marley_conf; 
  delete fGeneratorActions[kMarley]; 
  fGeneratorActions[kMarley] = new marley::SLArMarleyGeneratorAction(fMarleyCfg); 

  marley::SLArMarleyGeneratorAction* marley_gen = 
      (marley::SLArMarleyGeneratorAction*)fGeneratorActions[kMarley]; 
    marley_gen->SetVertexGenerator(fBulkGenerator); 
  return; 
}

void SLArPrimaryGeneratorAction::SetExternalConf(G4String external_cfg) {
  SLArExternalGeneratorAction* gen = 
    (SLArExternalGeneratorAction*)fGeneratorActions[kExternalGen]; 
  gen->SetVertexGenerator(fBoxGenerator);
  gen->SourceExternalConfig(external_cfg); 
  return; 
}


void SLArPrimaryGeneratorAction::SetBackgroundConf(G4String background_conf)
{
  fIncludeBackground = true; 
  fBackgoundModelCfg = background_conf; 
  SLArBackgroundGeneratorAction* bkgGen = 
    (SLArBackgroundGeneratorAction*)fGeneratorActions[kBackground];
   bkgGen->BuildBackgroundTable(fBackgoundModelCfg);
   return;
}

void SLArPrimaryGeneratorAction::SetGENIEEvntExt(G4int evntID) { // --JM
  // Set event ID to start simulation at
  printf("Setting GENIE starting event ID to %i.",evntID);
  fGENIEEvntNum = evntID;
}

void SLArPrimaryGeneratorAction::SetGENIEFile(G4String filename) { // --JM
  printf("Setting GENIE file as:\n\t %s.",filename.data());
  fGENIEFile = filename;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
