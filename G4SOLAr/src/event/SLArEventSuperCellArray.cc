/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArEventSuperCellArray.cc
 * @created     Thur Oct 20, 2022 16:16:03 CEST
 */
#include "event/SLArEventSuperCellArray.hh"

ClassImp(SLArEventSuperCellArray)


SLArEventSuperCellArray::SLArEventSuperCellArray()
  : TNamed(), fNhits(0), fIsActive(true) {}

SLArEventSuperCellArray::SLArEventSuperCellArray(const SLArEventSuperCellArray& ev)
  : TNamed(ev) 
{
  fNhits = ev.fNhits; 
  fIsActive = ev.fIsActive; 
  for (const auto &sc : ev.fSuperCellMap) {
    fSuperCellMap.insert(
        std::make_pair(sc.first, SLArEventSuperCell(sc.second) ) );
  }
  return;
}

SLArEventSuperCellArray::SLArEventSuperCellArray(const SLArCfgSuperCellArray& cfg) 
  : SLArEventSuperCellArray()
{
  SetName(cfg.GetName());
  //ConfigSystem(cfg); 
  return;
}

SLArEventSuperCellArray::~SLArEventSuperCellArray() {
  for (auto &scevent : fSuperCellMap) {
    scevent.second.ResetHits();
    //delete scevent.second;
  }
  fSuperCellMap.clear(); 
}

SLArEventSuperCell& SLArEventSuperCellArray::GetOrCreateEventSuperCell(const int scIdx) {
  auto it = fSuperCellMap.find(scIdx); 

  if (it != fSuperCellMap.end()) {
    //printf("SLArEventAnode::CreateEventMegatile(%i) WARNING: Megatile nr %i already present in SuperCell Array %s register\n", scIdx, scIdx, fName.Data());
    return fSuperCellMap.find(scIdx)->second;
  }
  else {
    fSuperCellMap.insert( std::make_pair(scIdx, SLArEventSuperCell(scIdx)) );
    auto& sc_event = fSuperCellMap[scIdx];
    sc_event.SetBacktrackerRecordSize( fLightBacktrackerRecordSize ); 

    return sc_event;
  }
}

SLArEventSuperCell& SLArEventSuperCellArray::RegisterHit(const SLArEventPhotonHit& hit, int sc_idx) {
  if (sc_idx < 0) sc_idx = hit.GetTileID();

  auto& sc_event = GetOrCreateEventSuperCell(sc_idx);
  sc_event.RegisterHit(hit); 

  fNhits++;
  return sc_event;

  //} else {
    //printf("SLArEventSuperCellArray::RegisterHit WARNING\n"); 
    //printf("SuperCell with ID %i is not in store [%i,%i,%i]\n", 
        //sc_idx, hit->GetMegaTileIdx(), hit->GetRowTileNr(), hit->GetTileNr()); 
    //return 0; 
  //}
}

int SLArEventSuperCellArray::ResetHits() {
  int nn = 0; 
  for (auto &sc : fSuperCellMap) {
    nn += sc.second.ResetHits(); 
  }
  fSuperCellMap.clear();
  fNhits = 0; 
  return nn; 
}

void SLArEventSuperCellArray::SetActive(bool is_active) {
  fIsActive = is_active; 
  for (auto &sc : fSuperCellMap) {
    sc.second.SetActive(is_active); 
  }
}

