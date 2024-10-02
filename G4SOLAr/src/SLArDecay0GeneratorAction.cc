/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDecay0GeneratorAction.cc
 * @created     Fri Jan 13, 2023 12:05:02 CET
 */

#include "SLArDecay0GeneratorAction.hh"
#include "SLArAnalysisManager.hh"

// Standard library:
#include <iostream>
#include <random>
#include <limits>

// BxDecay0:
#include <bxdecay0/decay0_generator.h>
#include <bxdecay0/std_random.h>
#include <bxdecay0/event.h>       
#include <bxdecay0/bb_utils.h>
#include <bxdecay0/mdl_event_op.h>

// Geant4:
#include <globals.hh>
#include <G4ParticleMomentum.hh>
#include <G4ParticleGun.hh>
#include <G4Gamma.hh>
#include <G4Electron.hh>
#include <G4Positron.hh>
#include <G4Alpha.hh>
#include <G4SystemOfUnits.hh>
#include <G4RunManager.hh>

// This project:
#include "SLArDecay0GeneratorMessenger.hh"

// rapidjson
#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

namespace gen {
namespace {
  /// Hacked particle gun with a special ResetParticleData method
  struct Decay0ParticleGun
    : public G4ParticleGun
  {
  public:
    Decay0ParticleGun() = default;
    Decay0ParticleGun(G4int numberofparticles)
      : G4ParticleGun(numberofparticles) {}
    Decay0ParticleGun(G4ParticleDefinition* particleDef, 
                  G4int numberofparticles = 1)
      : G4ParticleGun(particleDef, numberofparticles) {}
    virtual ~Decay0ParticleGun() override = default;
    Decay0ParticleGun(const Decay0ParticleGun&) = delete;
    const Decay0ParticleGun& operator=(const Decay0ParticleGun&) = delete;
    G4bool operator==(const Decay0ParticleGun&) const = delete;
    G4bool operator!=(const Decay0ParticleGun&) const = delete;
    void ResetParticleData()
    {
      NumberOfParticlesToBeGenerated = 1;
      particle_definition = nullptr;
      G4ThreeVector zero;
      particle_momentum_direction = (G4ParticleMomentum)zero;
      particle_energy = 0.0;
      particle_momentum = 0.0;
      particle_position = zero;
      particle_time = 0.0;
      particle_polarization = zero;
      particle_charge = 0.0;
    }
  };
}

namespace bxdecay0_g4{

  void SLArDecay0GeneratorAction::Decay0Config_t::print(std::ostream & out_, const std::string & indent_) const
  {
    out_ << indent_ << "Configuration interface: \n";
    out_ << indent_ << "|-- Decay category : '" << decay_category << "'\n";
    out_ << indent_ << "|-- Nuclide : '" << nuclide << "'\n";
    out_ << indent_ << "|-- Seed : " << seed << "\n";
    if (decay_category == "dbd") {
      out_ << indent_ << "|-- DBD mode : " << dbd_mode << "\n";
      out_ << indent_ << "|-- DBD level : " << dbd_level << "\n";
      out_ << indent_ << "|-- DBD min energy (MeV) : " << dbd_min_energy_MeV << "\n";
      out_ << indent_ << "|-- DBD max energy (MeV) : " << dbd_max_energy_MeV << "\n";
    }
    out_ << indent_ << "|-- Debug : " << std::boolalpha << debug << "\n";
    out_ << indent_ << "`-- Use MDL : " << std::boolalpha << use_mdl << "\n";
    if (use_mdl) {
      out_ << indent_ << "    " << "|-- Target particle name : " << mdl_target_name << "\n";
      out_ << indent_ << "    " << "|-- Target particle rank : " << mdl_target_rank << "\n";
      out_ << indent_ << "    " << "|-- Cone longitude       : " << mdl_cone_longitude << " degrees\n";
      out_ << indent_ << "    " << "|-- Cone colatitude      : " << mdl_cone_colatitude << " degrees\n";
      out_ << indent_ << "    " << "|-- Cone aperture        : " << mdl_cone_aperture << " degrees\n";
      out_ << indent_ << "    " << "|-- Cone aperture 2      : " << mdl_cone_aperture2 << " degrees\n";
      out_ << indent_ << "    " << "`-- Error on missing target particle : " << std::boolalpha << mdl_error_on_missing_particle << "\n";
    }
    return;
  }

  bool SLArDecay0GeneratorAction::Decay0Config_t::is_valid_base() const
  {
    if (decay_category != "background" and decay_category != "dbd") {
      fprintf(stderr, "Decay0Config_t::is_valid_base() ERROR: decay_category is neither \"background\" nor \"dbd\" - %s not valid\n", decay_category.data()); 
      return false;
    }
    if (nuclide.empty()) {
      fprintf(stderr, "Decay0Config_t::is_valid_base() ERROR: nuclide is empty\n"); 
      return false;
    }
    if (seed < 1) {
      fprintf(stderr, "Decay0Config_t::is_valid_base() ERROR: seed < 1 (%ld)\n", seed); 
      return false;
    }
    if (decay_category == "dbd") {
      if (dbd_mode < 1) { // valid DBD modes/mechanisms start from 1):
        return false;
      }
      // Fix issue #22 (valid daughter levels start from 0):
      if (dbd_level < 0) { 
        return false;
      }
    }
    return true;
  }

  bool SLArDecay0GeneratorAction::Decay0Config_t::is_valid_mdl() const
  {
    if (use_mdl) {
      if (mdl_target_name.empty()) return false;
      if (mdl_target_rank < -1) return false;
    }
    return true;
  }

  bool SLArDecay0GeneratorAction::Decay0Config_t::is_valid() const
  {
    return is_valid_base() and is_valid_mdl();
  }
  
  void SLArDecay0GeneratorAction::Decay0Config_t::reset_base()
  {
    decay_category = "";
    nuclide = "";
    seed = 1;
    dbd_mode = 0;
    dbd_level = 0;
    dbd_min_energy_MeV = -1.0;
    dbd_max_energy_MeV = -1.0;
    debug = false;
    return;
  }
   
  void SLArDecay0GeneratorAction::Decay0Config_t::reset_mdl()
  {
    use_mdl = false;
    mdl_target_name = "";
    mdl_target_rank = -1;
    mdl_cone_longitude  = 0.0;
    mdl_cone_colatitude = 0.0;
    mdl_cone_aperture   = 0.0;
    mdl_cone_aperture2  = -1.0; // Unused cone aperture rectangular cut
    mdl_error_on_missing_particle = false;
    return;
  }
   
  void SLArDecay0GeneratorAction::Decay0Config_t::reset() 
  {
    reset_base();
    reset_mdl();
    return;
  }
 
  // Private configuration structure for the unexposed BxDecay0 library:
  struct Configuration
  {
    /// Category of the decay
    bxdecay0::decay0_generator::decay_category_type decay_category = bxdecay0::decay0_generator::DECAY_CATEGORY_UNDEFINED;
    /// Name of the decaying isotope
    std::string   nuclide = {};      
    /// Seed for the pseudo-random number generator
    unsigned int  seed = 314159; 
    /// Daughter's energy level (default: ground state)
    int           dbd_level = 0;  
    /// Double beta decay mode (only for DBD mode)
    bxdecay0::dbd_mode_type dbd_mode = bxdecay0::DBDMODE_UNDEF; 
    /// Minimum sum energy in MeV (only for some DBD isotopes and modes)
    double dbd_min_energy_MeV = std::numeric_limits<double>::quiet_NaN();
    /// Maximum sum energy in MeV (only for some DBD isotopes and modes)
    double dbd_max_energy_MeV = std::numeric_limits<double>::quiet_NaN();
    bool   debug = false; ///< Debug flag
    
  };

  // Private configuration structure for the MDL event op of the unexposed BxDecay0 library:
  struct MdlEventOpConfiguration
  {
    bool   use_mdl = false; // Do not use MDL
    bxdecay0::particle_code code = bxdecay0::INVALID_PARTICLE; // All types of particles
    int    rank = -1; // All ranks
    double cone_phi = 0.0; // Z-axis
    double cone_theta = 0.0; // Z-axis
    double cone_aperture = 0.0; // Zero aperture
    double cone_aperture2 = std::numeric_limits<double>::quiet_NaN(); // No aperture rectangular cut 
    bool   error_on_missing_particle = false; // Pass if not applicable

  };
  
  // PIMPL-ized embedded private BxDecay0 driver and associated resources
  struct SLArDecay0GeneratorAction::pimpl_type
  {
    pimpl_type(SLArDecay0GeneratorAction * action);
    
    ~pimpl_type();

    bxdecay0::decay0_generator & get_decay0();

    void destroy_decay0();
    
    void destroy_generator();
    
    void destroy_prng();

    void destroy();

    std::default_random_engine & get_generator();

    bxdecay0::std_random & get_prng();

    // Attributes:
    SLArDecay0GeneratorAction *  action = nullptr;     // Mother action
    Configuration                config;               // Configuration of the BxDecay0 driver
    MdlEventOpConfiguration      mdl_config;           // Configuration of the MDL post-generation event operation in the BxDecay0 driver
    std::default_random_engine * pgenerator = nullptr; // Low level random generator (depends on config for its seeding)
    bxdecay0::std_random *       pprng = nullptr;      // High level random generator (depends on pgenerator)
    bxdecay0::decay0_generator * pdecay0 = nullptr;    // BxDecay0 generator driver (depends on pprng and config)

  };

  void SLArDecay0GeneratorAction::pimpl_type::destroy()
  {
    destroy_decay0();
    destroy_prng();
    destroy_generator();
    return;
  }

  void SLArDecay0GeneratorAction::pimpl_type::destroy_decay0()
  {
    if (action->IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::destroy_decay0: Entering...\n";
    if (pdecay0 != nullptr) {
      if (action->IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::destroy_decay0: Destroying the BxDecay0 driver...\n";
      pdecay0->reset();
      delete pdecay0;
      pdecay0 = nullptr;
    }
    if (action->IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::destroy_decay0: Exiting...\n";
    return;
  }
  
  bxdecay0::decay0_generator & SLArDecay0GeneratorAction::pimpl_type::get_decay0()
  {
    if (action->IsTrace()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::get_decay0: Entering...\n";
    if (action->ConfigHasChanged()) {
      if (action->IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::get_decay0: Configuration has changed\n";
      if (action->GetConfiguration().is_valid()) {
        action->ApplyConfiguration();
        if (pdecay0 != nullptr) {
          if (action->IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::get_decay0: Destroying the current BxDecay0 generator instance because of a new config...\n";
          destroy();
        }
      } else {
        std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::get_decay0: Invalid configuration!\n";
        exit( EXIT_FAILURE );  
      }
    }
    if (pdecay0 == nullptr) {
      if (action->IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::get_decay0: Instantiating and configuring a new BxDecay0 generator...\n";
      pdecay0 = new bxdecay0::decay0_generator;
      // Configure the BxDecay0 generator:
      pdecay0->set_debug(config.debug);
      pdecay0->set_decay_category(config.decay_category);
      pdecay0->set_decay_isotope(config.nuclide);
      double toallevents = 1.0;
      bool use_specific_erange = false;
      if (config.decay_category == bxdecay0::decay0_generator::DECAY_CATEGORY_DBD) {
        pdecay0->set_decay_dbd_level(config.dbd_level);
        pdecay0->set_decay_dbd_mode(config.dbd_mode);
        if (! std::isnan(config.dbd_min_energy_MeV) or !std::isnan(config.dbd_max_energy_MeV)) {
          // limited energy range
          double emin_MeV = 0.0;
          double emax_MeV = 5000.0;
          if (! std::isnan(config.dbd_min_energy_MeV)) {
            emin_MeV = config.dbd_min_energy_MeV;
          }
          if (! std::isnan(config.dbd_max_energy_MeV)) {
            emax_MeV = config.dbd_max_energy_MeV;
          }
          pdecay0->set_decay_dbd_esum_range(emin_MeV, emax_MeV);
          use_specific_erange = true;
        }
      }
      if (mdl_config.use_mdl) {
        // Install the MDL post-generation operation in the generator
        if (action->IsDebug()) {
          std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::get_decay0: "
                    << "Install the MDL post-generation operation in the BxDecay0 generator...\n";
        }
        bxdecay0::event_op_ptr mdlPtr(new bxdecay0::momentum_direction_lock_event_op(pdecay0->is_debug()));
        bxdecay0::momentum_direction_lock_event_op & mdl = dynamic_cast<bxdecay0::momentum_direction_lock_event_op&>(*mdlPtr);
        if (mdl_config.cone_aperture2 >= 0.0) {
          mdl.set_with_aperture_rectangular_cut(mdl_config.code, 
                                                mdl_config.rank,               
                                                mdl_config.cone_phi,
                                                mdl_config.cone_theta,   
                                                mdl_config.cone_aperture,        
                                                mdl_config.cone_aperture2,        
                                                mdl_config.error_on_missing_particle);
        } else {
          mdl.set(mdl_config.code, 
                  mdl_config.rank,               
                  mdl_config.cone_phi,
                  mdl_config.cone_theta,   
                  mdl_config.cone_aperture,        
                  mdl_config.error_on_missing_particle);
        }
        pdecay0->add_operation(mdlPtr); 
      }
      if (action->IsDebug()) {
        pdecay0->smart_dump(std::cerr,"BxDecay0 Generator: ", "[debug] ");
      }
      pdecay0->initialize(get_prng());
      if (use_specific_erange) {
        toallevents = pdecay0->get_bb_params().toallevents;
        if (action->IsInfo()) std::cerr << "[info] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::get_decay0: BxDecay0 Geant4 extension library: toallevents=" << toallevents << "\n";
      }
    }
    return *pdecay0;
  }

  void SLArDecay0GeneratorAction::pimpl_type::destroy_prng()
  {
    if (pprng != nullptr) {
      if (pdecay0 != nullptr) destroy_decay0();
      if (action->IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::destroy_prng: Destroying the high level random generator...\n";
      delete pprng;
      pprng = nullptr;
    }
    return;
  }

  void SLArDecay0GeneratorAction::pimpl_type::destroy_generator()
  {
    if (pgenerator != nullptr) {
      if (pprng != nullptr) destroy_prng();
      if (action->IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::destroy_generator: Destroying the low level random generator...\n";
      delete pgenerator;
      pgenerator = nullptr;
    }
    return;
  }
  
  std::default_random_engine & SLArDecay0GeneratorAction::pimpl_type::get_generator()
  {
    if (pgenerator == nullptr) {
      if (action->IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::get_generator: Instantiating a new low level random generator with seed=" << config.seed << "...\n";
      pgenerator = new std::default_random_engine(config.seed);
    }
    return *pgenerator;
  }

  bxdecay0::std_random & SLArDecay0GeneratorAction::pimpl_type::get_prng()
  {
    if (pprng == nullptr) {
      if (action->IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::get_prng: Instantiating a new high level random generator...\n";
      pprng = new bxdecay0::std_random(get_generator());
    }
    return *pprng;
  }

  SLArDecay0GeneratorAction::pimpl_type::pimpl_type(SLArDecay0GeneratorAction * action_) : pgenerator(nullptr), pdecay0(nullptr)
  {
    action = action_;
    if (action->IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::ctor: Construction of the BxDecay0 Geant4 Plugin PIMPL...\n";
    return;
  }

  SLArDecay0GeneratorAction::pimpl_type::~pimpl_type()
  {
    destroy();
    if (action->IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::dtor: Destruction of the BxDecay0 Geant4 Plugin PIMPL.\n";
    return;
  }

  void SLArDecay0GeneratorAction::SetVerbosity(int level_)
  {
    _verbosity_ = level_ < 0 ? 0 : level_;
    return;
  }

  int SLArDecay0GeneratorAction::GetVerbosity() const
  {
    return _verbosity_;
  }
  
  bool SLArDecay0GeneratorAction::IsInfo() const
  {
    return _verbosity_ > 0;
  }
  
  bool SLArDecay0GeneratorAction::IsDebug() const
  {
    return _verbosity_ > 1;
  }
 
  bool SLArDecay0GeneratorAction::IsTrace() const
     {
    return _verbosity_ > 2;
  }

  void SLArDecay0GeneratorAction::Dump(std::ostream & out_) const
  {
    out_ << "=== bxdecay0_g4::SLArDecay0GeneratorAction ===\n";
    out_ << "Verbosity : " << _verbosity_ << "\n";
    out_ << "Vertex generator : " << (fVtxGen ? "yes" : "no") << "\n";
    out_ << "Messenger : " << (_messenger_ ? "yes" : "no") << "\n";
    fConfig.print(out_);
    out_ << "Configuration has changed : " << std::boolalpha << _config_has_changed_ << "\n";
    out_ << "PIMPL : " << (_pimpl_ ? "yes" : "no") << "\n";
    if (_pimpl_ and _pimpl_->pdecay0) {
      _pimpl_->pdecay0->smart_dump(out_, "Decay0 generator instance: ", "");
    }
    return;
  }
   
  SLArDecay0GeneratorAction::SLArDecay0GeneratorAction(const G4String label, int verbosity_)
    : SLArBaseGenerator(label)
    , _particle_gun_(nullptr)
  {
    SetVerbosity(verbosity_);
    if (IsDebug()) std::cerr << "[debug] Instantiating BxDecay0 Geant4 Plugin PIMPL...\n";
    _pimpl_.reset(new pimpl_type(this));
    if (IsDebug()) std::cerr << "[debug] Instantiating Geant4 particle gun...\n";
    // _particle_gun_ = new G4ParticleGun(1);
    _particle_gun_ = std::make_unique<Decay0ParticleGun>(1);
    if (IsDebug()) std::cerr << "[debug] Instantiating messenger...\n";
    _messenger_ = new SLArDecay0GeneratorMessenger(this);
    return;
  }
  
  SLArDecay0GeneratorAction::SLArDecay0GeneratorAction(const Decay0Config_t & config_inter_, const G4String label, int verbosity_)
    : SLArDecay0GeneratorAction(label, verbosity_)
  {
    SetConfiguration(config_inter_);
    return;
  }
 
  SLArDecay0GeneratorAction::~SLArDecay0GeneratorAction()
  {
    //if (_owned_vertex_generator_ and _vertex_generator_ != nullptr) {
      //if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::~SLArDecay0GeneratorAction: Destroying the vertex generator...\n";
     //delete _vertex_generator_;
      //_vertex_generator_ = nullptr;
    //}
    if (_messenger_) {
      if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::~SLArDecay0GeneratorAction: Terminating messenger...\n";
      delete _messenger_;
    }
    if (_particle_gun_) {
      if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::~SLArDecay0GeneratorAction: Terminating Geant4 particle gun...\n";
      //delete _particle_gun_;
    }
    if (_pimpl_) {
      if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::~SLArDecay0GeneratorAction: Terminating BxDecay0 Geant4 Plugin PIMPL stuff...\n";
      _pimpl_.reset();
    }
    return;
  }

  const SLArDecay0GeneratorAction::Decay0Config_t & SLArDecay0GeneratorAction::GetConfiguration() const
  {
    return fConfig;
  }

  SLArDecay0GeneratorAction::Decay0Config_t & SLArDecay0GeneratorAction::GrabConfiguration() 
  {
    if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::GrabConfiguration: Destroy PIMPL material, if any...\n";
    // SetConfigHasChanged(true);
    return fConfig;
  }

  void SLArDecay0GeneratorAction::SetConfigHasChanged(bool changed_)
  {
    if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::SetConfigHasChanged: changed=" << changed_ << "\n";
    _config_has_changed_ = changed_;
    return;
  }
  
  bool SLArDecay0GeneratorAction::ConfigHasChanged() const
  {
    return _config_has_changed_;
  }

  void SLArDecay0GeneratorAction::SetConfiguration(const Decay0Config_t & config_inter_)
  {
    if (IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::SetConfiguration: Entering...\n";
    fConfig = config_inter_;
    SetConfigHasChanged(true);
    if (IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::SetConfiguration: Exiting...\n";
    return;
  }

  void SLArDecay0GeneratorAction::DestroyConfiguration()
  {
    if (IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::DestroyConfiguration: Entering...\n";
    if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::DestroyConfiguration: Entering...\n";
    _pimpl_->destroy();
    fConfig.reset();
    _config_has_changed_ = false;
    if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::DestroyConfiguration: Exiting...\n";
    if (IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::DestroyConfiguration: Exiting...\n";
    return;
  }

  void SLArDecay0GeneratorAction::ApplyConfiguration()
  {
    if (IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Entering...\n";
    if (not fConfig.is_valid()) {
      std::cerr << "[error] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Invalid configuration!\n";
      return;
    }
    bool error = false;
    if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::pimpl_type::get_decay0: Destroying the old BxDecay0 generator because of a new config to be applied...\n";
    _pimpl_->destroy();
    {
      // Set a default configuration:
      Configuration dummyConfig;
      _pimpl_->config = dummyConfig;
      MdlEventOpConfiguration dummyMdlDummy;
      _pimpl_->mdl_config = dummyMdlDummy;
    };
    
    _pimpl_->config.debug = fConfig.debug;

    if (not error) {
      if (fConfig.seed <= 0) {
        std::cerr << "[error] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Invalid seed value " << fConfig.seed << "!\n";
        error = true;
      } else {
        _pimpl_->config.seed = fConfig.seed;
      }
    }

    if (not error) {
      if (fConfig.decay_category == "dbd") {
        _pimpl_->config.decay_category = bxdecay0::decay0_generator::DECAY_CATEGORY_DBD;
      } else if (fConfig.decay_category == "background") {
        _pimpl_->config.decay_category = bxdecay0::decay0_generator::DECAY_CATEGORY_BACKGROUND;
        printf("SLArDecay0GeneratorAction::ApplyConfiguration(): setting \"background\" decay category to _pimpl_\n"); 
      } else {
        std::cerr << "[error] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Unsupported decay category '" << fConfig.decay_category << "'!\n";
        error = true;
      }
    }

    if (not error) {
      if (_pimpl_->config.decay_category == bxdecay0::decay0_generator::DECAY_CATEGORY_DBD) {
        if (bxdecay0::dbd_isotopes().count(fConfig.nuclide) == 0) {
          std::cerr << "[error] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Unsupported DBD nuclide '" << fConfig.nuclide << "'!\n";
          error = true;
        } 
      } else if (_pimpl_->config.decay_category == bxdecay0::decay0_generator::DECAY_CATEGORY_DBD) {
        if (bxdecay0::background_isotopes().count(fConfig.nuclide) == 0) {
          std::cerr << "[error] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Unsupported background nuclide '" << fConfig.nuclide << "'!\n";
          error = true;
        } 
      }
      if (not error) {
        _pimpl_->config.nuclide = fConfig.nuclide;
      }
    }

    if (not error and _pimpl_->config.decay_category == bxdecay0::decay0_generator::DECAY_CATEGORY_DBD) {
      if (fConfig.dbd_mode < bxdecay0::DBDMODE_MIN or fConfig.dbd_mode > bxdecay0::DBDMODE_MAX) {
        std::cerr << "[error] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Invalid DBD decay mode " << fConfig.dbd_mode << "!";
        error = true;
      } else {
        _pimpl_->config.dbd_mode = static_cast<bxdecay0::dbd_mode_type>(fConfig.dbd_mode);
      }
      if (not error) {
        if (fConfig.dbd_level < 0) {
          std::cerr << "[error] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Invalid DBD daughter level index " << fConfig.dbd_level << "!";
          error = true;
        } else {
          _pimpl_->config.dbd_level = (int) fConfig.dbd_level;
        }
      }
      if (not error) {
        if (fConfig.dbd_min_energy_MeV > 0.0) {
          _pimpl_->config.dbd_min_energy_MeV = (double) fConfig.dbd_min_energy_MeV;
        }
        if (fConfig.dbd_max_energy_MeV > 0.0) {
          _pimpl_->config.dbd_max_energy_MeV = (double) fConfig.dbd_max_energy_MeV;
          if (fConfig.dbd_min_energy_MeV > 0.0) {
            if (_pimpl_->config.dbd_min_energy_MeV >= _pimpl_->config.dbd_max_energy_MeV) {
              // Check bounds ordering of the energy range:
              std::cerr << "[error] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Invalid DBD energy range!";
              error = true;
            }
          }
        }
      }
    }

    if (fConfig.use_mdl) {
      MdlEventOpConfiguration & mdlConfig = _pimpl_->mdl_config;
      if (not error) {
        mdlConfig.use_mdl = fConfig.use_mdl;
        if (fConfig.mdl_target_name == "*" or fConfig.mdl_target_name == "all") {
          mdlConfig.code = bxdecay0::INVALID_PARTICLE; 
        } else if (fConfig.mdl_target_name == "e-" or fConfig.mdl_target_name == "electron") {
          mdlConfig.code = bxdecay0::ELECTRON;
        } else if (fConfig.mdl_target_name == "e+" or fConfig.mdl_target_name == "positron") {
          mdlConfig.code = bxdecay0::POSITRON;
        } else if (fConfig.mdl_target_name == "g" or fConfig.mdl_target_name == "gamma") {
          mdlConfig.code = bxdecay0::GAMMA;
        } else if (fConfig.mdl_target_name == "n" or fConfig.mdl_target_name == "neutron") {
          mdlConfig.code = bxdecay0::NEUTRON;
        } else if (fConfig.mdl_target_name == "p" or fConfig.mdl_target_name == "proton") {
          mdlConfig.code = bxdecay0::PROTON;
        } else if (fConfig.mdl_target_name == "a" or fConfig.mdl_target_name == "alpha") {
          mdlConfig.code = bxdecay0::ALPHA;
        } else {
          std::cerr << "[error] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Invalid target particle label '" << fConfig.mdl_target_name << "'!";
          error = true;
        }
      }
      
      if (not error) {     
        if (fConfig.mdl_target_rank < 0) {
          mdlConfig.rank = -1; 
        } else {
          mdlConfig.rank = fConfig.mdl_target_rank; 
        }
      }
      
      if (not error) {
        // Convert degrees to radians:
        mdlConfig.cone_phi = fConfig.mdl_cone_longitude * M_PI / 180.0;
        mdlConfig.cone_theta = fConfig.mdl_cone_colatitude * M_PI / 180.0;
        mdlConfig.cone_aperture = fConfig.mdl_cone_aperture * M_PI / 180.0;
        mdlConfig.cone_aperture2 = std::numeric_limits<double>::quiet_NaN();
        if (fConfig.mdl_cone_aperture2 >= 0.0) {
          mdlConfig.cone_aperture2 = fConfig.mdl_cone_aperture2 * M_PI / 180.0;
        }
        mdlConfig.error_on_missing_particle = fConfig.mdl_error_on_missing_particle;
      }
    }
    
    if (error) {
      std::cerr << "[error] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Invalid configuration! Abort run!\n";
      G4RunManager::GetRunManager()->AbortRun();
    }
    SetConfigHasChanged(false);
    if (IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::ApplyConfiguration: Exiting...\n";
    return;
  }

  G4ParticleGun * SLArDecay0GeneratorAction::GetParticleGun()
  {
    return _particle_gun_.get();
  }
   
  bool SLArDecay0GeneratorAction::HasVertexGenerator() const
  {
    return fVtxGen != nullptr;
  }

  //void SLArDecay0GeneratorAction::SetVertexGenerator(SLArVertexGenerator& vertex_generator_)
  //{
    //if (_owned_vertex_generator_ and _vertex_generator_ != nullptr) {
      //delete _vertex_generator_;
      //_vertex_generator_ = nullptr;
      //_owned_vertex_generator_ = false;
    //}
    //_vertex_generator_ = &vertex_generator_;
    //_owned_vertex_generator_ = false;
    //return;
  //}
 
  //void SLArDecay0GeneratorAction::SetVertexGenerator(SLArVertexGenerator* vertex_generator_ptr_)
  //{
    //if (_owned_vertex_generator_ and _vertex_generator_ != nullptr) {
      //delete _vertex_generator_;
      //_vertex_generator_ = nullptr;
      //_owned_vertex_generator_ = false;
    //}
    //_vertex_generator_ = vertex_generator_ptr_;
    //_owned_vertex_generator_ = true;
    //return;
  //}
 
  //SLArVertexGenerator& SLArDecay0GeneratorAction::GetVertexGenerator()
  //{
    //return *_vertex_generator_;
  //}
 
  //const SLArVertexGenerator& SLArDecay0GeneratorAction::GetVertexGeneratorConst() const
  //{
    //return *_vertex_generator_;
  //}

  void SLArDecay0GeneratorAction::SetDecayTime(const double time) 
  {
    if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::SetDecayTime: Entering..." << '\n';
    _decaytime_ = time; 
    if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::SetDecayTime: _decaytime_ set to " << _decaytime_ << " s" << std::endl; 
    if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::SetDecayTime: Exiting" << '\n';
  }
    
  void SLArDecay0GeneratorAction::GeneratePrimaries(G4Event * event_)
  {
    if (IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::GeneratePrimaries: Entering..." << '\n';
    bxdecay0::event gendecay;

    for (int iev = 0; iev < fConfig.n_decays; iev++) {

      //printf("Setting gendecay time to %g\n", _decaytime_);
      _pimpl_->get_decay0().shoot(_pimpl_->get_prng(), gendecay);
      double event_time = 0.0 * CLHEP::second;
      //Force event reference time to be zero (even if the generator tells something else):

      gendecay.set_time(_decaytime_); 
      if (gendecay.has_time()) {
        event_time = gendecay.get_time() * CLHEP::second;
      }
      //printf("event time is %g\n", event_time);
      double particle_time = 0.0 * CLHEP::second;
      const auto & particles = gendecay.get_particles();
      if (IsDebug()) std::cerr << "[debug] bxdecay0_g4::SLArDecay0GeneratorAction::GeneratePrimaries: Nb particles=" << particles.size() << '\n';

      // Shoot the common vertex:
      G4ThreeVector vertex(0.0, 0.0, 0.0);
      if (HasVertexGenerator()) {
        if (not fVtxGen->HasNextVertex()) {
          G4RunManager::GetRunManager()->AbortRun();
          G4Exception("bxdecay0_g4::SLArDecay0GeneratorAction::GeneratePrimaries: ",
              "InvalidArgument",
              RunMustBeAborted,
              "Vertex generator has no more vertex available! Abort run!");
        }
        fVtxGen->ShootVertex(vertex);  
      }

      // Scan the list of BxDecay0 generated particles:
      for (const auto & particle : particles) {
        // Reset gun's internals:
        dynamic_cast<Decay0ParticleGun*>(_particle_gun_.get())->ResetParticleData();
        // Particle type:
        if (particle.is_electron()) {
          _particle_gun_->SetParticleDefinition(G4Electron::ElectronDefinition());
        } else if (particle.is_positron()) {
          _particle_gun_->SetParticleDefinition(G4Positron::PositronDefinition());
        } else if (particle.is_gamma()) {
          _particle_gun_->SetParticleDefinition(G4Gamma::GammaDefinition());
        } else if (particle.is_alpha()) {
          _particle_gun_->SetParticleDefinition(G4Alpha::AlphaDefinition());
        } else {
          throw std::logic_error("bxdecay9_g4::SLArDecay0GeneratorAction::GeneratePrimaries: Unsupported particle type!");
        }
        // Time:
        if (particle.has_time()) {
          particle_time = particle.get_time() * CLHEP::second;
        } 
        // Shift particle time by the event reference time:
        particle_time += event_time;

        //printf("Create particle at %g ns (event time %g)\n", 
        //particle_time, event_time );
        _particle_gun_->SetParticleTime(particle_time );
        // Momentum:
        G4ThreeVector momentum(particle.get_px() * CLHEP::MeV,
            particle.get_py() * CLHEP::MeV,
            particle.get_pz() * CLHEP::MeV);
        _particle_gun_->SetParticleMomentum(momentum);
        // Vertex:
        _particle_gun_->SetParticlePosition(vertex);
        // Push the BxDecay0 generated  particle in the stack of Geant4 primaries:
        _particle_gun_->GeneratePrimaryVertex(event_);
      }
    }
    if (IsTrace()) std::cerr << "[trace] bxdecay0_g4::SLArDecay0GeneratorAction::GeneratePrimaries: Exiting..." << '\n';
    return;
  }

  void SLArDecay0GeneratorAction::Configure(const rapidjson::Value& config) {
    if ( config.HasMember("nuclide")) {
      fConfig.nuclide = config["nuclide"].GetString();
    } else {
      throw std::invalid_argument("decay0 gen requires \"nuclide\" field.\n");
    }

    if ( config.HasMember("decay0_type") ) {
      G4String d0type = config["decay0_type"].GetString(); 
      fConfig.decay_category = d0type;
      printf("fConfig.decay_category: %s\n", fConfig.decay_category.data()); 
    }

    if (config.HasMember("n_decays")) {
      fConfig.n_decays = config["n_decays"].GetInt(); 
    }

    if (config.HasMember("seed")) {
      fConfig.seed = config["seed"].GetInt();
    } else {
      fConfig.seed = SLArAnalysisManager::Instance()->GetSeed(); 
    }

    if (config.HasMember("vertex_gen")) {
      ConfigureVertexGenerator( config["vertex_gen"] ); 
    }
    else {
      fVtxGen = std::make_unique<SLArPointVertexGenerator>();
    }

    SetConfiguration(fConfig); 
    return;
  }

  G4String SLArDecay0GeneratorAction::WriteConfig() const {
    G4String config_str = "";

    rapidjson::Document d; 
    d.SetObject(); 
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    G4String gen_type = GetGeneratorType();

    d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
    d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
    d.AddMember("nuclide", rapidjson::StringRef(fConfig.nuclide.data()), d.GetAllocator()); 

    const rapidjson::Document vtx_json = fVtxGen->ExportConfig(); 
    rapidjson::Value vtx_config;
    vtx_config.CopyFrom(vtx_json, d.GetAllocator()); 
    d.AddMember("vertex_generator", vtx_config, d.GetAllocator()); 

    d.Accept(writer);
    config_str = buffer.GetString();

    return config_str;
  }

} // close bxdecay0_g4 namespace
} // close gen namespace
