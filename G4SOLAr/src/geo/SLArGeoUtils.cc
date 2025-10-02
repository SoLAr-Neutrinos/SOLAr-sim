/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGeoUtils
 * @created     : Monday Feb 12, 2024 12:27:48 CET
 */

#include <cstdio>
#include <cstdlib>
#include <utility>
#include <unordered_set>
#include <regex>

#include "geo/SLArGeoUtils.hh"
#include "geo/detector/SLArPlaneParameterisation.hpp"
#include "G4PhysicalVolumeStore.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "geo/VolumeStruct.hh"


namespace geo {
  double get_bounding_volume_surface(const G4VSolid* solid) {
    static std::unordered_set<const G4VSolid*> flagged_solids;

    if (dynamic_cast<const G4Box*>(solid)) {
      const auto box = (G4Box*)solid;
      return box->GetSurfaceArea(); 
    }
    else {
      if (flagged_solids.find(solid) == flagged_solids.end()) {
        flagged_solids.insert(const_cast<G4VSolid*>(solid));
        printf("geo::get_bounding_volume_surface: WARNING "); 
        printf("get_bounding_volume_surface is only implemented for G4Box solids. "); 
        printf("Feel free to work on your solid's implementation and let me know!\n");
        printf("Using a box approximation.\n");
      }

      G4ThreeVector lo; 
      G4ThreeVector hi;
      G4ThreeVector dim; 
      solid->BoundingLimits(lo, hi);

      for (int i=0; i<3; i++) dim[i] = fabs(hi[i] - lo[i]); 

      G4double half_area = dim[0]*dim[1] + dim[0]*dim[2] + dim[1]*dim[2];

      return 2*half_area; 
    }
  }

  bool track_crosses_volume(const G4ThreeVector& vtx, const G4ThreeVector& momentum_dir, const G4String& pv_name) {
    G4PhysicalVolumeStore* pvs = G4PhysicalVolumeStore::GetInstance();

    const G4VPhysicalVolume* world = pvs->GetVolume("World");
    const G4VSolid* world_solid = world->GetLogicalVolume()->GetSolid();

    const G4VPhysicalVolume* pv = pvs->GetVolume(pv_name);
    if (pv == nullptr) {
      printf("geo::track_crosses_volume: WARNING: physical volume %s not found\n", pv_name.data());
      exit(EXIT_FAILURE);
    }
    const G4VSolid* solid = pv->GetLogicalVolume()->GetSolid();

    const G4double step_len = 1.0*CLHEP::cm;
    bool inside = false;
    HepGeom::Point3D<G4double> pos(vtx.x(), vtx.y(), vtx.z());
    HepGeom::Vector3D<G4double> dir(momentum_dir.x(), momentum_dir.y(), momentum_dir.z());
    auto pv_transform = GetTransformToGlobal(pv);
    auto pt_transform = pv_transform.inverse();

    while (world_solid->Inside(pos) == kInside && inside == false) {
      const HepGeom::Point3D<G4double> xpos = pt_transform * pos;
      if (solid->Inside(xpos) == kInside) {
        inside = true;
        break;
      }
      pos += step_len*dir;
    }
    return inside;
  }

  std::vector<G4Transform3D> get_volume_transforms(const G4String& target_pv_name,
                                                   const G4String& mother_pv_name)
  {
    std::vector<G4Transform3D> transforms;
    G4PhysicalVolumeStore* pvs = G4PhysicalVolumeStore::GetInstance();
    const auto mother_pv = pvs->GetVolume(mother_pv_name);
    const auto target_pv = pvs->GetVolume(target_pv_name);
    if (!mother_pv) {
      G4cout << "Error: Mother physical volume " << mother_pv_name << " not found." << G4endl;
      exit(EXIT_FAILURE);
    }
    if (!target_pv) {
      G4cout << "Error: Target physical volume " << target_pv_name << " not found." << G4endl;
      exit(EXIT_FAILURE);
    }

    const auto mother_lv = mother_pv->GetLogicalVolume();

    std::vector<volume_navigation_info> navigation;
    
    bool gotcha = search_in_logical_volume(target_pv_name, mother_lv, navigation);
    if (!gotcha) {
      G4cout << "Error: Target physical volume " << target_pv_name << 
        " not found in logical volume " 
        << mother_lv->GetName() << G4endl;
      exit(EXIT_FAILURE);
    }

    // now loop through the navigation info to build the transformations
    G4Transform3D mother_transform = GetTransformToGlobal(mother_pv);
    collect_volume_transforms(mother_lv, navigation, transforms, mother_transform); 

    printf("Found %ld replicas of %s\n", transforms.size(), target_pv_name.data()); 
    int ii = 0; 
    for (const auto& tt : transforms) {
      printf("[%i]: (%g, %g, %g)\n", ii, 
          tt.getTranslation().x()*0.1, 
          tt.getTranslation().y()*0.1, 
          tt.getTranslation().z()*0.1);
    }

    return transforms;
  }

  bool search_in_logical_volume(const G4String& pv_name,
      const G4LogicalVolume* logicalVolume,
      std::vector<volume_navigation_info>& navigation_info) {
    if (!logicalVolume) {
      G4cout << "Logical volume is null" << G4endl;
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < logicalVolume->GetNoDaughters(); ++i) {
      G4VPhysicalVolume* daughter = logicalVolume->GetDaughter(i);
      if (!daughter) {
        fprintf(stderr, "Error: %s's daughter volume %i is null.\n", 
            logicalVolume->GetName().data(), i);
        continue;
      }

      if (daughter->GetName() == pv_name) {
        //G4cout << "FOUND volume " << pv_name << " inside logical volume " << logicalVolume->GetName() << G4endl;
        navigation_info.emplace_back(i, daughter->IsParameterised(), daughter->IsReplicated());
        return true;
      }

      auto daughter_lv = daughter->GetLogicalVolume();
      navigation_info.emplace_back(i, daughter->IsParameterised(), daughter->IsReplicated());
      if (search_in_logical_volume(pv_name, daughter_lv, navigation_info)) {
        return true;
      }
      // If we reach here, it means we did not find the volume in this branch
      navigation_info.pop_back();
    }

    return false;
  }

  void collect_volume_transforms(
      const G4LogicalVolume* logicalVolume,
      const std::vector<volume_navigation_info>& navigation_info,
      std::vector<G4Transform3D>& transforms,
      const G4Transform3D& currentTransform,
      size_t currentIndex) 
  {

    if (currentIndex >= navigation_info.size()) {
      transforms.push_back(currentTransform);
      return;
    }

    const auto& volume_info = navigation_info.at(currentIndex);
    if (volume_info.index < 0 || 
        volume_info.index >= logicalVolume->GetNoDaughters()) {
      G4cout << "Error: Invalid index in navigation info." << G4endl;
      exit(EXIT_FAILURE);
    }

    G4VPhysicalVolume* daughter_pv = logicalVolume->GetDaughter(volume_info.index);
    if (!daughter_pv) {
      fprintf(stderr, "Error: %s daughter volume at index %d is null.\n", 
          logicalVolume->GetName().data(), volume_info.index);
      exit(EXIT_FAILURE);
    }
    G4LogicalVolume* daughter_lv = daughter_pv->GetLogicalVolume();

    if (volume_info.is_parametrised) {
      const G4PVParameterised* ppv = dynamic_cast<const G4PVParameterised*>(daughter_pv); 
      if (!ppv) {
        fprintf(stderr, "ERROR: Unable to cast %s in a G4PVParameterisedVolume\n", 
            daughter_pv->GetName().data()); 
        exit(EXIT_FAILURE); 
      }

      const SLArPlaneParameterisation* plane_prmt = 
        dynamic_cast<const SLArPlaneParameterisation*>(ppv->GetParameterisation());
      if (plane_prmt) {
        const auto prmt_data = get_plane_replication_data(ppv); 
        for (int i = 0; i< prmt_data.fNreplica; i++) {
          G4ThreeVector trans = plane_prmt->ComputeTranslation(i); 
          G4Transform3D newTransform = currentTransform * G4Transform3D(G4RotationMatrix(), trans);

          collect_volume_transforms(
              daughter_lv, navigation_info, transforms, newTransform, currentIndex+1);
        }
      }
      else {
        fprintf(stderr, "Error: collect_volume_transforms only treats parameterised volumes"); 
        fprintf(stderr, "using SLArPlaneParameterisation.\n"); 
        exit(EXIT_FAILURE); 
      }
    }
    else {
      G4RotationMatrix* rot = daughter_pv->GetRotation(); 
      G4ThreeVector trans = daughter_pv->GetTranslation(); 

      G4Transform3D newTransform = currentTransform * G4Transform3D(rot ? *rot : G4RotationMatrix(), trans);

      collect_volume_transforms(
          daughter_lv, navigation_info, transforms, newTransform, currentIndex+1); 
    }

    return;
  }

/*
 *  VolumeStruct* SearchInLogicalVolume (G4LogicalVolume* logicalVolume, const G4String& pv_name) {
 *    if (!logicalVolume) {
 *      G4cout << "Logical volume is null" << G4endl;
 *      return nullptr;
 *    }
 *    
 *    for (int i = 0; i < logicalVolume->GetNoDaughters(); ++i) {
 *      G4VPhysicalVolume* daughter = logicalVolume->GetDaughter(i);
 *      if (!daughter) {
 *          G4cout << "Error: Daughter volume is null." << G4endl;
 *          continue;
 *      }
 *      
 *      if (daughter->GetLogicalVolume()->GetName() == pv_name) {
 *          G4cout << "FOUND volume " << pv_name << " inside logical volume " << logicalVolume->GetName() << G4endl;
 *          return new VolumeStruct(daughter->GetLogicalVolume(), daughter);
 *      }
 *
 *      if (daughter->IsParameterised()) {
 *          //G4cout << "Exploring parametrised daughter volume: " << daughter->GetName() << G4endl;
 *          auto result = SearchInLogicalVolume(daughter->GetLogicalVolume(), pv_name);
 *          if (result != nullptr) {
 *              return result;
 *          }
 *      }
 *    
 *      else {
 *        //G4cout << "Exploring unparametrised daughter volume: " << daughter->GetName() << G4endl;
 *        auto result = SearchInLogicalVolume(daughter->GetLogicalVolume(), pv_name);
 *          if (result != nullptr) {
 *              return result;
 *          }
 *      }
 *    }
 *
 *    return nullptr;
 *  }
 *
 */
/*
 *  VolumeStruct* SearchLogicalVolumeInParametrisedVolume(const G4String& pv_name, const G4String& param_vol_name) {
 *    auto volumeStore = G4PhysicalVolumeStore::GetInstance();
 *    if (!volumeStore) {
 *        G4cout << "Error: G4PhysicalVolumeStore is not initialized." << G4endl;
 *        return nullptr;
 *    }
 *
 *
 *    for (auto vol : *volumeStore) {
 *      if (vol->GetName() == param_vol_name) {
 *          auto logicalVolume = vol->GetLogicalVolume();
 *          if (!logicalVolume) {
 *              G4cout << "Error: LogicalVolume is null for " << param_vol_name << G4endl;
 *              return nullptr;
 *          }
 *
 *          return SearchInLogicalVolume(logicalVolume, pv_name);
 *      }
 *    }
 *
 *    G4cout << "Parametrised volume " << param_vol_name << " not found in G4PhysicalVolumeStore." << G4endl;
 *    return nullptr;
 *  }
 */
}
