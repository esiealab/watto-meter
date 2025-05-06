function sendRequest(url) {
    fetch(url).then(() => location.reload());
}

function updateDeviceAndToggleMeasure(event) {
    event.preventDefault();
    const deviceName = document.getElementById('deviceName').value;
    fetch(`/setDevice?device=${encodeURIComponent(deviceName)}`)
        .then(() => {
            // After setting the device, toggle measurement
            const button = document.getElementById('measureButton');
            const isMeasuring = button.classList.contains('stop');
            const url = isMeasuring ? '/stopMeasures' : '/startMeasures';
            return fetch(url);
        })
        .then(() => location.reload());
}