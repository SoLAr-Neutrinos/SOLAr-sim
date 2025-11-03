/**
 * @author      : guff (guff@guff-gssi)
 * @file        : SLArGeoInfo
 * @created     : giovedì ago 01, 2019 09:58:31 CEST
 */

#ifndef SLArGEOINFO_HH
                   
#define SLArGEOINFO_HH

#include <ostream>
#include <map>
#include <G4ThreeVector.hh>

#include <rapidjson/document.h>
#include <rapidjson/allocators.h>

class SLArGeoInfo {
  public:
    struct ParInfo {
      G4double value = {}; 
      G4bool   isResolved = false;
      G4String refVolume = {};
      G4String refAnchor = {};
      G4String localAnchor = {};
      
      inline ParInfo() = default;
      
      inline ParInfo(G4double val) : value(val), isResolved(true) {}
      
      inline ParInfo(
          G4String ref_vol, G4String ref_anchor, G4String local_anchor, G4double val = 0.0
          ) : value(val), isResolved(false),
              refVolume(ref_vol), refAnchor(ref_anchor), localAnchor(local_anchor) {}

      inline ParInfo& operator= (const G4double val) {
        value = val;
        isResolved = true;
        return *this;
      }
      
      inline friend std::ostream& operator<< (std::ostream& os, const ParInfo& p) {
        os << "value: " << p.value 
            << ", isResolved: " << (p.isResolved ? "true" : "false")
            << ", refVolume: " << p.refVolume << "." << p.refAnchor
            << ", localAnchor: " << p.localAnchor;
        return os;
      }
    };

    SLArGeoInfo();
    SLArGeoInfo(const SLArGeoInfo &geo);
    SLArGeoInfo(G4String mat);
    ~SLArGeoInfo();

    void     RegisterGeoPar  (G4String str, G4double val);
    void     RegisterGeoPar  (std::pair<G4String, ParInfo> p);
    void     SetGeoPar       (G4String str, G4double val);
    void     SetGeoPar       (std::pair<G4String, ParInfo> p);
    bool     Contains        (G4String str);
    void     DumpParMap      ();
    G4double GetGeoPar       (G4String str);
    std::pair<G4String, ParInfo>
             GetGeoPair      (G4String str);
    bool     ReadFromJSON    (const rapidjson::Value::ConstArray&); 
    bool     ReadFromJSON    (const rapidjson::Value::ConstObject&, const char* prefix = ""); 

  private:
    std::map<G4String, ParInfo>  fGeoPar;
};



#endif /* end of include guard SLArGEOINFO_H */

