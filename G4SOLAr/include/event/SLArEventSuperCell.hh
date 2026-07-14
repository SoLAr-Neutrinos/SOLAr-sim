/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArEventSuperCell.hh
 * @created     : Thur Oct 20, 2022 15:28:53 CEST
 */

#ifndef SLAREVENTSUPERCELL_HH

#define SLAREVENTSUPERCELL_HH

#include <iostream>
#include <map>
#include "event/SLArEventHitsCollection.hh"
#include "event/SLArEventPhotonHit.hh"

class SLArEventSuperCell : public SLArEventHitsCollection<SLArEventPhotonHit> {
  public: 
    SLArEventSuperCell(); 
    SLArEventSuperCell(int idx); 
    SLArEventSuperCell(const SLArEventSuperCell& ev); 
    ~SLArEventSuperCell(); 

    inline int GetIdx() const {return fIdx;}
    inline int GetNhits() const {return fNhits;}
    double GetTime() const; 
    double GetTime(EPhProcess proc) const;
    bool IsActive() {return fIsActive;}
    void PrintHits() const ; 

  protected:

  public:
    ClassDef(SLArEventSuperCell, 2); 
}; 


#endif /* end of include guard SLAREVENTSUPERCELL_HH */

