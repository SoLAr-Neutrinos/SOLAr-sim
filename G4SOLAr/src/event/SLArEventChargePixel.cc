/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventChargePixel.cc
 * @created     : Fri Nov 11, 2022 14:58:07 CET
 */

#include "event/SLArEventChargePixel.hh"

ClassImp(SLArEventChargePixel)

SLArEventChargePixel::SLArEventChargePixel() 
  : SLArEventHitsCollection<SLArEventChargeHit>()
{
  fClockUnit = 50;
}

SLArEventChargePixel::SLArEventChargePixel(const int& idx, const SLArEventChargeHit& hit)
  : SLArEventHitsCollection<SLArEventChargeHit>(idx) 
{
  fName = Form("EvPix%i", fIdx); 
  fClockUnit = 50; 
  RegisterHit(hit); 
}

SLArEventChargePixel::SLArEventChargePixel(const SLArEventChargePixel& right) 
  : SLArEventHitsCollection<SLArEventChargeHit>(right) 
{}
