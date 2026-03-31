/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : test_vector_float.C
 * @created     : Thursday Nov 21, 2024 17:30:00 CET
 */

#include <iostream>
#include <vector>
#include "TFile.h"
#include "TTree.h"
#include "SLArUnit.hpp"

int test_vector_float()
{
  TFile *f = new TFile("test_vector_float.root", "RECREATE");
  TTree *t = new TTree("t", "t");
  std::vector<Float_t> v;
  t->Branch("v", &v);

  const double unit_val = unit::Unit2Val( "cm" );

  for (int i = 0; i < 10; i++) {
    v.clear();
    for (int j = 0; j < i; j++) {
      v.push_back( unit_val * j);
    }
    t->Fill();
  }

  t->Write();
  f->Close();
    
  return 0;
}

int main (int argc, char *argv[]) {
  test_vector_float();
  return 0;
}
