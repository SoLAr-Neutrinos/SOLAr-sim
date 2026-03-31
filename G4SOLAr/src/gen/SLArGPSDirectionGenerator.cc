/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGPSDirectionGenerator
 * @created     : Thursday Feb 13, 2025 15:43:05 CET
 */

#include "SLArGPSDirectionGenerator.hh"
#include "SLArDebugUtils.hh"

namespace gen{
namespace direction{

  void SLArGPSDirectionGenerator::Config(const rapidjson::Value& config) {
    if (config.HasMember("type") == false) {
      fprintf(stderr, "SLArGPSDirectionGenerator ERROR: Missing mandatory \"type\" field\n");
      exit(EXIT_FAILURE);
    }
    G4String type = config["type"].GetString();
    fAngDist->SetAngDistType( type );

    // configure rotation of the reference frame
    if (config.HasMember("rot1")) {
      debug::require_json_type(config["rot1"], rapidjson::kArrayType);
      G4ThreeVector rot1;
      rot1.set( 
          config["rot1"][0].GetDouble(), 
          config["rot1"][1].GetDouble(), 
          config["rot1"][2].GetDouble() );
      fGPSConfig.rot1.set( rot1.x(), rot1.y(), rot1.z() );
      fAngDist->DefineAngRefAxes("angref1", rot1);
    }
    if (config.HasMember("rot2")) {
      debug::require_json_type(config["rot2"], rapidjson::kArrayType);
      G4ThreeVector rot2;
      rot2.set( 
          config["rot2"][0].GetDouble(), 
          config["rot2"][1].GetDouble(), 
          config["rot2"][2].GetDouble() );
      fGPSConfig.rot2.set( rot2.x(), rot2.y(), rot2.z() );
      fAngDist->DefineAngRefAxes("angref2", rot2);
    }

    // set min and max theta angle (theta = 0 -> -z direction) 
    if (config.HasMember("mintheta")) {
      debug::check_json_value_field(config["mintheta"], "mintheta");
      G4double mintheta = unit::ParseJsonVal(config["mintheta"]);
      fAngDist->SetMinTheta( mintheta );
      fGPSConfig.mintheta = mintheta;
    }
    if (config.HasMember("maxtheta")) {
      debug::check_json_value_field(config["maxtheta"], "maxtheta");
      G4double maxtheta = unit::ParseJsonVal(config["maxtheta"]);
      fAngDist->SetMaxTheta( maxtheta );
      fGPSConfig.maxtheta = maxtheta;
    }

    // set min and max phi angle
    if (config.HasMember("minphi")) {
      debug::check_json_value_field(config["minphi"], "minphi");
      G4double minphi = unit::ParseJsonVal(config["minphi"]);
      fAngDist->SetMinPhi( minphi );
      fGPSConfig.minphi = minphi;
    }
    if (config.HasMember("maxphi")) {
      debug::check_json_value_field(config["maxphi"], "maxphi");
      G4double maxphi = unit::ParseJsonVal(config["maxphi"]);
      fAngDist->SetMaxPhi( maxphi );
      fGPSConfig.maxphi = maxphi;
    }

    // set the beam sigma in x and y
    if (config.HasMember("sigma_x")) {
      debug::check_json_value_field(config["sigma_x"], "sigma_x");
      G4double sigma_x = unit::ParseJsonVal(config["sigma_x"]);
      fAngDist->SetBeamSigmaInAngX( sigma_x );
      fGPSConfig.sigma_x = sigma_x;
    }
    if (config.HasMember("sigma_y")) {
      debug::check_json_value_field(config["sigma_y"], "sigma_y");
      G4double sigma_y = unit::ParseJsonVal( config["sigma_y"] ); 
      fAngDist->SetBeamSigmaInAngY( sigma_y );
      fGPSConfig.sigma_y = sigma_y;
    }
    // set the beam sigma in r
    if (config.HasMember("sigma_r")) {
      debug::check_json_value_field(config["sigma_r"], "sigma_r");
      G4double sigma_r = unit::ParseJsonVal( config["sigma_r"] );
      fAngDist->SetBeamSigmaInAngR( sigma_r );
      fGPSConfig.sigma_r = sigma_r;
    }

    // set the beam focus point 
    if (config.HasMember("focuspoint")) {
      debug::check_json_value_field(config["focuspoint"], "focuspoint");
      G4ThreeVector focuspoint;
      const auto& jfocuspoint = config["focuspoint"];
      const auto& jval = jfocuspoint["val"];
      debug::require_json_type(jval, rapidjson::kArrayType);
      G4double vunit = unit::GetJSONunit(jfocuspoint);
      focuspoint.set( 
          jval[0].GetDouble() * vunit, 
          jval[1].GetDouble() * vunit, 
          jval[2].GetDouble() * vunit );
      fGPSConfig.focus_point.set( focuspoint.x(), focuspoint.y(), focuspoint.z() );
      fAngDist->SetFocusPoint( focuspoint );
    }

    // check user hist information for theta and phi (user mode only) 
    if (config.HasMember("theta_hist")) {
      G4String type = fAngDist->GetDistType();
      if (type != "user") {
        fprintf(stderr, "SLArGPSDirectionGenerator WARNING: theta_hist can be used only with \"user\" generator\n");
        exit(EXIT_FAILURE);
      }
      fGPSConfig.theta_hist.Configure( config["theta_hist"] );
      TFile* fhist = TFile::Open(fGPSConfig.theta_hist.filename.data());
      TH1D* htheta = fhist->Get<TH1D>(fGPSConfig.theta_hist.objname.data());
      G4ThreeVector val;
      for (int i = 1; i <= htheta->GetNbinsX(); i++) {
        double bin_endge = htheta->GetBinLowEdge(i+1);
        double bin_content = htheta->GetBinContent(i);
        val.set( bin_endge, bin_content, 0.0 );
        fAngDist->UserDefAngTheta( val );
      }
      fhist->Close();
    }

    if (config.HasMember("phi_hist")) {
      G4String type = fAngDist->GetDistType();
      if (type != "user") {
        fprintf(stderr, "SLArGPSDirectionGenerator WARNING: phi_hist can be used only with \"user\" generator\n");
        exit(EXIT_FAILURE);
      }
      fGPSConfig.phi_hist.Configure( config["phi_hist"] );

      TFile* fhist = TFile::Open(fGPSConfig.phi_hist.filename.data());
      TH1D* hphi = fhist->Get<TH1D>(fGPSConfig.phi_hist.objname.data());
      G4ThreeVector val;
      for (int i = 1; i <= hphi->GetNbinsX(); i++) {
        double bin_endge = hphi->GetBinLowEdge(i+1);
        double bin_content = hphi->GetBinContent(i);
        val.set( bin_endge, bin_content, 0.0 );
        fAngDist->UserDefAngPhi( val );
      }
    }
    return;
  }

  void SLArGPSDirectionGenerator::Print() const {
    printf("GPS Direction Generator\n");
    printf("\t- type: %s\n", fAngDist->GetDistType().data());
    if (fGPSConfig.rot1.mag2()) {
      printf("\t- rot1: %g, %g, %g\n", fGPSConfig.rot1.x(), fGPSConfig.rot1.y(), fGPSConfig.rot1.z());
    }
    if (fGPSConfig.rot2.mag2()) {
      printf("\t- rot2: %g, %g, %g\n", fGPSConfig.rot2.x(), fGPSConfig.rot2.y(), fGPSConfig.rot2.z());
    }
    printf("\t- mintheta: %g\n", fGPSConfig.mintheta);
    printf("\t- maxtheta: %g\n", fGPSConfig.maxtheta);
    printf("\t- minphi: %g\n", fGPSConfig.minphi);
    printf("\t- maxphi: %g\n", fGPSConfig.maxphi);
    printf("\t- sigma_x: %g\n", fGPSConfig.sigma_x);
    printf("\t- sigma_y: %g\n", fGPSConfig.sigma_y);
    printf("\t- sigma_r: %g\n", fGPSConfig.sigma_r);
    if (fGPSConfig.focus_point.mag2()) {
      printf("\t- focus point: %g, %g, %g\n", fGPSConfig.focus_point.x(), fGPSConfig.focus_point.y(), fGPSConfig.focus_point.z());
    }
    if (fGPSConfig.theta_hist.filename.empty() == false) {
      printf("\t- theta_hist: %s, %s\n", fGPSConfig.theta_hist.filename.data(), fGPSConfig.theta_hist.objname.data());
    }
    if (fGPSConfig.phi_hist.filename.empty() == false) {
      printf("\t- phi_hist: %s, %s\n", fGPSConfig.phi_hist.filename.data(), fGPSConfig.phi_hist.objname.data());
    }
    return;
  } 

  const rapidjson::Document SLArGPSDirectionGenerator::ExportConfig() const {
    rapidjson::Document jdirConfig;
    jdirConfig.SetObject();
    auto &dalloc = jdirConfig.GetAllocator();

    jdirConfig.AddMember("type", rapidjson::StringRef(fAngDist->GetDistType().data()), dalloc);

    if (fGPSConfig.rot1.mag2()) {
      rapidjson::Value jrot1(rapidjson::kArrayType); 
      jrot1.PushBack(fGPSConfig.rot1.x(), dalloc);
      jrot1.PushBack(fGPSConfig.rot1.y(), dalloc);
      jrot1.PushBack(fGPSConfig.rot1.z(), dalloc);
      jdirConfig.AddMember("rot1", jrot1, dalloc);
    }
    if (fGPSConfig.rot2.mag2()) {
      rapidjson::Value jrot2(rapidjson::kArrayType); 
      jrot2.PushBack(fGPSConfig.rot2.x(), dalloc);
      jrot2.PushBack(fGPSConfig.rot2.y(), dalloc);
      jrot2.PushBack(fGPSConfig.rot2.z(), dalloc);
      jdirConfig.AddMember("rot2", jrot2, dalloc);
    }

    if (fGPSConfig.mintheta != 0.0 || fGPSConfig.maxtheta != M_PI) {
      jdirConfig.AddMember("mintheta", fGPSConfig.mintheta, dalloc);
      jdirConfig.AddMember("maxtheta", fGPSConfig.maxtheta, dalloc);
    }

    if (fGPSConfig.minphi != 0.0 || fGPSConfig.maxphi != 2*M_PI) {
      jdirConfig.AddMember("minphi", fGPSConfig.minphi, dalloc);
      jdirConfig.AddMember("maxphi", fGPSConfig.maxphi, dalloc);
    }

    if (fGPSConfig.sigma_x != 0.0 || fGPSConfig.sigma_y != 0.0) {
      jdirConfig.AddMember("sigma_x", fGPSConfig.sigma_x, dalloc);
      jdirConfig.AddMember("sigma_y", fGPSConfig.sigma_y, dalloc);
    }
    else if (fGPSConfig.sigma_r != 0.0) {
      jdirConfig.AddMember("sigma_r", fGPSConfig.sigma_r, dalloc);
    }

    if (fGPSConfig.focus_point.mag2()) {
      rapidjson::Value jfocus(rapidjson::kArrayType); 
      jfocus.PushBack(fGPSConfig.focus_point.x(), dalloc);
      jfocus.PushBack(fGPSConfig.focus_point.y(), dalloc);
      jfocus.PushBack(fGPSConfig.focus_point.z(), dalloc);
      jdirConfig.AddMember("focuspoint", jfocus, dalloc);
    }

    return jdirConfig;
  }
}
}
