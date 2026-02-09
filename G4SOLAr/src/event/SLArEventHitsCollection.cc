/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventHitsCollection.cc
 * @created     : Fri Nov 11, 2022 14:28:03 CET
 */

#include "event/SLArEventHitsCollection.hh"
#include "event/SLArEventChargeHit.hh"
#include "event/SLArEventPhotonHit.hh"

templateClassImp(SLArEventHitsCollection)

template<class T>
SLArEventHitsCollection<T>::SLArEventHitsCollection() 
  : TNamed(), fIdx(0), fIsActive(true), fNhits(0), fBacktrackerRecordSize(0), fClockUnit(1)
{}

template<class T>
SLArEventHitsCollection<T>::SLArEventHitsCollection(const int idx) 
  : TNamed(), fIdx(idx), fIsActive(true), fNhits(0), fBacktrackerRecordSize(0), fClockUnit(1) {}


template<class T>
SLArEventHitsCollection<T>::SLArEventHitsCollection(const int idx, const UShort_t clock) 
  : TNamed(), fIdx(idx), fIsActive(true), fNhits(0), 
    fBacktrackerRecordSize(0), fClockUnit(clock) {}

template<class T>
SLArEventHitsCollection<T>::SLArEventHitsCollection(const SLArEventHitsCollection<T>& other)
  : TNamed(other) 
{
  fIdx = other.fIdx; 
  fIsActive = other.fIsActive; 
  fNhits = other.fNhits; 
  fClockUnit = other.fClockUnit;
  fBacktrackerRecordSize = other.fBacktrackerRecordSize;

  if (!other.fHits.empty()) {
    fHits = HitsCollection_t(other.fHits); 
    fBacktrackerCollections = BacktrackerVectorCollection_t(other.fBacktrackerCollections);
  }
}

template<class T>
SLArEventHitsCollection<T>::~SLArEventHitsCollection() {
  ResetHits();
}

template<class T>
void SLArEventHitsCollection<T>::Copy(SLArEventHitsCollection& record) const 
{
  record.SetBacktrackerRecordSize( fBacktrackerRecordSize ); 
  record.SetIdx( fIdx ); 
  record.SetActive( fIsActive ); 
  record.SetClockUnit( fClockUnit ); 
  record.SetNhits( fNhits ); 

  for (const auto &hit : fHits) {
    record.GetHits()[hit.first] = hit.second;
  }   

  for (const auto &bktv : fBacktrackerCollections) {
    record.GetBacktrackerRecordCollection()[bktv.first] = bktv.second;
  }

  return;
}

template<class T>
int SLArEventHitsCollection<T>::RegisterHit(const T hit) {
  fHits[ConvertToClock<float>(hit.GetTime())]++; 
  fNhits++; 
  return fNhits;
}

//template<class T>
//bool SLArEventHitsCollection<T>::SortHits() {
  //std::sort(fHits.begin(), fHits.end(), T::CompareHitPtrs); 
  //return true; 
//}

template<class T>
int SLArEventHitsCollection<T>::ResetHits() {
  fHits.clear(); 
  for (auto &b : fBacktrackerCollections) {
    b.second.Reset();
  }
  fBacktrackerCollections.clear();
  fNhits = 0; 
  return fHits.size(); 
}

template<class T>
void SLArEventHitsCollection<T>::PrintHits() const {
  printf("Hit container ID: %i [%s]\n", fIdx, fName.Data());
  printf("- - - - - - - - - - - - - - - - - - - - - - -\n");
  for (auto &hit : fHits) {
    printf("[%u] : %i\n", hit.first*fClockUnit, hit.second);
  }
  printf("\n");
}

template<class T>
SLArEventBacktrackerVector& SLArEventHitsCollection<T>::GetBacktrackerVector(UShort_t key) {
  //printf("SLArEventHitsCollection[%i]::GetBacktrackerVector[%u]\n", fIdx, key);
  auto& bkt_vector = fBacktrackerCollections[key];
  if (bkt_vector.IsEmpty() == false) {
    //printf("[%i] Already have a backtrackervector at key: %u\n", fIdx, key);
    return bkt_vector;
  }
  else if (bkt_vector.IsEmpty() && fBacktrackerRecordSize <= 0) {
    //printf("BacktrackerRecordSize is %u. I should not be here...\n", 
        //fBacktrackerRecordSize);
    throw 4;
  }
  else if (bkt_vector.IsEmpty() && fBacktrackerRecordSize > 0) {
    //printf("initializing backtracker records vector to size %u\n", 
        //fBacktrackerRecordSize);
    bkt_vector.InitRecords(fBacktrackerRecordSize);
  }

  return bkt_vector;
}

template<class T> 
int SLArEventHitsCollection<T>::ZeroSuppression(const UShort_t threshold) {
  int hits_erased = 0; 
  //printf("ZeroSuppression threshold = %u\n", threshold);
  for (auto it = fHits.begin(); it != fHits.end(); ) {
    if (it->second < threshold) {
      auto key = it->first;
      //printf("deleting map entry with key [%u] having %u hits\n", key, it->second);
      fBacktrackerCollections.erase(key);
      hits_erased += it->second;
      it = fHits.erase(it);
    } else {
      ++it;
    }
  }
  return hits_erased;
}

template class SLArEventHitsCollection<SLArEventPhotonHit>; 
template class SLArEventHitsCollection<SLArEventChargeHit>; 
