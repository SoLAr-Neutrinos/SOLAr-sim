/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : neutron_argon_check.cc
 * @created     : Wednesday Nov 06, 2024 09:21:59 CET
 */

#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1D.h"

#include "event/SLArMCPrimaryInfo.hh"
#include "event/SLArMCEvent.hh"
#include "event/SLArEventTrajectory.hh"

const TString get_neutron_end_process(const SLArMCPrimaryInfo& primary)
{
  const auto& trajectories = primary.GetConstTrajectories();
  for (const auto& t : trajectories) {
    if (t->GetTrackID() == primary.GetTrackID() && t->GetPDGID() == 2112) {
      return t->GetEndProcess();
    }
  }
  return "";
}

int neutron_argon_check(const TString& file_path)
{
  TFile* file = new TFile(file_path);
  TTree* tree = file->Get<TTree>("EventTree");
  TString file_tag = file_path;
  file_tag.Resize( file_tag.Index(".root") );

  SLArMCEvent* event = 0;
  tree->SetBranchAddress("MCEvent", &event);

  TH1D* hGammaMultiplicity = new TH1D("hGammaMultiplicity_"+file_tag, "Gamma Multiplicity", 10, -0.5, 9.5);
  TH1D* hGammaEnergy = new TH1D("hGammaEnergy_"+file_tag, "Gamma Energy", 1500, 0, 15);
  std::map<TString, UInt_t> gamma_process_counts;
  std::map<TString, UInt_t> neutron_capture_counts;

  for (int i = 0; i < tree->GetEntries(); ++i) {
    tree->GetEntry(i);

    const auto& primary = event->GetPrimary(0);
    const auto& trajectories = primary.GetConstTrajectories();

    size_t gamma_multiplicity = 0;
    printf("\nEvent %i: ", event->GetEvNumber());
    const TString neutron_end_process = get_neutron_end_process(primary);
    if (neutron_end_process.Contains("nCapture + Ar") == false) {
      printf("Primary neutron destructor process: %s - skip event\n", neutron_end_process.Data());
      continue;
    }

    neutron_capture_counts[neutron_end_process]++;

    for (const auto& t : trajectories) {
      if (t->GetPDGID() == 22 && t->GetParentID() == primary.GetTrackID()) {
        hGammaEnergy->Fill(t->GetInitKineticEne());
        const TString& process = t->GetCreatorProcess();
        gamma_process_counts[process]++;
        gamma_multiplicity++;
      }
    }
    hGammaMultiplicity->Fill(gamma_multiplicity);
  }


  TH1D* hGammaCreatorProcess = new TH1D("hGammaCreatorProcess_"+file_tag, "Gamma Creator Process", gamma_process_counts.size(), 0, gamma_process_counts.size());
  TH1D* hNeutronCaptureProcess = new TH1D("hNeutronCaptureProcess_"+file_tag, "Neutron Capture Process", neutron_capture_counts.size(), 0, neutron_capture_counts.size());
  size_t ibin = 1;
  for(const auto& p : gamma_process_counts) {
    hGammaCreatorProcess->SetBinContent(ibin, p.second);
    hGammaCreatorProcess->GetXaxis()->SetBinLabel(ibin, p.first);
    ibin++;
  }

  ibin = 1;
  for (const auto& p : neutron_capture_counts) {
    hNeutronCaptureProcess->SetBinContent(ibin, p.second);
    hNeutronCaptureProcess->GetXaxis()->SetBinLabel(ibin, p.first);
    ibin++;
  }
  
  TCanvas* cMult = new TCanvas("cMult", "cMult", 800, 600);
  cMult->cd(); hGammaMultiplicity->Draw();

  TCanvas* cEnergy = new TCanvas("cEnergy", "cEnergy", 800, 600);
  cEnergy->cd(); hGammaEnergy->Draw();

  TCanvas* cProcess = new TCanvas("cProcess", "cProcess", 800, 600);
  cProcess->cd(); hGammaCreatorProcess->Draw();

  for (const auto& nc : neutron_capture_counts) {
    printf("Neutron capture process: %s - count: %i\n", nc.first.Data(), nc.second);
  }

  printf("\n");

  for (const auto& gp : gamma_process_counts) {
    printf("Gamma creator process: %s - count: %i\n", gp.first.Data(), gp.second);
  }

  TString output_file_name = file_path;
  output_file_name.ReplaceAll(".root", "_hist.root");
  TFile* output_file = new TFile(output_file_name, "RECREATE");
  hGammaMultiplicity->Write();
  hGammaEnergy->Write();
  hGammaCreatorProcess->Write();
  hNeutronCaptureProcess->Write();
  output_file->Close();

  file->Close();


    
  return 0;
}

