/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArEventChargePixel.hh
 * @created     : Fri Nov 11, 2022 13:51:31 CET
 */

#ifndef SLAREVENTCHARGEPIXEL_HH

#define SLAREVENTCHARGEPIXEL_HH

#include "event/SLArEventHitsCollection.hh"
#include "event/SLArEventChargeHit.hh"

class SLArEventChargePixel : public SLArEventHitsCollection<SLArEventChargeHit> {
  public: 
    SLArEventChargePixel(); 
    SLArEventChargePixel(const int&, const SLArEventChargeHit&); 
    SLArEventChargePixel(const SLArEventChargePixel&); 
    ~SLArEventChargePixel() {}

  private: 

  public: 
    ClassDef(SLArEventChargePixel, 1)
}; 


#endif /* end of include guard SLAREVENTCHARGEPIXEL_HH */

