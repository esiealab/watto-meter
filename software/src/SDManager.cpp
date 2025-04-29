#include "SDManager.h"

SDManager::SDManager() : fs(SD) {}

bool SDManager::begin(int csPin, int sck, int miso, int mosi) {
#ifdef REASSIGN_PINS
    SPI.begin(sck, miso, mosi, csPin);
    return fs.begin(csPin);
#else
    return fs.begin();
#endif
}

void SDManager::listDir(const char *dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\n", dirname);
    File root = fs.open(dirname);
    if (!root || !root.isDirectory()) {
        Serial.println("Failed to open directory or not a directory");
        return;
    }
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.printf("  DIR : %s\n", file.name());
            if (levels) listDir(file.path(), levels - 1);
        } else {
            Serial.printf("  FILE: %s  SIZE: %d\n", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}

void SDManager::createDir(const char *path) {
    Serial.printf("Creating Dir: %s\n", path);
    if (fs.mkdir(path)) Serial.println("Dir created");
    else Serial.println("mkdir failed");
}

void SDManager::removeDir(const char *path) {
    Serial.printf("Removing Dir: %s\n", path);
    if (fs.rmdir(path)) Serial.println("Dir removed");
    else Serial.println("rmdir failed");
}

void SDManager::readFile(const char *path) {
    Serial.printf("Reading file: %s\n", path);
    File file = fs.open(path);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }
    while (file.available()) Serial.write(file.read());
    file.close();
}

void SDManager::writeFile(const char *path, const char *message) {
    Serial.printf("Writing file: %s\n", path);
    File file = fs.open(path, FILE_WRITE);
    if (file.print(message)) Serial.println("File written");
    else Serial.println("Write failed");
    file.close();
}

void SDManager::appendFile(const char *path, const char *message) {
    Serial.printf("Appending to file: %s\n", path);
    File file = fs.open(path, FILE_APPEND);
    if (file.print(message)) Serial.println("Message appended");
    else Serial.println("Append failed");
    file.close();
}

void SDManager::renameFile(const char *path1, const char *path2) {
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) Serial.println("File renamed");
    else Serial.println("Rename failed");
}

void SDManager::deleteFile(const char *path) {
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path)) Serial.println("File deleted");
    else Serial.println("Delete failed");
}