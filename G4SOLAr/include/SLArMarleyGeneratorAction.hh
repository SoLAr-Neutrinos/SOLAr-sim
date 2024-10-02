/// @file
/// @copyright Copyright (C) 2016-2021 Steven Gardiner
/// @license GNU General Public License, version 3
//
// This file is part of MARLEY (Model of Argon Reaction Low Energy Yields)
//
// MARLEY is free software: you can redistribute it and/or modify it under the
// terms of version 3 of the GNU General Public License as published by the
// Free Software Foundation.
//
// For the full text of the license please see COPYING or
// visit http://opensource.org/licenses/GPL-3.0
//
// Please respect the MCnet academic usage guidelines. See GUIDELINES
// or visit https://www.montecarlonet.org/GUIDELINES for details.

#pragma once

// Standard library includes
#include <string>
#include <map>

// Geant4 includes
#include <G4VUserPrimaryGeneratorAction.hh>
#include <SLArVertextGenerator.hh>
#include <SLArBaseGenerator.hh>

// MARLEY includes
#include <marley/Generator.hh>

class G4Event;

namespace gen {
namespace marley {

class SLArMarleyGeneratorAction : public SLArBaseGenerator
{
  public:
    struct MarleyConfig_t {
      G4String marley_config_path {};
      G4double time = 0.0; 
      G4int    n_particles = 1; 
      EDirectionMode direction_mode = EDirectionMode::kRandomDir;
      G4ThreeVector direction {0, 0, 0};
    };
    SLArMarleyGeneratorAction(G4String label = "");
    ~SLArMarleyGeneratorAction() {};

    G4String GetGeneratorType() const override {return "marley";}
    EGenerator GetGeneratorEnum() const override {return kMarley;}

    void Configure(const rapidjson::Value& config) override;
    void SetupMarleyGen(const std::string& config_file_name);
    void SetupMarleyGen(); 
    virtual void GeneratePrimaries(G4Event*) override;
    void SetNuDirection(G4ThreeVector dir) {fMarleyConfig.direction.set(dir.x(), dir.y(), dir.z());} 
    G4ThreeVector GetNuDirection() {return fMarleyConfig.direction;}

    G4String WriteConfig() const override;

  protected:
    // MARLEY event generator object
    MarleyConfig_t fMarleyConfig;
    ::marley::Generator fMarleyGenerator;
    double SampleDecayTime(const double half_life) const;
    std::map<double, double> fHalfLifeTable;
    
};
}
}
