/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : solarfd_eve_pos.cc
 * @created     : Thursday Dec 12, 2024 11:05:50 CET
 */

#include <iostream>
#include "TFile.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TH3F.h"
#include "ROOT/RDataFrame.hxx"

#include "event/SLArMCEvent.hh"
#include "event/SLArMCPrimaryInfo.hh"
#include "event/SLArEventTrajectory.hh"

int solarfd_eve_pos(const TString& file_path)
{
  TChain chain_mcevent("EventTree"); 
  chain_mcevent.AddFile( file_path ); 
  SLArMCEvent* ev = nullptr; 
  chain_mcevent.SetBranchAddress("MCEvent", &ev); 

  TFile* test_file = new TFile("test_vertex.root", "recreate"); 
  TH3F* h3 = new TH3F("hvtx", "vertex coords;x [mm]; y [mm]; z [mm]", 50, -7e3, 7e3, 50, -7e3, 7e3, 100, -30e3, 30e3); 

  Long64_t n_entries = chain_mcevent.GetEntries(); 
  for (Long64_t i = 0; i< n_entries; i++) {
    chain_mcevent.GetEntry(i); 

    const auto& primaries  = ev->GetPrimaries(); 
    for (const auto& p : primaries) {
      if (p.GetID() == 11) {
        //const auto& t = p.GetConstTrajectories().at(0); 
        //const auto& v = t->GetPoints().front(); 
        //h3->Fill( v.fX, v.fY, v.fZ ); 
        const auto& v = p.GetVertex();
        h3->Fill( v[0], v[1], v[2] );
        continue;
      }
    }
  }

  h3->Write(); 
  test_file->Close(); 
    
  return 0;
}

int test_rdf(const TString& file_path)
{
  TChain chain_mcevent("EventTree");
  chain_mcevent.AddFile( file_path );

  ROOT::RDataFrame df( chain_mcevent );
  auto h3 = df
    .Define("vtx", 
      [](SLArMCEvent& ev) {
        const auto& primaries  = ev.GetPrimaries();
        for (const auto& p : primaries) {
          if (p.GetID() == 11) {
            const auto& t = p.GetConstTrajectories().at(0);
            const auto& pt = t->GetPoints().front();
            return std::vector<float>{pt.fX, pt.fY, pt.fZ};
          }
        }
        return std::vector<float>{0, 0, 0};
      }, {"MCEvent"})
    .Define("vtx_x", "vtx[0]")
    .Define("vtx_y", "vtx[1]")
    .Define("vtx_z", "vtx[2]")
    .Histo3D<float>({"hvtx", "vertex coords;x [mm]; y [mm]; z [mm]", 50, -7e3, 7e3, 50, -7e3, 7e3, 100, -30e3, 30e3}, 
        "vtx_x", "vtx_y", "vtx_z");

  TCanvas* cVtx = new TCanvas(); 
  h3->Draw("box");
  cVtx->Update(); 
  getchar();

  return 0;
}
