#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cctype>
#include <iomanip> 
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

//ANSI Escape codes for CLI Colors
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"

#define M_PI 3.14159265358979323846

struct Component {
    string type;
    double value;
};

bool isValidNumber(const string& str) {
    try {
        stod(str);
        return true;
    }
    catch (...) {
        return false;
    }
}

//Unit Parser Module
double parseEngineeringUnits(string input) {
    if (input.empty()) return 0.0;
    char suffix = input.back(); //pick suffix from the string
    double multiplier = 1.0; //initializing multiplier

    if (isalpha(suffix)) { //if: the last character fromt the string is an alphabet convert it into multiplier
        string numericPart = input.substr(0, input.size() - 1); //take the numeric part (excluding the suffix)
        if (!isValidNumber(numericPart)) {
            throw invalid_argument("Invalid numeric input");
        }

        double val = stod(numericPart); //use stod to convert numeric part into double
        switch (suffix) { //powers of 10 acc to suffix
        case 'p': multiplier = 1e-12; break;
        case 'n': multiplier = 1e-9;  break;
        case 'u': multiplier = 1e-6;  break;
        case 'm': multiplier = 1e-3;  break;
        case 'k': multiplier = 1e3;   break;
        case 'M': multiplier = 1e6;   break;
        case 'G': multiplier = 1e9;   break;
        default:  multiplier = 1.0;   break;
        }
        return val * multiplier; //converted value 
    }
    if (!isValidNumber(input)) {
        throw invalid_argument("Invalid numeric input");
    }
    return stod(input);
}

string formatEngineering(double value, string unit) {
    if (value == 0) return "0 " + unit;

    double absVal = abs(value);
    string prefix;
    double scaled = value;

    if (absVal >= 1e9) {
        scaled = value / 1e9;
        prefix = "G";
    }
    else if (absVal >= 1e6) {
        scaled = value / 1e6;
        prefix = "M";
    }
    else if (absVal >= 1e3) {
        scaled = value / 1e3;
        prefix = "k";
    }
    else if (absVal >= 1) {
        prefix = "";
    }
    else if (absVal >= 1e-3) {
        scaled = value * 1e3;
        prefix = "m";
    }
    else if (absVal >= 1e-6) {
        scaled = value * 1e6;
        prefix = "u";
    }
    else if (absVal >= 1e-9) {
        scaled = value * 1e9;
        prefix = "n";
    }
    else {
        scaled = value * 1e12;
        prefix = "p";
    }

    stringstream ss;
    ss << fixed << setprecision(3) << scaled;
    return ss.str() + " " + prefix + unit;
}

//Standard Values Module
double findBestE12(double idealValue) { //finding the standard value closest to the calculated value
    double E12_BASE[] = { 1.0, 1.2, 1.5, 1.8, 2.2, 2.7, 3.3, 3.9, 4.7, 5.6, 6.8, 8.2 }; //base values
    if (idealValue <= 0) return 0;
    double decade = pow(10, floor(log10(idealValue)));
    double baseTarget = idealValue / decade; //shifting decimal for comparing (e.g. convert 3456 to 3.456)
    double bestMatch = E12_BASE[0]; //assuming 1.0 is the closest value
    double minDiff = abs(baseTarget - E12_BASE[0]); //difference bw first value and the calc value

    for (int i = 1; i < 12; i++) { //looping to find the smallest difference
        double currentDiff = abs(baseTarget - E12_BASE[i]);
        if (currentDiff < minDiff) {
            minDiff = currentDiff;
            bestMatch = E12_BASE[i];
        }
    }
    // Also check first value of next decade
    double nextDecadeVal = E12_BASE[0] * decade * 10;
    if (abs(idealValue - nextDecadeVal) < abs(idealValue - bestMatch * decade)) {
        return nextDecadeVal;
    }
    return bestMatch * decade; //smallest difference means that value is the best match
}

// Error Calculation Module
double calculateError(double target, double actual) {
    if (target == 0) return 0.0;
    return (abs(actual - target) / target) * 100.0;
}

int main() {
    string userInput, bwInput, lInput;
    int top;

    cout << CYAN;
    cout << "\n========================================\n";
    cout << "        SYNTH-RLC DESIGN SYSTEM         \n";
    cout << "========================================\n";
    cout << RESET;

    cout << BLUE << "\n[ INPUT PARAMETERS ]\n" << RESET;

    //inputs:
    //taking input for target frequency
    double frequency;
    while (true) {
        cout << "Enter target frequency: ";
        cin >> userInput;

        try {
            frequency = parseEngineeringUnits(userInput);
            if (frequency <= 0) throw invalid_argument("Must be > 0");
            break;
        }
        catch (...) {
            cout << RED << "Invalid input. Try again.\n" << RESET;
        }
    }

    while (true) {
        cout << "Select Circuit Topology:\n1. Series RLC\n2. Parallel RLC\nSelection: ";
        cin >> top;

        if (top == 1 || top == 2) break;

        cout << RED << "Invalid choice. Enter 1 or 2.\n" << RESET;
    } //taking input for circuit topology

    //taking input for target bandwidth
    double bandwidth;
    while (true) {
        cout << "Enter Target Bandwidth: ";
        cin >> bwInput;

        try {
            bandwidth = parseEngineeringUnits(bwInput);
            if (bandwidth <= 0) throw invalid_argument("Must be > 0");
            break;
        }
        catch (...) {
            cout << RED << "Invalid input. Try again.\n" << RESET;
        }
    }

    //taking input for seed inductance value (to calculate ideal R, C)
    double L_ideal;
    while (true) {
        cout << "Enter Seed Inductor Value (e.g., 1m, 10u): ";
        cin >> lInput;

        try {
            L_ideal = parseEngineeringUnits(lInput);
            if (L_ideal <= 0) throw invalid_argument("Must be > 0");
            break;
        }
        catch (...) {
            cout << RED << "Invalid input. Try again.\n" << RESET;
        }
    }

    if (frequency <= 0 || bandwidth <= 0) {
        cout << RED << "Frequency and Bandwidth must be > 0!" << RESET << endl;
        return 1;
    }

    //since inductor is a wire in the shape of coil, in the real world, it always has some internal resistance
    string rlInput;
    //Taking input for inductor's internal resistance
    double R_internal;
    while (true) {
        cout << "Enter Inductor Internal Resistance (RL) [0 if ideal]: ";
        cin >> rlInput;

        try {
            R_internal = parseEngineeringUnits(rlInput);
            if (R_internal < 0) throw invalid_argument("Must be >= 0");
            break;
        }
        catch (...) {
            cout << RED << "Invalid input. Try again.\n" << RESET;
        }
    }

    if (L_ideal <= 0) {
        cout << RED << "Error: Inductor value must be greater than zero." << RESET << endl;
        return 1;
    }

    //Synthesis Engine (Ideal)
    double C_ideal, R_ideal;

    if (top == 1) { // Series RLC
        C_ideal = 1.0 / (L_ideal * pow(2 * M_PI * frequency, 2));
        double R_total_needed = 2 * M_PI * bandwidth * L_ideal;

        // Cater for Internal Resistance
        R_ideal = R_total_needed - R_internal;

        if (R_ideal < 0) {
            cout << YELLOW << "\nWARNING: Internal resistance (" << R_internal
                << " Ohms) is too high for the target Bandwidth!" << RESET << endl;
            R_ideal = 0;
        }
    }
    else { // Parallel RLC
        C_ideal = 1.0 / (L_ideal * pow(2 * M_PI * frequency, 2));

        // For parallel, R_total_needed is 1 / (2*pi*BW*C)
        double R_total_needed = 1.0 / (2 * M_PI * bandwidth * C_ideal);

        // Complex Catering for Parallel RL
        double XL = 2 * M_PI * frequency * L_ideal;
        double R_eq_parallel;

        if (R_internal == 0) {
            R_eq_parallel = 1e12; // simulate very large resistance (ideal case)
        }
        else {
            R_eq_parallel = (XL * XL) / R_internal;
        }

        if (R_eq_parallel > R_total_needed) {
            double denom = (1.0 / R_total_needed) - (1.0 / R_eq_parallel);
            if (abs(denom) < 1e-12) {
                R_ideal = R_total_needed;
            }
            else {
                R_ideal = 1.0 / denom;
            }
        }
        else {
            cout << YELLOW << "\nWARNING: Inductor internal resistance is too high "
                << "for the target bandwidth in parallel configuration!\n"
                << "Consider using a lower RL or a higher inductance value.\n" << RESET;
            R_ideal = R_total_needed;
        }
    }
    //Standard Values Mapping
    //apply fuctions to find the closest standard values to the calculated ones
    double R_real = findBestE12(R_ideal);
    double L_real = findBestE12(L_ideal);
    double C_real = findBestE12(C_ideal);

    //Display Results
    cout << fixed << setprecision(6);
    cout << GREEN << "\n--- Ideal (Theoretical) Values ---" << RESET << endl;
    cout << "Ideal R: " << R_ideal << " | Ideal L: " << L_ideal << " | Ideal C: " << C_ideal << endl;

    cout << GREEN << "\n--- Standard E12 (Market) Values ---" << RESET << endl;
    cout << left << setw(25) << "Final Resistance:" << formatEngineering(R_real, "Ω") << endl;
    cout << left << setw(25) << "Final Inductance:" << formatEngineering(L_real, "H") << endl;
    cout << left << setw(25) << "Final Capacitance:" << formatEngineering(C_real, "F") << endl;

    //Error Analyser Module

    //Recalculate the REAL frequency produced by the chosen E12 parts
    // Formula: f = 1 / (2 * PI * sqrt(L * C))
    double actualFreq = 1.0 / (2 * M_PI * sqrt(L_real * C_real));

    //Calculate the percentage deviation
    double freqError = calculateError(frequency, actualFreq);

    //Display the impact of using standard parts
    cout << BLUE << "\n--- Final Error Analysis ---" << RESET << endl;
    cout << "Target Frequency: " << frequency << " Hz" << endl;
    cout << "Actual Frequency: " << actualFreq << " Hz" << endl;
    cout << "Frequency Shift:  " << freqError << " %" << endl;

    if (freqError > 10.0) {
        cout << RED << "Warning: High frequency deviation due to E12 rounding!" << RESET << endl;
    }

    // Transient Analyzer Module
    double alpha, omega0;

    omega0 = 1.0 / sqrt(L_real * C_real);
    double f0 = omega0 / (2 * M_PI);

    if (top == 1) {
        // Series
        alpha = (R_real + R_internal) / (2.0 * L_real);
    }
    else {
        // Parallel
        alpha = 1.0 / (2.0 * R_real * C_real);
    }

    double Q = (alpha == 0) ? 1e12 : omega0 / (2.0 * alpha);
    double calculatedBW = f0 / Q;

    cout << YELLOW << "Quality Factor (Q): " << RESET << Q << endl;
    cout << YELLOW << "Actual Bandwidth: " << RESET << formatEngineering(calculatedBW, "Hz") << endl;
    if (Q < 0.5) {
        cout << RED << "Note: Low Q factor. Resonance peak will be very broad." << RESET << endl;
    }

    cout << YELLOW << "\n--- Transient Analysis ---" << RESET << endl;
    cout << "Damping Factor (alpha): " << alpha << endl;
    cout << "Resonant Frequency (omega0): " << omega0 << " rad/s" << endl;

    if (alpha == 0) { //the circuit is undamped 
        cout << "Circuit State: " << RED << "UNDAMPED" << RESET << endl;
    }
    else if (abs(alpha - omega0) < 1e-9) { //the circuit is critically damped
        cout << "Circuit State: " << GREEN << "CRITICALLY DAMPED" << RESET << endl;
    }
    else if (alpha < omega0) { //the circuit is underdamped 
        cout << "Circuit State: " << CYAN << "UNDERDAMPED (Oscillatory)" << RESET << endl;
    }
    else { //the circuit is overdamped
        cout << "Circuit State: " << BLUE << "OVERDAMPED (Non-oscillatory)" << RESET << endl;
    }

    //Frequency Response Profiler Module
    cout << CYAN << "\n[ FREQUENCY SWEEP ]\n" << RESET;
    ofstream csvFile("results.csv");
    if (!csvFile) {
        cout << RED << "Error creating CSV file!" << RESET << endl;
        return 1;
    }
    csvFile << "#Topology=" << (top == 1 ? "Series" : "Parallel") << "\n";
    csvFile << "Frequency(Hz),Impedance(Ohms)\n";

    cout << BLUE << "\n--- Generating Frequency Sweep Data (CSV) ---" << RESET << endl;
    cout << "Freq(Hz), Impedance(Mag)" << endl;

    double bestFreq = 0;
    double bestZ;

    // Initialize depending on circuit type
    if (top == 1) {
        bestZ = 1e12; // Series: looking for MIN
    }
    else {
        bestZ = 0;    // Parallel: looking for MAX
    }

    // Sweep from 0.5*f0 to 1.5*f0 in 100 steps
    double sweepSpan = (Q > 1.0) ? (2.0 / Q) : 2.0;  // wider sweep for low-Q
    sweepSpan = max(sweepSpan, 0.5);                    // minimum 50% either side
    double f_start = frequency * (1.0 - sweepSpan / 2.0);
    double f_end = frequency * (1.0 + sweepSpan / 2.0);
    if (f_start <= 0) f_start = frequency * 0.01;       // safety guard

    for (int i = 0; i <= 100; i++) {
        double f_sweep = f_start + (i * (f_end - f_start) / 100.0);
        double omega_s = 2.0 * M_PI * f_sweep;

        double XL = omega_s * L_real;
        double XC = 1.0 / (omega_s * C_real);
        double Z_mag;

        if (top == 1) { // Series Z = sqrt(R^2 + (XL - XC)^2)
            Z_mag = sqrt(pow(R_real + R_internal, 2) + pow(XL - XC, 2));
        }
        else { // Parallel 1/Z = sqrt((1/R)^2 + (1/XL - 1/XC)^2)
            if (R_real == 0 || XL == 0 || XC == 0) {
                Z_mag = 0;
            }
            else {
                double invZ = sqrt(pow(1.0 / R_real, 2) + pow((1.0 / XL) - (1.0 / XC), 2));
                Z_mag = 1.0 / invZ;
            }
        }

        //write to csv file
        csvFile << f_sweep << "," << Z_mag << "\n";

        //Track resonance point
        if (top == 1) {
            // Series: minimum impedance
            if (Z_mag < bestZ) {
                bestZ = Z_mag;
                bestFreq = f_sweep;
            }
        }
        else {
            //Parallel: maximum impedance
            if (Z_mag > bestZ) {
                bestZ = Z_mag;
                bestFreq = f_sweep;
            }
        }

        // Only print every 10th value to console to keep it clean
        if (i % 10 == 0) {
            cout << left << setw(15) << formatEngineering(f_sweep, "Hz")
                << " | " << formatEngineering(Z_mag, "Ω") << endl;
        }
    }
    csvFile.close();

    cout << CYAN << "\n--- Resonance Detected from Sweep ---" << RESET << endl;
    cout << "Resonant Frequency (from sweep): " << bestFreq << " Hz" << endl;
    cout << "Impedance at Resonance: " << bestZ << " Ohms" << endl;

    cout << GREEN << "\n✔ Analysis complete successfully!\n" << RESET;
    cout << "✔ CSV file generated: results.csv\n";
    cout << "✔ Graph plotted via Python\n";

    int result = system("python plot.py");

    if (result != 0) {
        cout << YELLOW << "Could not launch Python script automatically." << RESET << endl;
    }
    cout << CYAN << "\n========================================\n";
    cout << "              SUMMARY                   \n";
    cout << "========================================\n" << RESET;

    cout << "Topology: " << (top == 1 ? "Series RLC" : "Parallel RLC") << endl;
    cout << "Target Frequency: " << formatEngineering(frequency, "Hz") << endl;
    cout << "Resonant Frequency: " << formatEngineering(bestFreq, "Hz") << endl;
    cout << "Frequency Error: " << fixed << setprecision(2) << freqError << " %" << endl;
    system("pause");
    return 0;
}