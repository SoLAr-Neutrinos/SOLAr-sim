/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArBacktracker
 * @created     Friday Sep 29, 2023 10:30:22 CEST
 */

#include "SLArBacktracker.hh"
#include "event/SLArEventGenericHit.hh"
#include "event/SLArEventPhotonHit.hh"
#include "event/SLArEventBacktrackerRecord.hh"
#include "SLArEventAction.hh"

#include "G4RunManager.hh"

namespace backtracker {

const G4String BkTrkReadoutSystemTag[3] = {"charge", "vuv_sipm", "supercell"};

EBkTrkReadoutSystem GetBacktrackerReadoutSystem(const G4String sys) {
  EBkTrkReadoutSystem id = kNoSystem;
  if ( sys == "charge") {
    id = kCharge;
  }
  else if (sys == "vuv_sipm") {
    id = kVUVSiPM;
  }
  else if (sys == "supercell") {
    id = kSuperCell;
  }
  else {
    printf("backtraker::GetBacktrackerReadoutSystem() WARNING no backtracker readout system called \"%s\"\n", 
        sys.data());
  }
  return id;
}


const G4String BacktrackerLabel[5] = {"trkID", "ancestorID", "opticalProc", "sipm_nr", "originVolID"};

EBacktracker GetBacktrackerEnum(const G4String bkt) {
  EBacktracker id = kNoBacktracker;
  if ( bkt == "trkID") {
    id = kTrkID;
  }
  else if (bkt == "ancestorID") {
    id = kAncestorID;
  }
  else if (bkt == "opticalProc") {
    id = kOpticalProc;
  }
  else if (bkt == "sipm_nr") {
    id = kSiPMNr;
  }
  else {
    printf("backtraker::GetBacktrackerEnum() WARNING no backtracker called \"%s\"\n", 
        bkt.data());
  }

  return id;
}

SLArBacktracker::SLArBacktracker() : fName("backtracker")
{}

SLArBacktracker::SLArBacktracker(const G4String name) : fName(name)
{}

void SLArBacktrackerTrkID::Eval(SLArEventGenericHit* hit, SLArEventBacktrackerRecord* rec) {
  rec->UpdateCounter(hit->GetProducerTrkID());
}

void SLArBacktrackerAncestorID::Eval(SLArEventGenericHit* hit, SLArEventBacktrackerRecord* rec) {
  auto ev_action = (SLArEventAction*)G4RunManager::GetRunManager()->GetUserEventAction();
  int ancestor = ev_action->FindAncestorID(hit->GetPrimaryProducerTrkID()); 
  rec->UpdateCounter(ancestor);
}

void SLArBacktrackerOpticalProcess::Eval(SLArEventGenericHit* hit, SLArEventBacktrackerRecord* rec) {
  if (dynamic_cast<SLArEventPhotonHit*>(hit)) {
    auto ph_hit = dynamic_cast<SLArEventPhotonHit*>(hit);
    rec->UpdateCounter(ph_hit->GetProcess()); 
  }
  return;
}

void SLArBacktrackerSiPMNr::Eval(SLArEventGenericHit* hit, SLArEventBacktrackerRecord* rec) {
  if (dynamic_cast<SLArEventPhotonHit*>(hit)) {
    auto ph_hit = dynamic_cast<SLArEventPhotonHit*>(hit);
    rec->UpdateCounter(ph_hit->GetCellNr()); 
  }
  return;
}

void SLArBacktrackerOriginVolID::Eval(SLArEventGenericHit* hit, SLArEventBacktrackerRecord* rec) {
  if (dynamic_cast<SLArEventPhotonHit*>(hit)) {
    auto ph_hit = dynamic_cast<SLArEventPhotonHit*>(hit);
    rec->UpdateCounter(ph_hit->GetPhotonOriginVolumeID()); 
  }
  return;
}

}
