/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDebugManager.cc
 * @created     : Thursday Oct 23, 2025 16:40:00 CEST
 */

#include "SLArDebugManager.hh"
#include "G4ios.hh"
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>

SLArDebugManager& SLArDebugManager::Instance() {
  static SLArDebugManager instance;
  return instance;
}

SLArDebugManager::SLArDebugManager() {
  // Initialize all categories as disabled by default
  for(int i = 0; i < NUM_CATEGORIES; ++i) {
    runtime_enabled_[i] = false;
  }
}

void SLArDebugManager::Enable(Category cat) {
  if(cat < NUM_CATEGORIES) {
    runtime_enabled_[cat] = true;
  }
}

void SLArDebugManager::Disable(Category cat) {
  if(cat < NUM_CATEGORIES) {
    runtime_enabled_[cat] = false;
  }
}

void SLArDebugManager::EnableAll() {
  for(int i = 0; i < NUM_CATEGORIES; ++i) {
    runtime_enabled_[i] = true;
  }
}

void SLArDebugManager::DisableAll() {
  for(int i = 0; i < NUM_CATEGORIES; ++i) {
    runtime_enabled_[i] = false;
  }
}

bool SLArDebugManager::IsEnabled(Category cat) const {
#ifdef SLAR_DEBUG
  if(cat >= NUM_CATEGORIES) return false;

  // Check both compile-time availability and runtime flag
  switch(cat) {
    case PRIMARY_EVENT:
#ifdef DEBUG_PRIMARY_EVENT_AVAILABLE
      return runtime_enabled_[cat];
#else
      return false;
#endif

    case TRACKING:
#ifdef DEBUG_TRACKING_AVAILABLE
      return runtime_enabled_[cat];
#else
      return false;
#endif

    case OPTICAL:
#ifdef DEBUG_OPTICAL_AVAILABLE
      return runtime_enabled_[cat];
#else
      return false;
#endif

    case DETECTOR:
#ifdef DEBUG_DETECTOR_AVAILABLE
      return runtime_enabled_[cat];
#else
      return false;
#endif

    default:
      return false;
  }
#else
  return false;
#endif
}

const char* SLArDebugManager::GetCategoryName(Category cat) {
  switch(cat) {
    case PRIMARY_EVENT:   return "PRIMARY_EVENT";
    case TRACKING:        return "TRACKING";
    case OPTICAL:         return "OPTICAL";
    case DETECTOR:        return "DETECTOR";
    default:              return "UNKNOWN";
  }
}

void SLArDebugManager::LoadFromEnvironment() {
#ifdef SLAR_DEBUG
  const char* env = std::getenv("SLAR_DEBUG_CATEGORIES");
  if(!env) return;

  std::string str(env);
  std::istringstream iss(str);
  std::string token;

  // Parse comma-separated list: "SCINTILLATION,OPTICAL"
  while(std::getline(iss, token, ',')) {
    // Trim whitespace
    size_t start = token.find_first_not_of(" \t");
    size_t end = token.find_last_not_of(" \t");
    if(start != std::string::npos) {
      token = token.substr(start, end - start + 1);
    }

    if(token == "ALL") {
      EnableAll();
    } else if(token == "PRIMARY_EVENT") {
      Enable(PRIMARY_EVENT);
    } else if(token == "TRACKING") {
      Enable(TRACKING);
    } else if(token == "OPTICAL") {
      Enable(OPTICAL);
    } else if(token == "DETECTOR") {
      Enable(DETECTOR);
    }
  }
#endif
}

void SLArDebugManager::LoadFromFile(const std::string& filename) {
#ifdef SLAR_DEBUG
  std::ifstream file(filename);
  if(!file.is_open()) {
    G4cout << "Warning: Could not open debug config file: " << filename << G4endl;
    return;
  }

  std::string line;
  while(std::getline(file, line)) {
    // Skip empty lines and comments
    if(line.empty() || line[0] == '#') continue;

    // Simple format: "category on|off"
    std::istringstream iss(line);
    std::string cat_name, state;
    iss >> cat_name >> state;

    bool enable = (state == "on" || state == "1" || state == "true");

    if(cat_name == "PRIMARY_EVENT") {
      (enable) ? Enable(PRIMARY_EVENT) : Disable(PRIMARY_EVENT);
    } else if(cat_name == "TRACKING") {
      (enable) ? Enable(TRACKING) : Disable(TRACKING);
    } else if(cat_name == "OPTICAL") {
      (enable) ? Enable(OPTICAL) : Disable(OPTICAL);
    } else if(cat_name == "DETECTOR") {
      (enable) ? Enable(DETECTOR) : Disable(DETECTOR);
    }
  }
#endif
}

void SLArDebugManager::PrintStatus() const {
#ifdef SLAR_DEBUG
  G4cout << "\n=== Debug Manager Status ===" << G4endl;
  G4cout << "Debug infrastructure: ENABLED" << G4endl;

  for(int i = 0; i < NUM_CATEGORIES; ++i) {
    Category cat = static_cast<Category>(i);
    G4cout << "  " << GetCategoryName(cat) << ": ";

    if(IsEnabled(cat)) {
      G4cout << "ENABLED (runtime)" << G4endl;
    } else if(runtime_enabled_[i]) {
      G4cout << "DISABLED (not compiled in)" << G4endl;
    } else {
      G4cout << "DISABLED (runtime)" << G4endl;
    }
  }
  G4cout << "==========================\n" << G4endl;
#else
  G4cout << "Debug infrastructure: DISABLED (compile with -DENABLE_DEBUG=ON)" << G4endl;
#endif
}
