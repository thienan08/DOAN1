#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include <LittleFS.h>
#include <FirebaseESP32.h>
#include <Wire.h>
#include <addons/RTDBHelper.h>
#include <LiquidCrystal_I2C.h>
#include <WebServer.h>
#include <DNSServer.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define WIFI_SSID "Ngoc Thu"
#define WIFI_PASSWORD "0796937327@#"

#define DATABASE_URL "https://vutienluc-f6439-default-rtdb.firebaseio.com/" 
#define DATABASE_SECRET "AIzaSyCDfdFWiQWFeVaU8F81tVmOTRppC_7996U"
#define USER_EMAIL "tienlucboppy24@gmail.com"
#define USER_PASSWORD "Tienluc@123"

#define AP_SSID "ESP32_Setup"
#define AP_PASSWORD "12345678"
#define CONNECTION_TIMEOUT 20000 // 20 seconds timeout for WiFi connection
#define DNS_PORT 53
#define DHTTYPE DHT11
#define DHTPIN1 16
#define DHTPIN2 17
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); 
#define PMM1 18
#define PMM2 26
#define LED 14
#define LUA1 25
#define LUA2 33
#define BUZZER 32
#define BT1 5
#define BT2 23
#define BT3 4

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json; 
FirebaseJsonData result;

WebServer server(80);
DNSServer dnsServer;
bool apMode = false;
const bool debugs = 1;
float h1, t1, h2, t2; 
int pmm1, pmm2;
int lua1, lua2;
int thrt1, thrh1, thrt2, thrh2;
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;
const unsigned long debounceDelay = 20;
bool btPressed1 = false;
bool btPressed2 = false;
bool btPressed3 = false;  

// Thêm biến để lưu giá trị cũ
float prev_h1, prev_t1, prev_h2, prev_t2;
int prev_pmm1, prev_pmm2;
int prev_lua1, prev_lua2;
bool valuesChanged = false;
unsigned long lastLCDBacklightTime = 0;
const unsigned long LCD_BACKLIGHT_TIMEOUT = 10000; // 10 giây

// Forward declarations
void setupFirebase();
void startAPMode();
void readsensor();
void updatedht();
void thresholds();
void interruptBT1();
void interruptBT2();
void interruptBT3();
void blinkLED(int pin, int interval);
void updateLCD();


// HTML page for WiFi configuration
const char* configHTML = R"html(
<!DOCTYPE html>
<html>
<head>
  <title>WiFi Configuration</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; margin: 20px; text-align: center; }
    .form-container { max-width: 400px; margin: 0 auto; padding: 20px; border: 1px solid #ccc; border-radius: 5px; }
    input { width: 100%; padding: 10px; margin: 10px 0; box-sizing: border-box; }
    button { background-color: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }
  </style>
</head>
<body>
  <div class="form-container">
    <h2>WiFi Setup</h2>
    <form action="/save" method="post">
      <input type="text" name="ssid" placeholder="WiFi SSID" required>
      <input type="password" name="password" placeholder="WiFi Password" required>
      <button type="submit">Save and Connect</button>
    </form>
  </div>
</body>
</html>
)html";

// Setup Firebase
void setupFirebase() {
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  if(debugs==1){  
    Serial.println("Connecting to Firebase...");
  }
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  int retry = 0;
  while (!Firebase.ready() && retry < 5) {
    if(debugs==1){
      Serial.print(".");
    }
    delay(500);
    retry++;
  }
  if (Firebase.ready()) {
    if(debugs==1){
      Serial.println("\nFirebase connected successfully!");
    }
  } else {
    if(debugs==1){
      Serial.println("\nFirebase connection failed!");
      Serial.print("Error: ");
      Serial.println(fbdo.errorReason());
    }
  }
}

bool connectToWifi(const char* ssid, const char* password) {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > CONNECTION_TIMEOUT) {
      if(debugs==1){
        Serial.println("WiFi connection failed");
      }
      return false;
    }
    digitalWrite(2, HIGH);
    delay(20);
    digitalWrite(2, LOW);
    delay(20);
  }
  
  if(debugs==1){
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  return true;
}

void startAPMode() {
  apMode = true;
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  if(debugs==1){
    Serial.println("Access Point Mode");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  }
  
  // Configure DNS to capture all domains
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  
  // Root page handler
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", configHTML);
  });
  
  // Save credentials handler
  server.on("/save", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String confirmHTML = "<html><body><h2>Settings Saved</h2><p>Device will now connect to: ";
    confirmHTML += ssid;
    confirmHTML += "</p></body></html>";
    
    server.send(200, "text/html", confirmHTML);
    delay(2000);
    
    WiFi.softAPdisconnect(true);
    
    if (connectToWifi(ssid.c_str(), password.c_str())) {
      apMode = false;
      setupFirebase();
    } else {    // If connection fails, restart AP mode
      startAPMode();
    }
  });
  
  // Handle captive portal requests
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });
  
  server.begin();
  Serial.println("HTTP server started");
}

void readsensor();
void updatedht();
void thresholds();
void interruptBT1();
void interruptBT2();
void interruptBT3();
void blinkLED(int pin, int interval);
void updateLCD();

void setup() {
  if(debugs==1){
    Serial.begin(115200);
  }
  pinMode(2, OUTPUT);
  
  

  
  if (!connectToWifi(WIFI_SSID, WIFI_PASSWORD)) {
    startAPMode();
  } else {
    setupFirebase();
    dht1.begin();
    dht2.begin();
    pinMode(PMM1, INPUT); 
    pinMode(PMM2, INPUT);
    pinMode(LUA1, INPUT);
    pinMode(LUA2, INPUT);
    pinMode(BT1, INPUT_PULLUP);   
    pinMode(BT2, INPUT_PULLUP);
    pinMode(BT3, INPUT_PULLUP);
    pinMode(LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    lcd.init(I2C_SDA, I2C_SCL);
  	lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Connected WiFi");
    attachInterrupt(digitalPinToInterrupt(BT1), interruptBT1, FALLING);
    attachInterrupt(digitalPinToInterrupt(BT2), interruptBT2, FALLING);
    attachInterrupt(digitalPinToInterrupt(BT3), interruptBT3, FALLING);
    
    // Khởi tạo giá trị cũ
    prev_h1 = 0;
    prev_t1 = 0;
    prev_h2 = 0;
    prev_t2 = 0;
    prev_pmm1 = -1;
    prev_pmm2 = -1;
    prev_lua1 = -1;
    prev_lua2 = -1;
  }
}
int ttlcd = 0;
bool buzzer = false;
bool led = false;
void loop() {
  if (apMode) {
    dnsServer.processNextRequest();
    server.handleClient();
  } else {
    if(btPressed1){
      btPressed1 = false;
      Serial.println("BT1 pressed");
      Firebase.setInt(fbdo, "/controls/led", (int)!led);
    }
    if(btPressed2){
      btPressed2 = false;
      Serial.println("BT2 pressed");
      Firebase.setInt(fbdo, "/controls/buzzer", (int)!buzzer);
    }
    if(btPressed3){
      btPressed3 = false;
      Serial.println("BT3 pressed");
      ttlcd++;
      if(ttlcd==2) ttlcd=0;
      Serial.print("TTLCD: ");
      Serial.println(ttlcd);
      
      // Bật đèn nền LCD khi ấn nút
      lcd.backlight();
      lastLCDBacklightTime = millis();
        updateLCD();
    } 
//digitalWrite(BUZZER, HIGH);
    thresholds();
    readsensor();
    updatedht();
    updateLCD();
  }
}
 unsigned long currentTimeled; 
 unsigned long lastTimeled=0;

void readsensor(){
  h1 = dht1.readHumidity();
  t1 = dht1.readTemperature();
  h2 = dht2.readHumidity();
  t2 = dht2.readTemperature();
  pmm1 = digitalRead(PMM1);
  pmm2 = digitalRead(PMM2);
  lua1 = digitalRead(LUA1);
  lua2 = digitalRead(LUA2);
  
  // Kiểm tra nếu có sự thay đổi
  valuesChanged = false;
  if (abs(h1 - prev_h1) > 0.1 || abs(t1 - prev_t1) > 0.1 || 
      abs(h2 - prev_h2) > 0.1 || abs(t2 - prev_t2) > 0.1 || 
      pmm1 != prev_pmm1 || pmm2 != prev_pmm2 || 
      lua1 != prev_lua1 || lua2 != prev_lua2) {
    
    valuesChanged = true;
    lastLCDBacklightTime = millis();
    lcd.backlight(); // Bật đèn nền LCD khi có thay đổi
    
    // Cập nhật giá trị cũ
    prev_h1 = h1;
    prev_t1 = t1;
    prev_h2 = h2;
    prev_t2 = t2;
    prev_pmm1 = pmm1;
    prev_pmm2 = pmm2;
    prev_lua1 = lua1;
    prev_lua2 = lua2;
  } else {
    // Tắt đèn nền LCD sau khoảng thời gian không có thay đổi
    if (millis() - lastLCDBacklightTime > LCD_BACKLIGHT_TIMEOUT) {
      lcd.noBacklight();
    }
  }
  
  // Xử lý đèn và còi báo động khi có sự cố
  if ((t1>thrt1) || (h1>thrh1) || (t2>thrt2) || (h2>thrh2) || (pmm1==0) || (pmm2==0) || (lua1==0) || (lua2==0)) { 
    if(buzzer == 1) digitalWrite(BUZZER, HIGH);  
    else digitalWrite(BUZZER, LOW);
    if(led == 1) blinkLED(LED, 20); 
    else digitalWrite(LED, HIGH);
  } else {
    digitalWrite(BUZZER, LOW);
    digitalWrite(LED, HIGH);
  }
 
  if(debugs==1){
     Serial.print("ROOM1: H: ");
     Serial.print(h1);
     Serial.print(" T: ");
     Serial.println(t1);
     Serial.print("ROOM2: H: ");
     Serial.print(h2);
     Serial.print(" T: ");
    Serial.println(t2);
    Serial.print("PPM1: ");
    Serial.println(pmm1);     
    Serial.print("PPM2: ");
    Serial.println(pmm2);
    Serial.print("LUA1: ");
    Serial.println(lua1);
    Serial.print("LUA2: ");
    Serial.println(lua2);
  }
}
void interruptBT1(){
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime1 > debounceDelay) {
    btPressed1 = true;
    lastDebounceTime1 = currentTime;
    
  }
}
void interruptBT2(){
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime2 > debounceDelay) {
    btPressed2 = true;
    lastDebounceTime2 = currentTime;
  }
}     
void interruptBT3(){
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime3 > debounceDelay) {
    btPressed3 = true;
    lastDebounceTime3 = currentTime;
    
  }
} 
void updatedht(){
  if (valuesChanged) {
    json.set("/room1/temperature",t1);
    json.set("/room1/humidity",h1);
    json.set("/room2/temperature",t2);
    json.set("/room2/humidity",h2);
    json.set("/room1/gas",(int)!pmm1);
    json.set("/room2/gas",(int)!pmm2);
    json.set("/room1/flame",(int)!lua1);
    json.set("/room2/flame",(int)!lua2);
    if (Firebase.updateNode(fbdo, "/rooms", json)) {
     if(debugs==1){
         Serial.println("update done");
      }
    } else {
      if(debugs==1){
         Serial.print("error: ");
         Serial.println(fbdo.errorReason());
       }
     }
  }
}
void thresholds(){
if (Firebase.get(fbdo, "/")) {
      if (fbdo.dataType() == "json") {
        FirebaseJson &jsonData = fbdo.jsonObject();
        
        jsonData.get(result, "/thresholds/room1/temperature");
        if (result.success) thrt1 = result.to<int>();
        
        jsonData.get(result, "/thresholds/room1/humidity/high");
        if (result.success) thrh1 = result.to<int>();
        
        jsonData.get(result, "/thresholds/room2/temperature");
        if (result.success) thrt2 = result.to<int>();
        
        jsonData.get(result, "/thresholds/room2/humidity/high");
        if (result.success) thrh2 = result.to<int>();
        
        jsonData.get(result, "/controls/buzzer");
        if (result.success) buzzer = result.to<bool>();
        
        jsonData.get(result, "/controls/led");
        if (result.success) led = result.to<bool>();
        Serial.print("BUZZER: ");
        Serial.println(buzzer);
        Serial.print("LED: ");
        Serial.println(led);  
        Serial.print("THRT1: ");    
        Serial.println(thrt1);
        Serial.print("THRH1: ");
        Serial.println(thrh1);
        Serial.print("THRT2: ");
        Serial.println(thrt2);
        Serial.print("THRH2: ");
        Serial.println(thrh2);
       
      }
    }
}

void blinkLED(int pin, int interval) {
  static unsigned long previousMillis = 0;
  static bool ledState = false;
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    ledState = !ledState;
    digitalWrite(pin, ledState);
  }
}

void updateLCD() {  
 // lcd.clear();    
  
  if (lua1 == 0) {    
    lcd.setCursor(0, 0);    
    lcd.print("Chay !!!       ");    
    lcd.setCursor(0, 1);    
    lcd.print("ROOM 1         ");    
    return;  
  }    
  
  if (lua2 == 0) {    
    lcd.setCursor(0, 0);    
    lcd.print("Chay !!!      ");    
    lcd.setCursor(0, 1);    
    lcd.print("ROOM 2        ");    
    return;  
  }    
  
  if (pmm1 == 0) {    
    lcd.setCursor(0, 0);    
    lcd.print("WARRNING!!!!   ");    
    lcd.setCursor(0, 1);    
    lcd.print("ROOM 1        ");    
    return;  
  }    
  
 
  if (pmm2 == 0) {    
    lcd.setCursor(0, 0);    
    lcd.print("WARRNING!!!!     ");    
    lcd.setCursor(0, 1);    
    lcd.print("ROOM 2        ");    
    return;  
  }    
  
  if (ttlcd == 0) {    
    lcd.setCursor(0, 0);    
    lcd.print("ROOM 1:       ");    
    lcd.setCursor(0, 1);    
    lcd.print(t1); 
    lcd.write(223);  
    lcd.print("C ");    
    lcd.print(h1);    
    lcd.print("%      ");  
  }   
  else if (ttlcd == 1) {    
    lcd.setCursor(0, 0);    
    lcd.print("ROOM 2:      ");    
    lcd.setCursor(0, 1);    
    lcd.print(t2);  
    lcd.write(223);  
    lcd.print("C ");    
    lcd.print(h2);    
    lcd.print("%      ");  
  }
}