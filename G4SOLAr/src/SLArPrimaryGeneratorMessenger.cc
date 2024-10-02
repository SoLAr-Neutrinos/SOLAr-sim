/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArPrimaryGeneratorMessenger.cc
 * @created     Friday Dec 30, 2022 18:48:41 CET
 */


#include <cstdlib>
#include <SLArPrimaryGeneratorMessenger.hh>

#include <SLArPrimaryGeneratorAction.hh>
#ifdef SLAR_CRY
#include <cry/SLArCRYGeneratorAction.hh>
#endif
#include <SLArBulkVertexGenerator.hh>
#include <CLHEP/Units/SystemOfUnits.h>
#include <G4UIdirectory.hh>
#include <G4UIcmdWithADoubleAndUnit.hh>
#include <G4UIcmdWith3VectorAndUnit.hh>
#include <G4UIcmdWith3Vector.hh>
#include <G4UIcmdWithAString.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4UIcmdWithAnInteger.hh>
#include <G4UIcmdWithABool.hh>
#include <G4UIcmdWithAnInteger.hh>
#include <G4UIcmdWithoutParameter.hh>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

namespace gen {
SLArPrimaryGeneratorMessenger::SLArPrimaryGeneratorMessenger(SLArPrimaryGeneratorAction* SLArGun)
  : G4UImessenger(),
    fSLArAction(SLArGun)
{
  //fCmdGunDir = new G4UIdirectory("/SLAr/gen/");
  //fCmdGunDir->SetGuidance("PrimaryGenerator control");

  //fCmdGenerator= 
    //new G4UIcmdWithAString("/SLAr/gen/type", this);
  //fCmdGenerator->SetGuidance("Set SOLAr generator");
  //fCmdGenerator->SetGuidance("(ParticleGun, ParticleBomb, Decay0, Marley, ExternalGen, GENIE, CRY)");  
  //fCmdGenerator->SetParameterName("Mode", false);
  //fCmdGenerator->SetDefaultValue("ParticleGun");
  //fCmdGenerator->SetCandidates("ParticleGun ParticleBomb Decay0 Marley ExternalGen GENIE CRY");


  //fCmdParticle= 
    //new G4UIcmdWithAString("/SLAr/gen/particle", this);
  //fCmdParticle->SetGuidance("Set particle gun particle");
  //fCmdParticle->SetParameterName("particle_name", false);
  //fCmdParticle->SetDefaultValue("electron");

  //fCmdNumberOfParticles= 
    //new G4UIcmdWithAnInteger("/SLAr/gen/numberOfParticles", this);
  //fCmdNumberOfParticles->SetGuidance("Set number of particles for pgun and pbomb");
  //fCmdNumberOfParticles->SetParameterName("n_particles", false);
  //fCmdNumberOfParticles->SetDefaultValue(1);

  //fCmdBulkVol= 
    //new G4UIcmdWithAString("/SLAr/gen/volume", this); 
  //fCmdBulkVol->SetGuidance("Set bulk volume for bulk event generation"); 
  //fCmdBulkVol->SetGuidance("(Physical Volume name)"); 
  //fCmdBulkVol->SetParameterName("PhysVol", true, false); 
  //fCmdBulkVol->SetDefaultValue("target_lar_pv"); 

  //fCmdBulkVolFraction= 
    //new G4UIcmdWithADouble("/SLAr/gen/volumeFraction", this); 
  //fCmdBulkVol->SetGuidance("Set fraction of the bulk volume for event generation"); 
  //fCmdBulkVol->SetGuidance("<Volume Fraction>"); 
  //fCmdBulkVol->SetParameterName("volFrac", true, false); 
  //fCmdBulkVol->SetDefaultValue("1.0"); 

  //fCmdMarley= 
    //new G4UIcmdWithAString("/SLAr/gen/marleyconf", this); 
  //fCmdMarley->SetGuidance("Set MARLEY configuration file"); 
  //fCmdMarley->SetGuidance("(configuration file path)"); 
  //fCmdMarley->SetParameterName("marley_config", true, false); 
  //fCmdMarley->SetDefaultValue("marley_default.json"); 

  //fCmdBackgoundConf= 
    //new G4UIcmdWithAString("/SLAr/gen/backgroundconf", this); 
  //fCmdBackgoundConf->SetGuidance("Set backgound configuration file"); 
  //fCmdBackgoundConf->SetGuidance("(configuration file path)"); 
  //fCmdBackgoundConf->SetParameterName("background_config", true, false); 

  //fCmdExternalConf= 
    //new G4UIcmdWithAString("/SLAr/gen/externalconf", this); 
  //fCmdExternalConf->SetGuidance("Set external backgound configuration file"); 
  //fCmdExternalConf->SetGuidance("(configuration file path)"); 
  //fCmdExternalConf->SetParameterName("external_background_config", true, false); 

  //fCmdDirectionMode = 
    //new G4UIcmdWithAString("/SLAr/gen/SetDirectionMode", this);
  //fCmdDirectionMode->SetGuidance("Set direction mode (fixed, isotropic)");
  //fCmdDirectionMode->SetParameterName("DirectionMode", true);
  //fCmdDirectionMode->SetDefaultValue("fixed");

  //fCmdEnergy = 
    //new G4UIcmdWithADoubleAndUnit("/SLAr/gen/energy", this);
  //fCmdEnergy->SetGuidance("Set particle kinetic energy");
  //fCmdEnergy->SetParameterName("Energy", false);
  //fCmdEnergy->SetDefaultValue(1*CLHEP::MeV);

  //fCmdGunPosition = 
    //new G4UIcmdWith3VectorAndUnit("/SLAr/gen/gunPosition", this);
  //fCmdGunPosition->SetGuidance("Set position of the generated events");
  //fCmdGunPosition->SetParameterName("posX", "posY", "posZ", false);
  //fCmdGunPosition->SetDefaultValue(G4ThreeVector(0*CLHEP::cm, 0*CLHEP::cm, 0*CLHEP::cm));

  //fCmdGunDirection = 
    //new G4UIcmdWith3Vector("/SLAr/gen/direction", this);
  //fCmdGunDirection->SetGuidance("Set event momentum direction");
  //fCmdGunDirection->SetParameterName("p_x", "p_y", "p_z", false); 
  //fCmdGunDirection->SetDefaultValue( G4ThreeVector(0, 0, 1)); 

  fCmdGenConfig = 
    new G4UIcmdWithAString("/SLAr/gen/configure", this);
  fCmdGenConfig->SetGuidance("Set generator configuration");
  fCmdGenConfig->SetParameterName("ConfigFile", false);

  fCmdTracePhotons = 
    new G4UIcmdWithABool("/SLAr/phys/DoTracePhotons", this); 
  fCmdTracePhotons->SetGuidance("Set/unset tracing of optical photons"); 
  fCmdTracePhotons->SetParameterName("do_trace", false, true); 
  fCmdTracePhotons->SetDefaultValue(true);

  fCmdDriftElectrons = 
    new G4UIcmdWithABool("/SLAr/phys/DoDriftElectrons", this); 
  fCmdDriftElectrons->SetGuidance("Set/unset drift and collection of ionization electrons"); 
  fCmdDriftElectrons->SetParameterName("do_trace", false, true); 
  fCmdDriftElectrons->SetDefaultValue(true);

  //fCmdGENIEEvtSeed = 
    //new G4UIcmdWithAnInteger("/SLAr/gen/SetGENIENum",this);
  //fCmdGENIEEvtSeed->SetGuidance("Set starting GENIE event number");
  //fCmdGENIEEvtSeed->SetParameterName("GENIE_evt_num",true);
  //fCmdGENIEEvtSeed->SetDefaultValue(0);

  //fCmdGENIEFile = 
    //new G4UIcmdWithAString("/SLAr/gen/SetGENIEFile",this);
  //fCmdGENIEFile->SetGuidance("Set file name and path");
  //fCmdGENIEFile->SetParameterName("GENIE_file",false);
  //fCmdGENIEFile->SetDefaultValue("enubet_genie_seed.root");

#ifdef SLAR_CRY
  fCmdCRYFile = 
    new G4UIcmdWithAString("/SLAr/gen/CRY/file",this);
  fCmdCRYFile->SetGuidance("This reads the CRY definition from a file");
  fCmdCRYFile->AvailableForStates(G4State_PreInit,G4State_Idle);
   
  fCmdCRYInput = new G4UIcmdWithAString("/SLAr/gen/CRY/input",this);
  fCmdCRYInput->SetGuidance("CRY input lines");
  fCmdCRYInput->AvailableForStates(G4State_PreInit,G4State_Idle);

  fCmdCRYUpdate = new G4UIcmdWithoutParameter("/SLAr/gen/CRY/update",this);
  fCmdCRYUpdate->SetGuidance("Update CRY definition.");
  fCmdCRYUpdate->SetGuidance("This command MUST be applied before \"beamOn\" ");
  fCmdCRYUpdate->SetGuidance("if you changed the CRY definition.");
  fCmdCRYUpdate->AvailableForStates(G4State_Idle);

  fCmdCRYQuota = new G4UIcmdWithADoubleAndUnit("/SLAr/gen/CRY/quota", this); 
  fCmdCRYQuota->SetGuidance("Set cosmic-rays production quota (y)"); 
  fCmdCRYQuota->SetDefaultValue(0.0); 
  fCmdCRYQuota->SetDefaultUnit("m"); 
#endif

  fCmdVerbose = 
    new G4UIcmdWithAnInteger("/SLAr/gen/verbose", this); 
  fCmdVerbose->SetGuidance("verbose level"); 
  fCmdVerbose->SetParameterName("verbose", true, false); 
  fCmdVerbose->SetDefaultValue( 1 ); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArPrimaryGeneratorMessenger::~SLArPrimaryGeneratorMessenger()
{
  //delete fCmdGenerator;
  //delete fCmdParticle;
  //delete fCmdBulkVol;
  //delete fCmdEnergy;
  //delete fCmdNumberOfParticles;
  //delete fCmdBulkVolFraction;
  //delete fCmdMarley;
  //delete fCmdBackgoundConf;
  //delete fCmdGunPosition;
  //delete fCmdGunDirection;
  //delete fCmdGunDir;
  delete fCmdTracePhotons; 
  delete fCmdDriftElectrons;
  //delete fCmdGENIEEvtSeed;
  //delete fCmdGENIEFile;
#ifdef SLAR_CRY
  delete fCmdCRYFile; 
  delete fCmdCRYInput;
  delete fCmdCRYUpdate; 
  delete fCmdCRYQuota;
#endif
  delete fCmdVerbose;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPrimaryGeneratorMessenger::SetNewValue(
              G4UIcommand* command, G4String newValue)
{

  //if (command == fCmdGenerator) 
  //{
    //gen::EGenerator gen = gen::kParticleGun;
    //G4String strMode = newValue;
    //if      (G4StrUtil::contains(strMode, "Gun"   )) gen = gen::kParticleGun;
    //else if (G4StrUtil::contains(strMode, "Bomb"  )) gen = gen::kParticleBomb;
    //else if (G4StrUtil::contains(strMode, "Decay0")) gen = gen::kDecay0;
    //else if (G4StrUtil::contains(strMode, "Marley")) gen = gen::kMarley;
    //else if (G4StrUtil::contains(strMode, "ExternalGen")) gen = gen::kExternalGen;
    //else if (G4StrUtil::contains(strMode, "GENIE")) gen = gen::kGENIE;
//#ifdef SLAR_CRY
    //else if (G4StrUtil::contains(strMode, "CRY")) gen = gen::kCRY;
//#endif
    //else {
      //G4cout << "SLArPrimaryGeneratorMessenger: Unable to set generator " 
        //<< strMode.data() 
        //<< ". Generator is not defined of not properly linked."
        //<< G4endl;
    //}

    //fSLArAction->SetGenerator(gen);
  //} 
  //else if (command == fCmdBulkVol) { 
    //G4String vol = newValue; 
    ////fSLArAction->SetBulkName(vol); 
  //}
  //else if (command == fCmdParticle) {
    //G4String particle = newValue; 
    //fSLArAction->SetGunParticle(particle);
  //}
  //else if (command == fCmdBulkVolFraction) {
    //G4double frac = fCmdBulkVolFraction->GetNewDoubleValue(newValue); 
    ////fSLArAction->fBulkGenerator->SetFiducialFraction(frac);
  //}
  //else if (command == fCmdMarley) {
    //fSLArAction->SetMarleyConf(newValue); 
  //} 
  //else if (command == fCmdBackgoundConf) {
    //fSLArAction->SetBackgroundConf(newValue); 
  //}
  //else if (command == fCmdExternalConf) {
    //fSLArAction->SetExternalConf(newValue); 
  //}
  //else if (command == fCmdDirectionMode) {
    //if (G4StrUtil::contains(newValue, "fixed")) {
      //fSLArAction->SetDirectionMode(kFixed); 
    //} else if (
        //G4StrUtil::contains(newValue, "random") || 
        //G4StrUtil::contains(newValue, "isotropic")) 
    //{
      //fSLArAction->SetDirectionMode(kRandom);
    //} else {
      //G4cout << "WARNING: unknown key " << newValue
        //<< ". I will assume you want it isotropic" << G4endl; 
      //fSLArAction->SetDirectionMode(kRandom);
    //}
  //}
  //else if (command == fCmdGunPosition) {
    //G4ThreeVector pos = fCmdGunPosition->GetNew3VectorValue(newValue); 
    //fSLArAction->SetSourcePosition( pos ); 
  //}
  //else if (command == fCmdEnergy) {
    //G4double ekin = fCmdEnergy->GetNewDoubleValue(newValue); 
    //fSLArAction->SetGunEnergy(ekin);  
  //}
  //else if (command == fCmdGunDirection) {
    //G4ThreeVector dir = fCmdGunDirection->GetNew3VectorValue(newValue); 
    //fSLArAction->SetEventDirection(dir); 
  //}
  //else if (command == fCmdNumberOfParticles) {
    //G4int n = fCmdNumberOfParticles->GetNewIntValue(newValue); 
    //fSLArAction->SetGunNumberOfParticles(n);  
  //}
  if (command == fCmdTracePhotons) {
    bool do_trace = fCmdTracePhotons->GetNewBoolValue(newValue); 
    fSLArAction->SetTraceOptPhotons(do_trace); 
  }
  else if (command == fCmdDriftElectrons) {
    bool do_drift = fCmdDriftElectrons->GetNewBoolValue(newValue); 
    fSLArAction->SetDriftElectrons(do_drift); 
  }
  else if (command == fCmdGenConfig) {
    G4String config_file = newValue;
    fSLArAction->Configure( config_file ); 
  }
  //else if (command == fCmdGENIEEvtSeed) {
    //G4int evnt = fCmdGENIEEvtSeed->GetNewIntValue(newValue); 
    //fSLArAction->SetGENIEEvntExt(evnt); 
  //} //--JM
  //else if (command == fCmdGENIEFile) {
    //G4String filename = newValue; 
    //fSLArAction->SetGENIEFile(filename); 
  //} //--JM
//#ifdef SLAR_CRY
  //else if( command == fCmdCRYInput ) { 
    //cry::SLArCRYGeneratorAction* cryGen = dynamic_cast<cry::SLArCRYGeneratorAction*>(fSLArAction->GetGenerator(kCRY));
    //cryGen->InputCRY(); 
    //cryGen->GetMessInput().append(newValue);
    //cryGen->GetMessInput().append(" ");
  //}
  //else if ( command == fCmdCRYFile ) {
    //cry::SLArCRYGeneratorAction* cryGen = dynamic_cast<cry::SLArCRYGeneratorAction*>(fSLArAction->GetGenerator(kCRY));
    //cryGen->CRYFromFile( newValue ); 
  //}
  //else if ( command == fCmdCRYUpdate ) {
    //cry::SLArCRYGeneratorAction* cryGen = dynamic_cast<cry::SLArCRYGeneratorAction*>(fSLArAction->GetGenerator(kCRY));
    //cryGen->UpdateCRY(); 
  //}
  //else if ( command == fCmdCRYQuota ) {
    //cry::SLArCRYGeneratorAction* cryGen = dynamic_cast<cry::SLArCRYGeneratorAction*>(fSLArAction->GetGenerator(kCRY));
    //G4double y_cry = fCmdCRYQuota->GetNewDoubleValue( newValue ); 
    //cryGen->SetGenQuota( y_cry ); 
  //}
//#endif
  else if (command == fCmdVerbose) {
    G4String verbose_str = newValue; 
    fSLArAction->SetVerboseLevel( std::stoi( verbose_str ) ); 
  }

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
}
