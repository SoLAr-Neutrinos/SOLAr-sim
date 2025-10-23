/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDebugManager.hh
 * @created     : Thursday Oct 23, 2025 15:28:57 CEST
 */

#ifndef SLAR_DEBUG_MANAGER_HH
#define SLAR_DEBUG_MANAGER_HH

#include <string>
#include <map>

class SLArDebugManager {
  public:
    enum Category {
      PRIMARY_EVENT,
      TRACKING,
      OPTICAL,
      DETECTOR,
      NUM_CATEGORIES
    };

    // Get singleton instance
    static SLArDebugManager& Instance();

    // Runtime enable/disable
    void Enable(Category cat);
    void Disable(Category cat);
    void EnableAll();
    void DisableAll();

    // Check if category is enabled (both compile-time and runtime)
    bool IsEnabled(Category cat) const;

    // Get category name for printing
    static const char* GetCategoryName(Category cat);

    // Load settings from environment variable or file
    void LoadFromEnvironment();
    void LoadFromFile(const std::string& filename);

    // Print current status
    void PrintStatus() const;

  private:
    SLArDebugManager();
    ~SLArDebugManager() = default;

    // Prevent copying
    SLArDebugManager(const SLArDebugManager&) = delete;
    SLArDebugManager& operator=(const SLArDebugManager&) = delete;

    bool runtime_enabled_[NUM_CATEGORIES];
};

// Convenience macros for debug messages
#ifdef SLAR_DEBUG

// Basic debug message
#define DEBUG_MSG(category, msg) \
  do { \
    if(SLArDebugManager::Instance().IsEnabled(SLArDebugManager::category)) { \
      G4cout << "[" << SLArDebugManager::GetCategoryName(SLArDebugManager::category) \
      << "] " << msg << G4endl; \
    } \
  } while(0)

// Debug message with verbosity level
#define DEBUG_MSG_V(category, level, msg) \
  do { \
    if(SLArDebugManager::Instance().IsEnabled(SLArDebugManager::category) && \
        G4RunManager::GetRunManager()->GetVerboseLevel() >= level) { \
      G4cout << "[" << SLArDebugManager::GetCategoryName(SLArDebugManager::category) \
      << "] " << msg << G4endl; \
    } \
  } while(0)

// Debug message with function name
#define DEBUG_MSG_FUNC(category, msg) \
  do { \
    if(SLArDebugManager::Instance().IsEnabled(SLArDebugManager::category)) { \
      G4cout << "[" << SLArDebugManager::GetCategoryName(SLArDebugManager::category) \
      << "::" << __func__ << "] " << msg << G4endl; \
    } \
  } while(0)

#else
// When SLAR_DEBUG is not defined, all macros compile to nothing
#define DEBUG_MSG(category, msg) do {} while(0)
#define DEBUG_MSG_V(category, level, msg) do {} while(0)
#define DEBUG_MSG_FUNC(category, msg) do {} while(0)
#endif

#endif // SLAR_DEBUG_MANAGER_HH
