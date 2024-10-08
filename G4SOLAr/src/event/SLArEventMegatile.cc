/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventMegatile
 * @created     : Wed Aug 10, 2022 13:47:24 CEST
 */

#include <memory>
#include <SLArAnalysisManager.hh>
#include "event/SLArEventMegatile.hh"

ClassImp(SLArEventMegatile)


SLArEventMegatile::SLArEventMegatile() 
  : fIdx(0), fIsActive(true), fNhits(0), 
    fLightBacktrackerRecordSize(0), fChargeBacktrackerRecordSize(0)
{}

SLArEventMegatile::SLArEventMegatile(const SLArEventMegatile& right) 
  : TNamed(right) 
{
  fIdx = right.fIdx; 
  fNhits = right.fNhits; 
  fIsActive = right.fIsActive; 
  fLightBacktrackerRecordSize = right.fLightBacktrackerRecordSize;
  fChargeBacktrackerRecordSize = right.fChargeBacktrackerRecordSize;
  for (const auto &evtile : right.fTilesMap) {
    fTilesMap[evtile.first] = evtile.second;
  }
}


SLArEventMegatile::SLArEventMegatile(const SLArCfgMegaTile* cfg) 
  : SLArEventMegatile()
{
  SetIdx(cfg->GetIdx());
  SetName(cfg->GetName());
  //ConfigModule(cfg); 
}


int SLArEventMegatile::ResetHits() {
  int nhits = 0;
  for (auto &tile : fTilesMap) {
    nhits += tile.second.GetNhits(); 
    tile.second.ResetHits(); 
    //delete tile.second;
  }

  fTilesMap.clear();
  
  return nhits; 
}


SLArEventMegatile::~SLArEventMegatile()
{
  ResetHits();
  //fTilesMap.clear(); 
}

int SLArEventMegatile::ConfigModule(const SLArCfgMegaTile* cfg) {
  int ntiles = 0; 
  for (auto &cfgTile : cfg->GetConstMap()) {
    int id_tile = cfgTile.GetID(); 
    fTilesMap.insert(std::make_pair(id_tile, SLArEventTile(id_tile) ));
    ++ntiles; 
  }

  return ntiles; 
}


SLArEventTile& SLArEventMegatile::GetOrCreateEventTile(const int& tileId) 
{
  auto it  = fTilesMap.find(tileId); 
  if (it != fTilesMap.end()) {
    //printf("SLArEventMegatile::CreateEventTile(%i) WARNING: Tile nr %i already present in MegatTile %i register\n", tileIdx, tileIdx, fIdx);
    return fTilesMap.find(tileId)->second;
  }
  else {
    fTilesMap.insert( std::make_pair(tileId, SLArEventTile(tileId) ) );  
    auto& t_event = fTilesMap[tileId];
    t_event.SetBacktrackerRecordSize( fLightBacktrackerRecordSize ); 
    t_event.SetChargeBacktrackerRecordSize( fChargeBacktrackerRecordSize ); 
    return t_event;
  }
}


SLArEventTile& SLArEventMegatile::RegisterHit(const SLArEventPhotonHit& hit, const int idx) {
  int tile_idx = idx;
  if (idx < 0) {
    tile_idx = hit.GetTileID();
  }

  fNhits++; 
  
  auto& tile_ev = GetOrCreateEventTile(tile_idx);
  tile_ev.RegisterHit(hit);
  return tile_ev;
}

int SLArEventMegatile::GetNPhotonHits() const {
  int nhits = 0;
  for (const auto &tile : fTilesMap) {
    nhits += tile.second.GetNhits(); 
  }

  return nhits; 
}

int SLArEventMegatile::GetNChargeHits() const {
  int nhits = 0;
  for (const auto &tile : fTilesMap) {
    nhits += tile.second.GetPixelHits(); 
  }

  return nhits; 
}


void SLArEventMegatile::SetActive(bool is_active) {
  for (auto &tile : fTilesMap) {
    tile.second.SetActive(is_active); 
  } 
  return;
}

//bool SLArEventMegatile::SortHits() {
  //for (auto &tile : fTilesMap) {
    //tile.second->SortHits(); 
  //}

  //return true;
//}
