#include "SLArGENIEGeneratorAction.hh"
#include <G4String.hh>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <cstdio>

namespace gen {

//***********************************************************************
//************************** CONSTRUCTORS *******************************

void SLArGENIEGeneratorAction::Initialize()
{
  TFile *GENIEInput = TFile::Open(fConfig.genie_file_path);

  m_gtree = (TTree*) GENIEInput->Get(fConfig.genie_tree_key);

  m_gtree->SetBranchAddress("EvtNum",&gVar.EvtNum);
  m_gtree->SetBranchAddress("StdHepN",&gVar.nPart);
  m_gtree->SetBranchAddress("StdHepPdg",&gVar.pdg);
  m_gtree->SetBranchAddress("StdHepStatus",&gVar.status);
  m_gtree->SetBranchAddress("StdHepP4",&gVar.p4);
  m_gtree->SetBranchAddress("StdHepX4",&gVar.x4);
  m_gtree->SetBranchAddress("EvtVtx",&gVar.vtx);
}

SLArGENIEGeneratorAction::SLArGENIEGeneratorAction(const G4String label) 
  : SLArBaseGenerator(label), m_gtree(0)
{}

SLArGENIEGeneratorAction::SLArGENIEGeneratorAction(const G4String label, const G4String genie_file)
  : SLArBaseGenerator(label), m_gtree(0)
{}

SLArGENIEGeneratorAction::~SLArGENIEGeneratorAction()
{}

void SLArGENIEGeneratorAction::Configure(const rapidjson::Value& config) {
  assert( config.HasMember("genie_file_path") ); 
  fConfig.genie_file_path = config["genie_file_path"].GetString(); 

  if (config.HasMember("genie_tree_key")) {
    fConfig.genie_tree_key = config["genie_tree_key"].GetString();
  } else {
    fConfig.genie_tree_key = "enubetG"; 
  }

  if (config.HasMember("tree_first_entry")) {
    fConfig.tree_first_entry = config["tree_first_entry"].GetInt(); 
  }

  Initialize(); 
  return;
}

G4String SLArGENIEGeneratorAction::WriteConfig() const {
  G4String config_str = "";

  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  G4String gen_type = GetGeneratorType();

  d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  d.AddMember("genie_file_path", rapidjson::StringRef(fConfig.genie_file_path.data()), d.GetAllocator());
  d.AddMember("genie_tree_key", rapidjson::StringRef(fConfig.genie_tree_key.data()), d.GetAllocator());
  d.AddMember("tree_first_entry", fConfig.tree_first_entry, d.GetAllocator()); 

  d.Accept(writer);
  config_str = buffer.GetString();
  return config_str;
}


//***********************************************************************

//***********************************************************************
//************************** EVENT FUNCTIONS ****************************

void SLArGENIEGeneratorAction::GeneratePrimaries(G4Event *ev)
{
  // No idea about the units, need to think about what we need
  
  int evtNum = ev->GetEventID() + fConfig.tree_first_entry;
  m_gtree->GetEntry(evtNum);
  std::cout << "   GENIE TTree event selection: " << evtNum << std::endl;

  size_t particle_idx = 0; // Think this can be done in a better way

  G4ThreeVector vtx(0.,0.,0.);
  vtx.set(gVar.vtx[0]*10, gVar.vtx[1]*10, gVar.vtx[2]*10);
  std::vector<G4PrimaryVertex*> primary_vertices;

  for (int i=0; i<gVar.nPart; i++){

    if (gVar.status[i] == 1){ // 0 - incoming; 1 - outgoing; x - virtual
      G4PrimaryParticle *particle = new G4PrimaryParticle(gVar.pdg[i],
          gVar.p4[i][0]*1E3,
          gVar.p4[i][1]*1E3,
          gVar.p4[i][2]*1E3,
          gVar.p4[i][3]*1E3);
      auto vertex = new G4PrimaryVertex(vtx, 0.); // Not sure which is better here
      //    auto vertex = new G4PrimaryVertex(vtx, gVar.vtx[3]); 
      vertex->SetPrimary(particle);
      primary_vertices.push_back(vertex);

      particle_idx++;
    }
  }

  for (const auto& vertex : primary_vertices)
    ev->AddPrimaryVertex(vertex);
}

//***********************************************************************


//***********************************************************************
//***********************************************************************

void SLArGENIEGeneratorAction::SetGENIEEvntExt(G4int evntID)
{
  fConfig.tree_first_entry = evntID;
}


//***********************************************************************
}
