/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGeneratorConfig
 * @created     : Wednesday Feb 12, 2025 18:38:40 CET
 */

#ifndef SLARGENERATORCONFIG_HH

#define SLARGENERATORCONFIG_HH

#include <iostream>
#include "TFile.h"
#include "rapidjson/document.h"


namespace gen {
  enum  EDetectorSite {kSURF = 0};

  enum  EEnergyMode {kFixed = 0, kCustom = 1, kExtSpectrum = 2}; 

  enum  EGenerator {
    kParticleGun=0
      ,kParticleBomb=1 
      ,kDecay0=2 
      ,kMarley=3 
      ,kBackground=4 
      ,kExternalGen=5
      ,kGENIE=6
#ifdef SLAR_CRY
      ,kCRY=7
#endif 
#ifdef SLAR_RADSRC
      ,kRadSrc=8
#endif // DEBUG
      ,kUndefinedGen = 99

  };

  static const std::map<std::string, EGenerator> genMap = {
    {"particlegun", EGenerator::kParticleGun}
    ,{"particlebomb", EGenerator::kParticleBomb} 
    ,{"decay0", EGenerator::kDecay0}
    ,{"marley", EGenerator::kMarley}
    ,{"external", EGenerator::kExternalGen}
    ,{"genie", EGenerator::kGENIE}
#ifdef SLAR_CRY
    ,{"cry", EGenerator::kCRY}
#endif
#ifdef SLAR_RADSRC
    ,{"radsrc", EGenerator::kRadSrc}
#endif
  };

  static inline EGenerator GetGeneratorIndex(const std::string& gen_type) {
    EGenerator kGen = EGenerator::kUndefinedGen;
    if (genMap.find(gen_type) != genMap.end()) {
      kGen = genMap.find(gen_type)->second;
    }
    else {
      fprintf(stderr, "Unknown generator %s\n", gen_type.data()); 
      exit(EXIT_FAILURE); 
    }
    return kGen;
  }

  static inline void printGeneratorType() {
    printf("Available primary generators:\n");
    for (const auto& gen : genMap) {
      printf("\t- %s\n", gen.first.data());
    }
    return;
  }


  struct ExtSourceInfo_t {
    std::string filename; 
    std::string objname;
    std::string type; 

    ExtSourceInfo_t() : filename(), objname(), type() {}; 
    inline const bool is_empty() {
      return ( filename.empty() && objname.empty() ); 
    }

    inline void Configure(const rapidjson::Value& jconfig) {
      if (jconfig.HasMember("filename") == false || 
          jconfig.HasMember("objname") == false) {
        fprintf(stderr, "ExtHistInfo_t::Configure ERROR. fields \"filename\" and \"objname\" required\n"); 
        exit(EXIT_FAILURE); 
      }
      filename = jconfig["filename"].GetString(); 
      objname = jconfig["objname"].GetString(); 
      if (jconfig.HasMember("type")) {
        type = jconfig["type"].GetString(); 
      }
      return; 
    }

    const rapidjson::Document ExportConfig() const {
      rapidjson::Document hist_doc; 
      hist_doc.SetObject(); 

      char buffer[200];
      int len = snprintf(buffer, sizeof(buffer), "%s", filename.data());
      rapidjson::Value jfilename;
      jfilename.SetString(buffer, len, hist_doc.GetAllocator());
      hist_doc.AddMember("filename", jfilename, hist_doc.GetAllocator()); 

      rapidjson::Value jobjname(rapidjson::kStringType);
      len = snprintf(buffer, sizeof(buffer), "%s", objname.data()); 
      jobjname.SetString(buffer, len, hist_doc.GetAllocator()); 
      hist_doc.AddMember("objname", jobjname, hist_doc.GetAllocator()); 

      rapidjson::Value jtype(rapidjson::kStringType);
      len = snprintf(buffer, sizeof(buffer), "%s", type.data()); 
      jtype.SetString(buffer, len, hist_doc.GetAllocator()); 
      hist_doc.AddMember("type", jtype, hist_doc.GetAllocator()); 
      return hist_doc; 
    }


  };

  struct EnergyConfig_t {
    EEnergyMode mode = EEnergyMode::kFixed; 
    std::string energy_distribution_label = {};
    double energy_value = 0.0;
    double energy_tmp = {}; 
    ExtSourceInfo_t spectrum_hist = {}; 
    double energy_min = {}; 
    double energy_max = {}; 
    double temperature = {}; 
    double eta = {}; 
    double energy_mean = {}; 
    double beta = {};
    std::vector<double> energy_bin_left = {};
    std::vector<double> energies = {};
    std::vector<double> weights = {};
    std::vector<double> prob_densities = {};
    std::string interpolation_rule = {};
  };

  struct GenConfig_t {
    int n_particles = 1; 
    double specific_activity = 0.0;
    EnergyConfig_t ene_config = {}; 
  };


}


#endif /* end of include guard SLARGENERATORCONFIG_HH */

