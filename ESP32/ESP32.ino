/*
 *  This sketch sends random data over UDP on a ESP32 device
 *
 */
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

// WiFi network credentials, udp server ip
const char * networkName = "Polyhack";
const char * networkPswd = "Polyhack";

//------------------------------------------------------------------
const char * udpAddress = "10.42.0.55";
const int udpPort = 41234;
boolean connected = false;
WiFiUDP udp;
int x2,y2,z2 = 0;
//------------------------------------------------------------------

const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

//------------------------------------------------------------------

#define SERVO_LOW 88
//#define SERVO_HIGH 8888
#define SERVO_HIGH 8800
#define TIMER_WIDTH 16

//------------------------------------------------------------------

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(WiFi.localIP(),udpPort);
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
    }
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}



void setupIMU(){
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(115200);

  delay(10);
}

void setupWiFi(){
  delay(1000);
  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
}

void setupServo() {
   ledcSetup(1, 50, TIMER_WIDTH); // channel 1, 50 Hz, 16-bit width
   ledcAttachPin(2, 1);   // GPIO 22 assigned to channel 1
   ledcAttachPin(4, 2);
   ledcAttachPin(5, 3);
}
void writeServo(int servo, float pos){
  ledcWrite(servo, int((.5*pos+.5)*float((SERVO_HIGH-SERVO_LOW)))+SERVO_LOW);
}


void updateIMU(){
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
}

void sendPacket(int x, int y, int z){
  //only send data when connected
  if(connected){
    //Send a packet
    udp.beginPacket(udpAddress,udpPort);
    //udp.printf("Seconds since boot: %u", millis()/1000);
    //udp.printf(formCoordPack(1,1,-1),7);
    uint8_t* packet = new uint8_t[7];
    packet[0] = 'x';
    packet[1] = uint8_t( (x&0xFF00)>>8 );
    packet[2] = uint8_t( x&0x00FF );
    packet[3] = uint8_t( (y&0xFF00)>>8 );
    packet[4] = uint8_t( y&0x00FF );
    packet[5] = uint8_t( (z&0xFF00)>>8 );
    packet[6] = uint8_t( z&0x00FF );
    udp.write(packet,7);
    udp.endPacket();
    //char* buff = new char();
    //udp.read(buff,1);
    //Serial.println(buff);
  }
}

void sendHeartbeat(){
  //only send data when connected
  if(connected){
    //Send a packet
    udp.beginPacket(udpAddress,udpPort);
    udp.endPacket();
    //char* buff = new char();
    //udp.read(buff,1);
    //Serial.println(buff);
  }
}

void handlePacket(){
  if(int len = udp.parsePacket()){
      uint8_t* buff = new uint8_t[len]();
      udp.read(buff,len);
      x2 = int16_t((buff[1]<<8)|buff[2]);
      y2 = int16_t((buff[3]<<8)|buff[4]);
      z2 = int16_t((buff[5]<<8)|buff[6]);
      //Serial.println((int) buff[0]);
      Serial.print(x2);
      Serial.print(", ");
      Serial.print(y2);
      Serial.print(", ");
      Serial.println(z2);
      Serial.print(len);
      udp.flush();
      delete [] buff;
   }
}

void setup(){
   Serial.begin(115200);
   delay(500);
   setupWiFi();
   delay(200);
   setupIMU();
   setupServo();
   sendPacket(0,0,-1);
}
float clamp(float val){
  if(val>1) return 1;
  if(val<-1) return -1;
  return val;
}
void loop(){
  //sendPacket(AcX,AcY,AcZ);
  //updateIMU();
  sendHeartbeat();
  handlePacket();
  float anglex = atan2(y2,z2);
  float angley = atan2(x2,z2);
  float anglez = atan2(x2,y2);
  float mag = sqrt(y2*y2+x2*x2+z2*z2);
  
  writeServo(1,clamp(float(x2)/mag)*.9);
  writeServo(2,clamp(float(y2)/mag)*.9);
  writeServo(3,clamp(float(z2)/mag)*.9);
  //Wait for 1 second
  delay(50);
}




