#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5    // RFID SS pin (D5)
#define RST_PIN 22  // RFID RST pin
#define BUZZER_PIN 13 // Buzzer pin

MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "WIFI_NAME";  // Replace with your WiFi SSID
const char* password = "PASSWORD";  // Replace with your WiFi password
const char* googleScriptURL = "APP_SCRIPT_LINK"; // Your Google Sheets API URL

void setup() {
    Serial.begin(115200);
    SPI.begin(); 
    rfid.PCD_Init();
    
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
}

void loop() {
    if (!rfid.PICC_IsNewCardPresent()) return;
    if (!rfid.PICC_ReadCardSerial()) return;

    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        uid += String(rfid.uid.uidByte[i], HEX);
    }
    Serial.println("Tag UID: " + uid);

    // Use tone() instead of digitalWrite() for passive buzzer
    tone(BUZZER_PIN, 1000);  // 1000 Hz sound
    delay(1000); 
    noTone(BUZZER_PIN);      // Stop sound

    sendToGoogleSheet(uid);

    delay(5000);
}


void sendToGoogleSheet(String uid) {
    String name = getNameFromUID(uid); // Get name from UID

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(googleScriptURL) + "?name=" + name + "&uid=" + uid;  // Send both Name & UID
        http.begin(url);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            Serial.println("Data sent: " + name + " (" + uid + ")");
        } else {
            Serial.println("Error sending data");
        }

        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }
}

// Function to return a name based on the UID
String getNameFromUID(String uid) {
    if (uid == "132846da") return "Nandhu";
    if (uid == "19282a2") return "Red-eye";
    return "Unknown User";
}
