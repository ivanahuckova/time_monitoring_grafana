#include <Arduino.h>
#include <PromLokiTransport.h>
#include <GrafanaLoki.h>

#include "certificates.h"
#include "config_secret.h" 

// Create a transport and client object for sending our data
PromLokiTransport transport;
LokiClient client(transport);


LokiStream tf(2, 150, "{job=\"timefidget\",id=\""ID"\"}");
LokiStreams streams(1,1024);

const char* id = ID;
const char* formatString = "id=\"%s\" type=add pos=%s project=\"%s\"";

void setupClient() {
  Serial.println("Setting up client...");

  uint8_t serialTimeout;
  while (!Serial && serialTimeout < 50) {
    delay(100);
    serialTimeout++;
  }
  
  // Configure and start the transport layer
  transport.setUseTls(true);
  transport.setCerts(grafanaCert, strlen(grafanaCert));
  transport.setWifiSsid(WIFI_SSID);
  transport.setWifiPass(WIFI_PASSWORD);
  transport.setDebug(Serial);  // Remove this line to disable debug logging of the client.
  if (!transport.begin()) {
      Serial.println(transport.errmsg);
      while (true) {};
  }

  // Configure the client
  client.setUrl(GC_LOKI_URL);
  client.setPath(GC_LOKI_PATH);
  client.setPort(GC_PORT);
  client.setUser(GC_LOKI_USER);
  client.setPass(GC_LOKI_PASS);
  client.setDebug(Serial);  // Remove this line to disable debug logging of the client.
  if (!client.begin()) {
      Serial.println(client.errmsg);
      while (true) {};
  }

  // Add our stream objects to the streams object
  streams.addStream(tf);
  streams.setDebug(Serial);  // Remove this line to disable debug logging of the write request serialization and compression.
}

void sendToLoki(const char* pos, const char* projectName) {
  char str1[100];
  snprintf(str1, 100, formatString, id, pos, projectName);
  if (!tf.addEntry(client.getTimeNanos(), str1, strlen(str1))) {
    Serial.println(tf.errmsg);
  }
  Serial.print("Sending Project: ");
  Serial.println(projectName);
  LokiClient::SendResult res = client.send(streams);
  if (res != LokiClient::SendResult::SUCCESS) {
    Serial.println("Failed to send to Loki");
    if (client.errmsg) {
      Serial.println(client.errmsg);
    }
    if (transport.errmsg) {
      Serial.println(transport.errmsg);
    }
  }
  // Reset Streams
  tf.resetEntries();
}
 
void setup() {
    Serial.begin(115200);
    setupClient();
}
 
void loop() {
    int z = analogRead(ZPIN); //read from zpin
    int y = analogRead(YPIN); //read from zpin
    int x = analogRead(XPIN); //read from zpin
   if (y < 1600 ) {
    // We don't want to log anything
   } else {
       if (x > 1500 && x < 2000 && z < 1700 ) {
           Serial.println(P1);
           sendToLoki("1", P1);
       } else if (x > 2000 && z < 1800) {
           Serial.println(P2);
           sendToLoki("2", P2);
       } else if (x > 2100 && z < 2250) {
           Serial.println(P3);
           sendToLoki("3", P3);
       } else if (x > 1800 && x < 2100 && z > 2150) {
           Serial.println(P4);
           sendToLoki("4", P4);
       } else if (x < 1800 && z > 1900 && z < 2200) {
           Serial.println(P5);
           sendToLoki("5", P5);
       } else if (x < 1700 && z < 2000) {
           Serial.println(P6);
           sendToLoki("6", P6);
       }
   }


    delay(INTERVAL * 1000); //wait for 1 second
}
