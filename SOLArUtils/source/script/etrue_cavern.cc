/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : etrue_cavern.cc
 * @created     : Friday Mar 06, 2026 14:36:16 CET
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <getopt.h>
#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TPRegexp.h"
#include "TH1D.h"
#include "TH2D.h"

#include "event/SLArGenRecords.hh"
#include "event/SLArMCTruth.hh"
#include "event/SLArEventTrajectory.hh"

void print_usage() {
  printf("SoLAr etrue_cavern: Calibrate hit collection\n"); 
  printf("Usage: "); 
  printf("hit_calibration\n"); 
  printf("\t[--input_list | -l] input file list\n"); 
  printf("\t[--output     | -o] output filename\n\n");
  printf("\t[--buffer     | -b] buffer thickness (in mm)");
  exit( EXIT_SUCCESS );
}

const double tpc_half_x = 7.5e3; 
const double tpc_half_y = 6.52e3;
const double tpc_half_z = 30.25e3; 

bool is_inside_buffer(const double& x, const double& y, const double& z, const double& buffer_len)
{
  double tpc_half_x_fv = tpc_half_x - buffer_len;
  double tpc_half_y_fv = tpc_half_y - buffer_len;
  double tpc_half_z_fv = tpc_half_z - buffer_len;

  if      ( fabs(x) > tpc_half_x_fv ) return true;
  else if ( fabs(y) > tpc_half_y_fv ) return true; 
  else if ( fabs(z) > tpc_half_z_fv ) return true;
  
  return false;
}

double min_distance_from_membrane(const double& x, const double& y, const double& z) {
  std::vector<double> diff(3, 0.0);
  diff[0] = tpc_half_x*0.1 - fabs(x); 
  diff[1] = tpc_half_y*0.1 - fabs(y); 
  diff[2] = tpc_half_z*0.1 - fabs(z);

  std::sort( diff.begin(), diff.end() );

  return diff.front();
}

int main (int argc, char *argv[]) {
  // define analysis constants
  const double drift_velocity = 158.2; // cm/s
  const double argon_w = 23.6e-6; // MeV
  const double recombination_factor = 1.0 / 0.6; 
  bool is_neutron = false;
  bool debug = false;
  
  TString mc_file_list = {}; 
  TString output_filename = {};

  // Parse command line options
  const char* short_opts = "l:o:b:dnh";
  // Define long options
  static struct option long_options[] = {
    {"mcfile_list", required_argument, 0, 'l'},
    {"output", required_argument, 0, 'o'},
    {"neutron", no_argument, 0, 'n'},
    {"debug", no_argument, 0, 'd'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;

  while ( (c = getopt_long(argc, argv, short_opts, long_options, &option_index)) != -1) {
    switch(c) {
      case 'l' :
        mc_file_list = optarg;
        break;
      case 'o' :
        output_filename = optarg;
        break;
      case 'n' :
        is_neutron = true;
        break;
      case 'd' :
        debug = true;
        break;
      case 'h' : 
        print_usage(); 
        exit( EXIT_SUCCESS ); 
        break;
      case '?' : 
        printf("solar_sim error: unknown flag %c\n", optopt);
        print_usage(); 
        exit( EXIT_FAILURE ); 
        break;
    }
  }

  // setup the chain
  TChain chain_gen("GenTree");
  TChain chain_truth("EventTree"); 

  std::string line; 
  std::ifstream file_list; 
  file_list.open(mc_file_list.Data());
  if (file_list.is_open() == false) {
    std::cerr << "Error opening file list: " << mc_file_list.Data() << std::endl;
    return 1;
  }
  
  TPRegexp rgx_seed("[0-9]{10}");

  while (std::getline(file_list, line)) {
    TString mc_file_path = line;
    const int seed_pos = mc_file_path.Index( rgx_seed );

    TFile* mc_file = TFile::Open(mc_file_path);
    if (mc_file == nullptr || mc_file->IsZombie()) {
      std::cerr << "Error: mc_file " << mc_file_path << " does not exist or is zombie. Skip" << std::endl; 
      continue;
    }

    TTree* eve_tree = mc_file->Get<TTree>("EventTree");
    if (eve_tree == nullptr) {
      std::cerr << "Error: EventTree not found in file " << mc_file_path.Data() << std::endl;
      continue;
    }

    mc_file->Close();

    chain_truth.Add(mc_file_path);
    chain_gen.Add(mc_file_path);
  }

  Long64_t nentries = chain_truth.GetEntries();
  chain_truth.SetBranchStatus("EventAnode", 0); 
 
  SLArMCTruth* truth = nullptr;
  chain_truth.SetBranchAddress("MCTruth", &truth);

  TFile* output_file = new TFile(output_filename, "recreate");
  Long64_t ientry = 0;
  double edep = 0.0; 
  double edep_had = 0.0; 
  TString n_killer = ""; 
  double primary_energy = 0.0;
  double track_edep = 0.0;
  double dfm = 0.0;
  double x_mean = 0.0;
  double y_mean = 0.0;
  double z_mean = 0.0;

  const TString killer = "nCapture + Ar40 -> gamma + Ar41";

  TTree* output_tree = new TTree("EdepTree", "Energy deposition tree");
  output_tree->Branch("ientry", &ientry);
  output_tree->Branch("edep", &edep);
  output_tree->Branch("edep_had", &edep_had);
  output_tree->Branch("primary_energy", &primary_energy);
  output_tree->Branch("x_mean", &x_mean);
  output_tree->Branch("y_mean", &y_mean);
  output_tree->Branch("z_mean", &z_mean);
  output_tree->Branch("dfm", &dfm);
  if (is_neutron)
    output_tree->Branch("n_killer", &n_killer);

  for (Long64_t i = 0; i < nentries; i++) {
    chain_truth.GetEntry(i);
    if (debug && i % 1000 == 0) {
      std::cout << "Processing entry " << i << " / " << nentries << std::endl;
    }

    ientry = i;
    edep = 0.0; 
    edep_had = 0.0;
    primary_energy = 0.0;
    if (is_neutron) n_killer = "";
    x_mean = 0.0;
    y_mean = 0.0;
    z_mean = 0.0;

    const SLArMCPrimaryInfo& primary = truth->GetPrimary(0); 
    primary_energy = primary.GetEnergy();
    const auto& tracks = primary.GetConstTrajectories(); 
    for (const auto& t : tracks) {
      if (t->GetTime() > 1e6) continue;
      if (is_neutron && t->GetTrackID() == 1) {
        n_killer = t->GetEndProcess(); 
      }
      const auto& traj_points = t->GetConstPoints(); 
      track_edep = 0.0;
      for (const auto& p : traj_points) {
        if (p.fLAr) { 
          track_edep += p.fEdep; 
          x_mean += p.fX * p.fEdep;
          y_mean += p.fY * p.fEdep;
          z_mean += p.fZ * p.fEdep;
        }
      }
      if ( fabs(t->GetPDGID()) > 1000 ) edep_had += track_edep;
      else edep += track_edep;
    }

    x_mean /= 10.0*(edep + edep_had);
    y_mean /= 10.0*(edep + edep_had);
    z_mean /= 10.0*(edep + edep_had);

    dfm = min_distance_from_membrane(x_mean, y_mean, z_mean);

    output_tree->Fill();
  }

  output_file->cd();
  output_tree->Write();
  output_file->Close();
  
  return 0; 
}
