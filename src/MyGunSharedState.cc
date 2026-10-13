#include "MyGunSharedState.hh"
#include "G4SystemOfUnits.hh"

namespace {
    // Bien static file-scope: dung chung cho MOI thread trong cung 1 process.
    // Khoi tao gia tri mac dinh khop voi MyPrimaryGeneratorAction ban dau
    // (carbon, 662 keV, danh sach quet rong).
    G4String gParticleName = "carbon";
    G4double gParticleEnergy = 662.0 * keV;
    std::vector<G4String> gParticleScanList;
    std::vector<G4double> gEnergyScanListMeV;
}

namespace MyGunShared {

    void SetParticleName(const G4String& name) {
        gParticleName = name;
    }

    void SetParticleEnergy(G4double energy) {
        gParticleEnergy = energy;
    }

    void SetParticleScanList(const std::vector<G4String>& list) {
        gParticleScanList = list;
    }

    void SetEnergyScanListMeV(const std::vector<G4double>& list) {
        gEnergyScanListMeV = list;
    }

    G4String GetParticleName() {
        return gParticleName;
    }

    G4double GetParticleEnergy() {
        return gParticleEnergy;
    }

    const std::vector<G4String>& GetParticleScanList() {
        return gParticleScanList;
    }

    const std::vector<G4double>& GetEnergyScanListMeV() {
        return gEnergyScanListMeV;
    }

}
