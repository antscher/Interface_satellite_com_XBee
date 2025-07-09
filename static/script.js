const connectForm = document.getElementById('connect-form');
const captureButton = document.getElementById('capture-button');
const abortButton = document.getElementById('abort-button');
const photo = document.getElementById('photo');
const unlinkButton = document.getElementById('unlink-button');
const refreshBtns = document.querySelectorAll('.refresh-btn');

connectForm.addEventListener('submit', function (e) {
  e.preventDefault();
  const port = document.getElementById('port').value;

  fetch('/connect', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ port: port })
  })
    .then(res => res.json())
    .then(data => {
      alert(data.message || (data.success ? 'Connected!' : 'Failed to connect'));
      if (data.success) {
        captureButton.disabled = false;
        unlinkButton.disabled = false;
        document.getElementById('port').disabled = true;
        // Enable refresh buttons
        refreshBtns.forEach(btn => btn.disabled = false);
      }
    });
});

captureButton.addEventListener('click', function () {
  captureButton.disabled = true;
  abortButton.disabled = false;

  fetch('/capture')
    .then(res => res.json())
    .then(data => {
      if (data.success && data.filename) {
        photo.src = '/static/' +  data.filename + '?t=' + new Date().getTime();
        photo.style.display = 'block';
      } else {
        alert(data.message || 'Capture failed');
      }
    })
    .catch(err => {
      alert("Error: " + err.message);
    })
    .finally(() => {
      captureButton.disabled = false;
      abortButton.disabled = true;
    });
});

abortButton.addEventListener('click', function () {
  fetch('/abort')
    .then(() => {
      alert("Capture aborted.");
      captureButton.disabled = false;
      abortButton.disabled = true;
    });
});

unlinkButton.addEventListener('click', function () {
  fetch('/disconnect', {
    method: 'POST'
  })
    .then(res => res.json())
    .then(data => {
      alert(data.message || (data.success ? 'Disconnected!' : 'Failed to disconnect'));
      if (data.success) {
        captureButton.disabled = true;
        unlinkButton.disabled = true;
        abortButton.disabled = true;
        document.getElementById('port').disabled = false;
        photo.style.display = 'none';
        // Disable refresh buttons
        refreshBtns.forEach(btn => btn.disabled = true);
      }
    });
});

function updateSensorData() {
  fetch('/sensor_data')
    .then(response => response.json())
    .then(data => {
      document.getElementById('temp-value').textContent = data.temperature + " °C";
      document.getElementById('humidity-value').textContent = data.humidity + " %";
      document.getElementById('pressure-value').textContent = data.pressure + " hPa";

      document.getElementById('acc-x').textContent = data.acc_x;
      document.getElementById('acc-y').textContent = data.acc_y;
      document.getElementById('acc-z').textContent = data.acc_z;

      document.getElementById('gyro-x').textContent = data.gyro_x;
      document.getElementById('gyro-y').textContent = data.gyro_y;
      document.getElementById('gyro-z').textContent = data.gyro_z;
    });
}

// Appel toutes les secondes
setInterval(updateSensorData, 1000);

// Optionnel : appel au chargement
updateSensorData();


document.getElementById("uplink-command").addEventListener("input", function () {
  const value = this.value.trim();
  document.getElementById("uplink-button").disabled = value.length === 0;
});

document.getElementById("uplink-button").addEventListener("click", function () {
  const input = document.getElementById("uplink-command").value.trim();

  fetch("/uplink", {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify({ command: input })
  })
  .then(response => response.json())
  .then(data => {
    if (data.success) {
      alert("✅ Commande envoyée avec succès !");
    } else {
      alert("❌ Erreur : " + data.error);
    }
  });
});

// Define command mapping as in app.py
const CMD_MAP = {
  "TAKE_PICTURE": "0x13 0xAA 0xB0 0x01",
  "REFRESH_15":   "0x13 0xAA 0x15 0x01",
  "REFRESH_30":   "0x13 0xAA 0x30 0x01",
  "REFRESH_45":   "0x13 0xAA 0x45 0x01",
  "REFRESH_60":   "0x13 0xAA 0x60 0x01",
  "NO_REFRESH":   "0x13 0xAA 0x00 0x01"
};

// Add event listeners for refresh rate buttons
refreshBtns.forEach(btn => {
  btn.addEventListener('click', function () {
    let cmdKey = btn.getAttribute('data-cmd');
    let cmd = CMD_MAP[cmdKey] || cmdKey;
    fetch('/uplink', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ command: cmd })
    })
      .then(res => res.json())
      .then(data => {
        if (data.success) {
          alert('Refresh rate command sent!');
        } else {
          alert(data.error || 'Failed to send refresh command');
        }
      });
  });
});
