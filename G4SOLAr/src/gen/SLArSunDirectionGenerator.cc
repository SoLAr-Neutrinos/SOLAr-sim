/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArSunDirectionGenerator.cc
 * @created     : Wednesday Feb 12, 2025 15:40:22 CET
 */

#include "SLArSunDirectionGenerator.hh"

namespace gen{
  namespace direction{

    void SLArSunDirectionGenerator::Config(const rapidjson::Value& dir_config) {
      if (dir_config.IsObject() == false) {
        fprintf(stderr, "ConfigureDirection ERROR: Direction configuration must be an object\n"); 
        exit(EXIT_FAILURE); 
      }
      if (dir_config.HasMember("nadir_hist") == false) {
        fprintf(stdout, "SLArSunDirectionGenerator WARNING: Missing \"nadir_hist\" field\n"); 
        const rapidjson::Value& jnadir_hist = dir_config["nadir_hist"]; 
        fNadirHistInfo.Configure(jnadir_hist);
      }
    }

    void SLArSunDirectionGenerator::ShootDirection(G4ThreeVector& direction) {
      SLArRunAction* run_action = (SLArRunAction*)G4RunManager::GetRunManager()->GetUserRunAction(); 
      SLArRandom* slar_random = run_action->GetTRandomInterface(); 
      if (fNadirDistribution == nullptr) {
        TH1D* hist_nadir = get_from_rootfile<TH1D>(
            fNadirHistInfo.objname, fNadirHistInfo.objname); 
        fNadirDistribution = std::unique_ptr<TH1D>( std::move(hist_nadir) ); 
        printf("SLArSunDirectionGenerator::ShootDirection() Sourcing nadir angle distribution\n"); 
        printf("fNadirDistribution ptr: %p\n", fNadirDistribution.get());
      }
      const double cos_nadir = fNadirDistribution->GetRandom( slar_random->GetEngine().get() );
      const double sin_nadir = sqrt(1-cos_nadir*cos_nadir); 
      const double theta = 102.5*TMath::DegToRad(); 
      const double cos_theta = cos( theta ); 
      const double sin_theta = sin( theta ); 
      const double phi = -slar_random->GetEngine()->Uniform(0, M_PI);
      const double cos_phi = cos( phi ); 
      const double sin_phi = sin( phi );

      fTmpDirection.set(
          cos_theta*sin_nadir*cos_phi + sin_theta*sin_nadir*sin_phi,
          -cos_nadir, 
          -sin_theta*sin_nadir*cos_phi + cos_theta*sin_nadir*sin_phi
          );
      fTmpDirection = fTmpDirection.unit(); 
      direction.set( fTmpDirection.x(), fTmpDirection.y(), fTmpDirection.z() );
      return;
    }


    const rapidjson::Document SLArSunDirectionGenerator::ExportConfig() const
    {
      rapidjson::Document dir_info; 
      dir_info.SetObject(); 
      auto& allocator = dir_info.GetAllocator();

      G4String gen_type = GetType();
      char buffer[50];
      int len = snprintf(buffer, sizeof(buffer), "%s", gen_type.data());
      rapidjson::Value str_gen_type;
      str_gen_type.SetString(buffer, len, allocator);

      dir_info.AddMember("mode", str_gen_type, allocator); 
      if (fNadirHistInfo.objname.empty() == false) {
        const rapidjson::Document nadir_hist_doc = fNadirHistInfo.ExportConfig(); 
        rapidjson::Value nadir_hist_info; 
        nadir_hist_info.CopyFrom(nadir_hist_doc, allocator); 
        dir_info.AddMember("nadir_hist", nadir_hist_info, allocator);
      }
      
      return dir_info; 
    }
}
}
