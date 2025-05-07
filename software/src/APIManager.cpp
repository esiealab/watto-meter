#include "APIManager.h"

APIManager::APIManager(int port, String mainFolder)
    : server(port), startMeasures(false), device("MyPhone"), mainFolder(mainFolder) {
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
}

void APIManager::begin() {
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) { handleRoot(request); });
    server.on("/startMeasures", HTTP_GET, [this](AsyncWebServerRequest *request) { handleStartMeasures(request); });
    server.on("/stopMeasures", HTTP_GET, [this](AsyncWebServerRequest *request) { handleStopMeasures(request); });
    server.on("/setDevice", HTTP_GET, [this](AsyncWebServerRequest *request) { handleSetDevice(request); });
    server.on("/downloadFile", HTTP_GET, [this](AsyncWebServerRequest *request) { handleDownloadFile(request); });
    server.on("/deleteFile", HTTP_GET, [this](AsyncWebServerRequest *request) { handleDeleteFile(request); });
    server.on("/listFiles", HTTP_GET, [this](AsyncWebServerRequest *request) { handleListCsvFiles(request); });

    server.serveStatic("/", SPIFFS, "/");

    server.begin();
    Serial.println("API server started");
}

void APIManager::handleRoot(AsyncWebServerRequest *request) {
    String path = "/index.html";
    File file = SPIFFS.open(path);
    if (!file) {
        request->send(500, "text/plain", "Failed to load HTML file");
        return;
    }

    // Lire le contenu du fichier HTML
    String html = "";
    while (file.available()) {
        html += char(file.read());
    }
    file.close();

    // Générer la liste des fichiers
    String fileList = "";
    File root = SD.open("/");
    File currentFile = root.openNextFile();
    while (currentFile) {
        String fileName = currentFile.name();
        if (fileName.endsWith(".csv")) {  // Only include .csv files
            fileList += "<li class='list-group-item d-flex justify-content-between align-items-center'>";
            fileList += "<span style='word-break: break-word;'>" + fileName + "</span>";  // Allow long file names to wrap
            fileList += "<div style='flex-shrink: 0;'>";
            fileList += "<a href='/downloadFile?file=" + fileName + "' class='btn btn-success btn-sm me-2'><i class=\"fa-solid fa-download\"></i></a>";
            fileList += "<a href='/deleteFile?file=" + fileName + "' class='btn btn-danger btn-sm' onclick=\"return confirm('Are you sure?')\"><i class=\"fa-solid fa-trash-can\"></i></a>";
            fileList += "</div>";
            fileList += "</li>";
        }
        currentFile = root.openNextFile();
    }

    // Remplacer les placeholders par les valeurs dynamiques
    html.replace("{{WIFI_SSID}}", WiFi.SSID());
    html.replace("{{WIFI_IP}}", WiFi.localIP().toString());
    html.replace("{{DEVICE_NAME}}", device);
    html.replace("{{MEASURING_CLASS}}", startMeasures ? "stop" : "start");
    html.replace("{{MEASURING_TEXT}}", startMeasures ? "Stop Measures" : "Start Measures");
    html.replace("{{FILE_LIST}}", fileList);

    // Envoyer le contenu HTML au client
    request->send(200, "text/html", html);
}

void APIManager::handleStartMeasures(AsyncWebServerRequest *request) {
    startMeasures = true;

    // Vérifier si le paramètre "device" est présent
    if (request->hasArg("device")) {
        device = request->arg("device");
        Serial.println("Device name updated to: " + device);
    }

    // Répondre avec un message indiquant que les mesures ont commencé
    String responseMessage = "Measurements started";
    if (!device.isEmpty()) {
        responseMessage += " for device: " + device;
    }
    request->send(200, "text/plain", responseMessage);
}

void APIManager::handleStopMeasures(AsyncWebServerRequest *request) {
    startMeasures = false;
    request->send(200, "text/plain", "Measurements stopped");
}

void APIManager::handleSetDevice(AsyncWebServerRequest *request) {
    if (request->hasArg("device")) {
        device = request->arg("device");
        request->send(200, "text/plain", "Device name set to " + device);
    } else {
        request->send(400, "text/plain", "Device name not provided");
    }
}

void APIManager::handleDownloadFile(AsyncWebServerRequest *request) {
    if (!request->hasArg("file")) {
        request->send(400, "text/plain", "File name not provided");
        return;
    }

    String fileName = request->arg("file");
    Serial.println("Downloading file: " + fileName);
    File file = SD.open(mainFolder + fileName, FILE_READ);
    if (!file) {
        request->send(404, "text/plain", "File not found");
        return;
    }

    // Utiliser la méthode send pour envoyer le fichier en tant que flux asynchrone
    AsyncWebServerResponse *response = request->beginResponse(SD, mainFolder + fileName, "application/octet-stream");
    response->addHeader("Content-Disposition", "attachment; filename=" + fileName);
    request->send(response);
    file.close();
}

void APIManager::handleDeleteFile(AsyncWebServerRequest *request) {
    if (!request->hasArg("file")) {
        request->send(400, "text/plain", "File name not provided");
        return;
    }

    String fileName = request->arg("file");
    if (!SD.exists(mainFolder + fileName)) {
        request->send(404, "text/plain", "File not found");
        return;
    }

    if (SD.remove(mainFolder + fileName)) {
        // Créer une réponse asynchrone pour rediriger vers la page principale après suppression
        AsyncWebServerResponse *response = request->beginResponse(303);
        response->addHeader("Location", "/");
        response->setContentLength(0);
        request->send(response);
    } else {
        request->send(500, "text/plain", "Failed to delete file");
    }
}

void APIManager::handleListCsvFiles(AsyncWebServerRequest *request) {
    File root = SD.open("/");
    if (!root || !root.isDirectory()) {
        request->send(500, "application/json", "{\"error\":\"Failed to open directory\"}");
        return;
    }

    String json = "[";
    bool firstFile = true;

    File file = root.openNextFile();
    while (file) {
        String fileName = file.name();
        if (fileName.endsWith(".csv")) {
            if (!firstFile) {
                json += ",";
            }
            firstFile = false;

            // Ajouter les informations du fichier au JSON
            json += "{";
            json += "\"name\":\"" + fileName + "\",";
            json += "\"size\":" + String(file.size()) + ",";
            json += "\"timestamp\":\"" + String(file.getLastWrite()) + "\"";
            json += "}";
        }
        file = root.openNextFile();
    }

    json += "]";
    request->send(200, "application/json", json);
}

bool APIManager::isMeasuring() {
    return startMeasures;
}

String APIManager::getDeviceName() {
    return device;
}