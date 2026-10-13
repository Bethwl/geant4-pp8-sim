// ============================================================================
// MyDetectorConstruction.hh
// Definition of the MyDetectorConstruction class
//
// This class is responsible for building the entire detector geometry,
// defining materials, and assigning sensitive detectors to logical volumes.
// It is registered with the G4RunManager in main.cc.
// ============================================================================

#ifndef MyDetectorConstruction_h
#define MyDetectorConstruction_h

// ============================================================================
// Geant4 Includes
// ============================================================================
#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"

// ============================================================================
// MyDetectorConstruction
// ============================================================================
// Inherits from G4VUserDetectorConstruction.
//
// Responsibilities:
// - DefineMaterials(): load and register materials using G4NistManager.
// - Construct(): build geometry volumes and place them in the world.
// - ConstructSDandField(): attach sensitive detectors to logical volumes
//   (thread-safe).
// ============================================================================
class MyDetectorConstruction : public G4VUserDetectorConstruction {
public:
	// ========================================================================
	// Constructor / Destructor
	// ========================================================================
	MyDetectorConstruction();
	~MyDetectorConstruction() override;

	// ========================================================================
	// Construct(): Builds the geometry and returns the world physical volume
	// ========================================================================
	G4VPhysicalVolume* Construct() override;

	// ========================================================================
	// DefineMaterials(): Defines all materials used in the geometry
	// ========================================================================
	void DefineMaterials();

	// ========================================================================
	// ConstructSDandField(): Assigns sensitive detectors to logical volumes
	// ========================================================================
	void ConstructSDandField() override;

	G4double GetSampleDetectorDistance() const { return fSample_Detector_Distance; }
	G4String GetSampleMaterialName() const { return fSampleMaterialName; }
	G4double GetSampleThickness() const { return fSampleThickness; }

	// ========================================================================
	// Duoc goi tu macro thong qua G4GenericMessenger (can la PUBLIC de
	// G4GenericMessenger::DeclareMethod truy cap duoc)
	// ========================================================================
	void UpdateGeometry();
	void SetSampleMaterial(const G4String& name); // /DetectorConstruction/SampleMaterial S1..S9
	void SetSampleThickness(G4double t);           // /DetectorConstruction/SampleThickness X um

private:
	G4double fSample_Detector_Distance;

	G4VPhysicalVolume* World_Phys = nullptr;

	G4GenericMessenger* fMessenger;

	G4String fSampleMaterialName; // "S1".."S9", mac dinh "S5" (Cr2AlC)

	G4double fSampleThickness; // be day Sample, mac dinh 1 um (dieu chinh qua macro)
};

#endif // MyDetectorConstruction_h
