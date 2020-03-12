// demo: CAN-BUS Shield, receive data with interrupt mode
// when in interrupt mode, the data coming can't be too fast, must >20ms, or else you can use check mode
// loovee, 2014-6-13
/*


------------------------------------------------------------------------------------------------------------
     MSP2515_CAN FIZIKSEL BAGLANTILARI                   
------------------------------------------------------------------------------------------------------------
                    UNO         MEGA
                  -------     --------
         (INT)  *   (2)          (2)
         (SCK)  *   (13)         (52)
        (MOSI)  *   (11)         (51)
        (MISO)  *   (12)         (50)
          (CS)  *   (9)          (9)
         (GND)  *   GND
         (VCC)  *   +5V


*/
#include <SPI.h>
#include "mcp_can.h"
#include <TimerOne.h>


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10

/*
------------------------------------------------------------------------------------------------------------
     MCP2515 AYARLAMALARI                       
------------------------------------------------------------------------------------------------------------
*/
const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN);                   // Set CS pin
unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
unsigned char canId;

/*
------------------------------------------------------------------------------------------------------------
     DIGER DEGISKENLERIN ATAMALARI                       
------------------------------------------------------------------------------------------------------------
*/
int data_select = 0,
    _delay = 300;
String tx_message = "";
boolean key = true;
/*
------------------------------------------------------------------------------------------------------------
     GONDERILECEK VERILERIN DEGISKEN ATAMALARI                       
------------------------------------------------------------------------------------------------------------
*/
String hiz = "";
String x = "", test = "";
String akim = "";
String sicaklik[27];
String batarya[27], bat_toplam = "";
String max_sicaklik = "";
String rfid_aydinlatma[8];
String park = "", rfid = "", aydinlatma[7];



const int total_var_send = 4; //. Uzak bilgisayara gönderilecek toplam değişken sayısı. count() fonksiyonu için.
/*
------------------------------------------------------------------------------------------------------------
     CAN ID ATAMALARI                   
------------------------------------------------------------------------------------------------------------
*/
int sicaklik_id[] = {
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
  0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x07  //. 0x07 > Maximum sicaklik verisi
};

int batarya_id[] = {
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
  0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A  //. 0x4A Toplam batarya gerilimi.
};

int akim_id = 0x81;
int hiz_id = 0x06;
// (0)RFID # (1)Angel # (2)Dortlu # (3)Tavan # (4)Far # (5)Flasor # (6)L_signal # (7)R_Signal
int rfid_aydinlatma_id = 0x60;
int park_id = 0x61;

/*
------------------------------------------------------------------------------------------------------------
     SON                   
------------------------------------------------------------------------------------------------------------




------------------------------------------------------------------------------------------------------------
     SETUP                     
------------------------------------------------------------------------------------------------------------
*/
void setup()
{
    Serial.begin(9600);
    delay(500);
      if(key){
        while (CAN_OK != CAN.begin(CAN_500KBPS)){              // init can bus : baudrate = 500k 
            Serial.println("Init CAN BUS Shield again");
            delay(1000);
        }
        Serial.println("CAN BUS Shield init OK!");
        attachInterrupt(0, MCP2515_ISR, FALLING); // start interruptx
        Serial.println("Listening for any CAN-BUS data...");
  }

  Timer1.initialize(300000);
  Timer1.attachInterrupt(send_data);
  

}


/*
------------------------------------------------------------------------------------------------------------
     MCP2515 KESME FONKSIYONU                      
------------------------------------------------------------------------------------------------------------
*/
void MCP2515_ISR()
{
     flagRecv = 1;
     if(flagRecv){
     // check if get data
      flagRecv = 0;        // clear flag
        while (CAN_MSGAVAIL == CAN.checkReceive()){
          // read data,  len: data length, buf: data buf
          CAN.readMsgBuf(&len, buf);
            canId = CAN.getCanId();
//          Serial.println("Data received!");
//          Serial.println(canId,HEX);

          
              if (canId == akim_id){
                akim = "";
                    for(int i = 0; i < 5; i++){
                      akim += (char)buf[i];
                    }
              }
              else if (canId == hiz_id){
                hiz = "";
                    for(int i = 0; i < 5; i++){
                      hiz += (char)buf[i];
                    }
              }
              else{
                for (int j = 0; j < 27; j++){      
                   if (canId == sicaklik_id[j]){
                    sicaklik[j] = "";              
                      for(int i = 0; i < 5; i++){
                        sicaklik[j] += (char)buf[i];
                      }
                   }
                   if (canId == batarya_id[j]){
                    batarya[j] = "";
                      for(int i = 0; i < 5; i++){
                        batarya[j] += (char)buf[i];
                      } 
                   }
                }
            }
        }
    }
}

/*
------------------------------------------------------------------------------------------------------------
     LOOP                      
------------------------------------------------------------------------------------------------------------
*/
void loop(){
    //send_data();
}
//void deLay(){
//  delay(_delay);
//}
/*
------------------------------------------------------------------------------------------------------------
     DATA GONDERME FONKSIYONU                      
------------------------------------------------------------------------------------------------------------
*/
void send_data(){
    switch(data_select){
      // 0: "AKIM + HIZ + MAX SICAKLIK + TOPLAM VOLTAJ" gonder:
        case 0:{
          max_sicaklik = sicaklik[26];
          String tx_message = "A" + String(akim);
          tx_message += "*" + hiz;
          tx_message += "*" + max_sicaklik;
          tx_message += "*" + batarya[26]; //.Toplam Gerilim
          Serial.print(tx_message);
          count();
        }break;
        
      // 1: "SICAKLIK PART (1)" gonder:
        case 1:{
          String sp = "SA";
          for(int t = 0; t < 13; t++){
            sp += sicaklik[t] + "*";
          }
          Serial.print(sp);
          count();
        }break;

      // 2: "SICAKLIK PART (2)" gonder:
        case 2:{        
          String sp = "SB";           
          for(int t = 13; t < 26; t++){
           sp += sicaklik[t] + "*";
          }
          Serial.print(sp);
          count();
        }break;
        
      // 4: "BATARYA GERILIMLERINI" gonder:
        case 3:{
          String bp = "BA";
          for(int t = 0; t < 13; t++){
            bp += String(batarya[t]) + "*";
          }
          Serial.print(bp);
          count();
        }break;
        
        case 4:{
          String bp = "BB";
          for(int t = 13; t < 26; t++){
            bp += String(batarya[t]) + "*";
          }
          Serial.print(bp);         
          count();
        }break;
    }

}


/*
------------------------------------------------------------------------------------------------------------
    DATA SELECT COUNTER                      
------------------------------------------------------------------------------------------------------------
*/
void count(){
  
    data_select++;
    //deLay();
    
    if(data_select > total_var_send){
      data_select = 0;
    }
  
   
}
/*
------------------------------------------------------------------------------------------------------------
 >>>    DOSYA SONU                  
------------------------------------------------------------------------------------------------------------
*/
