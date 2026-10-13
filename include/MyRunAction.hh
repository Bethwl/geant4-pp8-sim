// ============================================================================
// MyRunAction.hh
// Accumulates:
//   - NaI Edep + pulse counts (existing — for MAC)
//   - Process counts (compt/phot/conv/Rayl/other) for first interaction → R
//   - Ion energy loss + path length → Mass Stopping Power
// ============================================================================

#ifndef MYRUNACTION_HH
#define MYRUNACTION_HH

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "globals.hh"

#include "MyPrimaryGeneratorAction.hh"

class G4Run;

class MyRunAction : public G4UserRunAction {
public:
    MyRunAction();
    ~MyRunAction() override;

    void BeginOfRunAction(const G4Run* run) override;
    void EndOfRunAction(const G4Run* run) override;

    // === NaI detector accumulation (for MAC measurement) ===
    void AddEdepNaIDet(G4double edep);
    void AddCountedNaIDet();

    // === First interaction counters (for R = N_compton/N_total) ===
    void AddProcessCount(const G4String& processName);

    // === Ion energy loss tracking (for Mass Stopping Power) ===
    void AddIonEnergyLoss(G4double dE, G4double dx) {
        fIonEnergyLoss += dE;
        fIonPathLength += dx;
    }

private:
    // NaI accumulables (existing)
    G4Accumulable<G4double> fEdepNaIDet;
    G4Accumulable<G4double> fPulseNaIDet;

    // Process counters (NEW)
    G4Accumulable<G4double> fNCompton;
    G4Accumulable<G4double> fNPhoto;
    G4Accumulable<G4double> fNPair;
    G4Accumulable<G4double> fNRayleigh;
    G4Accumulable<G4double> fNOther;

    // Ion energy loss (NEW — for MSP)
    G4Accumulable<G4double> fIonEnergyLoss;
    G4Accumulable<G4double> fIonPathLength;

    MyPrimaryGeneratorAction* fParticleGun = nullptr;
};

#endif // MYRUNACTION_HH
