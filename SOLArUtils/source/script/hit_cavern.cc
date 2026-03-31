/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : hit_cavern.cc
 * @created     : Friday Jan 10, 2025 16:48:26 CET
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

#include "event/SLArGenRecords.hh"

void print_usage() {
  printf("SoLAr hit_calibration: Calibrate hit collection\n"); 
  printf("Usage: "); 
  printf("hit_calibration\n"); 
  printf("\t[--input_list | -l] input file list\n"); 
  printf("\t[--output     | -o] output filename\n");
  printf("\t[--buffer     | -b] buffer thickness (in mm)\n\n");
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
  diff[0] = 0.1*tpc_half_x - fabs(x); 
  diff[1] = 0.1*tpc_half_y - fabs(y); 
  diff[2] = 0.1*tpc_half_z - fabs(z);

  std::sort( diff.begin(), diff.end() );

  return diff.front();
}

int main (int argc, char *argv[]) {
  // define analysis constants
  const double drift_velocity = 158.2; // cm/s
  const double argon_w = 23.6e-6; // MeV
  const double recombination_factor = 1.0 / 0.6; 
  
  TString hit_file_list = {}; 
  TString output_filename = {};

  // Parse command line options
  const char* short_opts = "l:o:h";
  // Define long options
  static struct option long_options[] = {
    {"hitfile_list", required_argument, 0, 'l'},
    {"output", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  int c;

  while ( (c = getopt_long(argc, argv, short_opts, long_options, &option_index)) != -1) {
    switch(c) {
      case 'l' :
        hit_file_list = optarg;
        break;
      case 'o' :
        output_filename = optarg;
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

  // setup output file
  TFile* output_file = new TFile(output_filename, "recreate");
  TTree* output_tree = new TTree("CalibratedHitTree", "Calibrated hit collection tree");
  double neutrino_energy = 0.0; 
  double collected_energy = 0.0; 
  double calibrated_energy = 0.0;
  double drift_coordinate = 0.0;
  double cluster_x = 0; 
  double cluster_y = 0; 
  double cluster_z = 0;
  double cluster_wd = 0.0;
  output_tree->Branch("enu", &neutrino_energy); 
  output_tree->Branch("ecol", &collected_energy); 
  output_tree->Branch("ecal", &calibrated_energy); 
  output_tree->Branch("drift", &drift_coordinate); 
  output_tree->Branch("cluster_x", &cluster_x); 
  output_tree->Branch("cluster_y", &cluster_y); 
  output_tree->Branch("cluster_z", &cluster_z); 
  output_tree->Branch("cluster_wd", &cluster_wd); 

  // setup the chain
  TChain chain_hit("HitTree");

  std::string line; 
  std::ifstream file_list; 
  file_list.open(hit_file_list.Data());
  if (file_list.is_open() == false) {
    std::cerr << "Error opening file list: " << hit_file_list.Data() << std::endl;
    return 1;
  }
  
  TPRegexp rgx_seed("[0-9]{10}");

  while (std::getline(file_list, line)) {
    TString event_file_path = line;
    const int seed_pos = event_file_path.Index( rgx_seed );
    TString hit_file_path = event_file_path; 
    //hit_file_path.Insert(seed_pos, "hit_");

    TFile* hit_file = TFile::Open(hit_file_path);
    if (hit_file == nullptr || hit_file->IsZombie()) {
      std::cerr << "Error: hit_file " << hit_file_path << " does not exist or is zombie. Skip" << std::endl; 
      continue;
    }

    TTree* hit_tree = hit_file->Get<TTree>("HitTree");
    if (hit_tree == nullptr) {
      std::cerr << "Error: HitTree not found in file " << hit_file_path.Data() << std::endl;
      continue;
    }

    hit_file->Close();

    chain_hit.Add(hit_file_path);
  }

  Long64_t nentries = chain_hit.GetEntries();
  
  std::vector<float>* hit_q = nullptr;
  std::vector<float>* hit_x = nullptr;
  std::vector<float>* hit_y = nullptr;
  std::vector<float>* hit_z = nullptr;
  chain_hit.SetBranchAddress("hit_q", &hit_q);
  chain_hit.SetBranchAddress("hit_x", &hit_x);
  chain_hit.SetBranchAddress("hit_y", &hit_y);
  chain_hit.SetBranchAddress("hit_z", &hit_z);

  for (Long64_t i = 0; i < nentries; i++) {
    chain_hit.GetEntry(i);

    double total_q = 0.0; 
    // compute the weighted mean drift distance
    double x_mean = 1.0; 
    double y_mean = 1.0; 
    double z_mean = 1.0; 

    double ww = 0.0; 

    for (size_t ihit = 0; ihit < hit_x->size(); ihit++) {
      const double& xx = hit_x->at(ihit); 
      const double& yy = hit_y->at(ihit); 
      const double& zz = hit_z->at(ihit); 

      //if ( is_inside_buffer(xx, yy, zz, buffer_len) ) {
        //continue;
      //}

      const double& q_hit = hit_q->at(ihit); 
      
      total_q += q_hit;
      x_mean += xx * q_hit;
      y_mean += yy * q_hit; 
      z_mean += zz * q_hit; 
      ww += q_hit;
    }

    if (total_q == 0) {
      // set default values for empty clusters, fill and skip
      cluster_x = 0.0;
      cluster_y = 0.0;
      cluster_z = 0.0;
      cluster_wd = 0.0;

      neutrino_energy = 0.0;
      collected_energy = 0.0;
      calibrated_energy = 0.0;
      drift_coordinate = 0.0;
      
      output_tree->Fill();

      continue;
    }

    const double w_scale = 1.0 / ww; 
    x_mean *= (w_scale * 0.1); // convert to cm
    y_mean *= (w_scale * 0.1); // convert to cm
    z_mean *= (w_scale * 0.1); // convert to cm


    if (y_mean < 0) {
      drift_coordinate = y_mean + tpc_half_y*0.1;
    }
    else {
      drift_coordinate = tpc_half_y*0.1 - y_mean;
    }

    collected_energy = total_q * argon_w * recombination_factor;
    //double tau_elec = 0.04586717986370039*collected_energy + 5.278259363488911;
    double tau_elec = 0.107*0.04586717986370039*collected_energy + 5.278259363488911;

    calibrated_energy = collected_energy * exp(drift_coordinate / (drift_velocity * tau_elec) );
    
    cluster_x = x_mean; 
    cluster_y = y_mean; 
    cluster_z = z_mean;

    cluster_wd = min_distance_from_membrane(cluster_x, cluster_y, cluster_z);

    output_tree->Fill();
  }

  output_file->cd();
  output_tree->Write();
  output_file->Close();

  return 0;
}



