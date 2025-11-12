/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetectorConstructionMsgr
 * @created     : Wednesday Nov 12, 2025 11:49:00 CET
 */

#include "detector/SLArDetectorConstructionMsgr.hh"
#include "detector/SLArDetectorConstruction.hh"
#include "G4UIcmdWithAString.hh"


SLArDetectorConstructionMsgr::SLArDetectorConstructionMsgr(SLArDetectorConstruction* det)
    : fDetector(det)
{
    fCmdCheckOverlaps = new G4UIcmdWithAString("/SLAr/geometry/checkOverlaps", this);
    fCmdCheckOverlaps->SetGuidance("Checks all physical volumes for overlaps.");
    fCmdCheckOverlaps->SetGuidance("Usage: /geometry/checkOverlaps [fatal|warn]");
    fCmdCheckOverlaps->SetParameterName("mode", false);
    fCmdCheckOverlaps->SetCandidates("fatal warn");
}

SLArDetectorConstructionMsgr::~SLArDetectorConstructionMsgr() {
    delete fCmdCheckOverlaps;
}

void SLArDetectorConstructionMsgr::SetNewValue(G4UIcommand* cmd, G4String val) {
    if (cmd == fCmdCheckOverlaps) {
        if (val == "fatal") {
            fDetector->CheckOverlaps(true);
        } else if (val == "warn") {
            fDetector->CheckOverlaps(false);
        } else {
            G4cerr << "Unknown mode: " << val << ". Use 'fatal' or 'warn'." << G4endl;
        }
    }
}


