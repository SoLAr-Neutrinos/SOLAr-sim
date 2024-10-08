/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventAnode.cc
 * @created     : Wed Aug 10, 2022 14:39:21 CEST
 */

#include <memory>
#include <SLArAnalysisManager.hh>
#include "event/SLArEventAnode.hh"
#include "config/SLArCfgMegaTile.hh"

ClassImp(SLArEventAnode)

SLArEventAnode::SLArEventAnode() : TNamed(),
    fID(0), fNhits(0), fIsActive(true), 
    fLightBacktrackerRecordSize(0), fChargeBacktrackerRecordSize(0), 
    fZeroSuppressionThreshold(0)
{}

SLArEventAnode::SLArEventAnode(const SLArEventAnode& right) 
  : TNamed(right) 
{
  fID = right.fID; 
  fNhits = right.fNhits;
  fIsActive = right.fIsActive; 
  fLightBacktrackerRecordSize = right.fLightBacktrackerRecordSize;
  fChargeBacktrackerRecordSize = right.fChargeBacktrackerRecordSize;
  fZeroSuppressionThreshold = right.fZeroSuppressionThreshold;
  for (const auto &mgev : right.fMegaTilesMap) {
    fMegaTilesMap[mgev.first] = SLArEventMegatile(mgev.second);
  }
}

SLArEventAnode::SLArEventAnode(const SLArCfgAnode& cfg) : SLArEventAnode() {
  SetName(cfg.GetName()); 
  //ConfigSystem(cfg); 
  return;
}

SLArEventAnode::~SLArEventAnode() {
  for (auto &mgtile : fMegaTilesMap) {
    mgtile.second.ResetHits();
    //delete mgtile.second;
  }
  fMegaTilesMap.clear(); 
}


int SLArEventAnode::ConfigSystem(const SLArCfgAnode& cfg) {
  int imegatile = 0; 
  fID = cfg.GetIdx(); 
  for (const auto &mtcfg : cfg.GetConstMap()) {
    int megatile_id = mtcfg.GetID(); 
    if (fMegaTilesMap.count(megatile_id) == 0) {
      fMegaTilesMap.insert( std::make_pair( megatile_id, SLArEventMegatile(&mtcfg) ) );
      imegatile++;
    }
  }
  return imegatile;
}

SLArEventMegatile& SLArEventAnode::GetOrCreateEventMegatile(const int mtIdx) {
  auto it = fMegaTilesMap.find(mtIdx);
  if (it != fMegaTilesMap.end()) {
    //printf("SLArEventAnode::CreateEventMegatile(%i): Megatile nr %i already present in anode %i register\n", mtIdx, mtIdx, fID);
    //getchar();
    return fMegaTilesMap.find(mtIdx)->second;
  }
  else {
    fMegaTilesMap.insert( std::make_pair(mtIdx, SLArEventMegatile()) );  
    auto& mt_event = fMegaTilesMap[mtIdx];
    mt_event.SetIdx(mtIdx); 
    mt_event.SetLightBacktrackerRecordSize( fLightBacktrackerRecordSize); 
    mt_event.SetChargeBacktrackerRecordSize( fChargeBacktrackerRecordSize ); 
    //printf("SLArEventAnode::CreateEventMegatile(%i): Creating new Megatile nr %i in anode %i register with bktracker record size %u[q] - %u[l]\n",
        //mtIdx, mtIdx, fID, fChargeBacktrackerRecordSize, fLightBacktrackerRecordSize);
    //getchar();
    return mt_event;
  }
}

SLArEventTile& SLArEventAnode::RegisterHit(const SLArEventPhotonHit& hit, int mgtile_idx, int tile_idx) {
  if (mgtile_idx < 0) mgtile_idx = hit.GetMegaTileID(); 
  if (tile_idx < 0) tile_idx = hit.GetTileID(); 
  auto& mt_event = GetOrCreateEventMegatile(mgtile_idx);
  auto& t_event = mt_event.RegisterHit(hit, tile_idx);
  return t_event;
  //} else {
    //printf("SLArEventAnode::RegisterHit WARNING\n"); 
    //printf("Megatile with ID %i is not in store\n", mgtile_idx); 
    //CreateEventMegatile(hit->GetMegaTileIdx());
    //return 0; 
  //}
}

SLArEventChargePixel& SLArEventAnode::RegisterChargeHit(const SLArCfgAnode::SLArPixIdx& pixIdx, const SLArEventChargeHit& hit) {
  const int& mgtile_idx = pixIdx.at(0);
  const int& tile_idx = pixIdx.at(1); 
  const int& pix_idx = pixIdx.at(2); 

  auto& mt_event = GetOrCreateEventMegatile(mgtile_idx); 
  auto& t_event = mt_event.GetOrCreateEventTile(tile_idx);
  auto& p_event = t_event.RegisterChargeHit(pix_idx, hit); 

  return p_event;
  //} else {
    //printf("SLArEventAnode::RegisterHit WARNING\n"); 
    //printf("Megatile with ID %i is not in store\n", mgtile_idx); 
    //CreateEventMegatile(hit->GetMegaTileIdx());
    //return 0; 
  //}
}

int SLArEventAnode::ResetHits() {

  //printf("SLArEventAnode::ResetHits() clear event on anode %i\n", fID);
  int nn = 0; 
  for (auto &mgtile : fMegaTilesMap) {
    nn += mgtile.second.ResetHits(); 
  }

  fMegaTilesMap.clear();
  return nn; 
}

void SLArEventAnode::SetActive(bool is_active) {
  fIsActive = is_active; 
  for (auto &mgtile : fMegaTilesMap) {
    mgtile.second.SetActive(is_active); 
  }
}

Int_t SLArEventAnode::ApplyZeroSuppression() {
  Int_t erasedHits = 0; 

  //printf("SLArEventAnode::ApplyZeroSuppression() Running zero-suppression with threshold %i\n", fZeroSuppressionThreshold);
  for (auto& mt_itr : fMegaTilesMap) {
    auto& tile_map = mt_itr.second.GetTileMap();
    for (auto it_t = tile_map.begin(); it_t != tile_map.end(); it_t++) {
      auto& pix_map = it_t->second.GetPixelEvents(); 
      for (auto it_pix = pix_map.begin(); it_pix!=pix_map.end(); ) {
        auto pix_key = it_pix->first;
        erasedHits += it_pix->second.ZeroSuppression( fZeroSuppressionThreshold ); 
        if (it_pix->second.GetHits().empty()) {
          it_pix = pix_map.erase(it_pix);
        } 
        else {
          it_pix++;
        }
      }
    }
  }

  return erasedHits;
}

//bool SLArEventAnode::SortHits() {
  //int isort = true;
  //for (auto &mgtile : fMegaTilesMap) {
    //isort *= mgtile.second->SortHits(); 
  //}
  //return isort;
//}
