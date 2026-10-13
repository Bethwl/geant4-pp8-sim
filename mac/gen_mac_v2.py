#!/usr/bin/env python3
"""
gen_mac_v2.py - Tao file .mac cho Geant4 tu file SRIM_data.xlsx.

File Excel dau vao co 3 sheet: Carbon, Helium, Hydrogen (ung voi hat
carbon, alpha, proton). MOI sheet chua 2 BANG xep chong nhau, CUNG mot
luoi nang luong (keV), CUNG thu tu 9 cot vat lieu:

    Bang 1: "Mass Stopping Power (keV / (mg/cm2))"
            Energy (keV) | H2O | Ti2AlC | Ti2AlN | V2AlC | Cr2AlC | Nb2AlC
                          | Ta2AlC | Ti3SiC2 | Ti3AlC2
    (2 dong trong, roi)
    Bang 2: "Projected Range (cm)"
            Energy (keV) | H2O | Ti2AlC | ... (cung thu tu cot nhu tren)

9 cot vat lieu duoc anh xa THEO DUNG THU TU sang S1..S9:
    S1=H2O  S2=Ti2AlC  S3=Ti2AlN  S4=V2AlC  S5=Cr2AlC
    S6=Nb2AlC  S7=Ta2AlC  S8=Ti3SiC2  S9=Ti3AlC2

Ten sheet -> ten hat trong Geant4 (/MyGun/particleType):
    Carbon   -> carbon
    Helium   -> alpha
    Hydrogen -> proton

Voi MOI to hop (vat lieu, hat, nang luong E), be day mau duoc tinh
RIENG cho tung nang luong tu bang Projected Range:

    thickness_um(E) = Range_cm(E) * 1e4 (cm->um) * 0.1% = Range_cm(E) * 10

Ket qua: xuat 1 file .mac / (vat lieu x hat) = 9 x 3 = 27 file, theo dung
dinh dang:

    /DetectorConstruction/SampleMaterial S1
    /MyGun/particleType alpha
    /DetectorConstruction/SampleThickness <t1> um
    /MyGun/energy <E1> keV
    /run/beamOn 100000
    /DetectorConstruction/SampleThickness <t2> um
    /MyGun/energy <E2> keV
    /run/beamOn 100000
    ...

Cach dung:
    python3 gen_mac_v2.py SRIM_data.xlsx --outdir ./mac_output
    python3 gen_mac_v2.py SRIM_data.xlsx --beamon 100000
"""
import argparse
import sys
from pathlib import Path
import pandas as pd

# Thu tu cot vat lieu trong Excel -> ten S1..S9 (DUNG THEO THU TU XUAT HIEN)
MATERIAL_ORDER = ["H2O", "Ti2AlC", "Ti2AlN", "V2AlC", "Cr2AlC",
                   "Nb2AlC", "Ta2AlC", "Ti3SiC2", "Ti3AlC2"]

SHEET_TO_PARTICLE = {
    "Carbon": "carbon",
    "Helium": "alpha",
    "Hydrogen": "proton",
}

RANGE_CM_TO_THICKNESS_UM_FACTOR = 10.0  # cm->um (*1e4) roi *0.1% (*1e-3) = *10


def split_sheet(df_raw: pd.DataFrame):
    """Tach 1 sheet (doc header=None) thanh 2 DataFrame: stopping power va
    projected range, dua vao dong chua chu 'Projected'."""
    split_idx = None
    for i in range(len(df_raw)):
        v = df_raw.iloc[i, 0]
        if isinstance(v, str) and "Projected" in v:
            split_idx = i
            break
    if split_idx is None:
        raise ValueError("Khong tim thay bang 'Projected Range' trong sheet.")

    cols = df_raw.iloc[1].tolist()  # dong header cot (Energy, H2O, ...)

    sp = df_raw.iloc[2:split_idx - 1].reset_index(drop=True).copy()
    rng = df_raw.iloc[split_idx + 2:].reset_index(drop=True).copy()
    sp.columns = cols
    rng.columns = cols
    sp = sp.dropna(subset=[cols[0]]).reset_index(drop=True)
    rng = rng.dropna(subset=[cols[0]]).reset_index(drop=True)

    for c in cols:
        sp[c] = pd.to_numeric(sp[c], errors="coerce")
        rng[c] = pd.to_numeric(rng[c], errors="coerce")

    return sp, rng, cols


def sample_tag(material_name: str) -> str:
    idx = MATERIAL_ORDER.index(material_name)  # raise neu khong khop
    return f"S{idx + 1}"


def fmt_num(x: float) -> str:
    """Format so gon gang, bo so 0 thua, khong dung ky hieu khoa hoc."""
    s = f"{x:.6f}".rstrip("0").rstrip(".")
    return s if s else "0"


def main():
    ap = argparse.ArgumentParser(
        description="Tao 27 file .mac (vat lieu x hat) tu SRIM_data.xlsx.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    ap.add_argument("excel", help="Duong dan file SRIM_data.xlsx")
    ap.add_argument("--outdir", default="./mac_output", help="Thu muc xuat file .mac")
    ap.add_argument("--beamon", type=int, default=100000, help="So event /run/beamOn (mac dinh 100000)")
    args = ap.parse_args()

    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    xl = pd.ExcelFile(args.excel)
    generated = []

    for sheet in xl.sheet_names:
        if sheet not in SHEET_TO_PARTICLE:
            print(f"[BO QUA] Sheet '{sheet}' khong khop ten hat nao, bo qua.")
            continue
        particle = SHEET_TO_PARTICLE[sheet]

        df_raw = pd.read_excel(xl, sheet_name=sheet, header=None, dtype=object)
        sp, rng, cols = split_sheet(df_raw)
        energy_col = cols[0]

        if sp[energy_col].tolist() != rng[energy_col].tolist():
            print(f"[LOI] Sheet '{sheet}': luoi nang luong cua bang Stopping "
                  f"Power va Projected Range KHONG khop nhau. Dung lai.")
            sys.exit(1)

        energies = sp[energy_col].tolist()  # keV, da theo thu tu tang dan trong file

        for material in cols[1:]:
            if material not in MATERIAL_ORDER:
                print(f"[BO QUA] Cot vat lieu '{material}' khong nam trong "
                      f"danh sach S1..S9 da biet, bo qua.")
                continue
            tag = sample_tag(material)

            lines = [
                f"/DetectorConstruction/SampleMaterial {tag}",
                f"/MyGun/particleType {particle}",
            ]
            for e_kev, r_cm in zip(energies, rng[material].tolist()):
                if pd.isna(e_kev) or pd.isna(r_cm):
                    continue
                thickness_um = r_cm * RANGE_CM_TO_THICKNESS_UM_FACTOR
                lines.append(f"/DetectorConstruction/SampleThickness {fmt_num(thickness_um)} um")
                lines.append(f"/MyGun/energy {fmt_num(e_kev)} keV")
                lines.append(f"/run/beamOn {args.beamon}")

            fname = f"run_{tag}_{particle}.mac"
            (outdir / fname).write_text("\n".join(lines) + "\n")
            generated.append((tag, material, particle, len(energies), fname))
            print(f"  {tag} ({material}) x {particle}: {len(energies)} muc nang luong -> {fname}")

    # file tong: chay lan luot tat ca
    with open(outdir / "run_all.mac", "w") as f:
        f.write("/run/initialize\n")
        f.write("/control/verbose 2\n")
        for tag, material, particle, n, fname in generated:
            f.write(f"/control/execute {fname}\n")

    print(f"\n[DONE] Da tao {len(generated)} file .mac (1 file / vat lieu x hat) "
          f"trong: {outdir.resolve()}")
    print(f"       run_all.mac: chay tuan tu toan bo {len(generated)} file tren.")


if __name__ == "__main__":
    main()
