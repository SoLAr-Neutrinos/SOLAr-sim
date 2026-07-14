/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArEventChargePixel.cc
 * @created     : Fri Nov 11, 2022 14:58:07 CET
 */

#include "event/SLArEventChargePixel.hh"

ClassImp(SLArEventChargePixel)

SLArEventChargePixel::SLArEventChargePixel() 
  : SLArEventHitsCollection<SLArEventChargeHit>()
{
  fClockUnit = 100;
}

SLArEventChargePixel::SLArEventChargePixel(const int& idx, const SLArEventChargeHit& hit)
  : SLArEventHitsCollection<SLArEventChargeHit>(idx) 
{
  fName = Form("EvPix%i", fIdx); 
  fClockUnit = 100; 
  RegisterHit(hit); 
}

SLArEventChargePixel::SLArEventChargePixel(const SLArEventChargePixel& right) 
  : SLArEventHitsCollection<SLArEventChargeHit>(right) 
{}
