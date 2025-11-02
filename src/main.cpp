#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "Device.h"
#include "TelegramHandler.h"

// ==================== CONFIGURACIÓN ====================
const char* SSID = "Wokwi-GUEST";
const char* PASS = "";
const char* BOT_TOKEN = "8306829882:AAFOVJIXxXQhbgHjnBWYxudMhXb0WbIE-74";
const String CHAT_ID = "7965286702";
const unsigned long TIEMPO_ESCANEO = 1000; 

// Pines
const int LED_VERDE = 23;
const int LED_AZUL = 2;
const int DHT_PIN = 4;
const int POT_PIN = 34;

// Objetos
Device device(128, 64, -1, DHT_PIN, DHT22);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
TelegramHandler telegram(&bot, &device, LED_VERDE, LED_AZUL, POT_PIN);

// Variables
unsigned long tiempoAnterior = 0;

// ==================== SETUP ====================
void setup() {
    Serial.begin(9600);

    // Configurar pines
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_AZUL, OUTPUT);
    pinMode(POT_PIN, INPUT);

    // Estado inicial LEDs
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AZUL, LOW);

    // Inicializar display y sensor
    device.begin();
    telegram.mostrarMensajeInicial();  // Muestra mensaje inicial en display

    // Conectar WiFi
    Serial.print("Conectando a la red ");
    Serial.println(SSID);
    WiFi.begin(SSID, PASS);
    secured_client.setInsecure();  // Para pruebas en Wokwi

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nConectado a la red WiFi. Dirección IP: " + WiFi.localIP().toString());

    // Mensaje inicial en Telegram
    bot.sendMessage(CHAT_ID, "🤖 ESP32 iniciado correctamente\n🌐 IP: " + WiFi.localIP().toString(), "Markdown");

    // Mostrar teclado inicial
    telegram.mostrarTeclado(CHAT_ID);
}

// ==================== LOOP ====================
void loop() {
    if (millis() - tiempoAnterior > TIEMPO_ESCANEO) {
        telegram.procesarMensajes();  // Todo el manejo de mensajes se hace dentro de la librería
        tiempoAnterior = millis();
    }

    delay(500);
}