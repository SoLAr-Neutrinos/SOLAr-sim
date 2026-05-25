/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArEventGenericHit.cc
 * @created     : Fri nov 11, 2022 14:07:12 CET
 */

#include "event/SLArEventGenericHit.hh"

ClassImp(SLArEventGenericHit)

SLArEventGenericHit::SLArEventGenericHit()
  : fTime(-1), fProducerTrkID(-1), fPrimaryProducerTrkID(-1) {}

SLArEventGenericHit::SLArEventGenericHit(float t, int prodTrkID, int primaryTrkID)
  : fTime(t), fProducerTrkID(prodTrkID), fPrimaryProducerTrkID(primaryTrkID) {}


SLArEventGenericHit::SLArEventGenericHit(const SLArEventGenericHit& other) 
  : TObject(other) 
{
  fTime = other.fTime; 
  fProducerTrkID = other.fProducerTrkID;
  fPrimaryProducerTrkID = other.fPrimaryProducerTrkID;
}

bool SLArEventGenericHit::operator<(const SLArEventGenericHit& other) const {
  return fTime < other.fTime; 
}

bool SLArEventGenericHit::CompareHitPtrs(const SLArEventGenericHit* left, const SLArEventGenericHit* right) {
  return left->fTime < right->fTime; 
}
