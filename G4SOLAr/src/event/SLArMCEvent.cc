/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArMCEvent
 * @created     : mercoledì ago 10, 2022 11:52:40 CEST
 */

#include <event/SLArMCEvent.hh>
#include <TRandom3.h>
#include <cstdio>
#include <typeinfo>
ClassImp(SLArMCEvent)


SLArMCEvent::SLArMCEvent() : TObject(),
  fEvNumber(0), fDirection{0, 0, 0}
{
   fSLArPrimary.reserve(50);
}

SLArMCEvent::SLArMCEvent(const SLArMCEvent& ev) : TObject(ev)
{
  fEvNumber = ev.fEvNumber;
  fDirection = ev.fDirection;

  for (const auto& p : ev.fSLArPrimary) {
    fSLArPrimary.push_back( SLArMCPrimaryInfo(p) );
  }

  for (const auto& itr : ev.fEvAnode) {
    fEvAnode[itr.first] = SLArEventAnode(itr.second);
  }

  for (const auto & itr : ev.fEvSuperCellArray) {
    fEvSuperCellArray[itr.first] = SLArEventSuperCellArray(itr.second);
  }
}

SLArMCEvent::~SLArMCEvent()
{
  std::cerr << "Deleting SLArMCEvent..." << std::endl;
  for (auto &evAnode : fEvAnode) {
    evAnode.second.ResetHits();
    //delete evAnode.second;
  }
  fEvAnode.clear(); 

  for ( auto &scArray : fEvSuperCellArray ) {
    scArray.second.ResetHits();
    //delete scArray.second;
  }
  fEvSuperCellArray.clear(); 

  //for ( auto &p : fSLArPrimary) {
    //delete p; 
  //}
  fSLArPrimary.clear();
  std::cerr << "~SLArMCEvent DONE" << std::endl;
}


int SLArMCEvent::ConfigAnode(const std::map<int, SLArCfgAnode>& anodeCfg)
{
  for (const auto& anode : anodeCfg) {
    fEvAnode.insert(std::make_pair(anode.first, SLArEventAnode(anode.second)));
    fEvAnode[anode.first].SetID(anode.second.GetIdx());
  }

  return fEvAnode.size();
}

int SLArMCEvent::ConfigSuperCellSystem(const SLArCfgSystemSuperCell& supercellSysCfg)
{
  for (const auto& scArray : supercellSysCfg.GetConstMap()) {
    if (fEvSuperCellArray.count(scArray.first)) {
      printf("SLArMCEvent::ConfigSuperCellSystem() WARNING: "); 
      printf("SuperCelll array with index %i is aleady stored in the MCEvent. Skipping.\n", scArray.first);
      continue;
    }

    fEvSuperCellArray.insert(std::make_pair(scArray.first, SLArEventSuperCellArray(scArray.second)));
    fEvSuperCellArray[scArray.first].ConfigSystem(scArray.second);
  }

  return fEvSuperCellArray.size();
}

SLArEventAnode& SLArMCEvent::GetEventAnodeByID(const int& id) {
  for (auto &anode : fEvAnode) {
    if (anode.second.GetID() == id) {return anode.second;}
  }

  throw 4;
}

int SLArMCEvent::SetEvNumber(int nEv)
{
  fEvNumber = nEv;
  return fEvNumber;
}

void SLArMCEvent::Reset()
{
  for (auto &anode : fEvAnode) {
    anode.second.ResetHits();
  }

  for (auto &scArray : fEvSuperCellArray) {
    scArray.second.ResetHits(); 
  }

  //for (auto &p : fSLArPrimary) {
    //delete p;
  //}
  fSLArPrimary.clear(); 

  fDirection = {0, 0, 1};
  fEvNumber = -1;
}

void SLArMCEvent::SetDirection(double* dir) {
  if (dir) {
    fDirection.at(0) = dir[0];  
    fDirection.at(1) = dir[1];  
    fDirection.at(2) = dir[2];  
  } 
}

void SLArMCEvent::SetDirection(double px, double py, double pz) {
    fDirection.at(0) = px;  
    fDirection.at(1) = py;  
    fDirection.at(2) = pz;  
}

bool SLArMCEvent::CheckIfPrimary(int trkId) const {
  bool is_primary = false; 
  for (const auto &p : fSLArPrimary) {
    if (trkId == p.GetTrackID()) {
      is_primary = true; 
      break;
    }
  }
  return is_primary; 
}

size_t SLArMCEvent::RegisterPrimary(SLArMCPrimaryInfo& p) {
  fSLArPrimary.push_back( std::move(p) );
  return fSLArPrimary.size();
}


