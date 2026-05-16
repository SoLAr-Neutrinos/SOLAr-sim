/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetSiPM.hh
 * @created     : Wednesday May 13, 2026 10:32:35 CEST
 */

#ifndef SLARDETSIPM_HH

#define SLARDETSIPM_HH

#include "detector/OpDet/SLArOpticalDetector.hh"

class SLArDetSiPM : public SLArOpticalDetector
{
  public: 
    inline SLArDetSiPM() : SLArOpticalDetector() {
      fOpDetType = EOpDetType::kSiPM;
      fOpDetTypeName = "SiPM";
    }

    inline SLArDetSiPM(const G4String& name) : SLArOpticalDetector(name) {
      fOpDetType = EOpDetType::kSiPM;
    }

    inline SLArDetSiPM(const SLArDetSiPM& detSiPM) : SLArOpticalDetector(detSiPM) {
      fMatSiPM = new SLArMaterial(*detSiPM.fMatSiPM); 
      fMatSiPMCapsule = new SLArMaterial(*detSiPM.fMatSiPMCapsule);
      fBaseMaterial = new SLArMaterial(*detSiPM.fBaseMaterial);
      fSiPMActive = new SLArBaseDetModule(*detSiPM.fSiPMActive);
    }
    
    ~SLArDetSiPM() = default;

    void BuildMaterial(G4String materials_db) override;
    void BuildOpticalDetector() override;
    void SetVisAttributes(const int&) override;
    G4LogicalSkinSurface* BuildLogicalSkinSurface();
    SLArBaseDetModule* GetActiveVolume() { return fSiPMActive; }
    SLArBaseDetModule* GetActiveVolume() const { return fSiPMActive; }

  private:
    SLArMaterial*  fBaseMaterial = {};
    SLArMaterial*  fMatSiPM = {};
    SLArMaterial*  fMatSiPMCapsule = {};
    SLArBaseDetModule* fSiPMActive = {};
};



#endif /* end of include guard SLARDETSIPM_HH */

