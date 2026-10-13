// ============================================================================
// MyPrimaryGeneratorAction.hh
// Definition of the MyPrimaryGeneratorAction class
//
// This class defines the primary particle source of the simulation.
// It uses G4ParticleGun to shoot particles with user-defined
// properties (particle type, energy, direction, position, etc.).
//
// Responsibilities:
// - Configure the particle gun in the constructor.
// - Generate primaries at the start of each event.
// - Provide getters for particle type and energy (used by RunAction).
// ============================================================================

#ifndef MYPRIMARYGENERATORACTION_HH
#define MYPRIMARYGENERATORACTION_HH

// ============================================================================
// Geant4 Includes
// ============================================================================
#include "MyDetectorConstruction.hh"
#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4ParticleGun.hh>
#include <G4GenericMessenger.hh>
#include <globals.hh>
#include <vector>

// Forward Declarations
class G4Event;

// ============================================================================
// MyPrimaryGeneratorAction
// ============================================================================
class MyPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
	// Constructor / Destructor
	MyPrimaryGeneratorAction(const MyDetectorConstruction* detector);
	virtual ~MyPrimaryGeneratorAction();

	// ========================================================================
	// Main Method
	// ========================================================================
	// Called by Geant4 at the beginning of each event to generate
	// the primary vertex and particle(s).
	void GeneratePrimaries(G4Event* anEvent) override;

	// ========================================================================
	// Getter Methods (used by RunAction)
	// ========================================================================
	G4String GetParticleName() const;
	G4double GetParticleEnergy() const;

	// ========================================================================
	// Danh sach quet (dung khi muon tinh MSP cho NHIEU to hop hat+nang luong
	// trong CUNG 1 lan /run/beamOn, xem MyRunAction::EndOfRunAction)
	// ========================================================================
	const std::vector<G4String>& GetParticleScanList() const;
	const std::vector<G4double>& GetEnergyScanListMeV() const;

private:
	// Duoc goi tu macro thong qua G4GenericMessenger
	void SetParticleType(const G4String& name); // /MyGun/particleType proton|alpha|carbon
	void SetEnergy(G4double energy);             // /MyGun/energy <value> <unit>
	void SetParticleList(const G4String& csv);   // /MyGun/particleList proton,alpha,carbon
	void SetEnergyListKeV(const G4String& csv);  // /MyGun/energyListKeV 15,20,25,...

	// Particle gun to shoot primaries into the simulation
	G4ParticleGun* fParticleGun;
	const MyDetectorConstruction* fDetector; // Biến lưu trữ con trỏ

	G4GenericMessenger* fMessenger;
	G4String fParticleChoice; // "proton" | "alpha" | "carbon"
	G4double fEnergy;

	std::vector<G4String> fParticleScanList;   // danh sach hat can quet (co the rong)
	std::vector<G4double> fEnergyScanListMeV;  // danh sach nang luong can quet, MeV (co the rong)
};

#endif // MYPRIMARYGENERATORACTION_HH
