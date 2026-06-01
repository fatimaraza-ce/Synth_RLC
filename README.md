# SYNTH-RLC

[![Documentation](https://img.shields.io/badge/Documentation-Project%20Report-blue?style=for-the-badge&logo=adobeacrobatreader&logoColor=white)](./docs/SYNTH_RLC_Project_Report.pdf)

> **A C++ circuit synthesis engine that works backwards from a target frequency response; computing, mapping, and analyzing real RLC circuits.**

Most circuit simulators (like LTspice) take a circuit and tell you how it behaves. SYNTH-RLC does the opposite: you give it the behavior you want, and it tells you what circuit to build.

## What it does

Given a target resonant frequency, bandwidth, and a seed inductance value, SYNTH-RLC:

1. **Synthesizes** the ideal R and C values for series or parallel RLC topologies
2. **Accounts for real inductor behavior**: compensates for internal winding resistance (Rₗ) in both topologies
3. **Maps to E12 standard components**: finds the nearest commercially available values in the E12 series
4. **Quantifies the error**: calculates the frequency deviation introduced by component rounding
5. **Classifies transient behavior**: computes α, ω₀, and Q; classifies as undamped / underdamped / critically damped / overdamped
6. **Sweeps frequency response**: generates a 101-point impedance sweep, exported to `results.csv`
7. **Plots the result**: automatically launches a Python script to visualize the impedance curve

## Features

- **Engineering unit input**: type `10k`, `47n`, `1m` directly; no scientific notation required
- **Dual topology**: full series and parallel RLC support with topology-correct formulas throughout
- **Internal resistance compensation**: subtracts Rₗ from required R (series), or models it as parallel equivalent resistance via Q-transformation (parallel)
- **E12 mapping with decade-boundary correction**: catches edge cases where rounding up to the next decade is more accurate
- **Dynamic sweep width**: scales the frequency sweep window based on Q factor, so both narrow and wide resonances are fully captured
- **Color-coded terminal output**: warnings (yellow/red) for excessive Rₗ, large frequency deviation, and low Q; results (green/cyan) for synthesis output
- **CSV export + Python visualization**: `results.csv` and auto-launched `plot.py` for graphical frequency response

## Theory at a glance

| Parameter | Series RLC | Parallel RLC |
|---|---|---|
| Resonant frequency | `f₀ = 1 / (2π√LC)` | `f₀ = 1 / (2π√LC)` |
| Bandwidth | `BW = R / (2πL)` | `BW = 1 / (2πRC)` |
| Damping factor α | `R / 2L` | `1 / (2RC)` |
| Quality factor Q | `f₀ / BW` | `f₀ / BW` |

**Damping classification** (with 5% tolerance around the critical boundary, since exact critical damping is unachievable with discretized E12 components):

| Condition | State |
|---|---|
| α = 0 | Undamped |
| α < ω₀ | Underdamped (oscillatory) |
| α ≈ ω₀ | Critically damped |
| α > ω₀ | Overdamped |

**E12 series base values** (repeated per decade):
`1.0 · 1.2 · 1.5 · 1.8 · 2.2 · 2.7 · 3.3 · 3.9 · 4.7 · 5.6 · 6.8 · 8.2`

## Getting started

### Prerequisites

- A C++ compiler — GCC/g++ or MSVC
- Python 3 with `matplotlib` (for frequency response plots)

```bash
pip install matplotlib
```

### Build & run

```bash
# Clone
git clone https://github.com/fatimaraza-ce/Synth_RLC.git
cd Synth_RLC

# Compile (GCC)
g++ -o synth_rlc main.cpp -lm

# Run
./synth_rlc
```

> **Windows (MSVC):** Open in Visual Studio and build normally. The executable runs from `x64\Debug\` — copy `plot.py` into that folder so the auto-launch works.

### Python plotter

`plot.py` reads `results.csv` (generated automatically on each run) and produces an impedance vs. frequency graph saved as `plot.png`. It is launched automatically via `system("python plot.py")` at the end of each run. If Python is not in your PATH, run it manually:

```bash
python plot.py
```

---

## Usage

The program runs interactively in the terminal. All inputs accept engineering suffix notation.

```
========================================
        SYNTH-RLC DESIGN SYSTEM
========================================

[ INPUT PARAMETERS ]
Enter target frequency: 10k
Select Circuit Topology:
1. Series RLC
2. Parallel RLC
Selection: 1
Enter Target Bandwidth: 1k
Enter Seed Inductor Value (e.g., 1m, 10u): 10m
Enter Inductor Internal Resistance (RL) [0 if ideal]: 0
```

**Supported input suffixes:**

| Suffix | Multiplier |
|---|---|
| `p` | × 10⁻¹² |
| `n` | × 10⁻⁹ |
| `u` | × 10⁻⁶ |
| `m` | × 10⁻³ |
| `k` | × 10³ |
| `M` | × 10⁶ |
| `G` | × 10⁹ |

All inputs are validated in a `while(true)` loop — bad values print a red error and re-prompt without crashing.

---

## Sample output (Series RLC, 10 kHz)

```
--- Ideal (Theoretical) Values ---
Ideal R: 62.832 ohms | Ideal L: 10.000 mH | Ideal C: 25.330 nF

--- Standard E12 (Market) Values ---
Final Resistance:         68.000 ohms
Final Inductance:         10.000 mH
Final Capacitance:        27.000 nF

--- Final Error Analysis ---
Target Frequency:  10000.00 Hz
Actual Frequency:   9667.15 Hz
Frequency Shift:    3.14 %

Quality Factor (Q): 8.95
Actual Bandwidth:   1.080 kHz
Circuit State: UNDERDAMPED (Oscillatory)

✔ Analysis complete successfully!
✔ CSV file generated: results.csv
✔ Graph plotted via Python
```

---

## Module overview

| Module | Function | Description |
|---|---|---|
| Unit Parser | `parseEngineeringUnits()` | Converts suffix strings (`10k`, `47n`) to `double` |
| Input Validation | — | `while(true)` + `try-catch` around every prompt |
| Synthesis Engine | — | Computes ideal R and C from f₀, BW, L, Rₗ, topology |
| E12 Mapper | `findBestE12()` | Maps any value to nearest E12 standard, with next-decade boundary check |
| Error Analyser | `calculateError()` | Percentage deviation between target and actual f₀ |
| Transient Analyser | — | Computes α, ω₀, Q; classifies damping state |
| Frequency Profiler | — | 101-point impedance sweep with dynamic width based on Q |
| CSV Export | — | Writes `results.csv` via `ofstream` |
| Python Plotter | `plot.py` | Reads CSV, plots impedance curve, saves `plot.png` |

---

## Test results summary

7 test cases covering both topologies, edge cases, and invalid input — all passed, no crashes.

| # | Topology | Target f₀ | BW | Rₗ | Freq Error | Q | State |
|---|---|---|---|---|---|---|---|
| TC1 | Series | 10 kHz | 1 kHz | 0 Ω | 3.14% | 8.95 | Underdamped |
| TC2 | Parallel | 50 kHz | 25 kHz | 0 Ω | 0.66% | 2.15 | Underdamped |
| TC3 | Series | 10 kHz | 1 kHz | 50 Ω | 3.14% | 9.82 | Underdamped |
| TC4 | Series | 10 kHz | 1 kHz | 100 Ω | 3.14% | 6.09 | Underdamped ⚠ |
| TC5 | Series | 10 kHz | 19.4 kHz | 0 Ω | 3.14% | 0.51 | Critically Damped |
| TC6 | Series | 10 kHz | 50 kHz | 0 Ω | 3.14% | 0.18 | Overdamped ⚠ |
| TC7 | — | invalid inputs | — | — | — | — | All rejected gracefully |

## Known limitations

- Only E12 component series is supported (E24/E96 would reduce frequency errors significantly)
- Frequency sweep resolution is fixed at 101 points — very high-Q circuits may miss the resonance peak
- Python visualization requires manual setup if Python is not in the system PATH
- `system()` call for the plotter is platform-dependent

## Built with

- **C++** — core computation engine (`<cmath>`, `<fstream>`, `<sstream>`, `<iomanip>`, standard library only, no external dependencies)
- **Python / matplotlib** — frequency response visualization

## References

- Sadiku & Alexander — *Fundamentals of Electric Circuits*, McGraw-Hill
- IEC 60063:2015 — *Preferred Number Series for Resistors and Capacitors*
- [Matplotlib documentation](https://matplotlib.org)
- [LTspice — Analog Devices](https://www.analog.com/en/design-center/design-tools-and-calculators/ltspice-simulator.html) *(the simulator that motivated this project)*

*Originally developed as a semester project and expanded into a practical circuit synthesis tool.*
