/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventAction.cc
 * @created     : giovedì nov 03, 2022 12:36:27 CET
 */

#include "SLArAnalysisManager.hh"
#include "SLArBacktrackerManager.hh"
#include "SLArEventAction.hh"
#include "SLArRunAction.hh"
#include "SLArReadoutTileHit.hh"
#include "SLArSuperCellHit.hh"
#include "SLArDetectorConstruction.hh"
#include "detector/TPC/SLArLArHit.hh"
#include "physics/SLArElectronDrift.hh"
#include "detector/TPC/SLArExtScorerSD.hh"
#include "detector/TPC/SLArExtHit.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4SDManager.hh"
#include <G4PhysicalVolumeStore.hh>

#include "G4ios.hh"
#include <cstdio>


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArEventAction::SLArEventAction()
: G4UserEventAction(), 
  fTileHCollID  (-2), 
  fSuperCellHCollID(-5)
{
  // set printing per each event
  G4RunManager::GetRunManager()->SetPrintProgress(1);

  fHitCount                = 0;
  fPhotonCount_Scnt        = 0;
  fPhotonCount_Cher        = 0;
  fPhotonCount_WLS         = 0;
  fAbsorptionCount         = 0;
  fReadoutTileHits         = 0; 
  fSuperCellHits           = 0; 
  fBoundaryAbsorptionCount = 0;
  fTotEdep                 = 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArEventAction::~SLArEventAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArEventAction::BeginOfEventAction(const G4Event*)
{

#ifdef SLAR_DEBUG
  printf("SLArEventAction::BeginOfEventAction()\n");
#endif

  G4SDManager* sdManager = G4SDManager::GetSDMpointer();
  if (fTileHCollID == -2) 
    fTileHCollID  = sdManager->GetCollectionID("ReadoutTileColl"  );
  if (fSuperCellHCollID == -5) 
    fSuperCellHCollID = sdManager->GetCollectionID("SuperCellColl"); 
  if (fLArHCollID.empty()) {
    auto detConstruction = (SLArDetectorConstruction*)
      G4RunManager::GetRunManager()->GetUserDetectorConstruction(); 
    for (const auto &tpc : detConstruction->GetDetTPCs() ) {
      auto coll_id = 
        sdManager->GetCollectionID("TPC"+std::to_string(tpc.first)+"Coll");
      fLArHCollID.push_back(coll_id); 
    }
  }

  #ifdef SLAR_EXTERNAL
  if (fExtScorerHCollID.empty()) {
    auto scorer_pv = G4PhysicalVolumeStore::GetInstance()->GetVolume("cavern_scorer_pv"); 
    if (scorer_pv) {
      const auto sd = (SLArExtScorerSD*)scorer_pv->GetLogicalVolume()->GetSensitiveDetector(); 
      fExtScorerHCollID.push_back( sd->GetHitsCollectionID() ); 
    }
    else {
      printf("SLArEventAction::BeginofEventAction() WARNING cannot find volume 'cavern_scorer_pv' in Pysical Volume Store\n");
    }
  }
  #endif // DEBUG

#ifdef SLAR_DEBUG
    G4cout << "SLArEventAction::BeginOfEventAction():" << G4endl;
    G4cout << "ReadoutTile ID = " << fTileHCollID   << G4endl;
    G4cout << "SuperCell ID   = " << fSuperCellHCollID << G4endl;
    G4cout << "LAr volume ID  = "; 
    for (const auto &id : fLArHCollID) G4cout << id << " "; 
    G4cout << G4endl;
    G4cout << "Ext background scorer volume ID  = "; 
    for (const auto &id : fExtScorerHCollID) G4cout << id << " ";
    G4cout << G4endl;
#endif

    // reset counters
    fHitCount                = 0;
    fPhotonCount_Scnt        = 0;
    fPhotonCount_Cher        = 0;
    fPhotonCount_WLS         = 0;
    fAbsorptionCount         = 0;
    fBoundaryAbsorptionCount = 0;
    fTotEdep                 = 0;
    fReadoutTileHits         = 0; 
    fSuperCellHits           = 0; 

    return;
}     

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArEventAction::EndOfEventAction(const G4Event* event)
{
  G4int verbose = G4EventManager::GetEventManager()->GetVerboseLevel(); 
#ifdef SLAR_DEBUG
  printf("SLArEventAction::EndOfEventAction()\n"); 
#endif

    G4HCofThisEvent* hce = event->GetHCofThisEvent();
    if (!hce) 
    {
        G4ExceptionDescription msg;
        msg << "No hits collection of this event found." << G4endl; 
        G4Exception("SLArEventAction::EndOfEventAction()",
                    "SLArCode001", JustWarning, msg);
        return;
    }   
    SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();

    //RecordEventLAr( event );

    if ( !SLArAnaMgr->GetAnodeCfg().empty() ) {
      RecordEventReadoutTile ( event, verbose );
    }

    if (SLArAnaMgr->GetPDSCfg()) {
      if (verbose > 1) printf("Recording SuperCell hits...\n");
      RecordEventSuperCell( event, verbose );
      if (verbose > 1) printf("DONE\n");
    }

     
    auto slar_event = SLArAnaMgr->GetEvent();
    slar_event->SetEvNumber(event->GetEventID());

    // apply zero suppression to charge signal
    for (auto &evAnode : slar_event->GetEventAnode()) {
      short thrs = evAnode.second.GetZeroSuppressionThreshold(); 
      if (thrs > 0) {
        evAnode.second.ApplyZeroSuppression();
      }
    }
    
    SLArAnaMgr->FillEvTree();

    if (verbose > 0) {
      printf("SLArEventAction::EndOfEventAction()\n"); 
      printf("OpticalPhoton Monitor:\nCherenkov: %i\nScintillation: %i\n\n", 
          fPhotonCount_Cher, fPhotonCount_Scnt);
      printf("Primary particles:\n");
      auto& primaries = slar_event->GetPrimaries();
      for (const auto &p : primaries ) {
        printf("%s - %g MeV - trk ID %i\n", 
            p.GetParticleName().Data(), p.GetEnergy(), p.GetTrackID());
        printf("\t%i scintillation ph\n\t%i Cerenkov photons\n", 
            p.GetTotalScintPhotons(), p.GetTotalCerenkovPhotons()); 
        printf("ReadoutTile Photon Hits: %i\nSuperCell Photon Hits: %i\n\n", 
            fReadoutTileHits, fSuperCellHits);
        printf("Charge Collection Monitor:\n"); 
        for (const auto &evanode : slar_event->GetEventAnode()) {
          printf("\t- %s - %lu MT hit(s))\n", 
              evanode.second.GetName(), evanode.second.GetConstMegaTilesMap().size()); 
        }
        printf("\n");
      }
    }

#ifdef SLAR_EXTERNAL
    RecordEventExtScorer( event, verbose ); 
#endif 

    fParentIDMap.clear(); 
    fExtraProcessInfo.clear(); 

    SLArAnaMgr->GetEvent()->Reset();
}

void SLArEventAction::RecordEventReadoutTile(const G4Event* ev, const G4int& verbose)
{

  G4HCofThisEvent* hce = ev->GetHCofThisEvent();

  if (fTileHCollID!= -2) 
  {
    // Get hits collections 
    SLArReadoutTileHitsCollection* hHC1 
      = static_cast<SLArReadoutTileHitsCollection*>(hce->GetHC(fTileHCollID));

    if ( (!hHC1) ) 
    {
      G4ExceptionDescription msg;
      msg << "Some of hits collections of this event not found." 
        << G4endl; 
      G4Exception("SLArEventAction::RecordEventReadoutTile",
          "SLArCode001", JustWarning, msg);
      return;
    }   

    SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
    auto bktManager = SLArAnaMgr->GetBacktrackerManager( backtracker::kVUVSiPM ); 

    // Fill histograms
    G4int n_hit = hHC1->entries();

    for (G4int i=0;i<n_hit;i++) {
      SLArReadoutTileHit* hit = (*hHC1)[i];
      if (!hit) { 
#ifdef SLAR_DEBUG
        G4cout << "SLArEventAction::RecordEventReadoutTile(): "
          << "No hits on SiPMs or "
          << "issue in hit collection readout";
#endif

        break;
      }

      G4ThreeVector localPos = hit->GetLocalPos();
      G4ThreeVector worldPos = hit->GetWorldPos();
      G4double time = hit->GetTime();
      G4double wavelen = hit->GetPhotonWavelength(); 
      G4int anode_idx = hit->GetAnodeIdx();
      G4int mtrow_nr = hit->GetRowMegaTileIdx(); 
      G4int mgtile_nr = hit->GetMegaTileIdx(); 
      G4int rowtile_nr = hit->GetRowTileIdx(); 
      G4int tile_nr = hit->GetTileIdx(); 

#ifdef SLAR_DEBUG
      G4cout << "SLArEventAction::RecordEventReadoutTile() hit nr " << i << G4endl;
      printf("Tile idx [%i, %i, %i, %i]\n", mtrow_nr, mgtile_nr, rowtile_nr, tile_nr);
      G4cout << "x    = " << G4BestUnit(worldPos.x(), "Length") << "; "
             << "y    = " << G4BestUnit(worldPos.y(), "Length") << "; "
             << "time = " << G4BestUnit(time, "Time") << G4endl;
#endif
      
      SLArEventPhotonHit dstHit(
          time, 
          hit->GetPhotonProcessId(), 
          wavelen);
      dstHit.SetLocalPos(localPos.x(), localPos.y(), localPos.z());
      dstHit.SetTileInfo(mtrow_nr, mgtile_nr, rowtile_nr, tile_nr); 
      dstHit.SetRowCellNr(hit->GetRowCellNr()); 
      dstHit.SetCellNr(hit->GetCellNr()); 
      dstHit.SetProducerTrkID( hit->GetProducerID() ); 

      auto& ev_anode = SLArAnaMgr->GetEvent()->GetEventAnodeByID(anode_idx);
      auto& ev_tile = ev_anode.RegisterHit(dstHit);

      if (bktManager) {
        if (bktManager->IsNull() == false) {
          auto records = 
            ev_tile.GetBacktrackerVector( ev_tile.ConvertToClock(dstHit.GetTime()) );

          for (size_t ib = 0; ib < bktManager->GetBacktrackers().size(); ib++) {
            bktManager->GetBacktrackers().at(ib)->Eval(&dstHit, 
                &records.GetRecords().at(ib));
          }
        }
      }
      
    }

    // Sort hits on PMTs
    //for (auto &evAnode : SLArAnaMgr->GetEvent()->GetEventAnode()) {
      //evAnode.second->SortHits();
    //}
    

    // Print diagnostics
    G4int printModulo = 
      G4RunManager::GetRunManager()->GetPrintProgress();
    if ( printModulo==0 || ev->GetEventID() % printModulo != 0) return;
  }

  if (verbose > 1) printf("SLArEventAction::RecordEventReadoutTile() DONE\n");
}

void SLArEventAction::RecordEventSuperCell(const G4Event* ev, const G4int& verbose)
{

  G4HCofThisEvent* hce = ev->GetHCofThisEvent();
  if (fSuperCellHCollID != -5) 
  {
    // Get hits collections 
    SLArSuperCellHitsCollection* hHC1 
      = static_cast<SLArSuperCellHitsCollection*>(hce->GetHC(fSuperCellHCollID));

    if ( (!hHC1) ) 
    {
      G4ExceptionDescription msg;
      msg << "Some of hits collections of this event not found." 
        << G4endl; 
      G4Exception("SLArEventAction::RecordEventSuperCell",
          "SLArCode001", JustWarning, msg);
      return;
    }   
    SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
    auto bktManager = SLArAnaMgr->GetBacktrackerManager( backtracker::kSuperCell ); 

    G4int n_hit = hHC1->entries();
    for (G4int i=0;i<n_hit;i++) {
      SLArSuperCellHit* hit = (*hHC1)[i];
      if (!hit) { 
#ifdef SLAR_DEBUG
        G4cout << "SLArEventAction::RecordEventSuperCell(): "
          << "No hits on SuperCell or "
          << "issue in hit collection readout";
#endif

        break;
      }

      G4ThreeVector localPos = hit->GetLocalPos();
      G4ThreeVector worldPos = hit->GetWorldPos();
      G4double      time     = hit->GetTime();
      G4double      wavelen  = hit->GetPhotonWavelength(); 
      //G4int         id       = hit->GetSuperCellIdx(); 
      G4int         array_nr = hit->GetSuperCellArrayNo(); 
      G4int         cellrow_nr = hit->GetSuperCellRowNo(); 
      G4int         cell_nr    = hit->GetSuperCellNo(); 


#ifdef SLAR_DEBUG
      G4cout << "SLArEventAction::RecordEventSuperCell()" << G4endl;
      printf("SuperCell id [%i, %i, %i]\n", cell_nr, cellrow_nr, array_nr);
      G4cout << "x    = " << G4BestUnit(worldPos.x(), "Length") << "; "
             << "y    = " << G4BestUnit(worldPos.y(), "Length") << "; "
             << "time = " << G4BestUnit(time, "Time") << G4endl;
#endif
      
      SLArEventPhotonHit dstHit(
          time, 
          hit->GetPhotonProcessId(), 
          wavelen);
      dstHit.SetLocalPos(localPos.x(), localPos.y(), localPos.z());
      dstHit.SetTileInfo(0, array_nr, cellrow_nr, cell_nr); 
      dstHit.SetProducerTrkID( hit->GetProducerID() ); 

      auto& ev_sc = SLArAnaMgr->GetEvent()->GetEventSuperCellArray(array_nr).RegisterHit(dstHit);

      if (bktManager) {
        if (bktManager->IsNull() == false) {
          SLArEventBacktrackerVector& records = 
            ev_sc.GetBacktrackerVector( ev_sc.ConvertToClock<float>(dstHit.GetTime()) );

          for (size_t ib = 0; ib < bktManager->GetBacktrackers().size(); ib++) {
            bktManager->GetBacktrackers().at(ib)->Eval(&dstHit, 
                &records.GetRecords().at(ib));
          }
        }
      }
      
      //delete dstHit;
    }
    

    // Sort hits on PMTs
    //printf("Sorting hits...\n"); 
    //for (auto &evSCArray : SLArAnaMgr->GetEvent()->GetEventSuperCellArray()) {
      //evSCArray.second->SortHits(); 
    //}

    // Print diagnostics
    G4int printModulo = 
      G4RunManager::GetRunManager()->GetPrintProgress();
    if ( printModulo==0 || ev->GetEventID() % printModulo != 0) return;
  }

  if (verbose > 2) printf("SLArEventAction::RecordEventSuperCell() DONE\n");
}


void SLArEventAction::RecordEventLAr(const G4Event* ev, const G4int& verbose)
{
//#ifdef SLAR_DEBUG
  //printf("  -> RecordEventLAr()\n");
//#endif

  G4HCofThisEvent* hce = ev->GetHCofThisEvent();
  if (fLArHCollID.empty()) return;
  else 
  {
    // recover analysis manager
    //SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
    for (const auto &id : fLArHCollID) {
      SLArLArHitsCollection* hHC1 
        = static_cast<SLArLArHitsCollection*>(hce->GetHC(id));

      SLArLArHit* hit = (*hHC1)[0];
      fTotEdep = hit->GetDepositedEnergy();
    }

  }


//#ifdef SLAR_DEBUG
  //printf("     DONE\n"); 
//#endif
}

void SLArEventAction::RecordEventExtScorer(const G4Event* ev, const G4int& verbose) {
  G4HCofThisEvent* hce = ev->GetHCofThisEvent();
  if (fExtScorerHCollID.empty()) return;

#ifdef SLAR_EXTERNAL
  auto anaMngr = SLArAnalysisManager::Instance(); 
  for (const auto& id : fExtScorerHCollID) {
    SLArExtHitsCollection* hHC1 = static_cast<SLArExtHitsCollection*>(hce->GetHC(id)); 
    if (verbose > 1) printf("SLArExtHitsCollection hce[%i] = %p\n", id, static_cast<void*>(hHC1)); 
    if (!hHC1) {
      G4ExceptionDescription msg;
      msg << "Some of hits collections of this event not found." 
        << G4endl; 
      G4Exception("SLArEventAction::RecordEventExtScorer",
          "SLArCode001", JustWarning, msg);
      return;
    }   

    for (size_t i = 0; i < hHC1->entries(); i++) {
      SLArExtHit* scorer_hit = (*hHC1)[i];   
      //printf("recording scorer hit:\n"); 
      //scorer_hit->Print(); 
      auto& ext_record = anaMngr->GetExternalRecord();
      ext_record->Reset(); 
      ext_record->SetEvNumber( ev->GetEventID() ); 
      ext_record->SetPDGCode( scorer_hit->fPDGCode ); 
      ext_record->SetTrackID( scorer_hit->fTrkID ); 
      ext_record->SetParentID( scorer_hit->fParentID ); 
      ext_record->SetOriginVol( scorer_hit->fOriginVol ); 
      ext_record->SetOriginEnergy( scorer_hit->fOriginEnergy ); 
      ext_record->SetEnergyAtLAr( scorer_hit->fEnergy ); 
      ext_record->SetWeight( scorer_hit->fWeight ); 
      ext_record->SetCreator( scorer_hit->fCreator.data() ); 
      ext_record->SetTime( scorer_hit->fTime ); 
      ext_record->SetVertex( scorer_hit->fVertex );

      anaMngr->GetExternalsTree()->Fill(); 
    }
  }

#endif
}

void SLArEventAction::RegisterNewTrackPID(int trk_id, int p_id) {
  fParentIDMap.insert( std::make_pair(trk_id, p_id) ); 
  return;
}

void SLArEventAction::RegisterNewProcessExtraInfo(const TrackIdHelpInfo_t& trkHelp, G4String& proc) {
  if (fExtraProcessInfo.count(trkHelp)) {
    printf("SLArEventAction::RegisterNewProcessExtraInfo WARNING trk with pdg %i, parent %i and 4-p (%g, %g, %g, %g) already has detailed process info assigned (%s)\n", 
        trkHelp.pdg, 
        trkHelp.parent,
        trkHelp.quadrimomentum[0], trkHelp.quadrimomentum[1], 
        trkHelp.quadrimomentum[2], trkHelp.quadrimomentum[3],
        fExtraProcessInfo[trkHelp].data());
  }
  fExtraProcessInfo.insert( std::make_pair(trkHelp, proc) ); 
  return;
}


int SLArEventAction::FindAncestorID(int trkid) {
  int primary = -1; 
  int pid = trkid; 

  SLArAnalysisManager* anaMngr = SLArAnalysisManager::Instance(); 
  auto& primaries = anaMngr->GetEvent()->GetPrimaries(); 
  bool caught = false; 
#ifdef SLAR_DEBUG
  printf("SLArEventAction::FindAncestorID() Lookging for primary parent of %i among\n", trkid);
  for (const auto& _pid : fParentIDMap) {
    printf("%i - PID: %i\n", _pid.first, _pid.second); 
  }

  printf("\nList of primaries: \n");
  for (const SLArMCPrimaryInfo& p : primaries) {
    printf("%s - PID: %i\n", p.GetParticleName().Data(), p.GetTrackID());
  }
  //getchar(); 
#endif

  while ( !caught ) {
    pid = fParentIDMap[trkid];
#ifdef SLAR_DEBUG
    printf("local parent id: %i\n", pid);
#endif

    for (const auto& p : primaries) {
      if (pid == p.GetTrackID()) {
        primary = pid; 
        caught = true;
      }
    }

    trkid = pid; 
//#ifdef SLAR_DEBUG
//    getchar(); 
//#endif
  }

#ifdef SLAR_DEBUG
    printf("Caught! returning %i\n", primary);
    //getchar(); 
#endif

  return primary; 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

