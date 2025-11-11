/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArCRYGeneratorAction.hh
 * @created     : Wednesday Mar 13, 2024 14:19:20 CET
 */

#ifndef SLARCRYGENERATORACTION_HH

#define SLARCRYGENERATORACTION_HH

#include <string>

#include <SLArBaseGenerator.hh>

#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4ParticleGun.hh>
#include <CRYSetup.h>
#include <CRYGenerator.h>
#include <CRYParticle.h>
#include <CRYUtils.h>
#include <TRandom3.h>

namespace gen {
namespace cry {

template<class T>
class RNGWrapper { 
  public:
    static void set(T* object, double (T::*func)(void));
    static double rng(void);
  private:
    static T* m_obj;
    static double (T::*m_func)(void);
};

template<class T> T* RNGWrapper<T>::m_obj;

template<class T> double (T::*RNGWrapper<T>::m_func)(void);

template<class T> void RNGWrapper<T>::set(T* object, double (T::*func)(void)) {
  m_obj = object; m_func = func;
}

template<class T> double RNGWrapper<T>::rng(void) { return (m_obj->*m_func)(); }

class SLArCRYGeneratorAction : public SLArBaseGenerator 
{
  public: 
    struct CRYConfig_t : public GenConfig_t {
      std::map<G4String, G4bool> activeParticles {
        {"electrons", false}, 
        {"gammas", false}, 
        {"muons", false},
        {"neutrons", false}, 
        {"pions", false},
        {"protons", false} 
      }; 
      G4String date {}; 
      G4double latitude {}; 
      G4int altitude = 0;
      G4int n_particles_min = 1; 
      G4int n_particles_max = 1000;
      G4double box_lenght = 1.0*CLHEP::m; 
      G4double cry_gen_y  = 0.0*CLHEP::m; 
      G4String cry_mess_input {}; 
      void activate_particle(const G4String); 
      void to_input(); 
      G4String volume_crossing = {};
    }; 
    SLArCRYGeneratorAction(const G4String label); 
    virtual ~SLArCRYGeneratorAction() {} 

    virtual void GeneratePrimaries(G4Event* ev) override; 

    void SourceConfiguration(const rapidjson::Value& config) override; 
    void Configure() override;
    //G4String WriteConfig() const override;

    G4String GetGeneratorType() const override {return "cry";}
    EGenerator GetGeneratorEnum() const override {return EGenerator::kCRY;}

    void InputCRY();
    void UpdateCRY();
    void SetGenQuota(const G4double y) {fConfig.cry_gen_y = y;}
    G4double GetGenQuota() const {return fConfig.cry_gen_y;}
    G4String& GetMessInput() {return fConfig.cry_mess_input;}

  private:
    CRYConfig_t fConfig; 
    std::vector<CRYParticle*> *vect; // vector of generated particles
    std::unique_ptr<G4ParticleGun> particleGun;
    std::unique_ptr<CRYGenerator> gen;
    G4int fInputState;
}; 
}
}


#endif /* end of include guard SLARCRYGENERATORACTION_HH */

