#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "TMath.h"

struct PDEPoint_t {
    double energy = {};
    double pde = {};
    bool operator<(const PDEPoint_t& other) const { return energy < other.energy; }
};

int get_vuv_sipm_pde(const char* filename = "vuv_sipm_pde.txt") {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    std::vector<PDEPoint_t> pde_data;
    double wavelength, pde;

    std::string line;
    std::getline(infile, line);
    std::getline(infile, line);
    std::getline(infile, line);

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        if (!(iss >> wavelength >> pde)) { break; }
        double energy = TMath::HC() / (wavelength*1e-9) / TMath::Qe();
        pde_data.push_back({energy, pde / 100.0});
    }

    std::sort(pde_data.begin(), pde_data.end());
    for (const auto& point : pde_data) {
        std::cout << point.energy << ", ";
    }
    std::cout << std::endl;

    for (const auto& point : pde_data) {
        std::cout << point.pde << ", ";
    }

    return 0;
}


