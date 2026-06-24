# TNVR Management System 🐾

> **A C++ OOP console application for managing Trap-Neuter-Vaccinate-Return (TNVR) animal welfare records** — tracks dogs and cats with full CRUD operations and persistent file-based storage.

[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![Node.js](https://img.shields.io/badge/Node.js-Web%20Bridge-339933?logo=node.js&logoColor=white)](https://nodejs.org/)
[![OOP](https://img.shields.io/badge/Pattern-OOP%20%2F%20Polymorphism-blueviolet)]()
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)

---

## 📖 Description

**TNVR Management System** is a command-line application built in C++ that helps animal welfare organizations manage stray animal records for Trap-Neuter-Vaccinate-Return programs. It stores health status, vaccination records, neutering status, species-specific attributes (breed size for dogs, feral status for cats), and persists all data to a local CSV-formatted text file.

The system demonstrates core Object-Oriented Programming principles: **abstraction** (pure virtual `getSpecies()`), **inheritance** (`Dog` and `Cat` derived from `Animal`), **polymorphism** (virtual `printDetails()`), and **encapsulation** (getters/setters). A lightweight Node.js + Express web server bridges the C++ engine to a browser-based HTML dashboard, communicating via `child_process.spawn` stdin/stdout.

**Who is it for?** CS students studying OOP in C++; animal welfare NGOs wanting a simple digital record system; developers learning how to bridge C++ engines with web frontends.

---

## 📑 Table of Contents

1. [Features](#features)
2. [Project Structure](#project-structure)
3. [Installation](#installation)
4. [Usage](#usage)
5. [OOP Design](#oop-design)
6. [Contributing](#contributing)
7. [License & Contact](#license--contact)

---

## ✨ Features

- 🐕 **Dog & Cat records** — species-specific fields (dog breed size, cat feral status)
- ➕ **ADD** — register a new animal with tag number, age, health, vaccination, and neutering status
- 🗑️ **DELETE** — remove a record by unique tag number
- ✏️ **UPDATE** — modify health status and vaccination/neutering flags
- 📋 **LIST** — print all records in CSV format
- 💾 **Persistent Storage** — auto-saves to `tnvr_records.txt` on every `SAVE` and `EXIT`
- 🌐 **Web Dashboard** — Node.js server wraps the C++ binary for browser-based CRUD
- 🌱 **Auto-seed** — pre-loads two sample records if the database is empty on first launch

---

## 🗂️ Project Structure

```
TNVR-Management-System/
├── main.cpp          # Full C++ engine: Animal, Dog, Cat, TNVRManager, command loop
├── server.js         # Node.js bridge — spawns C++ process, exposes REST API
├── package.json
└── public/           # HTML/CSS web dashboard
```

---

## 🚀 Installation

### Prerequisites

| Tool | Purpose |
|------|---------|
| g++ (C++17) | Compile the C++ engine |
| Node.js ≥ 18 | Run the web bridge server |

### Steps

```bash
# 1. Clone the repository
git clone https://github.com/AneequeShahid/TNVR-Management-System.git
cd TNVR-Management-System

# 2. Compile the C++ engine
g++ -std=c++17 -o tnvr_engine main.cpp

# 3. Install Node.js dependencies
npm install

# 4. Start the web server (spawns the C++ binary automatically)
node server.js
```

Open [http://localhost:3000](http://localhost:3000) in your browser.

#### Running the C++ engine directly (CLI mode)

```bash
./tnvr_engine
```

---

## 💻 Usage

### CLI Commands

Once the C++ engine is running, type commands into stdin:

```
LIST                                          → Print all records
ADD Dog DG-201 18 Healthy 1 1 Medium          → Add a neutered, vaccinated medium dog
ADD Cat CT-405 8 Injured 0 1 1                → Add a feral, vaccinated cat
UPDATE DG-201 Recovering 1 1                  → Update dog DG-201's health
DELETE CT-405                                 → Remove cat record
SAVE                                          → Persist to tnvr_records.txt
EXIT                                          → Save and quit
```

### Web Dashboard

Use the browser UI at `http://localhost:3000` to perform the same operations via button clicks and forms.

---

## 🏛️ OOP Design

| Concept | Implementation |
|---------|---------------|
| **Abstraction** | `Animal` is an abstract base class with pure virtual `getSpecies()` |
| **Inheritance** | `Dog` and `Cat` inherit from `Animal` |
| **Polymorphism** | `printDetails()` and `serialize()` are virtual and overridden per species |
| **Encapsulation** | All member variables are `protected`; accessed via getters/setters |
| **Smart Pointers** | `vector<unique_ptr<Animal>>` manages memory automatically |
| **File I/O** | `saveToFile()` / `loadFromFile()` use `ofstream` / `ifstream` |

---

## 🤝 Contributing

1. Fork the repo
2. Create a feature branch: `git checkout -b feat/your-feature`
3. Commit: `git commit -m "feat: describe your change"`
4. Push and open a Pull Request

---

## 📄 License & Contact

MIT License — see [LICENSE](LICENSE) for details.

**Aneeque Shahid** · [@AneequeShahid](https://github.com/AneequeShahid) · aneequeshahid495@gmail.com
