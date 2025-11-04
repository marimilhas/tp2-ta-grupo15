#include "TelegramHandler.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define THINGSPEAK_API_KEY "OR9FHB2QZL1T73B5"
#define THINGSPEAK_URL "http://api.thingspeak.com/update"

TelegramHandler::TelegramHandler(UniversalTelegramBot* botInstance, Device* deviceInstance,
                                 int ledVerde, int ledAzul, int potPin) {
    bot = botInstance;
    device = deviceInstance;
    ledVerdePin = ledVerde;
    ledAzulPin = ledAzul;
    potePin = potPin;
    ledVerdeEstado = false;
    ledAzulEstado = false;
}

// ==================== Procesar mensajes de Telegram ====================
void TelegramHandler::procesarMensajes() {
    int numerosMensajes = bot->getUpdates(bot->last_message_received + 1);
    while (numerosMensajes) {
        mensajesNuevos(numerosMensajes);
        numerosMensajes = bot->getUpdates(bot->last_message_received + 1);
    }
}

// ==================== Manejo de mensajes ====================
void TelegramHandler::mandarMensajeInicial(String chat_id, String ipAddress)
{
    bot->sendMessage(chat_id, "🤖 ESP32 iniciado correctamente\n🌐 IP: " + ipAddress, "Markdown");

}

void TelegramHandler::mensajesNuevos(int numerosMensajes) {
    for (int i = 0; i < numerosMensajes; i++) {
        String chat_id = bot->messages[i].chat_id;
        String text = bot->messages[i].text;
        String from_name = bot->messages[i].from_name;

        Serial.println("\nMensaje de " + from_name + ": " + text);

        // COMANDO /start
        if (text == "/start" || text == "Menú principal") {
            String welcome = "¡Hola " + from_name + "! 👋\n";
            welcome += "🤖 *ESP32 Control Bot*\n\n";
            welcome += "💡 /led23on - LED verde ON\n";
            welcome += "💡 /led23off - LED verde OFF\n";
            welcome += "💡 /led2on - LED azul ON\n";
            welcome += "💡 /led2off - LED azul OFF\n";
            welcome += "🌡️ /dht22 - Sensor temperatura/humedad\n";
            welcome += "📊 /pote - Valor potenciómetro\n";
            welcome += "☁️ /platiot - Enviar datos IoT\n";
            welcome += "📟 /displayled - Mostrar estado LEDs\n";
            welcome += "📟 /displaypote - Mostrar estado potenciómetro\n";
            welcome += "📟 /displaydht - Mostrar estado sensor DHT22\n";

            bot->sendMessage(chat_id, welcome, "Markdown");
            mostrarTeclado(chat_id);
        }

        // COMANDOS LED
        else if (text == "/led23on") {
            digitalWrite(ledVerdePin, HIGH);
            ledVerdeEstado = true;
            bot->sendMessage(chat_id, "💚 LED VERDE ENCENDIDO", "");
        }
        else if (text == "/led23off") {
            digitalWrite(ledVerdePin, LOW);
            ledVerdeEstado = false;
            bot->sendMessage(chat_id, "💚 LED VERDE APAGADO", "");
        }
        else if (text == "/led2on") {
            digitalWrite(ledAzulPin, HIGH);
            ledAzulEstado = true;
            bot->sendMessage(chat_id, "💙 LED AZUL ENCENDIDO", "");
        }
        else if (text == "/led2off") {
            digitalWrite(ledAzulPin, LOW);
            ledAzulEstado = false;
            bot->sendMessage(chat_id, "💙 LED AZUL APAGADO", "");
        }

        // COMANDO DHT22
        else if (text == "/dht22") {
            float temperatura = device->readTemp();
            float humedad = device->readHum();

            if (isnan(humedad) || isnan(temperatura)) {
                bot->sendMessage(chat_id, "❌ Error leyendo sensor DHT22", "");
            } else {
                String respuesta = "🌡️ *Lectura DHT22*\n\n";
                respuesta += "🔥 Temperatura: " + String(temperatura, 1) + "°C\n";
                respuesta += "💧 Humedad: " + String(humedad, 1) + "%";
                bot->sendMessage(chat_id, respuesta, "Markdown");
            }
        }

        // COMANDO POTENCIÓMETRO
        else if (text == "/pote") {
            int lectura = analogRead(potePin);
            float voltaje = (lectura / 4095.0) * 3.3;

            String respuesta = "📊 *Potenciómetro*\n\n";
            respuesta += "🔢 Valor ADC: " + String(lectura) + "\n";
            respuesta += "⚡ Voltaje: " + String(voltaje, 2) + "V";

            bot->sendMessage(chat_id, respuesta, "Markdown");
        }

        // COMANDO PLATAFORMA IOT
        else if (text == "/platiot") {
            float temperatura = device->readTemp();
            float humedad = device->readHum();

            if (isnan(humedad) || isnan(temperatura)) {
                bot->sendMessage(chat_id, "❌ Error leyendo sensor para IoT", "");
            } else {
                bool exito = enviarDatosIoT(temperatura, humedad);
                if (exito) {
                    String respuesta = "☁️ *Datos enviados a IoT*\n\n";
                    respuesta += "📊 Temperatura: " + String(temperatura, 1) + "°C\n";
                    respuesta += "💧 Humedad: " + String(humedad, 1) + "%\n\n";
                    respuesta += "✅ Datos enviados correctamente";
                    bot->sendMessage(chat_id, respuesta, "Markdown");
                } else {
                    bot->sendMessage(chat_id, "❌ Error enviando datos a IoT", "");
                }
            }
        }

        // COMANDOS DISPLAY
        else if (text.startsWith("/display")) {
            comandoDisplay(chat_id, text);
        }

        // COMANDO NO RECONOCIDO
        else {
            bot->sendMessage(chat_id, "❌ Comando no reconocido. Use /start para ver comandos.", "");
            mostrarTeclado(chat_id);
        }
    }
}

// ==================== TECLADO ====================
void TelegramHandler::mostrarTeclado(String chat_id) {
    String keyboardJson = "[[\"/led23on\", \"/led23off\"], [\"/led2on\", \"/led2off\"], "
                          "[\"/dht22\", \"/pote\"], [\"/displayled\", \"/displaydht\", \"/displaypote\"], "
                          "[\"/platiot\", \"/start\"]]";
    bot->sendMessageWithReplyKeyboard(chat_id, "Usa los botones o escribe un comando:", "", keyboardJson, true);
}

// ==================== FUNCIONES DISPLAY ====================
void TelegramHandler::mostrarMensajeInicialDisplay()
{
    String mensaje = "ESP32\n";
    mensaje += "Telegram Bot\n";
    mensaje += "------------\n";
    mensaje += "Esperando\n";
    mensaje += "comandos...\n";
    device->showDisplay(mensaje.c_str());
}

void TelegramHandler::comandoDisplay(String chat_id, String text) {
    String comando = text.substring(8);

    if (comando == "led")
    {
        bot->sendMessage(chat_id, "📟 Estado de los led mostrado en el display", "");
        mostrarEstadoLedDisplay();
    }
    else if (comando == "pote")
    {
        bot->sendMessage(chat_id, "📟 Estado del potenciómetro mostrado en el display", "");
        mostrarEstadoPoteDisplay();
    }
    else if (comando == "dht" || comando == "dht22")
    {
        bot->sendMessage(chat_id, "📟 Estado del sensor DHT22 mostrado en el display", "");
        mostrarEstadoDHTDisplay();
    }
    else
    {
        mostrarComandoNoIdentificado(comando);
        bot->sendMessage(chat_id, "❌ Comando display no reconocido: " + comando, "");
    }
}

void TelegramHandler::mostrarEstadoLedDisplay() {
    String mensaje = "=== ESTADO LEDS ===\n\n";
    mensaje += "LED Verde (GPIO 23):\n";
    mensaje += ledVerdeEstado ? "> ENCENDIDO\n\n" : "> APAGADO\n\n";
    mensaje += "LED Azul (GPIO 2):\n";
    mensaje += ledAzulEstado ? "> ENCENDIDO" : "> APAGADO";

    device->showDisplay(mensaje.c_str());
}

void TelegramHandler::mostrarEstadoPoteDisplay() {
    int lectura = analogRead(potePin);
    float voltaje = (lectura / 4095.0) * 3.3;

    String mensaje = "=== POTENCIOMETRO ===\n\n";
    mensaje += "Valor ADC:\n> " + String(lectura) + "\n\n";
    mensaje += "Voltaje:\n> " + String(voltaje, 2) + "V";

    device->showDisplay(mensaje.c_str());
}

void TelegramHandler::mostrarEstadoDHTDisplay() {
    float temperatura = device->readTemp();
    float humedad = device->readHum();

    String mensaje = "=== SENSOR DHT22 ===\n\n";

    if (isnan(humedad) || isnan(temperatura)) {
        mensaje += "Error lectura\nsensor DHT22";
    } else {
        mensaje += "Temperatura:\n> " + String(temperatura, 1) + " C\n\n";
        mensaje += "Humedad:\n> " + String(humedad, 1) + " %";
    }

    device->showDisplay(mensaje.c_str());
}

void TelegramHandler::mostrarComandoNoIdentificado(String comando) {
    String mensaje = "=== COMANDO ERROR ===\n\n";
    mensaje += "Comando no reconocido:\n> " + comando + "\n\n";
    mensaje += "Comandos validos:\n> led, pote, dht";

    device->showDisplay(mensaje.c_str());
}

// ==================== FUNCIÓN IOT ====================
bool TelegramHandler::enviarDatosIoT(float temperatura, float humedad) {
    Serial.println("\n📡 Enviando a IoT:");
    Serial.println("   Temperatura: " + String(temperatura) + "°C");
    Serial.println("   Humedad: " + String(humedad) + "%");

    device->showDisplay("Enviando datos\na IoT...");

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String("http://api.thingspeak.com/update") +
                     "?api_key=" + THINGSPEAK_API_KEY +
                     "&field1=" + String(humedad) +
                     "&field2=" + String(temperatura);

        Serial.println("   URL: " + url);

        http.begin(url);
        int httpCode = http.GET();
        Serial.println("   Código HTTP: " + String(httpCode));

        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println("   Respuesta: " + payload);
            device->showDisplay("Datos enviados\ncorrectamente a IoT");

        } else {
            Serial.println("   ❌ Error en GET: " + String(http.errorToString(httpCode)));
        }

        http.end();
        return (httpCode == 200);
    } else {
        Serial.println("❌ No hay conexión WiFi");
        return false;
    }
}