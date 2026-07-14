/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : TChannelAnalyzer.hh
 * @created     : Tuesday Apr 16, 2024 14:31:44 CEST
 */

#ifndef TCHANNELANALYZER_HH

#define TCHANNELANALYZER_HH

#include "TRotation.h"
#include "event/SLArEventChargePixel.hh"
#include "config/SLArCfgAnode.hh"
#include "SLArRecoHits.hh"

class TChannelAnalyzer {
  public: 
    TChannelAnalyzer() {}
    virtual ~TChannelAnalyzer() {}

    inline UInt_t get_clock_unit() const {return fClockUnit;}
    inline Float_t get_channel_pedestal_rms() const {return fChannelPedestalRMS;}
    inline Float_t get_drift_velocity() const {return fDriftVelocity;}
    inline Float_t get_integration_window_us() const {return fIntegrationWindow_us;}
    inline Float_t get_reset_time_us() const {return fResetTime_us;}
    inline Float_t get_hit_threshold() const {return fHitThreshold;}

    inline void set_hit_threshold(const Float_t thr) {fHitThreshold = thr;}
    inline void set_integration_window_us(const Float_t win) {fIntegrationWindow_us = win;}
    inline void set_reset_time_us(const Float_t reset) {fResetTime_us = reset;}
    inline void set_drift_velocity(const Float_t v) {fDriftVelocity = v;}
    inline void set_drift_direction(const TVector3& v) {fDriftDirection = &v;}
    inline void set_tpc_center_position(const TVector3& pos) {fTPCCenterPosition = &pos;}
    inline void set_tpc_rotation(const TRotation& rot_inv) { if (fRotInv) delete fRotInv; fRotInv = &rot_inv; } 
    inline void set_channel_rms(const Float_t rms) {fChannelPedestalRMS = rms;}
    inline void set_anode_config(SLArCfgAnode* anode_cfg) {
      fCfgAnode = anode_cfg;
      fRot.SetXEulerAngles( anode_cfg->GetPhi(), anode_cfg->GetTheta(), anode_cfg->GetPsi() );      
    }
    inline void set_megatile_config(const SLArCfgMegaTile* mt_cfg) {fCfgMegaTile = mt_cfg;}
    inline void set_tile_config(const SLArCfgReadoutTile* tile_cfg) {fCfgTile = tile_cfg;}
    inline void set_tpc_id(const Int_t itpc) {fTPCID = itpc;} 

    int process_channel(const Int_t& pix_bin, const SLArEventChargePixel& pix_ev, reco::hitvarContainer& hitvars); 

  private: 
    Float_t fIntegrationWindow_us = {}; 
    Float_t fHitThreshold = {};
    Float_t fChannelPedestalRMS = {};
    Float_t fDriftVelocity = {};
    Float_t fResetTime_us =  0.6;
    UInt_t fClockUnit = 0;
    Int_t fTPCID = 0;
    TRotation fRot = {};
    const SLArCfgReadoutTile* fCfgTile = {}; 
    SLArCfgAnode* fCfgAnode = {};
    const SLArCfgMegaTile* fCfgMegaTile = {};
    const TVector3* fDriftDirection = {};
    const TVector3* fTPCCenterPosition = {};
    const TRotation* fRotInv = new TRotation();

    int record_hit(const Int_t& pix_bin, const UInt_t& q, const UInt_t& trigger_t, reco::hitvarContainer& hitvars);
    TVector3 get_bin_center(TH2PolyBin* bin, const TVector3& axis_x, const TVector3& axis_y);
};




#endif /* end of include guard TCHANNELANALYZER_HH */

