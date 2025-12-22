/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetectorConstructionMsgr.hh
 * @created     : Wednesday Nov 12, 2025 11:43:28 CET
 */

#ifndef SLARDETECTORCONSTRUCTIONMSGR_HH

#define SLARDETECTORCONSTRUCTIONMSGR_HH

#include "G4UImessenger.hh"

class G4UIcmdWithAString;
class SLArDetectorConstruction;

class SLArDetectorConstructionMsgr : public G4UImessenger {
  public:
    SLArDetectorConstructionMsgr(SLArDetectorConstruction* det);
    ~SLArDetectorConstructionMsgr();

    virtual void SetNewValue(G4UIcommand* command, G4String newValue);

  private:
    SLArDetectorConstruction* fDetector; 
    G4UIcmdWithAString* fCmdCheckOverlaps; 
};

#endif /* end of include guard SLARDETECTORCONSTRUCTIONMSGR_HH */

