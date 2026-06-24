const express = require('express');
const { spawn, execSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3005;

app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

const DB_FILE = path.join(__dirname, 'tnvr_records.txt');
const CPP_SOURCE = path.join(__dirname, 'main.cpp');
const CPP_BINARY = path.join(__dirname, 'tnvr_manager.exe');

let engineStatus = {
  mode: 'Simulation Fallback',
  compilerChecked: false,
  binaryReady: false,
  logs: []
};

// Log helper
function logMessage(msg) {
  const timestamp = new Date().toLocaleTimeString();
  engineStatus.logs.unshift(`[${timestamp}] ${msg}`);
  if (engineStatus.logs.length > 50) engineStatus.logs.pop();
  console.log(`[TNVR Engine] ${msg}`);
}

// 1. Attempt Compilation on Startup
function tryCompileCpp() {
  logMessage('Checking for C++ compilers...');
  
  // Try g++ first
  try {
    execSync('g++ --version', { stdio: 'ignore' });
    logMessage('g++ detected. Compiling main.cpp...');
    execSync(`g++ -std=c++14 "${CPP_SOURCE}" -o "${CPP_BINARY}"`);
    engineStatus.mode = 'C++ Binary (g++)';
    engineStatus.binaryReady = true;
    logMessage('C++ compilation successful.');
    return;
  } catch (err) {
    logMessage('g++ compiler not found or failed to compile.');
  }

  // Check if compiled binary already exists (pre-compiled)
  if (fs.existsSync(CPP_BINARY)) {
    engineStatus.mode = 'C++ Binary (Existing)';
    engineStatus.binaryReady = true;
    logMessage('Found pre-compiled tnvr_manager.exe binary.');
  } else {
    engineStatus.mode = 'Simulation Fallback';
    engineStatus.binaryReady = false;
    logMessage('Operating in simulated fallback mode (using pure JS engine).');
  }
}

// 2. Simulated JS Backend Logic (Fallback)
function getSimulatedRecords() {
  if (!fs.existsSync(DB_FILE)) {
    // Seed defaults
    const defaults = [
      'Dog,DG-101,24,Healthy,1,1,Medium',
      'Cat,CT-302,12,Injured,0,1,1'
    ].join('\n') + '\n';
    fs.writeFileSync(DB_FILE, defaults, 'utf8');
  }
  
  const content = fs.readFileSync(DB_FILE, 'utf8');
  return content.split('\n').filter(Boolean).map(line => {
    const tokens = line.split(',');
    return {
      species: tokens[0],
      tagNumber: tokens[1],
      age: parseInt(tokens[2], 10),
      healthStatus: tokens[3],
      isNeutered: tokens[4] === '1',
      isVaccinated: tokens[5] === '1',
      extra: tokens[6]
    };
  });
}

function saveSimulatedRecords(records) {
  const lines = records.map(r => {
    const neutered = r.isNeutered ? '1' : '0';
    const vaccinated = r.isVaccinated ? '1' : '0';
    return `${r.species},${r.tagNumber},${r.age},${r.healthStatus},${neutered},${vaccinated},${r.extra}`;
  });
  fs.writeFileSync(DB_FILE, lines.join('\n') + '\n', 'utf8');
}

// Helper to execute commands via C++ Process (or simulated fallback)
function executeCommand(inputCommand, callback) {
  if (!engineStatus.binaryReady) {
    // FALLBACK SIMULATOR
    logMessage(`[SIMULATOR] Processing: ${inputCommand.trim()}`);
    try {
      const records = getSimulatedRecords();
      const parts = inputCommand.trim().split(/\s+/);
      const cmd = parts[0];

      if (cmd === 'LIST') {
        const listOutput = records.map(r => {
          const neutered = r.isNeutered ? '1' : '0';
          const vaccinated = r.isVaccinated ? '1' : '0';
          return `${r.species},${r.tagNumber},${r.age},${r.healthStatus},${neutered},${vaccinated},${r.extra}`;
        }).join('\n');
        return callback(null, `[START_LIST]\n${listOutput}\n[END_LIST]`);
      } 
      
      if (cmd === 'ADD') {
        const species = parts[1];
        const tag = parts[2];
        const age = parseInt(parts[3], 10);
        let health = parts[4].replace(/_/g, ' ');
        const neutered = parts[5] === '1';
        const vaccinated = parts[6] === '1';
        const extra = parts[7];

        const index = records.findIndex(r => r.tagNumber === tag);
        const newRecord = { species, tagNumber: tag, age, healthStatus: health, isNeutered: neutered, isVaccinated: vaccinated, extra };
        if (index > -1) {
          records[index] = newRecord;
        } else {
          records.push(newRecord);
        }
        saveSimulatedRecords(records);
        return callback(null, `[SUCCESS] ${species} added`);
      }

      if (cmd === 'DELETE') {
        const tag = parts[1];
        const initialLen = records.length;
        const filtered = records.filter(r => r.tagNumber !== tag);
        saveSimulatedRecords(filtered);
        if (filtered.length < initialLen) {
          return callback(null, `[SUCCESS] Record deleted`);
        } else {
          return callback(null, `[ERROR] Tag not found`);
        }
      }

      if (cmd === 'UPDATE') {
        const tag = parts[1];
        let health = parts[2].replace(/_/g, ' ');
        const neutered = parts[3] === '1';
        const vaccinated = parts[4] === '1';

        const record = records.find(r => r.tagNumber === tag);
        if (record) {
          record.healthStatus = health;
          record.isNeutered = neutered;
          record.isVaccinated = vaccinated;
          saveSimulatedRecords(records);
          return callback(null, `[SUCCESS] Record updated`);
        } else {
          return callback(null, `[ERROR] Tag not found`);
        }
      }

      return callback(new Error(`Unknown cmd: ${cmd}`));
    } catch (e) {
      return callback(e);
    }
  }

  // C++ EXECUTABLE SUBPROCESS
  logMessage(`[C++] Spawning subprocess for: ${inputCommand.trim()}`);
  const child = spawn(CPP_BINARY);

  let stdoutData = '';
  let stderrData = '';

  child.stdout.on('data', (data) => {
    stdoutData += data.toString();
  });

  child.stderr.on('data', (data) => {
    stderrData += data.toString();
  });

  child.on('close', (code) => {
    if (code !== 0) {
      logMessage(`C++ Process closed with exit code ${code}. Error: ${stderrData}`);
      return callback(new Error(`C++ exited with code ${code}`));
    }
    callback(null, stdoutData);
  });

  // Write commands: the input command, followed by SAVE and EXIT to sync files
  child.stdin.write(`${inputCommand}\nSAVE\nEXIT\n`);
  child.stdin.end();
}

// 3. API Routes
app.get('/api/status', (req, res) => {
  res.json(engineStatus);
});

app.get('/api/records', (req, res) => {
  executeCommand('LIST', (err, output) => {
    if (err) {
      logMessage(`Failed to read records: ${err.message}`);
      return res.status(500).json({ error: 'Failed to retrieve database records' });
    }
    
    // Parse list from C++ output format
    const lines = output.split('\n');
    const records = [];
    let isList = false;

    for (let line of lines) {
      line = line.trim();
      if (line === '[START_LIST]') {
        isList = true;
        continue;
      }
      if (line === '[END_LIST]') {
        isList = false;
        continue;
      }
      if (isList && line) {
        const tokens = line.split(',');
        records.push({
          species: tokens[0],
          tagNumber: tokens[1],
          age: parseInt(tokens[2], 10),
          healthStatus: tokens[3],
          isNeutered: tokens[4] === '1',
          isVaccinated: tokens[5] === '1',
          extra: tokens[6]
        });
      }
    }
    res.json(records);
  });
});

app.post('/api/records', (req, res) => {
  const { species, tagNumber, age, healthStatus, isNeutered, isVaccinated, extra } = req.body;
  if (!species || !tagNumber || !age || !healthStatus) {
    return res.status(400).json({ error: 'Missing required parameters' });
  }

  // C++ expects healthStatus spaces to be underscores
  const formattedHealth = healthStatus.replace(/\s+/g, '_');
  const neuteredFlag = isNeutered ? '1' : '0';
  const vaccinatedFlag = isVaccinated ? '1' : '0';
  const formattedExtra = extra || (species === 'Cat' ? '0' : 'Medium');

  const cmd = `ADD ${species} ${tagNumber} ${age} ${formattedHealth} ${neuteredFlag} ${vaccinatedFlag} ${formattedExtra}`;
  
  executeCommand(cmd, (err, output) => {
    if (err) {
      logMessage(`Error adding animal: ${err.message}`);
      return res.status(500).json({ error: 'Failed to add animal' });
    }
    logMessage(`Add animal response: ${output.trim()}`);
    res.json({ message: 'Animal saved successfully' });
  });
});

app.put('/api/records/:tag', (req, res) => {
  const { healthStatus, isNeutered, isVaccinated } = req.body;
  const tag = req.params.tag;

  const formattedHealth = healthStatus.replace(/\s+/g, '_');
  const neuteredFlag = isNeutered ? '1' : '0';
  const vaccinatedFlag = isVaccinated ? '1' : '0';

  const cmd = `UPDATE ${tag} ${formattedHealth} ${neuteredFlag} ${vaccinatedFlag}`;

  executeCommand(cmd, (err, output) => {
    if (err) {
      logMessage(`Error updating animal: ${err.message}`);
      return res.status(500).json({ error: 'Failed to update animal' });
    }
    logMessage(`Update response: ${output.trim()}`);
    if (output.includes('[ERROR]')) {
      return res.status(404).json({ error: 'Tag number not found' });
    }
    res.json({ message: 'Animal updated successfully' });
  });
});

app.delete('/api/records/:tag', (req, res) => {
  const tag = req.params.tag;
  const cmd = `DELETE ${tag}`;

  executeCommand(cmd, (err, output) => {
    if (err) {
      logMessage(`Error deleting animal: ${err.message}`);
      return res.status(500).json({ error: 'Failed to delete animal' });
    }
    logMessage(`Delete response: ${output.trim()}`);
    if (output.includes('[ERROR]')) {
      return res.status(404).json({ error: 'Tag number not found' });
    }
    res.json({ message: 'Animal deleted successfully' });
  });
});

// Start Server & compile C++ engine
app.listen(PORT, () => {
  logMessage(`Server listening on http://localhost:${PORT}`);
  tryCompileCpp();
});
