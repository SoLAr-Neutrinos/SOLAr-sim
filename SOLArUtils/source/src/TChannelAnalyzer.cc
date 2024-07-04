/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : TChannelAnalyzer.cc
 * @created     : Tuesday Apr 16, 2024 14:33:52 CEST
 */

#include <iostream>
#include <TChannelAnalyzer.hh>
#include <TList.h>
#include <TRandom3.h>

int TChannelAnalyzer::process_channel(const Int_t& pix_bin, const SLArEventChargePixel& pix_ev, hitvarContainers_t& hitvars)
{
  int nhit = 0;
  if (pix_ev.GetNhits() < 200) return nhit;

  //printf("\t\tpixel [%i]: %i hits\n", pixel_itr.first, pixel_itr.second.GetNhits());
  if (fClockUnit == 0) fClockUnit = pix_ev.GetClockUnit(); 
  UInt_t window_int_cu = std::round(fIntegrationWindow * 1000 / fClockUnit);
  const auto& hit_stream = pix_ev.GetConstHits();
            
  UInt_t q = 0;
  UInt_t trigger_t = 0; 
  Bool_t sampling = false;
  UInt_t sampled_time = 0;
  for (auto stream_itr = hit_stream.begin(); stream_itr != hit_stream.end(); stream_itr++) {
    q += stream_itr->second;
    //printf("\t\t\t q = %u\n", q);
    if ( q > fHitThreshold ) {
      trigger_t = stream_itr->first;
      sampling = true;
    }

    if (sampling) {
      sampled_time = stream_itr->first - trigger_t;
      if (sampled_time > window_int_cu || std::next(stream_itr) == hit_stream.end()) {
        try {
          record_hit(pix_bin, q, trigger_t, hitvars);
        }
        catch (const std::exception& e) {
          std::cerr << e.what() << std::endl;
        }

        q = 0; 
        trigger_t = 0;
        sampling = false;
      } // end of record and reset
    } // end of sampling
      //getchar(); 
  } // end of loop over hit stream on pixel
  return nhit;
}

int TChannelAnalyzer::record_hit(const Int_t& pix_bin, const UInt_t& q, const UInt_t& trigger_t, hitvarContainers_t& hitvars) {
  //create hit and reset
  RecoHit_t hit; 
  hit.time = trigger_t * fClockUnit;
  TH2Poly* hbin = fCfgAnode->GetAnodeMap(2); 
  TH2PolyBin* bin = nullptr;
  bin = (TH2PolyBin*)hbin->GetBins()->At(pix_bin-1);

  //printf("%i: %p\n", pix_bin-1, static_cast<void*>(bin)); 
  if (bin != nullptr) {
    TVector3 pad_pos = get_bin_center( bin, fCfgAnode->GetAxis0(), fCfgAnode->GetAxis1() ); 
    TVector3 t_phys( fCfgTile->GetPhysX(), fCfgTile->GetPhysY(), fCfgTile->GetPhysZ() ); 
    TVector3 anode_pos( fCfgAnode->GetX(), fCfgAnode->GetY(), fCfgAnode->GetZ()); 
    anode_pos += (*fTPCCenterPosition); 
    TVector3 mt_pos ( fCfgMegaTile->GetX(), fCfgMegaTile->GetY(), fCfgMegaTile->GetZ()); 
    TVector3 t_pos ( fCfgTile->GetX(), fCfgTile->GetY(), fCfgTile->GetZ()); 

    TVector3 pad_anode_pos = mt_pos + t_pos;

    TVector3 pad_anode_phys = pad_anode_pos.Transform(fRot) + anode_pos + pad_pos;
    
    //printf("ANODE %i: anode_pos : %.2f, %.2f, %.2f mm\n", fCfgAnode->GetID(), anode_pos.x(), anode_pos.y(), anode_pos.z()); 
    //printf("MT %i: mt_pos: %.2f, %.2f, %.2f\n", fCfgMegaTile->GetIdx(), mt_pos.x(), mt_pos.y(), mt_pos.z());
    //printf("T %i: t_pos: %.2f, %.2f, %.2f\n", fCfgTile->GetIdx(), t_pos.x(), t_pos.y(), t_pos.z());
    //printf("P pos: %.2f, %.2f, %.2f mm\n", pad_pos.x(), pad_pos.y(), pad_pos.z()); 

    //printf("pad_anode_pos: %.2f, %.2f, %.2f mm\n", pad_anode_pos.x(), pad_anode_pos.y(), pad_anode_pos.z()); 
    //printf("pad_anode_phys: %.2f, %.2f, %.2f mm\n", pad_anode_phys.x(), pad_anode_phys.y(), pad_anode_phys.z()); 
    //getchar();

    TVector3 hit_phys = pad_anode_phys ;
    TVector3 drift_coordinate = fDriftVelocity * trigger_t * fClockUnit * (*fDriftDirection);
    TVector3 hit_coordinates = hit_phys + drift_coordinate;
    
    hit.x = hit_coordinates.x(); 
    hit.y = hit_coordinates.y(); 
    hit.z = hit_coordinates.z(); 
    hit.charge_true = q;
    hit.charge_reco = q + gRandom->Gaus(0, fChannelPedestalRMS); 
    hit.tpc_id = fTPCID;

    if (hit.charge_reco >= fHitThreshold) {
      hitvars.push_back( hit ); 
    }
  }
  else {
    char err_msg[200]; 
    sprintf(err_msg, "TChannelAnalyzer::record_hit(%i) ERROR: Cannot find associated bin for ch with index %i\n", pix_bin, pix_bin); 
    throw std::runtime_error(err_msg);
  }

  return 1;
}

TVector3 TChannelAnalyzer::get_bin_center(TH2PolyBin* bin, const TVector3& axis_x, const TVector3& axis_y) {
  TGraph* g = static_cast<TGraph*>(bin->GetPolygon());
  double x = 0.; 
  double y = 0.;
  for (int i=0; i<g->GetN()-1; i++) {
    x += g->GetX()[i]; 
    y += g->GetY()[i]; 
  }

  x /= (g->GetN() -1); 
  y /= (g->GetN() -1); 
  TVector3 bin_pos = axis_x*x + axis_y*y;

  return bin_pos;
}



