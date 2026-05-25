/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArEventChargeHit.cc
 * @created     : Fri Nov 11, 2022 13:44:52 CET
 */

#include "event/SLArEventChargeHit.hh"

ClassImp(SLArEventChargeHit)

SLArEventChargeHit::SLArEventChargeHit()
  : SLArEventGenericHit() {}

SLArEventChargeHit::SLArEventChargeHit(float time, int trkID, int primaryID) 
  : SLArEventGenericHit(time, trkID, primaryID) {}

SLArEventChargeHit::SLArEventChargeHit(const SLArEventChargeHit& h)
  : SLArEventGenericHit(h) 
{}

void SLArEventChargeHit::DumpInfo() {
  printf("charge hit at t = %g, trk id: %i, primaryID: %i\n", 
      fTime, fProducerTrkID, fPrimaryProducerTrkID);
}

