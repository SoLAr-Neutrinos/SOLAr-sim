/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArEventSuperCell.cc
 * @created     : Thur Oct 20, 2022 15:40:04 CEST
 */

#include "event/SLArEventSuperCell.hh"

ClassImp(SLArEventSuperCell)

SLArEventSuperCell::SLArEventSuperCell() 
  : SLArEventHitsCollection<SLArEventPhotonHit>() {}

SLArEventSuperCell::SLArEventSuperCell(int idx) 
  : SLArEventHitsCollection<SLArEventPhotonHit>(idx) {}

SLArEventSuperCell::SLArEventSuperCell(const SLArEventSuperCell& ev) 
  : SLArEventHitsCollection<SLArEventPhotonHit>(ev) {}

SLArEventSuperCell::~SLArEventSuperCell() {
  ResetHits();
}

double SLArEventSuperCell::GetTime() const {
  double t = -1; 
  if (fNhits) t = fHits.begin()->first * fClockUnit;
  return t;
}

double SLArEventSuperCell::GetTime(EPhProcess proc) const {
  printf("WARNING: SLArEventSuperCell::GetTime(EPhProcess) not implemented\n");
  return GetTime();
}

void SLArEventSuperCell::PrintHits() const
{
  std::cout << "SuperCell id: " << fIdx << std::endl;
  std::cout << "-----------------------------------------"
            << std::endl;
  int nhits = 0;
  for (auto &hit : fHits)
  {
    std::cout << "hit " << nhits << " at " << hit.first * fClockUnit 
              << " ns " << std::endl;
    nhits++;
  }

  std::cout << "\n" << std::endl;
  
}
