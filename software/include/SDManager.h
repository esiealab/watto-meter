#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"

class SDManager {
public:
    SDManager();
    bool begin(int csPin = -1, int sck = -1, int miso = -1, int mosi = -1);
    void listDir(const char *dirname, uint8_t levels);
    void createDir(const char *path);
    void removeDir(const char *path);
    void readFile(const char *path);
    void writeFile(const char *path, const char *message);
    void appendFile(const char *path, const char *message);
    void renameFile(const char *path1, const char *path2);
    void deleteFile(const char *path);

private:
    fs::SDFS &fs;
};

#endif // SD_MANAGER_H