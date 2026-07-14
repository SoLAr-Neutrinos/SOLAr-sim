/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArEventChargeHit.hh
 * @created     : Fri Nov 11, 2022 13:35:51 CET
 */

#ifndef SLAREVENTCHARGEHIT_HH

#define SLAREVENTCHARGEHIT_HH

#include "event/SLArEventGenericHit.hh"

class SLArEventChargeHit : public SLArEventGenericHit {
  public: 
    SLArEventChargeHit(); 
    SLArEventChargeHit(float time, int trkId=-1, int primaryID=-1);
    SLArEventChargeHit(const SLArEventChargeHit& h);
    ~SLArEventChargeHit() {}; 

    void   DumpInfo(); 

  private:

  public: 
    ClassDef(SLArEventChargeHit, 1);
};

#endif /* end of include guard SLAREVENTCHARGEHIT_HH */

