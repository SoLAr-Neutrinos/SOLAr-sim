/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : TChannelAnalyzer.hh
 * @created     : Tuesday Apr 16, 2024 14:31:44 CEST
 */

#ifndef TCHANNELANALYZER_HH

#define TCHANNELANALYZER_HH

#include <event/SLArEventChargePixel.hh>
#include <config/SLArCfgAnode.hh>
#include <SLArRecoHits.hpp>

class TChannelAnalyzer {
  public: 
    TChannelAnalyzer() {}
    virtual ~TChannelAnalyzer() {}

    inline void set_integration_window(const Float_t win) {fIntegrationWindow = win;}
    inline void set_drift_velocity(const Float_t v) {fDriftVelocity = v;}
    inline void set_drift_direction(const TVector3& v) {fDriftDirection = &v;}
    inline void set_tpc_center_position(const TVector3& pos) {fTPCCenterPosition = &pos;}
    inline void set_channel_rms(const Float_t rms) {fChannelPedestalRMS = rms;}
    inline void set_anode_config(SLArCfgAnode* anode_cfg) {
      fCfgAnode = anode_cfg;
      fRot.SetXEulerAngles( anode_cfg->GetPhi(), anode_cfg->GetTheta(), anode_cfg->GetPsi() );      
    }
    inline void set_megatile_config(const SLArCfgMegaTile* mt_cfg) {fCfgMegaTile = mt_cfg;}
    inline void set_tile_config(const SLArCfgReadoutTile* tile_cfg) {fCfgTile = tile_cfg;}
    inline void set_tpc_id(const Int_t itpc) {fTPCID = itpc;} 

    int process_channel(const Int_t& pix_bin, const SLArEventChargePixel& pix_ev, hitvarContainers_t& hitvars); 

  private: 
    Float_t fIntegrationWindow = {}; 
    Float_t fHitThreshold = {};
    Float_t fChannelPedestalRMS = {};
    Float_t fDriftVelocity = {};
    UInt_t fClockUnit = 0;
    Int_t fTPCID = 0;
    TRotation fRot = {};
    const SLArCfgReadoutTile* fCfgTile = {}; 
    SLArCfgAnode* fCfgAnode = {};
    const SLArCfgMegaTile* fCfgMegaTile = {};
    const TVector3* fDriftDirection = {};
    const TVector3* fTPCCenterPosition = {};

    int record_hit(const Int_t& pix_bin, const UInt_t& q, const UInt_t& trigger_t, hitvarContainers_t& hitvars);
    TVector3 get_bin_center(TH2PolyBin* bin, const TVector3& axis_x, const TVector3& axis_y);
};




#endif /* end of include guard TCHANNELANALYZER_HH */

