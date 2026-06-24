#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>

using namespace std;

// ==========================================
// Base Class: Animal (Polymorphism & Abstraction)
// ==========================================
class Animal {
protected:
    string tagNumber;
    int age;
    string healthStatus;
    bool isNeutered;
    bool isVaccinated;

public:
    Animal(string tag, int a, string health, bool neutered, bool vaccinated)
        : tagNumber(tag), age(a), healthStatus(health), isNeutered(neutered), isVaccinated(vaccinated) {}

    virtual ~Animal() {}

    // Pure Virtual Function
    virtual string getSpecies() const = 0;

    // Virtual Function
    virtual void printDetails() const {
        cout << "Tag No: " << tagNumber << "\n"
             << "Species: " << getSpecies() << "\n"
             << "Age: " << age << " months\n"
             << "Health: " << healthStatus << "\n"
             << "Neutered: " << (isNeutered ? "Yes" : "No") << "\n"
             << "Vaccinated: " << (isVaccinated ? "Yes" : "No") << "\n";
    }

    // Encapsulation: Getters and Setters
    string getTagNumber() const { return tagNumber; }
    void setHealthStatus(string health) { healthStatus = health; }
    void setNeutered(bool val) { isNeutered = val; }
    void setVaccinated(bool val) { isVaccinated = val; }
    
    virtual string serialize() const {
        return getSpecies() + "," + tagNumber + "," + to_string(age) + "," + healthStatus + "," + (isNeutered ? "1" : "0") + "," + (isVaccinated ? "1" : "0");
    }
};

// ==========================================
// Derived Classes (Inheritance)
// ==========================================
class Dog : public Animal {
    string breedSize; // Specific to dogs (small, medium, large)
public:
    Dog(string tag, int a, string health, bool neutered, bool vaccinated, string size)
        : Animal(tag, a, health, neutered, vaccinated), breedSize(size) {}

    string getSpecies() const override { return "Dog"; }

    void printDetails() const override {
        Animal::printDetails();
        cout << "Size: " << breedSize << "\n";
    }

    string serialize() const override {
        return Animal::serialize() + "," + breedSize;
    }
};

class Cat : public Animal {
    bool isFeral; // Specific to cats
public:
    Cat(string tag, int a, string health, bool neutered, bool vaccinated, bool feral)
        : Animal(tag, a, health, neutered, vaccinated), isFeral(feral) {}

    string getSpecies() const override { return "Cat"; }

    void printDetails() const override {
        Animal::printDetails();
        cout << "Feral: " << (isFeral ? "Yes" : "No") << "\n";
    }

    string serialize() const override {
        return Animal::serialize() + "," + (isFeral ? "1" : "0");
    }
};

// ==========================================
// Logistics Manager (File Stream Handling)
// ==========================================
class TNVRManager {
    vector<unique_ptr<Animal>> database;
    const string fileName = "tnvr_records.txt";

public:
    void addAnimal(unique_ptr<Animal> animal) {
        database.push_back(move(animal));
    }

    void listAllRecords() const {
        cout << "\n================ TNVR LOGISTICS REGISTRY ================\n";
        for (const auto& animal : database) {
            animal->printDetails();
            cout << "---------------------------------------------------------\n";
        }
    }

    // Save database records to local text file
    void saveToFile() const {
        ofstream outFile(fileName);
        if (!outFile) {
            cerr << "[ERROR] Could not open file for writing.\n";
            return;
        }
        for (const auto& animal : database) {
            outFile << animal->serialize() << "\n";
        }
        outFile.close();
        cout << "[SUCCESS] Saved " << database.size() << " records to " << fileName << "\n";
    }

    // Load database records from local text file
    void loadFromFile() {
        ifstream inFile(fileName);
        if (!inFile) {
            cout << "[INFO] No existing database file found. Starting fresh.\n";
            return;
        }
        database.clear();
        string line;
        while (getline(inFile, line)) {
            // Split CSV tokens
            vector<string> tokens;
            size_t pos = 0;
            while ((pos = line.find(',')) != string::npos) {
                tokens.push_back(line.substr(0, pos));
                line.erase(0, pos + 1);
            }
            tokens.push_back(line);

            if (tokens.size() >= 7) {
                string species = tokens[0];
                string tag = tokens[1];
                int age = stoi(tokens[2]);
                string health = tokens[3];
                bool neutered = tokens[4] == "1";
                bool vaccinated = tokens[5] == "1";

                if (species == "Dog") {
                    string size = tokens[6];
                    database.push_back(make_unique<Dog>(tag, age, health, neutered, vaccinated, size));
                } else if (species == "Cat") {
                    bool feral = tokens[6] == "1";
                    database.push_back(make_unique<Cat>(tag, age, health, neutered, vaccinated, feral));
                }
            }
        }
        inFile.close();
        cout << "[SUCCESS] Loaded " << database.size() << " records from " << fileName << "\n";
    }
};

// ==========================================
// Console App Entry Point
// ==========================================
int main() {
    TNVRManager manager;
    manager.loadFromFile();

    // Setup mock entries if database is empty
    cout << "Initializing logistics matching...\n";
    
    // Add a Dog and a Cat
    manager.addAnimal(make_unique<Dog>("DG-101", 24, "Healthy", true, true, "Medium"));
    manager.addAnimal(make_unique<Cat>("CT-302", 12, "Injured-Recovering", false, true, true));

    manager.listAllRecords();
    manager.saveToFile();

    return 0;
}
