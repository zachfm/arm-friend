/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */
#include <WiFi.h>
#include "esp32-hal-ledc.h"

const char* ssid     = "Polyhack";
const char* password = "Polyhack";

const char* host = "polyhack.herokuapp.com";

const char* streamId   = "4JdKDkegZaClE1DKLDe6C2joVbl";
const char* privateKey = "qEGRbajXqBSl372pj2zaCw0qaVZ";

#define COUNT_LOW 0
#define COUNT_HIGH 8888
#define TIMER_WIDTH 16
int i = COUNT_LOW;

void setupServo() {
   ledcSetup(1, 50, TIMER_WIDTH); // channel 1, 50 Hz, 16-bit width
   ledcAttachPin(2, 1);   // GPIO 22 assigned to channel 1
}

void setup()
{
    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    setupServo();
}

int value = 0;

void loop()
{
    //delay(50);
    ++value;

    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // We now create a URI for the request
    /*String url = "/input/";
    url += streamId;
    url += "?private_key=";
    url += privateKey;
    url += "&value=";
    url += value;*/
    String url = "/";
    /*
    Serial.print("Requesting URL: ");
    Serial.println(url);
    */
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");
    /*
    ledcWrite(1, i);
    if(i<COUNT_HIGH){
      i+=100;
    }else{
      i=COUNT_LOW;
    }
    */
    
    for (int i=COUNT_LOW ; i < COUNT_HIGH ; i=i+100)
    {
      ledcWrite(1, i);       // sweep servo 1
      //delay(50);
    }
}

/*
void loop() {
   for (int i=COUNT_LOW ; i < COUNT_HIGH ; i=i+100)
   {
      ledcWrite(1, i);       // sweep servo 1
      delay(50);
   }
}
*/





