#include <SoftwareSerial.h>
#define ultrasonic_trigger_pin 8
#define ultrasonic_echo_pin 9

const byte rxPin = 6;
const byte txPin = 7;

//----Var untuk ultarsonic
float lokasi_sensor = 75;
float batas_atas = lokasi_sensor - 15;
float batas_bawah = 0;

SoftwareSerial ESP8266 (rxPin, txPin);

String ssid ="863 Studio - Office";
String password="8cadb11f56";

unsigned long lastTimeMillis = 0;

void setup() {
  pinMode(ultrasonic_trigger_pin, OUTPUT);
  pinMode(ultrasonic_echo_pin, INPUT);
  Serial.begin(9600);   
  ESP8266.begin(115200);
  delay(2000);
  reset();
}

void reset() {
  ESP8266.println("AT+CWMODE=3");
  delay(1000);
  printResponse();
  ESP8266.println("AT+RST");
  if(ESP8266.find("OK")) 
  Serial.println("Module Reset");
}

void connectWifi() {
  String cmd = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
  ESP8266.println(cmd);
  delay(4000);
  if(ESP8266.find("OK")) {
    Serial.println("Connected!");
  }else{
    connectWifi();
    Serial.println("Cannot connect to wifi"); 
  }
}

void printResponse() {
  while (ESP8266.available()) {
    Serial.print((char)ESP8266.read()); 
  }
  Serial.println();
}

void loop() {
  connectWifi();
  lastTimeMillis = millis();

  ESP8266.println("AT+CIPMUX=1");
  delay(1000);
  printResponse();

  ESP8266.println("AT+CIPSTART=4,\"TCP\",\"192.168.0.101\",80");
  delay(1000);
  printResponse();
  
  String ketinggian_dan_status = bacaKetinggian();
  String cmd = "GET /automation_system/index.php/adi/api/post_data/"+ ketinggian_dan_status +"/ HTTP/1.1 \r\n" +  
  "Host : 192.168.0.101 \r\n" +
  "Content-Type: application/x-www-form-urlencoded \r\n " +
  "Accept: */* \r\n" + 
  "Connection:close \r\n\r\n";    
  int ln = cmd.length() + 4;
  ESP8266.println("AT+CIPSEND=4," + String(ln));
  delay(500);
  printResponse();
  
  ESP8266.println(cmd);
  delay(1000);
  printResponse();
  ESP8266.println(""); 
  delay(2000);

  while (ESP8266.available() > 0) {
    Serial.print((char)ESP8266.read());
    delay(50);
  }
  ESP8266.println("AT+CIPCLOSE");
  delay(1000);
  printResponse();
  Serial.println("\n");
  Serial.println("--------------------------------------------------------------------");
}

  //----Function dan method Ultrasonic
String bacaKetinggian(){
  Serial.println("Menjalankan Fungsi bacaKetinggian()");
  long duration, distance;
  int ketinggian_air;
  digitalWrite(ultrasonic_trigger_pin, LOW);  
  delayMicroseconds(2); 
  digitalWrite(ultrasonic_trigger_pin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(ultrasonic_trigger_pin, LOW);
  duration = pulseIn(ultrasonic_echo_pin, HIGH);
  distance = (duration/2) / 29.1;
  Serial.print("Sensor diletakan di ketinggian : ");
  Serial.println(lokasi_sensor); 
  ketinggian_air = (lokasi_sensor - distance) - 1;
  Serial.print("Ketinggian air saat ini :  ");
  Serial.println(ketinggian_air);
  String status_ketinggian = "";
  //untuk kondisi Tinggi
  if (ketinggian_air > batas_atas){
    status_ketinggian = "Tinggi";
    Serial.println("Ketinggian air sudah masuk batas atas, buka keran heula coy");
  }else

  //untuk kondisi normal
  if (ketinggian_air < batas_atas && ketinggian_air > (batas_atas - 20)){
    status_ketinggian = "Normal";
    Serial.println("Ketinggian air kondisi normal, woles aja coy");
  }else 

  //untuk kodisi tinggi
  if (ketinggian_air < (batas_atas - 15)){
    status_ketinggian = "Rendah";
    Serial.println("Ketinggian air kondisi surut, tutup pintu air bisi beuki saat");
  }
  delay(500);  
  String retVal = (String)ketinggian_air +"/"+ status_ketinggian;
  return retVal;
}


