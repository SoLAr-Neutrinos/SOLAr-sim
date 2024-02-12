/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArSteppingAction.cc
 * @created     Sat Apr 15, 2023 15:26:19 CEST
 * @brief       Implementation of the SLArSteppingAction class
 */

#include "SLArScintillation.h"
#include "SLArSteppingAction.hh"
#include "SLArUserPhotonTrackInformation.hh"
#include "SLArUserTrackInformation.hh"
#include "SLArAnalysisManager.hh"

#include "detector/SuperCell/SLArSuperCellSD.hh"
#include "detector/Anode/SLArReadoutTileSD.hh"

#include "G4VPhysicalVolume.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4HadProcesses.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4SDManager.hh"
#include "G4PVReplica.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArSteppingAction::SLArSteppingAction(SLArEventAction* ea, SLArTrackingAction* ta)
  : G4UserSteppingAction()
{ 
  fEventAction          = ea;
  fTrackinAction        = ta;
  fEventNumber = -1;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArSteppingAction::~SLArSteppingAction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

trj_point SLArSteppingAction::set_evtrj_point(const G4StepPoint* point, const int nel, const int nph) {
  trj_point step_point; 

  const auto pos = point->GetPosition(); 
  step_point.fX = pos.x(); 
  step_point.fY = pos.y(); 
  step_point.fZ = pos.z(); 
  step_point.fKEnergy = point->GetKineticEnergy(); 
  step_point.fEdep = 0.; 
  const auto physicalVol = point->GetPhysicalVolume();
  if (physicalVol) {
    step_point.fCopy = point->GetPhysicalVolume()->GetCopyNo(); 
    const auto material_name = 
      point->GetPhysicalVolume()->GetLogicalVolume()->GetMaterial()->GetName();
    if ( G4StrUtil::contains(material_name, "LAr") ) {
      step_point.fLAr = true;
    }
  }
  else {
    step_point.fCopy = -1;
    step_point.fLAr = false;
  }

  step_point.fNel = nel;
  step_point.fNph = nph; 

  return step_point;
}

void SLArSteppingAction::UserSteppingAction(const G4Step* step)
{
  G4Track* track = step->GetTrack();

  const G4ParticleDefinition* particleDef = track->GetDynamicParticle()->
    GetParticleDefinition();

  G4StepPoint* thePrePoint = step->GetPreStepPoint();
  G4VPhysicalVolume* thePrePV = thePrePoint->GetPhysicalVolume();

  G4StepPoint* thePostPoint = step->GetPostStepPoint();
  G4VPhysicalVolume* thePostPV = thePostPoint->GetPhysicalVolume();
  // handle exception of particles reaching the end of the world
  if (!thePostPV) thePostPV = thePrePV;

//#ifdef SLAR_DEBUG
  //printf("Particle: %s at [%.0f , %0.f, %0.f] - trkID %i- Boundary check: %s (%s) | %s (%s)\n", 
      //particleDef->GetParticleName().data(),
      //thePrePoint->GetPosition().x(), thePrePoint->GetPosition().y(), thePrePoint->GetPosition().z(), 
      //track->GetTrackID(),
      //thePrePV->GetName().c_str(), 
      //thePrePV->GetLogicalVolume()->GetMaterial()->GetName().c_str(), 
      //thePostPV->GetName().c_str(), 
      //thePostPV->GetLogicalVolume()->GetMaterial()->GetName().c_str());
//#endif


  
  if (track->GetParticleDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) {
    auto trkInfo = (SLArUserTrackInformation*)track->GetUserInformation(); 
    SLArEventTrajectory* trajectory = trkInfo->GimmeEvTrajectory();
    double edep = step->GetTotalEnergyDeposit();
    auto stepMngr = fTrackinAction->GetTrackingManager()->GetSteppingManager(); 
    int n_ph = 0; 
    int n_el = 0; 

    if (stepMngr->GetfStepStatus() != fAtRestDoItProc) {
      G4ProcessVector* process_vector = stepMngr->GetfPostStepDoItVector(); 
      for (size_t iproc = 0; iproc < stepMngr->GetMAXofPostStepLoops(); iproc++) {
        G4VProcess* proc = (*process_vector)[iproc]; 

        if (proc->GetProcessName() == "Scintillation") {
          SLArScintillation* scint_process = (SLArScintillation*)proc; 

          n_ph = scint_process->GetNumPhotons(); 
          n_el = scint_process->GetNumIonElectrons(); 
          
          break;
        } 
      }
    }

    if (trkInfo->CheckStoreTrajectory() == true) {
      if (trajectory->DoStoreTrajectoryPts()) {
        //printf("SLArSteppingAction::here we go\n"); 
        //printf("trajectory has %lu points\n", trajectory->GetPoints().size());
        if (trajectory->GetPoints().empty()) {
          // record origin point
          trj_point step_point = set_evtrj_point( thePrePoint, 0, 0 ); 
          trajectory->RegisterPoint(step_point); 
        }

        trj_point step_point = set_evtrj_point( thePostPoint, n_el, n_ph ); 
        trajectory->RegisterPoint(step_point); 
      }
    }

    trajectory->IncrementEdep( edep ); 
    trajectory->IncrementNion( n_el ); 
    trajectory->IncrementNph ( n_ph ); 

    //printf("SLArSteppingAction::UserSteppingAction: adding %i ph and %i e ion. to %s [%i]\n", 
        //n_ph, n_el, 
        //particleDef->GetParticleName().c_str(), track->GetTrackID());
    //printf("trk ID %i [%i], PDG ID %i [%i] - trj size %lu\n", 
        //track->GetTrackID(), 
        //trajectory.GetTrackID(), 
        //track->GetParticleDefinition()->GetPDGEncoding(),
        //trajectory.GetPDGID(), 
        //trajectory.GetPoints().size());
    //getchar(); 

    G4String terminator; 

#ifdef SLAR_EXTERNAL
   if (thePostPoint->GetStepStatus() == fGeomBoundary) {
      if ( G4StrUtil::contains(thePostPV->GetLogicalVolume()->GetMaterial()->GetName(), "LAr") )
      {
        track->SetTrackStatus( fStopAndKill ); 
        
        auto& ext_record = SLArAnalysisManager::Instance()->GetExternalRecord(); 
        auto iev = G4RunManager::GetRunManager()->GetCurrentRun()->GetNumberOfEvent();
        ext_record->SetEvNumber( iev ); 
        ext_record->SetValues( *trajectory ); 
        ext_record->SetEnergyAtLAr( thePrePoint->GetKineticEnergy() ); 
        ext_record->SetVertex(  thePostPoint->GetPosition().x(), 
                                thePostPoint->GetPosition().y(), 
                                thePostPoint->GetPosition().z()); 

        SLArAnalysisManager::Instance()->GetExternalsTree()->Fill(); 

        ext_record->Reset(); 

        terminator = "SLArUserInterfaceKiller";
      }
   }
#endif 

    if (track->GetTrackStatus() == fStopAndKill) {

      auto process = 
        const_cast<G4VProcess*>(step->GetPostStepPoint()->GetProcessDefinedStep()); 
      if (terminator.empty()){
        terminator = process->GetProcessName(); 
      }

      G4HadronicProcess* hproc = dynamic_cast<G4HadronicProcess*>(process);
      const G4Isotope* target = NULL;
      if (hproc) {
        G4String nuclearChannel = "";
        target = hproc->GetTargetIsotope();
        G4String targetName = "XXXX";  
        if (target) targetName = target->GetName();
        nuclearChannel += " + " + targetName + " -> ";

        std::map<G4ParticleDefinition*, G4int> particle_counter;
        const std::vector<const G4Track*>* secondary 
          = step->GetSecondaryInCurrentStep();  
        for (size_t lp=0; lp<(*secondary).size(); lp++) {
          auto particle = (*secondary)[lp]->GetDefinition(); 
          G4String name   = particle->GetParticleName();
          G4String type   = particle->GetParticleType();      
          G4double energy = (*secondary)[lp]->GetKineticEnergy();

          particle_counter[particle]++;
        }

        // nuclear channel
        const G4int kMax = 16;  
        const G4String convert[] = {"0","","2 ","3 ","4 ","5 ","6 ","7 ","8 ","9 ",
          "10 ","11 ","12 ","13 ","14 ","15 ","16 "};
        std::map<G4ParticleDefinition*,G4int>::iterator ip;               
        for (ip = particle_counter.begin(); ip != particle_counter.end(); ip++) {
          auto particle = ip->first;
          G4String name = particle->GetParticleName();      
          G4int nb = ip->second;
          if (nb > kMax) nb = kMax;   
          G4String Nb = convert[nb];    

          if (ip != particle_counter.begin()) nuclearChannel += " + ";
          nuclearChannel += /*Nb +*/ name;
        }

        terminator += nuclearChannel;
        //printf("terminator: %s\n", terminator.data());
        //getchar(); 

        for (const auto& sec_itr : *secondary) {
          G4double momentum_4[4] = {0}; 
          for (size_t i = 0; i < 3; i++) {
            momentum_4[i] = sec_itr->GetMomentum()[i];
          }
          momentum_4[3] = sec_itr->GetKineticEnergy(); 

          fEventAction->RegisterNewProcessExtraInfo(
              SLArEventAction::TrackIdHelpInfo_t(track->GetTrackID(), 
                sec_itr->GetDynamicParticle()->GetPDGcode(), 
                momentum_4),
              terminator);
        }

      }

      trkInfo->GimmeEvTrajectory()->SetEndProcess(terminator); 
    }
  }

  if (!thePostPV) return;

  G4OpBoundaryProcessStatus boundaryStatus=Undefined;
  static G4ThreadLocal G4OpBoundaryProcess* boundary = nullptr;

  if (particleDef == G4OpticalPhoton::OpticalPhotonDefinition() && 
      thePrePV != thePostPV) {

    SLArUserPhotonTrackInformation* phInfo = 
      (SLArUserPhotonTrackInformation*)track->GetUserInformation();

    //find the boundary process only once
    if(!boundary){
      G4ProcessManager* pm
        = track->GetDefinition()->GetProcessManager();
      G4int nprocesses = pm->GetProcessListLength();
      G4ProcessVector* pv = pm->GetProcessList();
      G4int i;
      for( i=0;i<nprocesses;i++){
        if((*pv)[i]->GetProcessName()=="OpBoundary"){
          boundary = (G4OpBoundaryProcess*)(*pv)[i];
//#ifdef SLAR_DEBUG
          //G4cout<< "Optical ph at " << thePrePV->GetName() 
            //<< "/" << thePostPV->GetName() << " boundary!" << G4endl; 
//#endif
          break;
        }
      }
    }


    //Was the photon absorbed by the absorption process
    // [from LXe example]
    //if(thePostPoint->GetProcessDefinedStep()->GetProcessName()
       //=="OpAbsorption"){
      //fEventAction->IncAbsorption();
      //phInfo->AddTrackStatusFlag(absorbed);
    //}

    boundaryStatus=boundary->GetStatus();
    //Check to see if the partcile was actually at a boundary
    //Otherwise the boundary status may not be valid
    //Prior to Geant4.6.0-p1 this would not have been enough to check
    if(thePostPoint->GetStepStatus()==fGeomBoundary){
      if(fExpectedNextStatus==StepTooSmall){
        if(boundaryStatus!=StepTooSmall){
          G4ExceptionDescription ed;
          ed << "SLArSteppingAction::UserSteppingAction(): "
            << "No reallocation step after reflection!"
            << G4endl;
          G4Exception("SLArSteppingAction::UserSteppingAction()", 
              "LXeExpl01",
              FatalException,ed,
              "Something is wrong with the surface normal or geometry");
        }
      }
      fExpectedNextStatus=Undefined;

      switch(boundaryStatus){
        case Absorption:
          {
#ifdef SLAR_DEBUG
            G4cout << "SLArSteppingAction::UserSteppingAction Absorption" << G4endl;
            printf("ph E = %.2f eV; pre/post step point volume: %s/%s\n", 
                track->GetTotalEnergy()*1e6,
                thePrePV->GetName().c_str(), thePostPV->GetName().c_str()); 
#endif
            if (phInfo) phInfo->AddTrackStatusFlag(boundaryAbsorbed);
            fEventAction->IncBoundaryAbsorption();
            break;
          }
        case NoRINDEX:
#ifdef SLAR_DEBUG
          printf("SLArSteppingAction::UserSteppingAction NoRINDEX\n");
          printf("ph E = %.2f eV; pre/post step point volume: %s/%s\n", 
              track->GetTotalEnergy()*1e6,
              thePrePV->GetName().c_str(), thePostPV->GetName().c_str()); 
#endif
          break;
        case Detection: 
          //Note, this assumes that the volume causing detection
          //is the photocathode because it is the only one with
          //non-zero efficiency
          {
            //Triger sensitive detector manually since photon is
            //absorbed but status was Detection
            G4TouchableHistory* touchable = 
              (G4TouchableHistory*)thePostPoint->GetTouchable(); 
#ifdef SLAR_DEBUG
            G4cout << "SLArSteppingAction::UserSteppingAction Detection" << G4endl;
#endif
            G4SDManager* SDman = G4SDManager::GetSDMpointer();
            G4String volName = touchable->GetVolume()->GetName();

            G4String sdNameSiPM  ="/tile/sipm";
            G4String sdNameSC    ="/supercell";

            SLArReadoutTileSD* sipmSD = nullptr;
            SLArSuperCellSD* supercellSD = nullptr; 

#ifdef SLAR_DEBUG
             printf("Detection in %s - copy id [%i]\n", 
                 volName.c_str(), touchable->GetCopyNumber(0)); 
             //getchar(); 
#endif

            if (phInfo) phInfo->AddTrackStatusFlag(hitPMT);
            if (volName=="SiPMActivePV") {
//#ifdef SLAR_DEBUG
              //printf("Copy No hierarchy: [%i, %i, %i, %i, %i, %i, %i, %i, %i, %i]\n", 
                  //touchable->GetCopyNumber(0), 
                  //touchable->GetCopyNumber(1),
                  //touchable->GetCopyNumber(2),
                  //touchable->GetCopyNumber(3),
                  //touchable->GetCopyNumber(4),
                  //touchable->GetCopyNumber(5), 
                  //touchable->GetCopyNumber(6), 
                  //touchable->GetCopyNumber(7), 
                  //touchable->GetCopyNumber(8),
                  //touchable->GetCopyNumber(9)
                  //);
              //for (int i=0; i<10; i++) {
                //printf("depth %i: %s\n", i, touchable->GetVolume(i)->GetName().c_str());   
              //}

              //getchar(); 
//#endif
              sipmSD = (SLArReadoutTileSD*)SDman->FindSensitiveDetector(sdNameSiPM);
              if(sipmSD) { 
                fEventAction->IncReadoutTileHitCount(); 
                sipmSD->ProcessHits_constStep(step, nullptr);
              } else {
#ifdef SLAR_DEBUG
                printf("SLArSteppingAction::UserSteppingAction::Detection WARNING\n"); 
                printf("%s is not recognized as SD\n", volName.c_str());
#endif
              }
            } else if (volName == "SuperCellCoating") {
#ifdef SLAR_DEBUG
              printf("Copy No hierarchy: [%i, %i, %i, %i, %i]\n", 
                  touchable->GetCopyNumber(0), 
                  touchable->GetCopyNumber(1),
                  touchable->GetCopyNumber(2),
                  touchable->GetCopyNumber(3),
                  touchable->GetCopyNumber(4)
                  );
              //getchar(); 
#endif

              supercellSD = (SLArSuperCellSD*)SDman->FindSensitiveDetector(sdNameSC);
              if(supercellSD) { 
                fEventAction->IncSuperCellHitCount(); 
                supercellSD->ProcessHits_constStep(step, nullptr);
              } else {
#ifdef SLAR_DEBUG
                printf("SLArSteppingAction::UserSteppingAction::Detection WARNING\n"); 
                printf("%s is not recognized as SD\n", volName.c_str());
#endif
              }
            } 
#ifdef SLAR_DEBUG
            else {
                printf("SLArSteppingAction::UserSteppingAction::Detection WARNING\n"); 
                printf("%s is not recognized as SD\n", volName.c_str());
                getchar(); 
            }
#endif
            
            track->SetTrackStatus( fStopAndKill );
            break;
          }
        default:
          break;
      }

    }
  }

//#ifdef SLAR_DEBUG
    //printf("PASSED\n");
//#endif

}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
