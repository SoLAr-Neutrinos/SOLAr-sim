#include "SLArDetectorConstruction.hh"
//#include "detector/TPC/SLArPENFilm.hh"

#include "SLArAnalysisManager.hh"
#include "SLArMaterial.hh"

#include "G4PhysicalConstants.hh"
#include "G4Box.hh"
#include "G4Trd.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4Tubs.hh"
#include "G4Ellipsoid.hh"
#include "G4Sphere.hh"
#include "G4Cons.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4VisAttributes.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4Material.hh"


SLArPENFilm::SLArPENFilm() :SLArBaseDetModule(),
  fMatPEN(nullptr), fSkinPENSurface(nullptr)
{
  fGeoInfo = new SLArGeoInfo();
}

SLArPENFilm::~SLArPENFilm() {
  std::cerr << "Deleting SLArPENFilm..." << std::endl;
  std::cerr << "SLArPENFilm DONE" << std::endl;

}

//G4VPhysicalVolume *SLArPENFilm::Construct()

void SLArPENFilm::BuildMaterial(G4String db_file)
{
  fMatPEN = new SLArMaterial();
  
  fMatPEN->SetMaterialID("PEN");
  fMatPEN->BuildMaterialFromDB(db_file);
}

void SLArPENFilm::BuildDefalutGeoParMap()
{
  G4cerr << "SLArPENFilm::BuildGeoParMap()" << G4endl;
  fGeoInfo->RegisterGeoPar("pos_x"       ,   0.5*CLHEP::mm);
  fGeoInfo->RegisterGeoPar("pos_y"       ,   0.0*CLHEP::mm);
  fGeoInfo->RegisterGeoPar("pos_z"       ,   0.0*CLHEP::mm);
  fGeoInfo->RegisterGeoPar("dim_y"       ,   150.0*CLHEP::cm);
  fGeoInfo->RegisterGeoPar("dim_z"       ,   200.0*CLHEP::cm);
  fGeoInfo->RegisterGeoPar("dim_x"       ,   5.0*CLHEP::mm);
  G4cerr << "Exit method\n" << G4endl;
}
  
void SLArPENFilm::BuildPEN()
{

  G4cerr << "SLArPENFilm::BuildPEN()" << G4endl;
  //Building Target
  G4cerr << "\tBuilding PEN Film" << G4endl;
  G4double PENX= fGeoInfo->GetGeoPar("dim_x");
  G4double PENY= fGeoInfo->GetGeoPar("dim_y");
  G4double PENZ= fGeoInfo->GetGeoPar("dim_z");
  
  //Create and fill fTarget
  G4double x_ = PENX*0.5;
  G4double y_ = PENY*0.5;
  G4double z_ = PENZ*0.5;
  
  fModSV = new G4Box("penfilm"+std::to_string(fID)+"_sv", x_, y_, z_);
  
  SetLogicVolume(new G4LogicalVolume(fModSV, fMatPEN->GetMaterial(), "penfilm"+std::to_string(fID)+"_lv"));
  
  G4OpticalSurface* OpPENSkin = new G4OpticalSurface("PENSkin");

  OpPENSkin->SetModel(unified);
  OpPENSkin->SetType(dielectric_dielectric);
  OpPENSkin->SetFinish(polished);


  std::vector<G4double> ph_ene1 = {0.025*CLHEP::eV, 0.075*CLHEP::eV, 0.125*CLHEP::eV, 0.175*CLHEP::eV, 0.225*CLHEP::eV, 0.275*CLHEP::eV, 0.325*CLHEP::eV, 0.375*CLHEP::eV, 0.425*CLHEP::eV, 0.475*CLHEP::eV, 0.525*CLHEP::eV, 0.575*CLHEP::eV, 0.625*CLHEP::eV, 0.675*CLHEP::eV, 0.725*CLHEP::eV, 0.775*CLHEP::eV, 0.825*CLHEP::eV, 0.875*CLHEP::eV, 0.925*CLHEP::eV, 0.975*CLHEP::eV, 1.025*CLHEP::eV, 1.075*CLHEP::eV, 1.125*CLHEP::eV, 1.175*CLHEP::eV, 1.225*CLHEP::eV, 1.275*CLHEP::eV, 1.325*CLHEP::eV, 1.375*CLHEP::eV, 1.425*CLHEP::eV, 1.475*CLHEP::eV, 1.525*CLHEP::eV, 1.575*CLHEP::eV, 1.625*CLHEP::eV, 1.675*CLHEP::eV, 1.725*CLHEP::eV, 1.775*CLHEP::eV, 1.825*CLHEP::eV, 1.875*CLHEP::eV, 1.925*CLHEP::eV, 1.975*CLHEP::eV, 2.025*CLHEP::eV, 2.075*CLHEP::eV, 2.125*CLHEP::eV, 2.175*CLHEP::eV, 2.225*CLHEP::eV, 2.275*CLHEP::eV, 2.325*CLHEP::eV, 2.375*CLHEP::eV, 2.425*CLHEP::eV, 2.475*CLHEP::eV, 2.525*CLHEP::eV, 2.575*CLHEP::eV, 2.625*CLHEP::eV, 2.675*CLHEP::eV, 2.725*CLHEP::eV, 2.775*CLHEP::eV, 2.825*CLHEP::eV, 2.875*CLHEP::eV, 2.925*CLHEP::eV, 2.975*CLHEP::eV, 3.025*CLHEP::eV, 3.075*CLHEP::eV, 3.125*CLHEP::eV, 3.175*CLHEP::eV, 3.225*CLHEP::eV, 3.275*CLHEP::eV, 3.325*CLHEP::eV, 3.375*CLHEP::eV, 3.425*CLHEP::eV, 3.475*CLHEP::eV, 3.525*CLHEP::eV, 3.575*CLHEP::eV, 3.625*CLHEP::eV, 3.675*CLHEP::eV, 3.725*CLHEP::eV, 3.775*CLHEP::eV, 3.825*CLHEP::eV, 3.875*CLHEP::eV, 3.925*CLHEP::eV, 3.975*CLHEP::eV, 4.025*CLHEP::eV, 4.075*CLHEP::eV, 4.125*CLHEP::eV, 4.175*CLHEP::eV, 4.225*CLHEP::eV, 4.275*CLHEP::eV, 4.325*CLHEP::eV, 4.375*CLHEP::eV, 4.425*CLHEP::eV, 4.475*CLHEP::eV, 4.525*CLHEP::eV, 4.575*CLHEP::eV, 4.625*CLHEP::eV, 4.675*CLHEP::eV, 4.725*CLHEP::eV, 4.775*CLHEP::eV, 4.825*CLHEP::eV, 4.875*CLHEP::eV, 4.925*CLHEP::eV, 4.975*CLHEP::eV, 5.025*CLHEP::eV, 5.075*CLHEP::eV, 5.125*CLHEP::eV, 5.175*CLHEP::eV, 5.225*CLHEP::eV, 5.275*CLHEP::eV, 5.325*CLHEP::eV, 5.375*CLHEP::eV, 5.425*CLHEP::eV, 5.475*CLHEP::eV, 5.525*CLHEP::eV, 5.575*CLHEP::eV, 5.625*CLHEP::eV, 5.675*CLHEP::eV, 5.725*CLHEP::eV, 5.775*CLHEP::eV, 5.825*CLHEP::eV, 5.875*CLHEP::eV, 5.925*CLHEP::eV, 5.975*CLHEP::eV, 6.025*CLHEP::eV, 6.075*CLHEP::eV, 6.125*CLHEP::eV, 6.175*CLHEP::eV, 6.225*CLHEP::eV, 6.275*CLHEP::eV, 6.325*CLHEP::eV, 6.375*CLHEP::eV, 6.425*CLHEP::eV, 6.475*CLHEP::eV, 6.525*CLHEP::eV, 6.575*CLHEP::eV, 6.625*CLHEP::eV, 6.675*CLHEP::eV, 6.725*CLHEP::eV, 6.775*CLHEP::eV, 6.825*CLHEP::eV, 6.875*CLHEP::eV, 6.925*CLHEP::eV, 6.975*CLHEP::eV, 7.025*CLHEP::eV, 7.075*CLHEP::eV, 7.125*CLHEP::eV, 7.175*CLHEP::eV, 7.225*CLHEP::eV, 7.275*CLHEP::eV, 7.325*CLHEP::eV, 7.375*CLHEP::eV, 7.425*CLHEP::eV, 7.475*CLHEP::eV, 7.525*CLHEP::eV, 7.575*CLHEP::eV, 7.625*CLHEP::eV, 7.675*CLHEP::eV, 7.725*CLHEP::eV, 7.775*CLHEP::eV, 7.825*CLHEP::eV, 7.875*CLHEP::eV, 7.925*CLHEP::eV, 7.975*CLHEP::eV, 8.025*CLHEP::eV, 8.075*CLHEP::eV, 8.125*CLHEP::eV, 8.175*CLHEP::eV, 8.225*CLHEP::eV, 8.275*CLHEP::eV, 8.325*CLHEP::eV, 8.375*CLHEP::eV, 8.425*CLHEP::eV, 8.475*CLHEP::eV, 8.525*CLHEP::eV, 8.575*CLHEP::eV, 8.625*CLHEP::eV, 8.675*CLHEP::eV, 8.725*CLHEP::eV, 8.775*CLHEP::eV, 8.825*CLHEP::eV, 8.875*CLHEP::eV, 8.925*CLHEP::eV, 8.975*CLHEP::eV, 9.025*CLHEP::eV, 9.075*CLHEP::eV, 9.125*CLHEP::eV, 9.175*CLHEP::eV, 9.225*CLHEP::eV, 9.275*CLHEP::eV, 9.325*CLHEP::eV, 9.375*CLHEP::eV, 9.425*CLHEP::eV, 9.475*CLHEP::eV, 9.525*CLHEP::eV, 9.575*CLHEP::eV, 9.625*CLHEP::eV, 9.675*CLHEP::eV, 9.725*CLHEP::eV, 9.775*CLHEP::eV, 9.825*CLHEP::eV, 9.875*CLHEP::eV, 9.925*CLHEP::eV, 9.975*CLHEP::eV};
  
  std::vector<G4double> ph_ene2 = {1.92072*CLHEP::eV, 1.92248*CLHEP::eV, 1.92547*CLHEP::eV, 1.92846*CLHEP::eV, 1.93146*CLHEP::eV, 1.93448*CLHEP::eV, 1.9375*CLHEP::eV, 1.94053*CLHEP::eV, 1.94357*CLHEP::eV, 1.94662*CLHEP::eV, 1.94969*CLHEP::eV, 1.95276*CLHEP::eV, 1.95584*CLHEP::eV, 1.95893*CLHEP::eV, 1.96203*CLHEP::eV, 1.96513*CLHEP::eV, 1.96825*CLHEP::eV, 1.97138*CLHEP::eV, 1.97452*CLHEP::eV, 1.97767*CLHEP::eV, 1.98083*CLHEP::eV, 1.984*CLHEP::eV, 1.98718*CLHEP::eV, 1.99037*CLHEP::eV, 1.99357*CLHEP::eV, 1.99678*CLHEP::eV, 2*CLHEP::eV, 2.00323*CLHEP::eV, 2.00647*CLHEP::eV, 2.00972*CLHEP::eV, 2.01299*CLHEP::eV, 2.01626*CLHEP::eV, 2.01954*CLHEP::eV, 2.02284*CLHEP::eV, 2.02614*CLHEP::eV, 2.02946*CLHEP::eV, 2.03279*CLHEP::eV, 2.03612*CLHEP::eV, 2.03947*CLHEP::eV, 2.04283*CLHEP::eV, 2.0462*CLHEP::eV, 2.04959*CLHEP::eV, 2.05298*CLHEP::eV, 2.05638*CLHEP::eV, 2.0598*CLHEP::eV, 2.06323*CLHEP::eV, 2.06667*CLHEP::eV, 2.07012*CLHEP::eV, 2.07358*CLHEP::eV, 2.07705*CLHEP::eV, 2.08054*CLHEP::eV, 2.08403*CLHEP::eV, 2.08754*CLHEP::eV, 2.09106*CLHEP::eV, 2.09459*CLHEP::eV, 2.09814*CLHEP::eV, 2.10169*CLHEP::eV, 2.10526*CLHEP::eV, 2.10884*CLHEP::eV, 2.11244*CLHEP::eV, 2.11604*CLHEP::eV, 2.11966*CLHEP::eV, 2.12329*CLHEP::eV, 2.12693*CLHEP::eV, 2.13058*CLHEP::eV, 2.13425*CLHEP::eV, 2.13793*CLHEP::eV, 2.14162*CLHEP::eV, 2.14533*CLHEP::eV, 2.14905*CLHEP::eV, 2.15278*CLHEP::eV, 2.15652*CLHEP::eV, 2.16028*CLHEP::eV, 2.16405*CLHEP::eV, 2.16783*CLHEP::eV, 2.17163*CLHEP::eV, 2.17544*CLHEP::eV, 2.17926*CLHEP::eV, 2.1831*CLHEP::eV, 2.18695*CLHEP::eV, 2.19081*CLHEP::eV, 2.19469*CLHEP::eV, 2.19858*CLHEP::eV, 2.20249*CLHEP::eV, 2.20641*CLHEP::eV, 2.21034*CLHEP::eV, 2.21429*CLHEP::eV, 2.21825*CLHEP::eV, 2.22222*CLHEP::eV, 2.22621*CLHEP::eV, 2.23022*CLHEP::eV, 2.23423*CLHEP::eV, 2.23827*CLHEP::eV, 2.24231*CLHEP::eV, 2.24638*CLHEP::eV, 2.25045*CLHEP::eV, 2.25455*CLHEP::eV, 2.25865*CLHEP::eV, 2.26277*CLHEP::eV, 2.26691*CLHEP::eV, 2.27106*CLHEP::eV, 2.27523*CLHEP::eV, 2.27941*CLHEP::eV, 2.28361*CLHEP::eV, 2.28782*CLHEP::eV, 2.29205*CLHEP::eV, 2.2963*CLHEP::eV, 2.30056*CLHEP::eV, 2.30483*CLHEP::eV, 2.30912*CLHEP::eV, 2.31343*CLHEP::eV, 2.31776*CLHEP::eV, 2.3221*CLHEP::eV, 2.32645*CLHEP::eV, 2.33083*CLHEP::eV, 2.33522*CLHEP::eV, 2.33962*CLHEP::eV, 2.34405*CLHEP::eV, 2.34848*CLHEP::eV, 2.35294*CLHEP::eV, 2.35741*CLHEP::eV, 2.3619*CLHEP::eV, 2.36641*CLHEP::eV, 2.37094*CLHEP::eV, 2.37548*CLHEP::eV, 2.38004*CLHEP::eV, 2.38462*CLHEP::eV, 2.38921*CLHEP::eV, 2.39382*CLHEP::eV, 2.39845*CLHEP::eV, 2.4031*CLHEP::eV, 2.40777*CLHEP::eV, 2.41245*CLHEP::eV, 2.41715*CLHEP::eV,2.42188*CLHEP::eV, 2.42661*CLHEP::eV, 2.43137*CLHEP::eV, 2.43615*CLHEP::eV, 2.44094*CLHEP::eV, 2.44576*CLHEP::eV, 2.45059*CLHEP::eV, 2.45545*CLHEP::eV, 2.46032*CLHEP::eV, 2.46521*CLHEP::eV, 2.47012*CLHEP::eV, 2.47505*CLHEP::eV, 2.48*CLHEP::eV, 2.48497*CLHEP::eV, 2.48996*CLHEP::eV, 2.49497*CLHEP::eV, 2.5*CLHEP::eV, 2.50505*CLHEP::eV, 2.51012*CLHEP::eV, 2.51521*CLHEP::eV, 2.52033*CLHEP::eV, 2.52546*CLHEP::eV, 2.53061*CLHEP::eV, 2.53579*CLHEP::eV, 2.54098*CLHEP::eV, 2.5462*CLHEP::eV, 2.55144*CLHEP::eV, 2.5567*CLHEP::eV, 2.56198*CLHEP::eV, 2.56729*CLHEP::eV, 2.57261*CLHEP::eV, 2.57796*CLHEP::eV, 2.58333*CLHEP::eV, 2.58873*CLHEP::eV, 2.59414*CLHEP::eV, 2.59958*CLHEP::eV, 2.60504*CLHEP::eV, 2.61053*CLHEP::eV, 2.61603*CLHEP::eV, 2.62156*CLHEP::eV, 2.62712*CLHEP::eV, 2.6327*CLHEP::eV, 2.6383*CLHEP::eV, 2.64392*CLHEP::eV, 2.64957*CLHEP::eV, 2.65525*CLHEP::eV, 2.66094*CLHEP::eV, 2.66667*CLHEP::eV, 2.67241*CLHEP::eV, 2.67819*CLHEP::eV, 2.68398*CLHEP::eV, 2.6898*CLHEP::eV, 2.69565*CLHEP::eV, 2.70153*CLHEP::eV, 2.70742*CLHEP::eV, 2.71335*CLHEP::eV, 2.7193*CLHEP::eV, 2.72527*CLHEP::eV, 2.73128*CLHEP::eV, 2.73731*CLHEP::eV, 2.74336*CLHEP::eV, 2.74945*CLHEP::eV, 2.75556*CLHEP::eV, 2.76169*CLHEP::eV, 2.76786*CLHEP::eV, 2.77405*CLHEP::eV, 2.78027*CLHEP::eV, 2.78652*CLHEP::eV, 2.79279*CLHEP::eV, 2.7991*CLHEP::eV, 2.80543*CLHEP::eV, 2.81179*CLHEP::eV, 2.81818*CLHEP::eV, 2.8246*CLHEP::eV, 2.83105*CLHEP::eV, 2.83753*CLHEP::eV, 2.84404*CLHEP::eV, 2.85057*CLHEP::eV, 2.85714*CLHEP::eV, 2.86374*CLHEP::eV, 2.87037*CLHEP::eV, 2.87703*CLHEP::eV, 2.88372*CLHEP::eV, 2.89044*CLHEP::eV, 2.8972*CLHEP::eV, 2.90398*CLHEP::eV, 2.9108*CLHEP::eV, 2.91765*CLHEP::eV, 2.92453*CLHEP::eV, 2.93144*CLHEP::eV, 2.93839*CLHEP::eV, 2.94537*CLHEP::eV, 2.95238*CLHEP::eV, 2.95943*CLHEP::eV, 2.96651*CLHEP::eV, 2.97362*CLHEP::eV, 2.98077*CLHEP::eV, 2.98795*CLHEP::eV, 2.99517*CLHEP::eV, 3.00242*CLHEP::eV, 3.00971*CLHEP::eV, 3.01703*CLHEP::eV, 3.02439*CLHEP::eV, 3.03178*CLHEP::eV, 3.03922*CLHEP::eV, 3.04668*CLHEP::eV, 3.05419*CLHEP::eV, 3.06173*CLHEP::eV, 3.06931*CLHEP::eV, 3.07692*CLHEP::eV, 3.08458*CLHEP::eV, 3.09227*CLHEP::eV, 3.1*CLHEP::eV, 3.10777*CLHEP::eV, 3.11558*CLHEP::eV, 3.12343*CLHEP::eV, 3.13131*CLHEP::eV, 3.13924*CLHEP::eV, 3.14721*CLHEP::eV, 3.15522*CLHEP::eV, 3.16327*CLHEP::eV, 3.17136*CLHEP::eV, 3.17949*CLHEP::eV, 3.18766*CLHEP::eV, 3.19588*CLHEP::eV, 3.20413*CLHEP::eV, 3.21244*CLHEP::eV, 3.22078*CLHEP::eV, 3.22917*CLHEP::eV, 3.2376*CLHEP::eV, 3.24607*CLHEP::eV, 3.25459*CLHEP::eV, 3.26316*CLHEP::eV,3.27177*CLHEP::eV, 3.28042*CLHEP::eV, 3.28912*CLHEP::eV, 3.29787*CLHEP::eV, 3.30667*CLHEP::eV, 3.31551*CLHEP::eV, 3.3244*CLHEP::eV, 3.33333*CLHEP::eV, 3.34232*CLHEP::eV, 3.35135*CLHEP::eV, 3.36043*CLHEP::eV, 3.36957*CLHEP::eV, 3.37875*CLHEP::eV, 3.38798*CLHEP::eV, 3.39726*CLHEP::eV, 3.40659*CLHEP::eV, 3.41598*CLHEP::eV, 3.42541*CLHEP::eV, 3.4349*CLHEP::eV, 3.44444*CLHEP::eV, 3.45404*CLHEP::eV, 3.46369*CLHEP::eV, 3.47339*CLHEP::eV, 3.48315*CLHEP::eV, 3.49296*CLHEP::eV, 3.50282*CLHEP::eV, 3.51275*CLHEP::eV, 3.52273*CLHEP::eV, 3.53276*CLHEP::eV, 3.54286*CLHEP::eV, 3.55301*CLHEP::eV, 3.56322*CLHEP::eV, 3.57349*CLHEP::eV, 3.58382*CLHEP::eV, 3.5942*CLHEP::eV, 3.60465*CLHEP::eV, 3.61516*CLHEP::eV, 3.62573*CLHEP::eV, 3.63636*CLHEP::eV, 3.64706*CLHEP::eV, 3.65782*CLHEP::eV, 3.66864*CLHEP::eV,3.67953*CLHEP::eV, 3.69048*CLHEP::eV, 3.70149*CLHEP::eV, 3.71257*CLHEP::eV, 3.72372*CLHEP::eV, 3.73494*CLHEP::eV, 3.74622*CLHEP::eV, 3.75758*CLHEP::eV, 3.769*CLHEP::eV, 3.78049*CLHEP::eV, 3.79205*CLHEP::eV, 3.80368*CLHEP::eV, 3.81538*CLHEP::eV, 3.82716*CLHEP::eV, 3.83901*CLHEP::eV, 3.85093*CLHEP::eV, 3.86293*CLHEP::eV, 3.875*CLHEP::eV, 3.88715*CLHEP::eV, 3.89937*CLHEP::eV, 3.91167*CLHEP::eV, 3.92405*CLHEP::eV, 3.93651*CLHEP::eV, 3.94904*CLHEP::eV, 3.96166*CLHEP::eV, 3.97436*CLHEP::eV, 3.98714*CLHEP::eV, 4*CLHEP::eV, 4.01294*CLHEP::eV, 4.02597*CLHEP::eV, 4.03909*CLHEP::eV, 4.05229*CLHEP::eV, 4.06557*CLHEP::eV, 4.07895*CLHEP::eV, 4.09241*CLHEP::eV, 4.10596*CLHEP::eV};
  
  std::vector<G4double> Rindex = {1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22,1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.22, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47,1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47,1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47};
  
  std::vector <G4double> WLSComponent = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.7033, 0.7233, 0.7434, 0.7635, 0.7836, 0.8036, 0.8237, 0.8438, 0.8638, 0.9085, 0.9587, 1.0088, 1.059, 1.1439, 1.2298, 1.3157, 1.3875, 1.4501, 1.5128, 1.5754, 1.5989, 1.61, 1.6212, 1.6323, 1.6322, 1.6179, 1.6035, 1.5892, 1.5748, 1.5605, 1.5461, 1.5318, 1.5174, 1.5031, 1.4757, 1.4254, 1.3752, 1.3249, 1.2835, 1.25, 1.2165, 1.183, 1.1481, 1.1079, 1.0677, 1.0275, 0.9873, 0.9586, 0.9461, 0.9335, 0.921, 0.935, 0.9684, 1.0019, 1.0353, 1.0602, 1.0602, 1.0602, 1.0602, 1.0602, 1.0837, 1.1088, 1.1339, 1.1576, 1.1676, 1.1776, 1.1877, 1.1977, 1.2178, 1.2624, 1.3069, 1.3515, 1.3961, 1.446, 1.4962, 1.5463, 1.5958, 1.6404, 1.6849, 1.7295, 1.7741, 1.8364, 1.9032, 1.9701, 2.0102, 2.0478, 2.0855, 2.1274, 2.2276, 2.3277, 2.4233, 2.4985, 2.5737, 2.6488, 2.7407, 2.8659, 2.9705, 3.0457, 3.1209, 3.196, 3.2946, 3.4114, 3.5282, 3.6338, 3.734, 3.8341, 3.9445, 4.0779, 4.2114, 4.3559, 4.5159, 4.6761, 4.856, 5.036, 5.208, 5.3747, 5.5413, 5.7174, 5.8973, 6.0905, 6.3152, 6.5162, 6.6763, 6.8363, 6.9963, 7.1563, 7.3788, 7.6531, 7.9426, 8.2295, 8.4542, 8.7004, 9.0174, 9.3344, 9.6514, 9.9685, 10.2855, 10.4774, 10.6574, 10.9412, 11.328, 11.7149, 12.1017, 12.4885, 12.8753, 13.2622, 13.649, 14.0358, 14.4226, 14.8095, 15.1963, 15.5831, 15.9699, 16.3568, 16.7436, 17.1304, 17.5139, 17.8459, 18.206, 18.584, 18.9841, 19.4696, 19.9286, 20.3798, 20.7118, 21.1383, 21.4874, 21.8325, 22.1975, 22.5624, 22.8947, 23.1905, 23.4152, 23.6399, 23.8646, 24.0665, 24.1667, 24.2668, 24.367, 24.4019, 24.422, 24.442, 24.4621, 24.4817, 24.4593, 24.437, 24.4147, 24.3924, 24.3331, 24.2577, 24.1823, 24.1068, 23.94, 23.7585, 23.5194, 23.2182, 22.9657, 22.6789, 22.1803, 21.7114, 21.2033, 20.6531, 20.0212, 19.3893, 18.6341, 17.9233, 17.2764, 16.6762, 16.065, 15.3992, 14.8049, 14.2967, 13.8368, 13.3293, 12.8117, 12.2154, 11.6049, 10.9553, 10.4472, 9.5443, 8.9919, 8.2398, 7.4866, 6.9587, 6.4308, 5.9029, 5.3749, 4.847, 4.3191, 3.7912, 3.2633, 2.7353, 2.2074, 1.6795, 1.1516, 0.938, 0.8374, 0.7367, 0.5925, 0.4414, 0.4019, 0.3684, 0.3349, 0.3014, 0.2892, 0.2892, 0.2892, 0.2892, 0.2715, 0.2491, 0.2268, 0.2045, 0.1975, 0.2076, 0.2176, 0.2277, 0.2377, 0.2477, 0.2578, 0.2678, 0.2778, 0.2879, 0.2979, 0.308, 0.318, 0.328, 0.3373, 0.3373, 0.3373, 0.3373, 0.3373, 0.3404, 0.3515, 0.3627, 0.3738, 0.385, 0.376, 0.366, 0.3559, 0.3459, 0.3373, 0.3373, 0.3373, 0.3373, 0.3373, 0.3303, 0.3102, 0.2901, 0.27, 0.2499, 0.2471, 0.2583, 0.2695, 0.2806, 0.2845, 0.2644, 0.2443, 0.2242, 0.2041};
  
  std::vector <G4double> AbsLength = {10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m,10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 10000*CLHEP::m, 1.63648e-6*CLHEP::m, 1.18436e-6*CLHEP::m, 9.1262e-7*CLHEP::m,  8.73213e-7*CLHEP::m, 9.42295e-7*CLHEP::m, 1.01826e-6*CLHEP::m, 1.2937e-6*CLHEP::m, 1.6736e-6*CLHEP::m, 1.9233e-6*CLHEP::m, 2.53853e-6*CLHEP::m, 3.24046e-6*CLHEP::m, 3.99722e-6*CLHEP::m, 4.97368e-6*CLHEP::m, 6.27938e-6*CLHEP::m, 7.11376e-6*CLHEP::m, 9.05781e-6*CLHEP::m, 9.99078e-6*CLHEP::m, 0.0000126622*CLHEP::m, 0.0000140574*CLHEP::m, 0.0000148975*CLHEP::m, 0.0000148975*CLHEP::m, 0.0000144822*CLHEP::m, 0.0000129912*CLHEP::m, 0.0000129912*CLHEP::m, 9.22762e-6*CLHEP::m, 7.0908e-6*CLHEP::m, 4.37966e-6*CLHEP::m, 3.43805e-6*CLHEP::m, 3.01646e-6*CLHEP::m, 2.36743e-6*CLHEP::m, 2.27883e-6*CLHEP::m, 2.32486e-6*CLHEP::m, 2.32486e-6*CLHEP::m, 2.60744e-6*CLHEP::m, 2.8229e-6*CLHEP::m, 3.05677e-6*CLHEP::m, 3.27694e-6*CLHEP::m, 3.4289e-6*CLHEP::m, 3.87109e-6*CLHEP::m, 4.06267e-6*CLHEP::m, 4.04944e-6*CLHEP::m, 4.04944e-6*CLHEP::m, 4.30774e-6*CLHEP::m, 4.59288e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m, 4.90939e-6*CLHEP::m};

  std::vector <G4double> Rayleigh = {1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm,1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 1000*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm, 0.00015*CLHEP::cm};
  
  /*
  auto PENProperties = new G4MaterialPropertiesTable();
  PENProperties->SetMaterialPropertiesTable(PENProperties);
  
  PENProperties->AddProperty("RINDEX", ph_ene1, Rindex, 200);
  PENProperties->AddProperty("WLSCOMPONENT", ph_ene2, WLSComponent, 200);
  PENProperties->AddProperty("WLSABSLENGTH", ph_ene1, AbsLength, 200);
  PENProperties->AddProperty("ABSLENGTH", ph_ene1, AbsLength, 200);
  PENProperties->AddProperty("RAYLEIGH", ph_ene1, Rayleigh, 200);
  PENProperties->AddConstProperty("WLSTIMECONSTANT", 20*CLHEP::ns);
  PENProperties->AddConstProperty("WLSMEANNUMBERPHOTONS", 1);
  */
 
  fSkinPENSurface = new G4LogicalSkinSurface("PENSkin", fModLV, OpPENSkin);
  G4cout << "The Logical volume is: " << fModLV->GetName() << G4endl;
  
 }

G4ThreeVector SLArPENFilm::GetPENCenter() {

  if (fGeoInfo->Contains("pos_x") &&
       fGeoInfo->Contains("pos_y") &&
       fGeoInfo->Contains("pos_z") ) {
    return G4ThreeVector(fGeoInfo->GetGeoPar("pos_x"),
        fGeoInfo->GetGeoPar("pos_y"),
        fGeoInfo->GetGeoPar("pos_z"));
  }

  return G4ThreeVector(0., 0., 0.);
}

void SLArPENFilm::SetVisAttributes()
{
  G4cout << "SLArPENFilm::SetVisAtrributes()" << G4endl;
  
  G4VisAttributes* visAttributes = new G4VisAttributes();
  
  visAttributes->SetVisibility(true);
  visAttributes->SetColor(0.534, 0.521, 0.185, 0.1);
  fModLV->SetVisAttributes( visAttributes );
  
  return;
}

void SLArPENFilm::Init(const rapidjson::Value& jconf) {
  assert(jconf.IsObject());
  auto jpenfilm = jconf.GetObject();
  
  assert(jpenfilm.HasMember("dimensions"));
  assert(jpenfilm.HasMember("position"));
  assert(jpenfilm.HasMember("copyID"));
  
  fID = jpenfilm["copyID"].GetInt();
  fGeoInfo->ReadFromJSON(jpenfilm["position"].GetObj(), "pos");
  fGeoInfo->ReadFromJSON(jpenfilm["dimensions"].GetArray());
  
  return;
}
 
