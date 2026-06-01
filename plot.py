import matplotlib.pyplot as plt
import csv

with open('results.csv') as f:
    first_line = f.readline()
    
is_series = "Series" in first_line
# --- Read CSV ---
freq = []
impedance = []

with open('results.csv', newline='') as csvfile:
    reader = csv.DictReader(csvfile)
    
    # Strip spaces from headers and map to correct names
    header_map = {h.strip(): h for h in reader.fieldnames}

    # Determine which keys to use
    freq_key = None
    imp_key = None
    for key in header_map:
        if 'freq' in key.lower():
            freq_key = header_map[key]
        elif 'imp' in key.lower():
            imp_key = header_map[key]
    
    if not freq_key or not imp_key:
        raise Exception("CSV headers not recognized. Need something like 'Frequency(Hz)' and 'Impedance(Ohms)'")

    for row in reader:
        freq.append(float(row[freq_key]))
        impedance.append(float(row[imp_key]))

# --- Determine Resonant Frequency ---
is_series = True  # Change to False if you know it's a parallel RLC
if is_series:
    res_index = impedance.index(min(impedance))
else:
    res_index = impedance.index(max(impedance))

res_freq = freq[res_index]
res_imp = impedance[res_index]

# --- Plotting ---
plt.figure(figsize=(12,6))
plt.plot(freq, impedance, marker='o', color='blue', label='Impedance')
plt.scatter(res_freq, res_imp, color='red', s=100, label=f'Resonance @ {res_freq:.2f} Hz')

plt.title('Frequency vs Impedance')
plt.xlabel('Frequency (Hz)')
plt.ylabel('Impedance (Ohms)')
plt.grid(True, which='both', linestyle='--', linewidth=0.5)
plt.legend()
plt.tight_layout()

# Save plot as PNG
plt.savefig('plot.png', dpi=300)
print(f"Resonant Frequency: {res_freq:.2f} Hz, Impedance: {res_imp:.2f} Ohms")
print("Graph saved as 'plot.png'.")

# Show plot
plt.show()