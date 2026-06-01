#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include "time.h"
#include <DHT.h> // <-- Librería del DHT

// ==========================================
// CONFIGURA AQUÍ TU WI-FI A MANO
// ==========================================
const char* ssid     = "iPhone";       // Nombre de tu red Wi-Fi
const char* password = "lann1ster";   // Contraseña de tu Wi-Fi
// ==========================================

// Configuración del sensor DHT11
#define DHTPIN 15     // Pin GPIO donde está conectado el cable de datos
#define DHTTYPE DHT11 // Tipo de sensor
DHT dht(DHTPIN, DHTTYPE);

// Configuración de zona horaria oficial para España (Península)
const char* timeZone  = "CET-1CEST,M3.5.0,M10.5.0/3"; 
const char* ntpServer = "pool.ntp.org";

TFT_eSPI tft = TFT_eSPI();

unsigned long previoMillis = 0;
const long intervalo = 1000; 

bool cursorVisible = true;
unsigned long previoCursorMillis = 0;

// Variables para el control de lectura del DHT sin congelar la pantalla
unsigned long previoDHTMillis = 0;
const long intervaloDHT = 2000; // El DHT11 se lee cada 2 segundos

void setup() {
  tft.init();
  tft.setRotation(1); // Modo horizontal 
  tft.fillScreen(TFT_BLACK); 
  
  // Marco decorativo verde Matrix
  tft.drawRect(5, 5, tft.width() - 10, tft.height() - 10, 0x03E0); 

  // Pantalla de carga estilo consola hacker
  tft.setTextSize(1);
  tft.setTextColor(0x07E0, TFT_BLACK); // Verde brillante
  tft.setCursor(15, 20);
  tft.print("Connecting to Matrix Wi-Fi...");

  // Iniciar el sensor DHT11
  dht.begin();

  // Iniciar la conexión inalámbrica
  WiFi.begin(ssid, password);
  
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 30) {
    delay(500);
    tft.print(".");
    intentos++;
  }

  tft.setCursor(15, 35);
  if (WiFi.status() == WL_CONNECTED) {
    tft.print("Access granted. Syncing time...");
    configTzTime(timeZone, ntpServer);
    delay(1500); 
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.print("Connection failed. Using local clock.");
  }

  tft.fillScreen(TFT_BLACK); 
  tft.drawRect(5, 5, tft.width() - 10, tft.height() - 10, 0x03E0); 
}

void loop() {
  unsigned long actualMillis = millis();

  // 1. RELOJ DE ALTA PRECISIÓN (Cada 1 segundo)
  if (actualMillis - previoMillis >= intervalo) {
    previoMillis = actualMillis;
    mostrarRelojMatrix();
  }

  // 2. LECTURA Y MUESTRA DEL DHT11 (Cada 2 segundos)
  if (actualMillis - previoDHTMillis >= intervaloDHT) {
    previoDHTMillis = actualMillis;
    mostrarDatosDHT();
  }

  // 3. EFECTO ANIMADO "WAKE UP, NEO..."
  if (actualMillis - previoCursorMillis >= 400) {
    previoCursorMillis = actualMillis;
    cursorVisible = !cursorVisible;
    
    tft.setTextSize(2);
    tft.setCursor(25, 110); // Bajado un poco para dar espacio al DHT
    tft.setTextColor(0x07E0, TFT_BLACK); 
    tft.print(" **pupp3t4bra**");
    
    if (cursorVisible) tft.print("_");
    else tft.print(" "); 
  }
}

void mostrarRelojMatrix() {
  struct tm timeinfo;
  
  if(!getLocalTime(&timeinfo)){
    return; 
  }

  char bufferHora[16];
  char bufferFecha[16];
  
  sprintf(bufferHora, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  sprintf(bufferFecha, "%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  
  // Renderizar HORA (Verde medio)
  tft.setTextSize(3); 
  tft.setTextColor(0x0400, TFT_BLACK); 
  tft.setCursor(45, 15);
  tft.print(bufferHora);
  
  // Renderizar FECHA (Verde tenue)
  tft.setTextSize(1); 
  tft.setTextColor(0x0200, TFT_BLACK); 
  tft.setCursor(85, 43);
  tft.print(bufferFecha);
}

void mostrarDatosDHT() {
  float humedad = dht.readHumidity();
  float celsius = dht.readTemperature();

  tft.setTextSize(1);
  tft.setCursor(40, 65);

  // Validar si el sensor responde correctamente
  if (isnan(humedad) || isnan(celsius)) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.print("Sensor Error: Disconnected");
    return;
  }

  // Renderizar los datos en pantalla
  tft.setTextColor(0x05E0, TFT_BLACK); // Verde Matrix intermedio
  tft.print("TEMP: ");
  tft.print(celsius, 1);
  tft.print(" C  |  HUM: ");
  tft.print(humedad, 1);
  tft.print(" % ");
}
