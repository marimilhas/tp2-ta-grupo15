#ifndef TELEGRAM_HANDLER_H
#define TELEGRAM_HANDLER_H

#include <UniversalTelegramBot.h>
#include "Device.h"

class TelegramHandler {
private:
    UniversalTelegramBot* bot;
    Device* device;
    int ledVerdePin;
    int ledAzulPin;
    int potePin;
    bool ledVerdeEstado;
    bool ledAzulEstado;

public:
    TelegramHandler(UniversalTelegramBot* botInstance, Device* deviceInstance,
                    int ledVerde, int ledAzul, int potPin);

    void procesarMensajes();       
    void mostrarTeclado(String chat_id);
    void mandarMensajeInicial(String chat_id, String ipAddress);
    void mostrarMensajeInicialDisplay();

private:
    void mensajesNuevos(int numerosMensajes);
    void comandoDisplay(String chat_id, String text);
    void mostrarEstadoLedDisplay();
    void mostrarEstadoPoteDisplay();
    void mostrarEstadoDHTDisplay();
    void mostrarComandoNoIdentificado(String comando);
    bool enviarDatosIoT(float temperatura, float humedad);
};

#endif