/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGenerator.cc
 * @created     : Saturday Mar 30, 2024 22:08:11 CET
 */

#include <G4Event.hh>
#include <G4EventManager.hh>
#include <G4IonTable.hh>

#include <SLArBaseGenerator.hh>
#include <SLArBulkVertexGenerator.hh>
#include <SLArBoxSurfaceVertexGenerator.hh>
#include <SLArAnalysisManager.hh>

namespace gen {
void SLArBaseGenerator::ConfigureVertexGenerator(const rapidjson::Value& config) {
  if (!config.HasMember("type")) {
    throw std::invalid_argument("vertex genrator missing mandatory \"type\" field\n");
  }
  
  G4String type = config["type"].GetString(); 
  G4cerr << "Building " << type.data() << " vertex generator" << G4endl;
  EVertexGenerator kGen = getVtxGenIndex( type ); 

  switch (kGen) {
    case (EVertexGenerator::kPoint) : 
      {
        fVtxGen = std::make_unique<SLArPointVertexGenerator>();
        try { fVtxGen->Config( config["config"] ); }        
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArPointVertexGenerator()" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }
    
    case (EVertexGenerator::kBulk) : 
      {
        fVtxGen = std::make_unique<SLArBulkVertexGenerator>(); 
        try {fVtxGen->Config( config["config"] );}
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArBulkVertexGenerator()" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }

    case (EVertexGenerator::kBoxVolSurface) : 
      {
        fVtxGen = std::make_unique<SLArBoxSurfaceVertexGenerator>();
        try {fVtxGen->Config( config["config"] );} 
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArBoxSurfaceVertexGenerator" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }

    default:
      {
        char err_msg[100];
        gen::printVtxGeneratorType();
        sprintf(err_msg, "Unable to find %s vertex generator among the available options\n", 
            type.data()); 
        std::cerr << err_msg << std::endl;
        exit( EXIT_FAILURE ); 
        break;      
      }
  }

  printf("[gen] %s vtx gen for generator %s: %p\n", 
      fVtxGen->GetType().data(), fLabel.data(), static_cast<void*>(fVtxGen.get())); 
  
  return;
}

void SLArBaseGenerator::RegisterPrimaries(const G4Event* anEvent, const G4int firstVertex) {

  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
  G4IonTable* ionTable = G4IonTable::GetIonTable(); 

  G4int total_vertices = anEvent->GetNumberOfPrimaryVertex(); 

  if (fVerbose) {
    printf("[gen] %s primary generator action produced %i vertex(ices)\n", 
        fLabel.data(), total_vertices - firstVertex); 
  }
  for (int i=firstVertex; i<total_vertices; i++) {
    //std::unique_ptr<SLArMCPrimaryInfoUniquePtr> tc_primary = std::make_unique<SLArMCPrimaryInfoUniquePtr>();
    SLArMCPrimaryInfo tc_primary;
    G4int np = anEvent->GetPrimaryVertex(i)->GetNumberOfParticle(); 
    if (fVerbose) {
      printf("vertex %i has %i particles at t = %g\n", i, np, 
          anEvent->GetPrimaryVertex(i)->GetT0()); 
    }
    for (int ip = 0; ip<np; ip++) {
      //printf("getting particle %i...\n", ip); 
      auto particle = anEvent->GetPrimaryVertex(i)->GetPrimary(ip); 
      G4String name = ""; 

      if (!particle->GetParticleDefinition()) {
        tc_primary.SetID  (particle->GetPDGcode()); 
        name = ionTable->GetIon( particle->GetPDGcode() )->GetParticleName(); 
        tc_primary.SetName(name);
        tc_primary.SetTitle(name + " [" + particle->GetTrackID() +"]"); 
      } else {
        tc_primary.SetID  (particle->GetPDGcode());
        name = particle->GetParticleDefinition()->GetParticleName(); 
        tc_primary.SetName(name);
        tc_primary.SetTitle(name + " [" + particle->GetTrackID() +"]"); 
      }

      tc_primary.SetTrackID(particle->GetTrackID());
      tc_primary.SetPosition(anEvent->GetPrimaryVertex(i)->GetX0(),
          anEvent->GetPrimaryVertex(i)->GetY0(), 
          anEvent->GetPrimaryVertex(i)->GetZ0());
      tc_primary.SetMomentum(
          particle->GetPx(), particle->GetPy(), particle->GetPz(), 
          particle->GetKineticEnergy());
      tc_primary.SetTime(anEvent->GetPrimaryVertex(i)->GetT0()); 
      tc_primary.SetGeneratorLabel( fLabel.data() ); 

#ifdef SLAR_DEBUG
      printf("Adding particle to primary output list\n"); 
      tc_primary.PrintParticle(); 
      //getchar();
#endif
      SLArAnaMgr->GetEvent().RegisterPrimary( tc_primary );
    }
  }


}

}

