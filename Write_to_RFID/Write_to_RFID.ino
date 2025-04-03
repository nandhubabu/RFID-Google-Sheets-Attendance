#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN  22  // Reset pin (Change as per your wiring)
#define SS_PIN   5   // SDA/SS pin (Change as per your wiring)

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create RFID instance
MFRC522::MIFARE_Key key;

// Define the block to write (Block 2 in sector 0)
int blockNum = 2;

// Data to be written (must be exactly 16 bytes)
byte blockData[16] = {"Red-eye        "}; // 16 bytes (7 + 9 spaces) // Ensure it is **exactly** 16 bytes

// Buffer to store read data
byte bufferLen = 18; // Buffer size must be **18 bytes** (16 + CRC)
byte readBlockData[18];

MFRC522::StatusCode status;

void setup() {
    Serial.begin(115200);
    SPI.begin(); 
    mfrc522.PCD_Init();
    Serial.println("Scan an RFID tag to write data...");

    // Set default authentication key (all F's)
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
}

void loop() {
    // Check if a new card is detected
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial()) return;

    Serial.println("\n**Card Detected**");

    // Print UID of the card
    Serial.print("Card UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();

    // Print card type
    Serial.print("PICC type: ");
    Serial.println(mfrc522.PICC_GetTypeName(mfrc522.PICC_GetType(mfrc522.uid.sak)));

    // Write data
    Serial.println("Writing to RFID block...");
    WriteDataToBlock(blockNum, blockData);

    // Read back the data
    Serial.println("Reading back from RFID block...");
    ReadDataFromBlock(blockNum, readBlockData);

    // Print the read data
    Serial.print("Data in Block ");
    Serial.print(blockNum);
    Serial.print(": ");
    for (int j = 0; j < 16; j++) {
        Serial.write(readBlockData[j]); // Print as ASCII
    }
    Serial.println();

    // Halt the card to prevent continuous re-reading
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

void WriteDataToBlock(int blockNum, byte blockData[]) {
    // Authenticate block
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Auth failed for Write: ");
        Serial.println(mfrc522.GetStatusCodeName(status));

        Serial.println("Data written: ");
        for (int i = 0; i < 16; i++) {
            Serial.write(blockData[i]);
        }
        Serial.println();

        return;
    }

    // Write data
    status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Writing failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    Serial.println("Write success!");
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
    // Authenticate block
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Auth failed for Read: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    // Read data
    status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Reading failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    Serial.println("Read success!");
}
