function sendRequest(url) {
    fetch(url).then(() => location.reload());
}

function updateDeviceAndToggleMeasure(event) {
    event.preventDefault();
    const deviceName = document.getElementById('deviceName').value;
    const button = document.getElementById('measureButton');
    const isMeasuring = button.classList.contains('stop');
    const url = isMeasuring ? '/stopMeasures' : `/startMeasures?device=${encodeURIComponent(deviceName)}`;
    return fetch(url).then(() => location.reload());
}