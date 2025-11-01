#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "Device.h"

// ==================== CONFIGURACIÓN ====================
const char *SSID = "Personal-647";
const char *PASS = "PUW9aaYPUd";
const char *BOT_TOKEN = "8306829882:AAFOVJIXxXQhbgHjnBWYxudMhXb0WbIE-74";
const String CHAT_ID = "7965286702";
const unsigned long TIEMPO_ESCANEO = 1000; 
// const char *mqtt_server = "test.mosquitto.org";
// const int mqtt_port = 1883;

// Pines
const int LED_VERDE = 23;
const int LED_AZUL = 2;
const int DHT_PIN = 4;
const int POT_PIN = 34;

// Objetos
Device _device(128, 64, -1, DHT_PIN, DHT22);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// Variables globales
unsigned long tiempoAnterior;
bool ledVerdeEstado = false;
bool ledAzulEstado = false;

// ==================== DECLARACIONES DE FUNCIONES ====================
void mensajesNuevos(int numerosMensajes);
void comandoDisplay(String chat_id, String text);
void mostrarMensajeInicial();
void mostrarEstadoLedDisplay();
void mostrarEstadoPoteDisplay();
void mostrarEstadoDHTDisplay();
void mostrarComandoNoIdentificado(String comando);
bool enviarDatosIoT(float temperatura, float humedad);

// ==================== SETUP ====================
void setup()
{
  Serial.begin(9600);

  // Inicializar pines
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(POT_PIN, INPUT);

  // Estado inicial LEDs
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AZUL, LOW);

  // Inicializar Device (display y sensor)
  _device.begin();
  mostrarMensajeInicial();

  // Conectar WiFi
  Serial.print("Conectando a la red ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASS);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.print("\nConectado a la red wifi. Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Mensaje de inicio
  bot.sendMessage(CHAT_ID, "✅ Sistema ESP32 iniciado correctamente\nIP: " + WiFi.localIP().toString(), "");
}

// ==================== LOOP PRINCIPAL ====================
void loop()
{
  // Verifica si hay datos nuevos en telegram cada 1 segundo
  if (millis() - tiempoAnterior > TIEMPO_ESCANEO)
  {
    int numerosMensajes = bot.getUpdates(bot.last_message_received + 1);

    while (numerosMensajes)
    {
      Serial.println("Comando recibido");
      mensajesNuevos(numerosMensajes);
      numerosMensajes = bot.getUpdates(bot.last_message_received + 1);
    }

    tiempoAnterior = millis();
  }
}

// ==================== MANEJO DE MENSAJES ====================
void mensajesNuevos(int numerosMensajes)
{
  for (int i = 0; i < numerosMensajes; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.println("Mensaje de " + from_name + ": " + text);

    // COMANDO /start
    if (text == "/start")
    {
      String welcome = "¡Hola " + from_name + "! 👋\n";
      welcome += "Sistema ESP32 Control Bot\n\n";
      welcome += "Comandos disponibles:\n";
      welcome += "💡 /led23on - LED verde ON\n";
      welcome += "💡 /led23off - LED verde OFF\n";
      welcome += "💡 /led2on - LED azul ON\n";
      welcome += "💡 /led2off - LED azul OFF\n";
      welcome += "🌡️ /dht22 - Sensor temperatura/humedad\n";
      welcome += "📊 /pote - Valor potenciómetro\n";
      welcome += "☁️ /platiot - Enviar datos IoT\n";
      welcome += "📟 /displayled - Mostrar estado LEDs\n";
      welcome += "📟 /displaypote - Mostrar potenciómetro\n";
      welcome += "📟 /displaydht - Mostrar sensor DHT22\n";

      bot.sendMessage(chat_id, welcome, "");
    }

    // COMANDOS LED
    else if (text == "/led23on")
    {
      digitalWrite(LED_VERDE, HIGH);
      ledVerdeEstado = true;
      bot.sendMessage(chat_id, "✅ LED VERDE ENCENDIDO", "");
      mostrarEstadoLedDisplay();
    }
    else if (text == "/led23off")
    {
      digitalWrite(LED_VERDE, LOW);
      ledVerdeEstado = false;
      bot.sendMessage(chat_id, "✅ LED VERDE APAGADO", "");
      mostrarEstadoLedDisplay();
    }
    else if (text == "/led2on")
    {
      digitalWrite(LED_AZUL, HIGH);
      ledAzulEstado = true;
      bot.sendMessage(chat_id, "✅ LED AZUL ENCENDIDO", "");
      mostrarEstadoLedDisplay();
    }
    else if (text == "/led2off")
    {
      digitalWrite(LED_AZUL, LOW);
      ledAzulEstado = false;
      bot.sendMessage(chat_id, "✅ LED AZUL APAGADO", "");
      mostrarEstadoLedDisplay();
    }

    // COMANDO DHT22 
    else if (text == "/dht22")
    {
      float temperatura = _device.readTemp();
      float humedad = _device.readHum();

      if (isnan(humedad) || isnan(temperatura))
      {
        bot.sendMessage(chat_id, "❌ Error leyendo sensor DHT22", "");
      }
      else
      {
        String respuesta = "🌡️ *Lectura DHT22*\n\n";
        respuesta += "📊 Temperatura: " + String(temperatura, 1) + "°C\n";
        respuesta += "💧 Humedad: " + String(humedad, 1) + "%";
        bot.sendMessage(chat_id, respuesta, "Markdown");
      }
    }

    // COMANDO POTENCIÓMETRO
    else if (text == "/pote")
    {
      int lectura = analogRead(POT_PIN);
      float voltaje = (lectura / 4095.0) * 3.3;

      String respuesta = "📊 *Potenciómetro*\n\n";
      respuesta += "🔢 Valor ADC: " + String(lectura) + "\n";
      respuesta += "⚡ Voltaje: " + String(voltaje, 2) + "V";

      bot.sendMessage(chat_id, respuesta, "Markdown");
    }

    // COMANDO PLATAFORMA IOT (USANDO LA LIBRERÍA DEVICE)
    else if (text == "/platiot")
    {
      float temperatura = _device.readTemp();
      float humedad = _device.readHum();

      if (isnan(humedad) || isnan(temperatura))
      {
        bot.sendMessage(chat_id, "❌ Error leyendo sensor para IoT", "");
      }
      else
      {
        bool exito = enviarDatosIoT(temperatura, humedad);
        if (exito)
        {
          String respuesta = "☁️ *Datos enviados a IoT*\n\n";
          respuesta += "📊 Temperatura: " + String(temperatura, 1) + "°C\n";
          respuesta += "💧 Humedad: " + String(humedad, 1) + "%\n";
          respuesta += "✅ Datos enviados correctamente";
          bot.sendMessage(chat_id, respuesta, "Markdown");
        }
        else
        {
          bot.sendMessage(chat_id, "❌ Error enviando datos a IoT", "");
        }
      }
    }

    // COMANDOS DISPLAY
    else if (text.startsWith("/display"))
    {
      comandoDisplay(chat_id, text);
    }
    
    // COMANDO NO RECONOCIDO
    else
    {
      bot.sendMessage(chat_id, "❌ Comando no reconocido. Use /start para ver comandos.", "");
    }
  }
}

// ==================== FUNCIONES DISPLAY ====================
void mostrarMensajeInicial()
{
  String mensaje = "SISTEMA ESP32\n";
  mensaje += "Telegram Bot\n";
  mensaje += "------------\n";
  mensaje += "Esperando\n";
  mensaje += "comandos...\n";
  mensaje += "------------\n";
  mensaje += "IP: " + WiFi.localIP().toString();

  _device.showDisplay(mensaje.c_str());
}

void comandoDisplay(String chat_id, String text)
{
  String comando = text.substring(8);

  if (comando == "led")
  {
    mostrarEstadoLedDisplay();
    bot.sendMessage(chat_id, "📟 Estado LED mostrado en display", "");
  }
  else if (comando == "pote")
  {
    mostrarEstadoPoteDisplay();
    bot.sendMessage(chat_id, "📟 Estado potenciómetro mostrado en display", "");
  }
  else if (comando == "dht" or comando == "dht22")
  {
    mostrarEstadoDHTDisplay();
    bot.sendMessage(chat_id, "📟 Estado DHT22 mostrado en display", "");
  }
  else
  {
    mostrarComandoNoIdentificado(comando);
    bot.sendMessage(chat_id, "❌ Comando display no reconocido: " + comando, "");
  }
}

void mostrarEstadoLedDisplay()
{
  String mensaje = "=== ESTADO LEDS ===\n\n";
  mensaje += "LED Verde (GPIO 23):\n";
  mensaje += ledVerdeEstado ? "> ENCENDIDO\n\n" : "> APAGADO\n\n";
  mensaje += "LED Azul (GPIO 2):\n";
  mensaje += ledAzulEstado ? "> ENCENDIDO" : "> APAGADO";

  _device.showDisplay(mensaje.c_str());
}

void mostrarEstadoPoteDisplay()
{
  int lectura = analogRead(POT_PIN);
  float voltaje = (lectura / 4095.0) * 3.3;

  String mensaje = "=== POTENCIOMETRO ===\n\n";
  mensaje += "Valor ADC:\n";
  mensaje += "> " + String(lectura) + "\n\n";
  mensaje += "Voltaje:\n";
  mensaje += "> " + String(voltaje, 2) + "V";

  _device.showDisplay(mensaje.c_str());
}

void mostrarEstadoDHTDisplay()
{
  float temperatura = _device.readTemp();
  float humedad = _device.readHum();

  String mensaje = "=== SENSOR DHT22 ===\n\n";

  if (isnan(humedad) || isnan(temperatura))
  {
    mensaje += "Error lectura\n";
    mensaje += "sensor DHT22";
  }
  else
  {
    mensaje += "Temperatura:\n";
    mensaje += "> " + String(temperatura, 1) + " C\n\n";
    mensaje += "Humedad:\n";
    mensaje += "> " + String(humedad, 1) + " %";
  }

  _device.showDisplay(mensaje.c_str());
}

// ==================== FUNCIÓN PARA COMANDO NO IDENTIFICADO ====================
void mostrarComandoNoIdentificado(String comando)
{
  String mensaje = "=== COMANDO ERROR ===\n\n";
  mensaje += "Comando no reconocido:\n";
  mensaje += "> " + comando + "\n\n";
  mensaje += "Comandos válidos:\n";
  mensaje += "> led, pote, dht";

  _device.showDisplay(mensaje.c_str());
}

// ==================== FUNCIÓN IOT ====================
bool enviarDatosIoT(float temperatura, float humedad)
{
  // Simular envío a plataforma IoT
  Serial.println("📡 Enviando a IoT:");
  Serial.println("   Temperatura: " + String(temperatura) + "°C");
  Serial.println("   Humedad: " + String(humedad) + "%");

  // Aquí implementarías el código específico para la plataforma IoT elegida
  // ThingSpeak, Arduino IoT Cloud, etc.

  delay(500);  // Simular tiempo de envío
  return true; // Simular éxito
}