#include "MyDetectorConstruction.hh"
#include "MySensitiveDetector.hh"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Region.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4RegionStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4RunManager.hh"

#include <set>

namespace GEO {
    // World Volume
    const G4double WorldSize = 3 * m;

    // Source
    const G4double Souce_Radius = 5 * mm;
    const G4double Source_Height = 5 * mm;

    // Sample
    // LUU Y: be day Sample (Sample_Z cu) KHONG con la hang so nua - da chuyen
    // thanh bien member fSampleThickness cua MyDetectorConstruction, dieu
    // chinh duoc qua macro: /DetectorConstruction/SampleThickness X um
    const G4double Sample_X = 2 * mm;
    const G4double Sample_Y = 2 * mm;

    // NaI Detector
    const G4double Solid_NaI_Radius = 25 * mm;
    const G4double Solid_NaI_Height = 100 * mm;

    // Lead Container
    const G4double Lead_Thickness = 5 * mm;
    const G4double Solid_Lead_Radius = Solid_NaI_Radius + Lead_Thickness;
    const G4double Solid_Lead_Height = Solid_NaI_Height + 2 * Lead_Thickness;
    const G4double Hollow_Lead_Radius = Solid_NaI_Radius;
    const G4double Hollow_Lead_Height = Solid_NaI_Height + Lead_Thickness;
}

MyDetectorConstruction::MyDetectorConstruction()
    : G4VUserDetectorConstruction(),
    fMessenger(nullptr),
    fSample_Detector_Distance(300.0 * mm),
    fSampleMaterialName("S5"),   // mac dinh: S5 = Cr2AlC (giu nhu cu)
    fSampleThickness(1.0 * um)   // mac dinh: 1 um (giu bang GEO::Sample_Z cu)
{
    fMessenger = new G4GenericMessenger(this, "/DetectorConstruction/", "Detector Construction Control");
    auto& fSample_Detector_Distancecmd = fMessenger->DeclareProperty("Sample_Detector_Distance", fSample_Detector_Distance, "Set Sample_Detector_Distance");
    fMessenger->DeclareMethod("update", &MyDetectorConstruction::UpdateGeometry, "Update geometry after parameter changes");

    fMessenger->DeclareMethod("SampleMaterial", &MyDetectorConstruction::SetSampleMaterial,
        "Chon vat lieu Sample: S1..S9 (S1=Water, S2..S9=MAX-phase). Vi du: /DetectorConstruction/SampleMaterial S5");

    // Lenh macro moi: doi be day Sample (chi rebuild hinh hoc khi goi lenh nay,
    // KHONG bi anh huong boi /MyGun/energy hay /MyGun/particleType, nen quet
    // nhieu nang luong tren cung 1 be day KHONG can rebuild hinh hoc moi lan).
    fMessenger->DeclareMethodWithUnit("SampleThickness", "um", &MyDetectorConstruction::SetSampleThickness,
        "Doi be day Sample (rebuild hinh hoc). Vi du: /DetectorConstruction/SampleThickness 10 um");
}
MyDetectorConstruction::~MyDetectorConstruction() {}

void MyDetectorConstruction::SetSampleMaterial(const G4String& name) {
    static const std::set<G4String> valid = { "S1","S2","S3","S4","S5","S6","S7","S8","S9" };

    if (valid.find(name) == valid.end()) {
        G4cout << "[DetectorConstruction] WARNING: '" << name
            << "' khong hop le. Cho phep: S1..S9. Giu nguyen '"
            << fSampleMaterialName << "'.\n";
        return;
    }

    fSampleMaterialName = name;
    G4cout << "[DetectorConstruction] Sample material set to: " << name << G4endl;

    // Tu dong xay lai hinh hoc de material moi co hieu luc ngay,
    // khong can goi rieng lenh /DetectorConstruction/update
    UpdateGeometry();
}

void MyDetectorConstruction::SetSampleThickness(G4double t) {
    if (t <= 0.0) {
        G4cout << "[DetectorConstruction] WARNING: be day phai > 0. Giu nguyen "
            << fSampleThickness / um << " um.\n";
        return;
    }

    fSampleThickness = t;
    G4cout << "[DetectorConstruction] Sample thickness set to: " << t / um << " um" << G4endl;

    // Rebuild hinh hoc ngay - lenh nay CHI can goi khi doi be day,
    // KHONG can goi lai moi lan doi nang luong (/MyGun/energy khong
    // dung den ham nay), nen quet nhieu nang luong tren cung 1 be day
    // se KHONG rebuild hinh hoc lai giua cac lan beamOn.
    UpdateGeometry();
}

void MyDetectorConstruction::DefineMaterials() {
    G4NistManager* NIST = G4NistManager::Instance();

    NIST->FindOrBuildMaterial("G4_AIR");
    NIST->FindOrBuildMaterial("G4_Galactic");
    NIST->FindOrBuildMaterial("G4_SODIUM_IODIDE");
    NIST->FindOrBuildMaterial("G4_Pb");

    // Vat lieu MAX-phase, dat ten ngan gon S1..S9, thanh phan theo % KHOI LUONG
    // (mass fraction). S1 = Water (tham chieu/chuan), S2..S9 = cac MAX-phase.

    // [1]. S1: Water (H2O)
    if (!G4Material::GetMaterial("S1", false)) {
        G4Material* S1 = new G4Material("S1", 1.0 * g / cm3, 2);
        S1->AddElement(NIST->FindOrBuildElement("H"), 0.111898);
        S1->AddElement(NIST->FindOrBuildElement("O"), 0.888102);
    }
    // [2]. S2: Ti2AlC
    if (!G4Material::GetMaterial("S2", false)) {
        G4Material* S2 = new G4Material("S2", 4.1 * g / cm3, 3);
        S2->AddElement(NIST->FindOrBuildElement("Ti"), 0.7106);
        S2->AddElement(NIST->FindOrBuildElement("Al"), 0.2003);
        S2->AddElement(NIST->FindOrBuildElement("C"), 0.0892);
    }
    // [3]. S3: Ti2AlN
    if (!G4Material::GetMaterial("S3", false)) {
        G4Material* S3 = new G4Material("S3", 4.3 * g / cm3, 3);
        S3->AddElement(NIST->FindOrBuildElement("Ti"), 0.7002);
        S3->AddElement(NIST->FindOrBuildElement("Al"), 0.1974);
        S3->AddElement(NIST->FindOrBuildElement("N"), 0.1025);
    }
    // [4]. S4: V2AlC
    if (!G4Material::GetMaterial("S4", false)) {
        G4Material* S4 = new G4Material("S4", 4.0 * g / cm3, 3);
        S4->AddElement(NIST->FindOrBuildElement("V"), 0.7232);
        S4->AddElement(NIST->FindOrBuildElement("Al"), 0.1915);
        S4->AddElement(NIST->FindOrBuildElement("C"), 0.0853);
    }
    // [5]. S5: Cr2AlC
    if (!G4Material::GetMaterial("S5", false)) {
        G4Material* S5 = new G4Material("S5", 5.17 * g / cm3, 3);
        S5->AddElement(NIST->FindOrBuildElement("Cr"), 0.7273);
        S5->AddElement(NIST->FindOrBuildElement("Al"), 0.1887);
        S5->AddElement(NIST->FindOrBuildElement("C"), 0.0840);
    }
    // [6]. S6: Nb2AlC
    if (!G4Material::GetMaterial("S6", false)) {
        G4Material* S6 = new G4Material("S6", 6.44 * g / cm3, 3);
        S6->AddElement(NIST->FindOrBuildElement("Nb"), 0.8265);
        S6->AddElement(NIST->FindOrBuildElement("Al"), 0.1200);
        S6->AddElement(NIST->FindOrBuildElement("C"), 0.0534);
    }
    // [7]. S7: Ta2AlC
    if (!G4Material::GetMaterial("S7", false)) {
        G4Material* S7 = new G4Material("S7", 11.46 * g / cm3, 3);
        S7->AddElement(NIST->FindOrBuildElement("Ta"), 0.9027);
        S7->AddElement(NIST->FindOrBuildElement("Al"), 0.0673);
        S7->AddElement(NIST->FindOrBuildElement("C"), 0.0300);
    }
    // [8]. S8: Ti3SiC2
    if (!G4Material::GetMaterial("S8", false)) {
        G4Material* S8 = new G4Material("S8", 4.5 * g / cm3, 3);
        S8->AddElement(NIST->FindOrBuildElement("Ti"), 0.7338);
        S8->AddElement(NIST->FindOrBuildElement("Si"), 0.1435);
        S8->AddElement(NIST->FindOrBuildElement("C"), 0.1228);
    }
    // [9]. S9: Ti3AlC2
    if (!G4Material::GetMaterial("S9", false)) {
        G4Material* S9 = new G4Material("S9", 4.21 * g / cm3, 3);
        S9->AddElement(NIST->FindOrBuildElement("Ti"), 0.7379);
        S9->AddElement(NIST->FindOrBuildElement("Al"), 0.1387);
        S9->AddElement(NIST->FindOrBuildElement("C"), 0.1235);
    }
}

G4VPhysicalVolume* MyDetectorConstruction::Construct() {
    DefineMaterials();
    G4NistManager* NIST = G4NistManager::Instance();
    G4double FITNESS = 0.1 * mm;

    // 1. World Volume
    G4Box* solidWorld = new G4Box("World", GEO::WorldSize / 2, GEO::WorldSize / 2, GEO::WorldSize / 2);
    G4LogicalVolume* World_LV = new G4LogicalVolume(solidWorld, NIST->FindOrBuildMaterial("G4_Galactic"), "WorldLV");
    World_Phys = new G4PVPlacement(nullptr, {}, World_LV, "World", nullptr, false, 0, true);

    // 2. Source
    G4Tubs* solidSource = new G4Tubs("Source", 0, GEO::Souce_Radius, GEO::Source_Height / 2, 0, 360 * deg);
    G4LogicalVolume* Source_LV = new G4LogicalVolume(solidSource, NIST->FindOrBuildMaterial("G4_Galactic"), "Source_LV");
    G4double Source_PosZ = 0;
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, Source_PosZ), Source_LV, "Source", World_LV, false, 0, true);

    // 3. Sample (vat lieu chon qua macro: /DetectorConstruction/SampleMaterial S1..S9,
    //    be day chon qua macro: /DetectorConstruction/SampleThickness X um)
    G4Box* solidSample = new G4Box("Sample", GEO::Sample_X / 2, GEO::Sample_Y / 2, fSampleThickness / 2);
    G4Material* sampleMaterial = G4Material::GetMaterial(fSampleMaterialName);
    if (!sampleMaterial) {
        G4cout << "[DetectorConstruction] WARNING: khong tim thay vat lieu '"
            << fSampleMaterialName << "', dung mac dinh S5.\n";
        sampleMaterial = G4Material::GetMaterial("S5");
    }
    G4LogicalVolume* Sample_LV = new G4LogicalVolume(solidSample, sampleMaterial, "Sample_LV");
    G4double Sample_PosZ = Source_PosZ - fSampleThickness / 2 - GEO::Source_Height / 2 - 5 * cm;
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, Sample_PosZ), Sample_LV, "Sample", World_LV, false, 0, true);

    // 4. NaI Crystal
    G4Tubs* solidNaI = new G4Tubs("NaI", 0, GEO::Solid_NaI_Radius, GEO::Solid_NaI_Height / 2, 0, 360 * deg);
    G4LogicalVolume* NaI_LV = new G4LogicalVolume(solidNaI, NIST->FindOrBuildMaterial("G4_SODIUM_IODIDE"), "NaI_LV");
    G4double NaI_PosZ = Sample_PosZ - fSampleThickness / 2 - fSample_Detector_Distance - GEO::Solid_NaI_Height / 2 - GEO::Lead_Thickness;
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, NaI_PosZ), NaI_LV, "NaI", World_LV, false, 0, true);

    // 5. Lead Container (Boolean Solid)
    G4Tubs* solidLead = new G4Tubs("SolidLead", 0, GEO::Solid_Lead_Radius, GEO::Solid_Lead_Height / 2, 0, 360 * deg);
    G4Tubs* hollowLead = new G4Tubs("HollowLead", 0, GEO::Hollow_Lead_Radius, GEO::Hollow_Lead_Height / 2 + FITNESS / 2, 0, 360 * deg);
    G4SubtractionSolid* leadContainer = new G4SubtractionSolid("LeadContainer", solidLead, hollowLead, nullptr, G4ThreeVector(0, 0, +GEO::Lead_Thickness / 2 + FITNESS / 2));
    G4LogicalVolume* Lead_LV = new G4LogicalVolume(leadContainer, NIST->FindOrBuildMaterial("G4_Pb"), "Lead_LV");
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, NaI_PosZ), Lead_LV, "LeadContainer", World_LV, false, 0, true);

    // VISUALIZATION
    G4VisAttributes* World_VisAtt = new G4VisAttributes(G4Colour(1, 1, 1, 0.0));
    World_VisAtt->SetVisibility(false);
    World_LV->SetVisAttributes(World_VisAtt);

    G4VisAttributes* Source_VisAtt = new G4VisAttributes(G4Colour(1, 0, 0));
    Source_VisAtt->SetForceSolid(true);
    Source_LV->SetVisAttributes(Source_VisAtt);

    G4VisAttributes* Sample_VisAtt = new G4VisAttributes(G4Colour(0.8, 0.5, 0.2));
    Sample_VisAtt->SetForceSolid(true);
    Sample_LV->SetVisAttributes(Sample_VisAtt);

    G4VisAttributes* NaI_VisAtt = new G4VisAttributes(G4Colour(0.5, 0, 0.5));
    NaI_VisAtt->SetForceSolid(true);
    NaI_LV->SetVisAttributes(NaI_VisAtt);

    G4VisAttributes* Lead_VisAtt = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5));
    Lead_VisAtt->SetForceSolid(true);
    Lead_LV->SetVisAttributes(Lead_VisAtt);

    return World_Phys;
}

void MyDetectorConstruction::UpdateGeometry() {
    auto runManager = G4RunManager::GetRunManager();
    runManager->ReinitializeGeometry();
    ConstructSDandField();
    G4cout << "\n================ Updated Geometry ================\n";
    G4cout << " Sample-Detector Distance: " << fSample_Detector_Distance / mm << " mm\n";
}

void MyDetectorConstruction::ConstructSDandField() {
    G4cout << "\n================ ConstructSDandField() =================\n";

    auto sdManager = G4SDManager::GetSDMpointer();
    auto LVS = G4LogicalVolumeStore::GetInstance();

    G4String SDname = "NaI_SD";
    auto NaI_Detector = dynamic_cast<MySensitiveDetector*>(sdManager->FindSensitiveDetector(SDname, false));
    if (!NaI_Detector) {
        NaI_Detector = new MySensitiveDetector(SDname, "NaIHitsCollection");
        sdManager->AddNewDetector(NaI_Detector);
    }

    for (auto lv : *LVS) {
        if (lv->GetName() == "NaI_LV") {
            lv->SetSensitiveDetector(NaI_Detector);
        }
    }

    G4cout << "========================================================\n\n";
}
