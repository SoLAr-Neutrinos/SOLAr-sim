/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : make_muon_hist
 * @created     : Monday May 19, 2025 12:34:15 CEST
 */

#include <iostream>
#include "TFile.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TH1D.h"

int make_muon_hist()
{
   TGraph* g = new TGraph("~/Downloads/muon_spectrum.txt");
   g->SetMarkerStyle(20);

   TF1* f = new TF1("fEval", [g](double*x, double*p) {return g->Eval(x[0], 0, "s");}, 
       1, 2e4, 0);

   // create logarithmically spaced bins between 1 and 2e4 
   const int n_bins = 506;
   const double x_min = 1.0e3;  const double log_xmin = log10(x_min);
   const double x_max = 2.e7; const double log_xmax = log10(x_max);
   const double log_bin_width = (log_xmax - log_xmin) / n_bins;

   std::vector<double> x_bins(n_bins+1, 0.0);
   for (int i=0; i<n_bins+1; i++)
   {
       x_bins[i] = TMath::Power(10, log_xmin + i*log_bin_width);
   }

   TH1D* h = new TH1D("h", "Muon spectrum", n_bins, &x_bins[0]);

   for (int i=0; i<n_bins; i++)
   {
      const double val = TMath::Max(0.0, f->Eval( h->GetBinCenter(i+1)*1e-3 ));
      h->SetBinContent(i+1, val);
   }

   TFile* f_out = new TFile("cosmic_muon_spectrum.root", "RECREATE");
   h->Write("zhu_li_beacom_mu_spectrum");
   f_out->Close();


   //TCanvas* c = new TCanvas("c", "Muon spectrum", 800, 600);
   //h->Draw("hist");
   //g->Draw("pl same");

   return 0;
}

