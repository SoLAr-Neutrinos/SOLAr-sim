/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArBacktrackerManager.hh
 * @created     Thursday Sep 28, 2023 18:10:56 CEST
 */

#ifndef SLARBACKTRACKERMANAGER_HH

#define SLARBACKTRACKERMANAGER_HH

#include <vector>
#include <functional>
#include "G4String.hh"
#include "SLArBacktracker.hh"

namespace backtracker{

class SLArBacktrackerManager {
  public: 
    SLArBacktrackerManager() {}; 
    ~SLArBacktrackerManager(); 

    inline std::vector<SLArBacktracker*>& GetBacktrackers() {return fBacktrackers;}
    inline const std::vector<SLArBacktracker*>& GetConstBacktrackers() const {return fBacktrackers;}

    G4bool RegisterBacktracker(SLArBacktracker* bkt); 
    G4bool RegisterBacktracker(const EBacktracker id, const G4String name = "");
    G4bool IsNull() const;

  protected:
    std::vector<SLArBacktracker*> fBacktrackers; 
}; 
}

#endif /* end of include guard SLARBACKTRACKERMANAGER_HH */

