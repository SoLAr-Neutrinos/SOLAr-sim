/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArGeoUtils.hpp
 * @created     Wed Apr 19, 2023 08:08:00 CEST
 */

#ifndef SLARGEOUTILS_HH

#define SLARGEOUTILS_HH

#include <map>
#include <regex>

#include "SLArRunAction.hh"

#include "G4ThreeVector.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Transform3D.hh"
#include "G4LogicalVolume.hh"
#include "G4RunManager.hh"
//#include "geo/VolumeStruct.hh"

#include "rapidjson/document.h"
#include "rapidjson/allocators.h"

class G4VSolid;

namespace geo {
  enum EBoxFace {kXplus=0, kXminus=1, kYplus=2, kYminus=3, kZplus=4, kZminus=5}; 
  static std::map<EBoxFace, G4ThreeVector> BoxFaceNormal  = {
    {kXplus, G4ThreeVector(-1, 0, 0)},
    {kXminus, G4ThreeVector(+1, 0, 0)},
    {kYplus, G4ThreeVector(0, -1, 0)},
    {kYminus, G4ThreeVector(0, +1, 0)},
    {kZplus, G4ThreeVector(0, 0, -1)},
    {kZminus, G4ThreeVector(0, 0, +1)}
  };

  double get_bounding_volume_surface(const G4VSolid* solid);

  inline std::vector<G4VPhysicalVolume*> GetPhysicalVolumes(const G4LogicalVolume* lv) 
  {
    G4PhysicalVolumeStore* pvs = G4PhysicalVolumeStore::GetInstance();
    std::vector<G4VPhysicalVolume*> pv_list;
    for (auto pv : *pvs) {
      if (pv->GetLogicalVolume() == lv) {
        pv_list.push_back(pv);
      }
    }

    return pv_list;
  }

  bool track_crosses_volume(const G4ThreeVector& vtx, const G4ThreeVector& dir, const G4String& pv_name);

  //VolumeStruct* SearchInLogicalVolume(G4LogicalVolume* logicalVolume, const G4String& pv_name);

  //VolumeStruct* SearchLogicalVolumeInParametrisedVolume(const G4String& pv_name, const G4String& param_vol_name);

  struct volume_navigation_info {
    int index; //!< Index of the volume in the navigation stack
    bool is_parametrised; //!< Is the volume parametrised?
    bool is_replicated; //!< Is the volume replicated?
    volume_navigation_info(int idx, bool param, bool rep)
        : index(idx), is_parametrised(param), is_replicated(rep) {}
  }; 

  std::vector<G4Transform3D> get_volume_transforms(
      const G4String& target_pv_name,
      const G4String& mother_pv_name);

  void collect_volume_transforms(
      const G4LogicalVolume* logicalVolume,
      const std::vector<volume_navigation_info>& navigation_info,
      std::vector<G4Transform3D>& transforms,
      const G4Transform3D& currentTransform, 
      size_t currentIndex = 0
      );

  bool search_in_logical_volume(
      const G4String& pv_name,
      const G4LogicalVolume* logicalVolume,
      std::vector<volume_navigation_info>& navigation_info);

  inline static const G4Transform3D GetTransformToGlobal(const G4VPhysicalVolume* pv) {
    G4Transform3D globalTransform = G4Transform3D::Identity;
    while (pv->GetMotherLogical() != nullptr) {
      G4Transform3D localTransform(pv->GetObjectRotationValue(), pv->GetObjectTranslation());
      globalTransform = localTransform * globalTransform;
      auto mother = pv->GetMotherLogical();
      const auto pv_list = GetPhysicalVolumes(mother);
      if (pv_list.size() > 1) {
        printf("GetTransformToGlobal() WARNING: more than one physical volume with the same logical volume\n");
      }
      pv = pv_list[0];
    }
    return globalTransform;
  }
  
  template <typename T>
  inline static const G4ThreeVector transform_frame_world_to_det(
      const HepGeom::Point3D<T>& world_pos) 
  {
    const SLArRunAction* run_action = 
      static_cast<const SLArRunAction*>(
          G4RunManager::GetRunManager()->GetUserRunAction()
          );
    const G4Transform3D& transform = run_action->GetTransformWorld2Det();
    const HepGeom::Point3D<double> pp =  transform * world_pos;
    return G4ThreeVector(pp.x(), pp.y(), pp.z());
  }

  inline static const G4ThreeVector transform_frame_world_to_det(
      const G4ThreeVector& world_pos) 
  {
    const HepGeom::Point3D<double> pos( world_pos );
    return transform_frame_world_to_det(pos);
  }


}



#endif /* end of include guard SLARGEOUTILS_HH */

