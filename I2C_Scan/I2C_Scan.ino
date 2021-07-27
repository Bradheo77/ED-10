#include <HardwareSerial.h>
#include <Wire.h>
 
void setup() {
    Wire.begin(32, 33);
    Serial.begin(115200);
    while(!Serial){
    ;
    }
    delay(3000);
    Serial.println("Program start...");  
    delay(3000);
}
 
void loop() {
    int statusTransmission;
    int flag = 1; 
    Serial.println("I2C Device Scanning....");
    
    delay(100);
    for(int i=0 ; i<127 ; i++)
    {
        Wire.beginTransmission(i);
        /* Slave 로 부터 정상적으로 ACK 가 도착하면 Wire.endTransmission() 이 '0'를 반환*/
        statusTransmission = Wire.endTransmission();
        if(statusTransmission == 0)
        {
            Serial.print("Connected I2C Device Address : 0x");
            if(i < 16)
                Serial.print("0");
            Serial.println(i,HEX);
            flag = 0;
        }
    }
    if(flag == 1){
        Serial.println("No I2C devices found");
    }
    Serial.println("Program End...");
    while(1);
}
