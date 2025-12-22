/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArBacktrackerManager.cc
 * @created     Friday Sep 29, 2023 12:56:18 CEST
 */

#include "SLArBacktrackerManager.hh"
#include "SLArBacktracker.hh"

namespace backtracker{

SLArBacktrackerManager::~SLArBacktrackerManager()
{
  for (auto &b : fBacktrackers) {
    delete b; b = nullptr;
  }
  fBacktrackers.clear();
}

G4bool SLArBacktrackerManager::RegisterBacktracker(SLArBacktracker* bkt) {
  if (!bkt) {
    return 0;
  }
  fBacktrackers.push_back(bkt);
  return 1;
}

G4bool SLArBacktrackerManager::RegisterBacktracker(const EBacktracker id, const G4String name) {
  G4bool status = false;
  G4String bkt_name = "";

  switch (id) {
    case kTrkID:
      {
        if (name.empty()) bkt_name = BacktrackerLabel[id];
        fBacktrackers.push_back( new SLArBacktrackerTrkID( bkt_name ));
        status = true;
        break;
      }
    case kAncestorID:
      {
        if (name.empty()) bkt_name = BacktrackerLabel[id];
        fBacktrackers.push_back( new SLArBacktrackerAncestorID( bkt_name ));
        status = true;
        break;
      }
    case kOpticalProc:
      {
        if (name.empty()) bkt_name = BacktrackerLabel[id];
        fBacktrackers.push_back( new SLArBacktrackerOpticalProcess( bkt_name ));
        status = true;
        break;
      }
    case kSiPMNr:
      {
        if (name.empty()) bkt_name = BacktrackerLabel[id];
        fBacktrackers.push_back( new SLArBacktrackerSiPMNr( bkt_name ));
        status = true;
        break;
      }
    case kOriginVolID:
      {
        if (name.empty()) bkt_name = BacktrackerLabel[id];
        fBacktrackers.push_back( new SLArBacktrackerOriginVolID( bkt_name ));
        status = true;
        break;
      }
    default:
      {
        break;
      }
  }

  printf("SLArBacktrackerManager::Registered backtracker %s with status [%i]\n", 
      BacktrackerLabel[id].data(), status);
  //getchar();

  return status;
}

G4bool SLArBacktrackerManager::IsNull() const {
  if (fBacktrackers.empty()) return true;
  else return false;
}
}
