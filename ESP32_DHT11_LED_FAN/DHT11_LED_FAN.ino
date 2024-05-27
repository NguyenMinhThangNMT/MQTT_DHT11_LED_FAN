#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>

//Dinh nghia chan
#define DHTPIN  15
#define LED     4
#define FAN    16
#define BTNLED  17
#define BTNFAN  5
#define DHTTYPE DHT11


//*************Thiet lap server MQTT**********************/
const char* ssid = "Thanh Binh Nguyen";
const char* password = "0972047678";


#define MQTT_SERVER "robotics.cloud.shiftr.io"
#define MQTT_PORT 1883
#define MQTT_USER "robotics"
#define MQTT_PASSWORD "Robot123"
#define MQTT_LDP_TOPIC "Topic_Send"

WiFiClient wifiClient;
PubSubClient client(wifiClient);

DHT dht(DHTPIN, DHTTYPE);
//lcd
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
//value 
float h,t;
unsigned long previousMillis=0;


//icon nhiet do
byte icontemp[8] =
{
    B00100,
    B01010,
    B01010,
    B01110,
    B01110,
    B11111,
    B11111,
    B01110
};

//Icon lcd
byte iconhumi[8] = 
{
    B00100,
    B00100,
    B01010,
    B01010,
    B10001,
    B10001,
    B10001,
    B01110,
};


//Ket noi wifi
void setup_wifi() {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

//Ham ket noi mqtt
void connect_to_broker() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP_32";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
            Serial.println("connected");
            client.subscribe(MQTT_LDP_TOPIC);
            client.subscribe("esp32/LED");
            client.subscribe("esp32/FAN");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}

//Ham callback MQTT
void callback(char* topic, byte* message, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;

    for (int i = 0; i < length; i++) {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
    }
    //Kiem tra topic 
    if (strcmp(topic, "esp32/LED") == 0) {
        controlDevice(LED, messageTemp);
    } else if (strcmp(topic, "esp32/FAN") == 0) {
        controlDevice(FAN, messageTemp);
    }
}


//Ham dieu khien thiet bi
void controlDevice(int pin, String state) {
    if (state == "ON") {
        digitalWrite(pin, HIGH);
    } else if (state == "OFF") {
        digitalWrite(pin, LOW);
    }
}

void setup() {
    Serial.begin(115200);
    dht.begin();
    setup_wifi();
    client.setServer(MQTT_SERVER, MQTT_PORT );
    client.setCallback(callback);
    connect_to_broker();
    //Khai bao mode cho ngoai vi
    pinMode(LED, OUTPUT);
    pinMode(FAN, OUTPUT);
    pinMode(BTNLED, INPUT);
    pinMode(BTNFAN, INPUT);
    //Hien thi  lcd
    lcd.init();
    lcd.backlight();
    lcd.createChar(1, icontemp);
    lcd.createChar(2, iconhumi);
}

void loop() {
  //Kiem tra ket noi lai khi mat ket noi
    client.loop();
    if (!client.connected()) {
        connect_to_broker();
    }
  //bien gia tri nhiet do , do am
    h = dht.readHumidity();
    t = dht.readTemperature();
  //Ham ngat 3 giay
    unsigned long currentMillis=millis();
    if(currentMillis-previousMillis>=3000){
      previousMillis=currentMillis;
      //Gui gia tri nhiet do,do am len MQTT
      client.publish("esp32/temperature", String(t).c_str());
      client.publish("esp32/humidity", String(h).c_str());
    

    }
    //Doc gia tri cua nut bam va topic cua den va quat
    if (digitalRead(BTNLED) == HIGH) {
        bool newState = !digitalRead(LED); 
        digitalWrite(LED, newState); 
        client.publish("esp32/LED", newState ? "ON" : "OFF");
    }
    if (digitalRead(BTNFAN) == HIGH) {
        bool newState = !digitalRead(FAN); 
        digitalWrite(FAN, newState); 
        client.publish("esp32/FAN", newState ? "ON" : "OFF");
    }

    //Hien thi len lcd
    lcd.setCursor(0, 0);
    lcd.write(1);
    lcd.setCursor(1, 0);
    lcd.print(":");
    lcd.setCursor(2, 0);
    lcd.print(t);

    lcd.setCursor(8, 0);
    lcd.write(2);
    lcd.setCursor(9, 0);
    lcd.print(":");
    lcd.setCursor(10, 0);
    lcd.print(h);

    lcd.setCursor(0, 1);
    lcd.print("LED:");
    lcd.print(digitalRead(LED) == HIGH ? "ON " : "OFF");

    lcd.setCursor(8, 1);
    lcd.print("FAN:");
    lcd.print(digitalRead(FAN) == HIGH ? "ON " : "OFF");

    
}


