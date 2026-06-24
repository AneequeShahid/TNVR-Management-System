#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

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
    int getAge() const { return age; }
    string getHealthStatus() const { return healthStatus; }
    bool getNeutered() const { return isNeutered; }
    bool getVaccinated() const { return isVaccinated; }
    
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
    
    string getBreedSize() const { return breedSize; }
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
    
    bool getFeral() const { return isFeral; }
};

// ==========================================
// Logistics Manager (File Stream Handling)
// ==========================================
class TNVRManager {
    vector<unique_ptr<Animal>> database;
    const string fileName = "tnvr_records.txt";

public:
    size_t getSize() const { return database.size(); }

    void addAnimal(unique_ptr<Animal> animal) {
        // Remove existing animal if it shares the same tag number
        database.erase(
            remove_if(database.begin(), database.end(),
                      [&animal](const unique_ptr<Animal>& existing) {
                          return existing->getTagNumber() == animal->getTagNumber();
                      }),
            database.end()
        );
        database.push_back(move(animal));
    }

    bool deleteAnimal(const string& tag) {
        auto beforeSize = database.size();
        database.erase(
            remove_if(database.begin(), database.end(),
                      [&tag](const unique_ptr<Animal>& animal) {
                          return animal->getTagNumber() == tag;
                      }),
            database.end()
        );
        return database.size() < beforeSize;
    }

    bool updateAnimal(const string& tag, const string& health, bool neutered, bool vaccinated) {
        for (auto& animal : database) {
            if (animal->getTagNumber() == tag) {
                animal->setHealthStatus(health);
                animal->setNeutered(neutered);
                animal->setVaccinated(vaccinated);
                return true;
            }
        }
        return false;
    }

    void listAllRecords() const {
        cout << "[START_LIST]\n";
        for (const auto& animal : database) {
            cout << animal->serialize() << "\n";
        }
        cout << "[END_LIST]\n";
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
            if (line.empty()) continue;
            
            // Split CSV tokens
            vector<string> tokens;
            size_t pos = 0;
            string temp = line;
            while ((pos = temp.find(',')) != string::npos) {
                tokens.push_back(temp.substr(0, pos));
                temp.erase(0, pos + 1);
            }
            tokens.push_back(temp);

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
// Console App Entry Point (Command Loop)
// ==========================================
int main() {
    TNVRManager manager;
    manager.loadFromFile();

    // Seed default database if completely empty on launch
    if (manager.getSize() == 0) {
        manager.addAnimal(make_unique<Dog>("DG-101", 24, "Healthy", true, true, "Medium"));
        manager.addAnimal(make_unique<Cat>("CT-302", 12, "Injured", false, true, true));
        manager.saveToFile();
    }

    cout << "[READY] TNVR C++ Engine Active\n";

    string command;
    while (cin >> command) {
        if (command == "LIST") {
            manager.listAllRecords();
        } 
        else if (command == "ADD") {
            string species, tag, health, extra;
            int age;
            int neuteredVal, vaccinatedVal;
            
            cin >> species >> tag >> age >> health >> neuteredVal >> vaccinatedVal >> extra;
            
            bool neutered = (neuteredVal == 1);
            bool vaccinated = (vaccinatedVal == 1);

            // Replace spaces encoded as underscores in health
            replace(health.begin(), health.end(), '_', ' ');

            if (species == "Dog") {
                manager.addAnimal(make_unique<Dog>(tag, age, health, neutered, vaccinated, extra));
                cout << "[SUCCESS] Dog added\n";
            } else if (species == "Cat") {
                bool feral = (extra == "1");
                manager.addAnimal(make_unique<Cat>(tag, age, health, neutered, vaccinated, feral));
                cout << "[SUCCESS] Cat added\n";
            } else {
                cout << "[ERROR] Invalid species\n";
            }
        } 
        else if (command == "DELETE") {
            string tag;
            cin >> tag;
            if (manager.deleteAnimal(tag)) {
                cout << "[SUCCESS] Record deleted\n";
            } else {
                cout << "[ERROR] Tag not found\n";
            }
        } 
        else if (command == "UPDATE") {
            string tag, health;
            int neuteredVal, vaccinatedVal;
            
            cin >> tag >> health >> neuteredVal >> vaccinatedVal;
            
            bool neutered = (neuteredVal == 1);
            bool vaccinated = (vaccinatedVal == 1);
            
            replace(health.begin(), health.end(), '_', ' ');
            
            if (manager.updateAnimal(tag, health, neutered, vaccinated)) {
                cout << "[SUCCESS] Record updated\n";
            } else {
                cout << "[ERROR] Tag not found\n";
            }
        } 
        else if (command == "SAVE") {
            manager.saveToFile();
        } 
        else if (command == "EXIT") {
            manager.saveToFile();
            cout << "[BYE] Exiting\n";
            break;
        } 
        else {
            cout << "[ERROR] Unknown command: " << command << "\n";
        }
    }

    return 0;
}
