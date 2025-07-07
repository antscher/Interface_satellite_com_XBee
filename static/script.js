const connectForm = document.getElementById('connect-form');
const captureButton = document.getElementById('capture-button');
const abortButton = document.getElementById('abort-button');
const photo = document.getElementById('photo');

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


