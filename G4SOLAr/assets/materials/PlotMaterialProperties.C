/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : PlotMaterialProperties
 * @created     : sabato set 10, 2022 13:56:21 CEST
 */

#include <iostream>
#include <cassert>
#include <fstream>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TRandom3.h"
#include "G4UIcommand.hh"
#include "wfms_root_styles.C"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/allocators.h"
#include "rapidjson/encodings.h"

void PlotMaterialProperties(const char* json_path)  
{
  FILE* cfg_file = std::fopen(json_path, "r");
  char readBuffer[65536]; 
  rapidjson::FileReadStream is(cfg_file, readBuffer, sizeof(readBuffer)); 

  rapidjson::Document d; 
  d.ParseStream(is); 

  std::vector<TGraph*> vGraph;

  if (!d.HasMember("PropertiesTable")) {
    printf("%s configuration file has no associate properties table\n", json_path); 
    return;
  } else {
    const rapidjson::Value& jptable = d["PropertiesTable"];
    assert(jptable.IsArray());
    const auto properties = jptable.GetArray(); 


    for (const auto& p : properties) {
      assert(p.HasMember("property")); 
      assert(p.HasMember("value")); 

      TString pname = p["property"].GetString(); 

      if (p["value"].IsArray()) {
        assert(p["value"].GetArray().Size() == 2); 
        std::vector<double> vE; vE.reserve(500); 
        std::vector<double> vP; vP.reserve(500); 
        std::vector<double> vL; vL.reserve(500);  
        for (const auto &v : p["value"].GetArray()) {
          assert(v.HasMember("var")); 
          assert(v.HasMember("val")); 
          assert(v["val"].IsArray()); 
          double vunit = 1.0; 
          if (v.HasMember("unit")) {
            vunit = G4UIcommand::ValueOf(v["unit"].GetString());
          }
          if (strcmp("Energy", v["var"].GetString()) == 0) {
            for (const auto &val : v["val"].GetArray()) {
              vE.push_back( val.GetDouble()*vunit ); 
              double E_ = vE.back(); 
              double lambda = TMath::HC() / (E_*1e6*TMath::Qe()) * 1e9; 
              vL.push_back(lambda); 
            }
          } else {
            for (const auto &val : v["val"].GetArray()) {
              vP.push_back( val.GetDouble()*vunit ); 
            } 
          }
        }
        
        TGraph* g = new TGraph(vE.size(), &vL[0], &vP[0]); 
        g->SetNameTitle(Form("g%s", pname.Data()), pname); 
        g->SetLineWidth(3); 
        g->GetXaxis()->SetTitleSize(0.055); 
        g->GetXaxis()->SetLabelSize(0.055); 
        g->GetYaxis()->SetTitleSize(0.055); 
        g->GetYaxis()->SetLabelSize(0.055); 
        g->GetXaxis()->SetTitle("Photon wavelength [nm]"); 
        g->GetXaxis()->CenterTitle(); 
        g->GetYaxis()->CenterTitle(); 

        vGraph.push_back(g); 
      }  
    }

  }

  int ig = 0; 
  //slide_default(); 
  //gROOT->SetStyle("slide_default");
  for (auto &g : vGraph) {
    new TCanvas(g->GetName(), g->GetTitle(), 100*ig, 20*ig, 800, 500);  
    gPad->SetLogy(1); 
    gPad->SetRightMargin(0.05); gPad->SetBottomMargin(0.12); gPad->SetLeftMargin(0.13); 
    gPad->SetTicks(1, 1); 
    gPad->SetGrid(1, 1); 
    g->SetLineColor(kAzure-4); 
    TString titl = g->GetTitle(); 
    if (titl.Contains("SCINTILLATIONCOMPONENT"))
      titl = "LAr scintillation spectrum"; 
    else if (titl.Contains("RAYLEIGH")) {
      titl = "LAr Rayleigh scattering length"; 
      g->GetYaxis()->SetTitle("Scattering length [mm]");

      // scale Rayleigh scattering length to 90 cm @ 128 nm
      /*
       *double scale = 900 / g->Eval(128); 
       *printf("RAYLEIGH SCALE FACTOR IS %g\n", scale); 
       *FILE* rayleigh_file; 
       *rayleigh_file = std::fopen("rayleigh90.txt", "w");
       *for (int ip=0; ip<g->GetN(); ip++) {
       *  fprintf(rayleigh_file, "%.2f, ", g->GetY()[ip]*scale); 
       *}
       *std::fclose(rayleigh_file); 
       */
    } 
    else if (titl.Contains("ABSLENGTH")) {
      titl = "LAr absorption length";
      g->GetYaxis()->SetTitle("Absorption length [mm]");
    } 
    else if (titl.Contains("RINDEX")) {
      titl = "LAr refractive index"; 
      g->GetYaxis()->SetTitle("Refractive index");
    }
    g->SetTitle(titl);  
    g->Draw("awl");
    ig++; 
  }

  return;
}

