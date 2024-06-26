

#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"

//v1.0

//v1.1
//Add microSD card test


HardwareSerial mySerial2(2);

#define IO_RXD2 47
#define IO_TXD2 48

#define IO_GSM_PWRKEY 4
#define IO_GSM_RST    5


/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32-S3
 *    D2      not used
 *    D3       not used
 *    CMD      11
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      12
 *    VSS      GND
 *    D0       13
 *    D1       not used
 */
//microSD/TF card in SDIO interface, 1 bit mode. we just need 3 PINs to drive sd/mmc

#define IO_SD_CMD    11
#define IO_SD_CLK    12
#define IO_SD_D0     13


void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}




void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print(F("Hello! ESP32-S3 AT command V1.1 Test"));
  mySerial2.begin(115200,SERIAL_8N1, IO_RXD2, IO_TXD2);
  pinMode(IO_GSM_RST, OUTPUT);
  digitalWrite(IO_GSM_RST, LOW);
  
  pinMode(IO_GSM_PWRKEY, OUTPUT);
  digitalWrite(IO_GSM_PWRKEY, HIGH);
  delay(3000);
  SD_MMC.setPins(IO_SD_CLK, IO_SD_CMD, IO_SD_D0);
  digitalWrite(IO_GSM_PWRKEY, LOW);

  delay(2000);

  int i = 0;
  Serial.println("\nTesting Module Response...\n");
  Serial.println("****");
  while (i) {
        mySerial2.println("AT");
        delay(500);
        if (mySerial2.available()) {
            String r = mySerial2.readString();
            Serial.println(r);
            if ( r.indexOf("OK") >= 0 ) {
                //reply = true;
                break;;
            }
            else
            {
              digitalWrite(IO_GSM_PWRKEY, HIGH);
              delay(3000);
              digitalWrite(IO_GSM_PWRKEY, LOW);
            }
        }
        delay(500);
        i--;
    }
    Serial.println("****\n");

//microSD card
   //bool begin(const char * mountpoint="/sdcard", bool mode1bit=false, bool format_if_mount_failed=false, int sdmmc_frequency=BOARD_MAX_SDMMC_FREQ, uint8_t maxOpenFiles = 5);
    if(!SD_MMC.begin("/sdcard", true, true)){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD_MMC.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD_MMC card attached");
        return;
    }

    Serial.print("SD_MMC Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);
    delay(1000);

}

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial.available() > 0) {
    mySerial2.write(Serial.read());
    yield();
  }
  while (mySerial2.available() > 0) {
    Serial.write(mySerial2.read());
    yield();
  }
}