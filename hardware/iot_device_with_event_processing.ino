#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// --- Definiciones ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define DHTPIN 4       // Pin de datos del DHT
#define DHTTYPE DHT11
#define LIGHTPIN 15    // Pin analógico para sensor de luz (LDR) - GPIO15/D15
#define LEDPIN 2       // Pin del LED integrado
#define MEASURE_INTERVAL 2    // Igualado al Código 1 (2 segundos)
#define ALERT_DURATION 10

// --- Instancias ---
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiClient net;
PubSubClient client(net);

// --- Configuración (Basada en Código 2) ---
const char ssid[] = "Lucy";
const char pass[] = "#1Sol.Rocky";

#define USER "jn.cordobap1"
const char MQTT_HOST[] = "32.192.84.37";
const int MQTT_PORT = 8082;
const char MQTT_USER[] = USER;
const char MQTT_PASS[] = "abc123";

// Tópicos (Corregidos para coincidir con datos reales)
const char MQTT_TOPIC_PUB[] = "colombia/cundinamarca/bogota/" USER "/out";
const char MQTT_TOPIC_SUB[] = "colombia/cundinamarca/bogota/" USER "/in";

// --- Variables Globales ---
time_t now;
unsigned long measureTime = 0;
unsigned long alertTime = 0;
String alert = "";
float temp;
float humi;
float luminosidad;
bool ledState = false;
unsigned long blinkTime = 0;
String alertType = "";
bool displayAvailable = false;  // Flag para saber si la pantalla funciona

// --- Funciones de Conexión ---

void mqtt_connect() {
  while (!client.connected()) {
    Serial.print("MQTT connecting ... ");
    if (client.connect(USER, MQTT_USER, MQTT_PASS)) {
      Serial.println("connected.");
      client.subscribe(MQTT_TOPIC_SUB);
    } else {
      Serial.print("Error, rc=");
      Serial.print(client.state());
      Serial.println(" reintentando en 5s");
      delay(5000);
    }
  }
}

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  String data = "";
  for (int i = 0; i < length; i++) {
    data += (char)payload[i];
  }
  
  Serial.println("Comando recibido: " + data);
  
  // Procesar diferentes tipos de comandos
  if (data.indexOf("TEMP_HIGH") >= 0) {
    alert = "TEMP ALTA";
    alertType = "TEMP_HIGH";
    alertTime = millis();
    Serial.println("Alerta: Temperatura alta detectada");
  }
  else if (data.indexOf("TEMP_LOW") >= 0) {
    alert = "TEMP BAJA";
    alertType = "TEMP_LOW";
    alertTime = millis();
    Serial.println("Alerta: Temperatura baja detectada");
  } 
  else if (data.indexOf("HUMIDITY_HIGH") >= 0) {
    alert = "HUMEDAD ALTA";
    alertType = "HUMIDITY_HIGH";
    alertTime = millis();
    Serial.println("Alerta: Humedad alta detectada");
  }
  else if (data.indexOf("HUMIDITY_LOW") >= 0) {
    alert = "HUMEDAD BAJA";
    alertType = "HUMIDITY_LOW";
    alertTime = millis();
    Serial.println("Alerta: Humedad baja detectada");
  }
  else if (data.indexOf("LIGHT_LOW") >= 0) {
    alert = "LUZ BAJA";
    alertType = "LIGHT_LOW";
    alertTime = millis();
    Serial.println("Alerta: Luminosidad baja detectada");
  }
  else if (data.indexOf("LIGHT_HIGH") >= 0) {
    alert = "LUZ ALTA";
    alertType = "LIGHT_HIGH";
    alertTime = millis();
    Serial.println("Alerta: Luminosidad alta detectada");
  }
  else if (data.indexOf("ANOMALY") >= 0) {
    alert = "ANOMALIA";
    alertType = "ANOMALY";
    alertTime = millis();
    Serial.println("Alerta: Anomalía estadística detectada");
  }
  else if (data.indexOf("ENERGY_OPTIMIZE") >= 0) {
    alert = "OPTIMIZAR";
    alertType = "ENERGY_OPTIMIZE";
    alertTime = millis();
    Serial.println("Alerta: Optimizar energía sugerido");
  }
  else if (data.indexOf("ENVIRONMENTAL_STRESS") >= 0) {
    alert = "ESTRES AMB";
    alertType = "ENVIRONMENTAL_STRESS";
    alertTime = millis();
    Serial.println("Alerta: Estrés ambiental detectado");
  }
  else if (data.indexOf("ALERT_OFF") >= 0) {
    alert = "";
    alertType = "";
    digitalWrite(LEDPIN, LOW);
    Serial.println("Alerta desactivada");
  }
  else if (data.indexOf("ALERT") >= 0) {
    alert = data;
    alertType = "GENERIC";
    alertTime = millis();
  }
}

// --- Funciones de Pantalla (Lógica del Código 1) ---

void startDisplay() {
  Serial.println("Intentando inicializar pantalla SH1106...");
  displayAvailable = false;
  
  // Inicializar SH1106 con dirección 0x3C
  if(!display.begin(0x3C, true)) {
    Serial.println("✗ Fallo SH1106 en 0x3C");
    // Intentar con dirección alternativa 0x3D
    if(!display.begin(0x3D, true)) {
      Serial.println("✗ Fallo SH1106 en 0x3D");
      Serial.println("→ Continuando sin pantalla...");
      return;
    } else {
      Serial.println("✓ Pantalla SH1106 encontrada en dirección 0x3D");
      displayAvailable = true;
    }
  } else {
    Serial.println("✓ Pantalla SH1106 encontrada en dirección 0x3C");
    displayAvailable = true;
  }
  
  // Si la pantalla está disponible, configurarla
  if (displayAvailable) {
    display.clearDisplay();
    display.display();
    
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 5);
    display.println("IOT System");
    display.setCursor(0, 25);
    display.println("SH1106 OK");
    display.display();
    delay(1000);
    
    Serial.println("✓ Pantalla SH1106 inicializada correctamente");
  }
}

void displayNoSignal() {
  Serial.println("Estado: Sin señal WiFi");
  
  // Solo intentar mostrar en pantalla si está disponible
  if (displayAvailable) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 5);
    display.println("Conectando WiFi...");
    display.setCursor(0, 25);
    display.println("Por favor espere");
    display.display();
  }
}

void displayHeader() {
  display.setTextSize(1);
  struct tm* tinfo;
  tinfo = localtime(&now);
  char hourChar[9];
  strftime(hourChar, sizeof(hourChar), "%H:%M:%S", tinfo);
  display.println("IOT Sensors   " + String(hourChar));
}

void displayMeasures() {
  display.println("");
  display.print("T: "); display.print(temp, 1);
  display.print("    H: "); display.print(humi, 1);
  display.println("");
  display.print("Luz: "); display.print(luminosidad, 0); display.println(" lx");
}

void displayMessage(String message) {
  display.setTextSize(1);
  display.println("\nMsg:");
  display.setTextSize(2);
  if (message == "OK") {
    display.println("    " + message); 
  } else {
    display.println(message); 
  }
}

// --- Función para leer sensor de luz ---
float readLightSensor() {
  int ldrValue = analogRead(LIGHTPIN);
  // Convertir lectura del LDR a lux aproximado
  // Esta fórmula puede necesitar calibración según tu sensor específico
  // Para ESP32: rango ADC es 0-4095 (12 bits)
  // Mapeo simple: 0 (oscuridad) a 1000 lx (luz brillante)
  float lux = map(ldrValue, 0, 4095, 0, 1000);
  
  // Opcional: aplicar una curva logarítmica más realista
  // float voltage = ldrValue * 3.3 / 4095.0;
  // float resistance = (10000.0 * voltage) / (3.3 - voltage); 
  // float lux = 500.0 / (resistance / 1000.0);  // Aproximación
  
  return lux;
}

// --- Función de diagnóstico I2C ---
void scanI2C() {
  Serial.println("Escaneando dispositivos I2C...");
  byte error, address;
  int nDevices = 0;

  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Dispositivo I2C encontrado en dirección 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
  }
  
  if (nDevices == 0) {
    Serial.println("No se encontraron dispositivos I2C");
  } else {
    Serial.print("Encontrados ");
    Serial.print(nDevices);
    Serial.println(" dispositivos I2C");
  }
}

// --- Función segura para usar la pantalla ---
void safeDisplayUpdate() {
  // Solo actualizar pantalla si está disponible
  if (!displayAvailable) {
    return;
  }
  
  // Estilo similar al código funcional para SH1106
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Encabezado
  display.setCursor(0, 0);
  display.println("SENSORES EN VIVO");

  // Temperatura
  display.setCursor(0, 15);
  display.print("Temp: ");
  display.print(temp, 1);
  display.println(" C");

  // Humedad
  display.setCursor(0, 30);
  display.print("Hum: ");
  display.print(humi, 1);
  display.println(" %");

  // Luminosidad
  display.setCursor(0, 45);
  display.print("Luz: ");
  display.print(luminosidad, 0);
  display.println(" lx");

  // Estado de alerta (si existe)
  if (alert != "") {
    display.setCursor(0, 55);
    display.println(alert);
  }

  display.display();
}

// --- Lógica Principal ---

void setup() {
  Serial.begin(115200);
  delay(2000); // Tiempo para que se abra el Monitor Serial
  
  Serial.println("\n=== INICIANDO SISTEMA IOT ===");
  Serial.println("Versión: Con manejo robusto de errores");
  
  // Configurar pines básicos primero
  pinMode(LIGHTPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);  // LED encendido para indicar inicio
  delay(500);
  digitalWrite(LEDPIN, LOW);
  
  Serial.println("✓ Pines configurados");
  
  // Inicializar I2C con configuración correcta
  Serial.println("Inicializando I2C...");
  Wire.begin(22, 21); // SDA=22, SCL=21 (como el código funcional)
  delay(100);
  
  // Diagnóstico I2C
  scanI2C();
  
  // Inicializar pantalla (sin colgarse)
  startDisplay();
  
  // Inicializar DHT
  Serial.println("Inicializando sensor DHT11...");
  dht.begin();
  delay(100);
  Serial.println("✓ DHT11 inicializado");
  
  // Conexión WiFi
  Serial.println("Conectando a WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    displayNoSignal();
    Serial.print(".");
    delay(500);
    wifiAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("✓ WiFi conectado");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("✗ Error conectando WiFi - Continuando...");
  }

  // Tiempo SNTP
  Serial.println("Configurando hora...");
  configTime(-5 * 3600, 0, "pool.ntp.org");
  now = time(nullptr);
  int timeAttempts = 0;
  while (now < 1510592825 && timeAttempts < 10) { 
    delay(500); 
    now = time(nullptr);
    timeAttempts++;
    Serial.print(".");
  }
  Serial.println();
  
  // MQTT
  Serial.println("Configurando MQTT...");
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(receivedCallback);
  
  Serial.println("✓ Sistema IOT iniciado con sensores:");
  Serial.println("  - Temperatura y Humedad (DHT11)");
  Serial.println("  - Luminosidad (LDR en pin D15)");
  Serial.println("  - LED de alerta (pin D2)");
  Serial.println("  - Procesamiento de eventos avanzado habilitado");
  Serial.println("  - Comandos soportados: TEMP_HIGH, TEMP_LOW, HUMIDITY_HIGH, HUMIDITY_LOW,");
  Serial.println("    LIGHT_LOW, LIGHT_HIGH, ANOMALY, ENERGY_OPTIMIZE, ENVIRONMENTAL_STRESS");
  Serial.print("  - Pantalla OLED: ");
  Serial.println(displayAvailable ? "✓ Disponible" : "✗ No disponible");
  Serial.println("================================");
  Serial.println("Iniciando loop principal...");
  
  // LED parpadeando para indicar que terminó setup
  for(int i=0; i<3; i++) {
    digitalWrite(LEDPIN, HIGH);
    delay(200);
    digitalWrite(LEDPIN, LOW);
    delay(200);
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    return;
  }
  
  if (!client.connected()) mqtt_connect();
  client.loop();

  now = time(nullptr);

  // Medición y Envío (Ahora incluye luminosidad)
  if ((millis() - measureTime) >= MEASURE_INTERVAL * 1000) {
    measureTime = millis();
    temp = dht.readTemperature();
    humi = dht.readHumidity();
    luminosidad = readLightSensor();

    // Validar lecturas
    if (!isnan(temp) && !isnan(humi)) {
      // Crear payload JSON con las tres variables
      String payload = "{\"temperatura\": " + String(temp, 1) + 
                      ", \"humedad\": " + String(humi, 1) + 
                      ", \"luminosidad\": " + String(luminosidad, 0) + "}";
      
      // Enviar por MQTT
      client.publish(MQTT_TOPIC_PUB, payload.c_str());
      
      // Debug en Serial
      Serial.print("📊 Datos: T=");
      Serial.print(temp, 1);
      Serial.print("°C, H=");
      Serial.print(humi, 1);
      Serial.print("%, Luz=");
      Serial.print(luminosidad, 0);
      Serial.println("lx");
      Serial.println("Datos enviados: " + payload);
    } else {
      Serial.println("Error: Lectura inválida de DHT11");
    }
  }

  // Procesamiento de Eventos y Control del LED
  String msg = "OK";
  if (alert != "") {
    msg = alert;
    
    // Control del LED según tipo de alerta
    if (alertType == "TEMP_HIGH") {
      // Parpadeo rápido para temperatura alta (250ms)
      if (millis() - blinkTime >= 250) {
        ledState = !ledState;
        digitalWrite(LEDPIN, ledState);
        blinkTime = millis();
      }
    } else if (alertType == "HUMIDITY_HIGH") {
      // Parpadeo lento para humedad alta (500ms)
      if (millis() - blinkTime >= 500) {
        ledState = !ledState;
        digitalWrite(LEDPIN, ledState);
        blinkTime = millis();
      }
    } else if (alertType == "HUMIDITY_LOW") {
      // Parpadeo rápido para humedad baja (200ms)
      if (millis() - blinkTime >= 200) {
        ledState = !ledState;
        digitalWrite(LEDPIN, ledState);
        blinkTime = millis();
      }
    } else if (alertType == "TEMP_LOW") {
      // Parpadeo ultra-rápido para temperatura baja (150ms)
      if (millis() - blinkTime >= 150) {
        ledState = !ledState;
        digitalWrite(LEDPIN, ledState);
        blinkTime = millis();
      }
    } else if (alertType == "LIGHT_LOW") {
      // LED encendido fijo para luminosidad baja
      digitalWrite(LEDPIN, HIGH);
    } else if (alertType == "LIGHT_HIGH") {
      // Parpadeo medio para luminosidad alta (350ms)
      if (millis() - blinkTime >= 350) {
        ledState = !ledState;
        digitalWrite(LEDPIN, ledState);
        blinkTime = millis();
      }
    } else if (alertType == "ANOMALY") {
      // Doble parpadeo rápido para anomalías estadísticas
      static int flashCount = 0;
      if (millis() - blinkTime >= 100) {
        ledState = !ledState;
        digitalWrite(LEDPIN, ledState);
        flashCount++;
        if (flashCount >= 4) { // 2 parpadeos completos
          delay(300); // Pausa breve
          flashCount = 0;
        }
        blinkTime = millis();
      }
    } else if (alertType == "ENERGY_OPTIMIZE") {
      // Parpadeo ultra-lento para optimización energética (1 segundo)
      if (millis() - blinkTime >= 1000) {
        ledState = !ledState;
        digitalWrite(LEDPIN, ledState);
        blinkTime = millis();
      }
    } else if (alertType == "ENVIRONMENTAL_STRESS") {
      // Parpadeo alternado rápido-lento para estrés ambiental
      static bool fastPhase = true;
      int interval = fastPhase ? 150 : 400;
      if (millis() - blinkTime >= interval) {
        ledState = !ledState;
        digitalWrite(LEDPIN, ledState);
        if (!ledState) fastPhase = !fastPhase; // Cambiar fase al apagar
        blinkTime = millis();
      }
    } else {
      // Parpadeo normal para otras alertas
      if (millis() - blinkTime >= 1000) {
        ledState = !ledState;
        digitalWrite(LEDPIN, ledState);
        blinkTime = millis();
      }
    }
    
    // Apagar alerta después del tiempo configurado
    if (millis() - alertTime >= ALERT_DURATION * 1000) {
      alert = "";
      alertType = "";
      digitalWrite(LEDPIN, LOW);
    }
  } else {
    digitalWrite(LEDPIN, LOW);
  }

  // Refrescar Pantalla de forma segura
  safeDisplayUpdate();
}