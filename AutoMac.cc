#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

int main() {
    // ------------------------------------------------------------------------
    // Parameters
    // ------------------------------------------------------------------------
    double step_distance = 10.0; // mm
    double min_distance = 300.0;   // mm
    double max_distance = 300.0; // mm

    // ------------------------------------------------------------------------
    // Energy list: 2.2 MeV → 10 MeV (2200 keV → 10000 keV), step = 100 keV
    // ------------------------------------------------------------------------
    std::vector<int> energies;
    for (int e = 10; e < 100; e += 10) {
        energies.push_back(e);
    }

    // 100,200,...,10000
    for (int e = 100; e < 1000; e += 100) {
        energies.push_back(e);
    }

    for (int e = 1000; e <= 15000; e += 1000) {
        energies.push_back(e);
    }

    // ------------------------------------------------------------------------
    // Create master macro file
    // ------------------------------------------------------------------------
    std::ofstream master(std::string(MACROPATH) + std::string("/run_all.mac"));
    if (!master) {
        std::cerr << "=== Failed to create master macro file ===" << std::endl;
        return 1;
    }

    master << "/control/verbose 2\n";

    // ------------------------------------------------------------------------
    // Loop over distances and energies
    // ------------------------------------------------------------------------
    for (double distance = min_distance; distance <= max_distance + 0.001; distance += step_distance) {
        for (int energy : {10, 11, 12, 13, 14, 15, 16, 17, 18, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 80, 90,
            100, 110, 120, 130, 140, 150, 160, 170, 180, 200, 225, 250, 275, 300, 325, 350, 375, 400, 450, 500,
            550, 600, 650, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 2000, 2250, 2500,
            2750, 3000, 3250, 3500, 3750, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 8000, 9000, 10000, 11000,
            12000, 13000, 14000, 15000, 16000, 17000, 18000, 20000, 22500, 25000, 27500, 30000, 32500, 35000,
            37500, 40000, 45000, 50000, 55000, 60000, 65000, 70000, 80000, 90000, 100000, 110000, 120000,
            130000, 140000, 150000, 160000, 170000, 180000, 200000, 225000, 250000, 275000, 300000, 325000,
            350000, 375000, 400000, 450000, 500000, 550000, 600000, 650000, 700000, 800000, 900000, 1000000}) {

            // Format strings for filename
            std::ostringstream ossD, ossE;
            ossD << std::fixed << std::setprecision(0) << distance;
            ossE << energy;

            std::string D_str = ossD.str();
            std::string E_str = ossE.str();

            // Filename for individual macro
            std::string filename = "run_D" + D_str + "_E" + E_str + ".mac";

            // ----------------------------------------------------------------
            // Write macro file
            // ----------------------------------------------------------------
            std::ofstream macfile(std::string(MACROPATH) + "/" + filename);
            if (!macfile) {
                std::cerr << "=== File Generating Failed === " << filename << std::endl;
                continue;
            }

            // Set source-detector distance and position
            //macfile << "/DetectorConstruction/Sample_Detector_Distance " << distance << "\n";
            //macfile << "/DetectorConstruction/update\n";
            macfile << "/gun/energy " << energy << " keV\n";

            // -------------------- Histogram setup --------------------
            //macfile << "/analysis/setFileName hist_D" << D_str << "_E" << E_str << "\n";

            // Run events
            macfile << "/run/beamOn 1000000\n";

            macfile.close();

            // Add macro to master macro
            master << "/control/execute " << filename << "\n";
            std::cout << "== Generated: " << filename << std::endl;
        }
    }

    // ------------------------------------------------------------------------
    // Finalize master macro
    // ------------------------------------------------------------------------
    master.close();
    std::cout << "\nAll macros generated successfully at: " << MACROPATH << std::endl;
    std::cout << "Master macro file: run_all.mac" << std::endl;
    std::cout << "Total combinations: "
        << ((int)((max_distance - min_distance) / step_distance + 1) * energies.size())
        << std::endl;

    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}
