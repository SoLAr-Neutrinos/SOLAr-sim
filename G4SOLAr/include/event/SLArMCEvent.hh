/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArMCEvent.hh
 * @created     : mercoledì ago 10, 2022 11:52:04 CEST
 */

#ifndef SLArMCEVENT_HH

#define SLArMCEVENT_HH

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <memory>

#include "event/SLArMCPrimaryInfo.hh"
#include "event/SLArEventAnode.hh"
#include "event/SLArEventSuperCellArray.hh"
#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgBaseSystem.hh"
#include "config/SLArCfgMegaTile.hh"
#include "config/SLArCfgSuperCellArray.hh"

/**
 * @brief   G4SOLAr MC event 
 * @details Object storing the full MC Event, including the trajectories 
 * of all the tracks (with the exception of optical photons), the number of 
 * scintillation photons and ionization electrons produced, and the
 * detected hits for each detector sub-system (Tile SiPMs, Tile pixels, SuperCell)
 *              
 */
class SLArMCEvent : public TObject
{
  public: 

    //! Empty constructor
    SLArMCEvent();
    //! Copy constructor
    SLArMCEvent(const SLArMCEvent&);
    //! Destructuor
    ~SLArMCEvent();

    //! Set the event number
    int SetEvNumber(int nEv);
    //! Return the event number
    int GetEvNumber() {return fEvNumber;}

    //! Set the event direction
    void SetDirection(double* dir = nullptr); 
    //! Set the event direction
    void SetDirection(double px, double py, double pz);
    //! Get the event direction
    inline std::array<double, 3> GetDirection() {return fDirection;}
    int ConfigAnode (const std::map<int, SLArCfgAnode>& anodeCfg);
    int ConfigSuperCellSystem (const SLArCfgSystemSuperCell& supercellSysCfg); 

    inline std::map<int, SLArEventAnode>& GetEventAnode() {return fEvAnode;}
    inline SLArEventAnode& GetEventAnodeByTPCID(const int& id) {return fEvAnode.find(id)->second;}
    SLArEventAnode& GetEventAnodeByID(const int& id); 
    inline std::map<int, SLArEventSuperCellArray>& GetEventSuperCellArray() {return fEvSuperCellArray;}
    inline SLArEventSuperCellArray& GetEventSuperCellArray(const int& id) {return fEvSuperCellArray.find(id)->second;}

    inline std::vector<SLArMCPrimaryInfo>& GetPrimaries() {return fSLArPrimary ;}
    inline SLArMCPrimaryInfo& GetPrimary(int ip) {return fSLArPrimary.at(ip);}
    inline SLArMCPrimaryInfo& GetPrimaryByTrkID(int id) {
      for (auto &p : fSLArPrimary) {
        if (p.GetTrackID() == id) return p;
      }

      printf("SLArMCEvent::GetPrimaryByTrkID WARNING: Unable to find primary wit track id %i returning the first primary in the list\n", 
          id);
      return fSLArPrimary.front();
    }
    bool  CheckIfPrimary(int trkId) const;

    size_t RegisterPrimary(SLArMCPrimaryInfo& p);
    void  Reset();

  private:
    int fEvNumber; //!< Event number
    std::array<double, 3>  fDirection; //!< Event Direction 
    //! Event's primary particles (and associated secondaries)
    std::vector<SLArMCPrimaryInfo> fSLArPrimary;  
    //! Event data structure of the readout tile system
    std::map<int, SLArEventAnode> fEvAnode;
    //! Event data structure of the super-cell system
    std::map<int, SLArEventSuperCellArray> fEvSuperCellArray; 

  public:
    ClassDef(SLArMCEvent, 3);
};

#endif /* end of include guard SLArEVENT_HH */

