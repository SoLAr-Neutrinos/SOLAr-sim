/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventTile
 * @created     : mercoledì ago 10, 2022 12:21:15 CEST
 */

#include <memory>
#include "event/SLArEventTile.hh"

ClassImp(SLArEventTile)


SLArEventTile::SLArEventTile() 
  : TNamed(), fChargeBacktrackerRecordSize(0), fSiPMBacktrackerRecordSize(0)
{}


SLArEventTile::SLArEventTile(const int idx) 
  : TNamed(), fChargeBacktrackerRecordSize(0), fSiPMBacktrackerRecordSize(0)
{
  fName = Form("EvTile%i", fIdx); 
}

SLArEventTile::SLArEventTile(const SLArEventTile& ev) 
  : TNamed(ev), fPixelHits(), fSiPMHits()
{
  fChargeBacktrackerRecordSize = ev.fChargeBacktrackerRecordSize;
  fSiPMBacktrackerRecordSize = ev.fSiPMBacktrackerRecordSize;
  if (!ev.fPixelHits.empty()) {
    for (const auto &qhit : ev.fPixelHits) {
      fPixelHits[qhit.first] = qhit.second;
    }
  }

  if (!ev.fSiPMHits.empty()) {
    for (const auto &phit : ev.fSiPMHits) {
      fSiPMHits[phit.first] = phit.second;
    }
  }
  return;
}



int SLArEventTile::ResetHits()
{
  for (auto &sipm : fSiPMHits) {
      sipm.second.ResetHits(); 
  }

  for (auto &pix : fPixelHits) {
      pix.second.ResetHits(); 
  }
  fPixelHits.clear(); 
  fSiPMHits.clear();

  return 0;
}


SLArEventTile::~SLArEventTile() {
  ResetHits();
}

double SLArEventTile::GetTime() const {
  if (fSiPMHits.empty()) return -1.;

  int t = std::numeric_limits<int>::max();
  const auto clock_unit = fSiPMHits.begin()->second.GetClockUnit();

  for (const auto& sipm_itr : fSiPMHits) {
    double sipm_time = sipm_itr.second.GetTime();
    if (sipm_time < t) {
      t = sipm_time;
    }
  }

  return t*clock_unit;
}


//bool SLArEventTile::SortPixelHits()
//{
  //for (auto &pixHit : fPixelHits) {
    //pixHit.second->SortHits(); 
  //}
  //return true;
//}

//bool SLArEventTile::SortHits() {
  //std::sort(fHits.begin(), fHits.end(), SLArEventPhotonHit::CompareHitPtrs);
  //SortPixelHits(); 
  //return true;
//}

void SLArEventTile::PrintHits() const
{
  printf("*********************************************\n");
  printf("Hit container ID: %i [%s]\n", fIdx, fName.Data());
  printf("*********************************************\n");
  for (auto &sipm : fSiPMHits) {
    sipm.second.Dump();
  }
  if (!fPixelHits.empty()) {
    printf("Pixel readout hits:\n");
    for (const auto &pix : fPixelHits) pix.second.PrintHits(); 
  }

  printf("\n"); 
  return;
}

SLArEventChargePixel& SLArEventTile::RegisterChargeHit(const int& pixID, const SLArEventChargeHit& qhit) {
  
  auto it = fPixelHits.find(pixID);

  if (it != fPixelHits.end()) {
    //printf("SLArEventTile::RegisterChargeHit(%i): pixel %i already hit.\n", pixID, pixID);
    it->second.RegisterHit(qhit); 
    return it->second;
  }
  else {
    //printf("SLArEventTile::RegisterChargeHit(%i): creating new pixel hit collection.\n", pixID);
    fPixelHits.insert(std::make_pair(pixID, SLArEventChargePixel(pixID, qhit)));
    auto& pixEv = fPixelHits[pixID];
    pixEv.SetBacktrackerRecordSize( fChargeBacktrackerRecordSize ); 
    return pixEv;  
  }

}

SLArEventSiPM& SLArEventTile::RegisterSiPMHit(const int& sipmID, const SLArEventPhotonHit& hit) {
  
  auto it = fSiPMHits.find(sipmID);

  if (it != fSiPMHits.end()) {
    //printf("SLArEventTile::RegisterSiPMHit(%i): sipm %i already hit.\n", sipmID, sipmID);
    it->second.RegisterHit(hit); 
    return it->second;
  }
  else {
    //printf("SLArEventTile::RegisterSiPMHit(%i): creating new sipm hit collection.\n", sipmID);
    fSiPMHits.insert(std::make_pair(sipmID, SLArEventSiPM(sipmID, hit)));
    auto& sipmEv = fSiPMHits[sipmID];
    sipmEv.SetBacktrackerRecordSize( fSiPMBacktrackerRecordSize ); 
    return sipmEv;  
  }

}

SLArEventSiPM& SLArEventTile::RegisterSiPMHit(const SLArEventPhotonHit& hit) {
  const int sipmID = hit.GetCellNr();
  return RegisterSiPMHit(sipmID, hit);
}


double SLArEventTile::GetPixelHits() const {
  double nhits = 0.;
  for (const auto &pixel : fPixelHits) {
    nhits += pixel.second.GetNhits(); 
  }

  return nhits; 
}

int SLArEventTile::GetSiPMHits() const {
  int nhits; 
  for (const auto &sipm : fSiPMHits) {
    nhits += sipm.second.GetNhits(); 
  }
  return nhits;
}
