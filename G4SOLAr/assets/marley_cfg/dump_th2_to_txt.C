/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : dump_th2_to_txt
 * @created     : Thursday Sep 05, 2024 13:36:28 CEST
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <TH2F.h>
#include <TFile.h>

int dump_th2_to_txt(const TString rootfile_path)
{
  TFile fsource(rootfile_path); 
  TH2F* hh = fsource.Get<TH2F>("hist"); 

  std::ofstream output; 
  TString output_name = rootfile_path; 
  output_name.Resize( output_name.Index(".root" )); 
  output_name.Append(".txt"); 
  output.open( output_name );

  for (size_t iy=1; iy <= hh->GetNbinsY(); iy++) {
    for (size_t ix=1; ix <= hh->GetNbinsX(); ix++) {
      float bc = hh->GetBinContent(ix, iy); 
      output << bc << "\t"; 
    }
    output << "\n";
  }

  output.close(); 
  fsource.Close(); 
  return 0;
}

int dump_txt_to_th2(const TString txtfile_path) {
  std::ifstream input; 
  input.open( txtfile_path ); 

  TString output_name = txtfile_path; 
  output_name.Resize( output_name.Index(".txt") ); 
  output_name.Append("_local.root"); 
  TFile foutput(output_name, "recreate"); 
  TH2F* h2 = new TH2F("b8_oscillogram", ";Energy [MeV];cos(#theta_{nadir})", 
      300, 0, 30, 2001, -1, 1); 

  std::string line; 
  size_t iy = 1; 
  while ( getline(input, line) ) {
    std::istringstream iss(line); 
    size_t ix = 1; 
    float bc; 
    while ( iss >> bc ) {
      h2->SetBinContent(ix, iy, bc);
      ix++;
    }

    iy++; 
  }

  h2->Write(); 
  foutput.Close(); 
  return 0; 
}
