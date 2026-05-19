#include "mmwave_for_xiao.h"


#define BUFFER_SIZE 256 // Größe des seriellen Puffers

Seeed_HSP24::Seeed_HSP24(Stream &serial)
    : _serial(&serial),
    _debugSerial(nullptr),
    bufferIndex(0),
    receiveStartTime(0),
    isInATMode(0),
    bufferIndex_hsp24(0)
{
}

Seeed_HSP24::Seeed_HSP24(Stream &serial, Stream &debugSerial)
    : _serial(&serial),
    _debugSerial(&debugSerial),
    bufferIndex(0),
    receiveStartTime(0),
    isInATMode(0),
    bufferIndex_hsp24(0)
{
}

// B35
int Seeed_HSP24::enterATMode()
{
    _serial->print("+++"); // Befehl +++ senden

    unsigned long startTime = millis(); // Startzeit merken

    while (true)
    {
        // Prüfen ob Daten zum Empfangen bereit sind
        while (_serial->available())
        {
            char receivedChar = _serial->read();
            buffer[bufferIndex] = receivedChar;
            bufferIndex++;

            // Empfangsstartzeit aktualisieren
            receiveStartTime = millis();
        }

        // Prüfen ob Sendebedingung erfüllt ist
        if (bufferIndex > 0 && (bufferIndex >= BUFFER_SIZE || millis() - receiveStartTime >= RECEIVE_TIMEOUT))
        {
            if (this->bufferIndex == 1 && buffer[0] == 'a')
            {
                _serial->print('a');
            }
            else if (buffer[0] == 'O' && buffer[1] == 'K')
            { // AT-Modus betreten
                if (_debugSerial != nullptr && _debugSerial->available() > 0)
                {
                    _debugSerial->println("Enter AT Mode Success!");
                }

                isInATMode = 1;
                // Puffer und Index zurücksetzen
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;
                return 1;
            }
            else
            {
                // Puffer und Index zurücksetzen
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;
                return 0;
            }
            // Puffer und Index zurücksetzen
            memset(this->buffer, 0, BUFFER_SIZE);
            this->bufferIndex = 0;
        }

        if (millis() - startTime > 5000)
        {             // Wartezeit von 5 Sekunden überschritten
            return 0; // Timeout, AT-Modus nicht erfolgreich betreten
        }
    }

    return 0; // AT-Modus nicht erfolgreich betreten
}

int Seeed_HSP24::exitATMode()
{
    _serial->println("at+reconn=1");

    unsigned long startTime = millis(); // Startzeit merken

    while (true)
    {
        // Prüfen ob Daten zum Empfangen bereit sind
        while (_serial->available())
        {
            char receivedChar = _serial->read();   // Daten vom seriellen Port empfangen
            buffer[bufferIndex] = receivedChar;    // Empfangene Daten in Puffer speichern
            bufferIndex++;

            // Empfangsstartzeit aktualisieren
            receiveStartTime = millis();
        }

        // Prüfen ob Sendebedingung erfüllt ist
        if (bufferIndex > 0 && (bufferIndex >= BUFFER_SIZE || millis() - receiveStartTime >= RECEIVE_TIMEOUT))
        {
            // Prüfen ob Puffer "ok" enthält
            char *found = strstr(this->buffer, "ok");
            if (found != NULL)
            {
                if (_debugSerial != nullptr && _debugSerial->available() > 0)
                {
                    _debugSerial->println("Setting Success!");
                }

                // Puffer und Index zurücksetzen
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;
                return 1;
            }
            else
            {
                if (_debugSerial != nullptr && _debugSerial->available() > 0)
                {
                    // Pufferdaten an anderen seriellen Port senden
                    for (int i = 0; i < this->bufferIndex; i++)
                    {
                        _debugSerial->print(buffer[i]);
                    }
                }

                // Puffer und Index zurücksetzen
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;
                return 0;
            }
        }

        if (millis() - startTime > 3000) // Wartezeit von 3 Sekunden überschritten
        {
            return 0; // Timeout, AT-Modus nicht erfolgreich verlassen
        }
    }
}

int Seeed_HSP24::checkBuffer()
{
    unsigned long startTime = millis(); // Startzeit merken

    while (true)
    {
        // Prüfen ob Daten zum Empfangen bereit sind
        while (_serial->available())
        {
            char receivedChar = _serial->read();   // Daten vom seriellen Port empfangen
            buffer[bufferIndex] = receivedChar;    // Empfangene Daten in Puffer speichern
            bufferIndex++;

            // Empfangsstartzeit aktualisieren
            receiveStartTime = millis();
        }

        // Prüfen ob Sendebedingung erfüllt ist
        if (bufferIndex > 0 && (bufferIndex >= BUFFER_SIZE || millis() - receiveStartTime >= RECEIVE_TIMEOUT))
        {
            // Prüfen ob Puffer "ok" enthält
            char *found = strstr(this->buffer, "ok");
            if (found != NULL)
            {
                if (_debugSerial != nullptr && _debugSerial->available() > 0)
                {
                    _debugSerial->println("Setting Success!");
                }

                // Puffer und Index zurücksetzen
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;

                return 1;
            }
            else
            {
                if (_debugSerial != nullptr && _debugSerial->available() > 0)
                {
                    // Pufferdaten an anderen seriellen Port senden
                    for (int i = 0; i < this->bufferIndex; i++)
                    {
                        _debugSerial->print(buffer[i]);
                    }
                }

                // Puffer und Index zurücksetzen
                memset(this->buffer, 0, BUFFER_SIZE);
                this->bufferIndex = 0;

                return 0;
            }
        }

        if (millis() - startTime > 3000) // Wartezeit von 3 Sekunden überschritten
        {
            return 0; // Timeout, AT-Modus nicht erfolgreich betreten
        }
    }
}

int Seeed_HSP24::sendATCommandWithExit(String command)
{
    _serial->println(command);
    int ret = checkBuffer();
    exitATMode();
    return ret;
}

int Seeed_HSP24::sendATCommand(String command)
{
    _serial->println(command);
    int ret = checkBuffer();
    return ret;
}

int Seeed_HSP24::getVer()
{
    enterATMode(); // AT-Modus betreten
    return sendATCommandWithExit("at+ver=?");
}

int Seeed_HSP24::setNetwork(String ssid, String password)
{
    enterATMode(); // AT-Modus betreten
    sendATCommand("at+netmode=2");
    sendATCommand("at+wifi_conf=" + ssid + ",none," + password);
    sendATCommandWithExit("at+net_commit=1");
    return 1;
}

Seeed_HSP24::TargetStatus status = Seeed_HSP24::TargetStatus::NoTarget;

// Aktive Meldung (Push)
const byte Seeed_HSP24::frameStart[Seeed_HSP24::FRAME_START_SIZE] = {0xF4, 0xF3, 0xF2, 0xF1};
const byte Seeed_HSP24::frameEnd[Seeed_HSP24::FRAME_END_SIZE] = {0xF8, 0xF7, 0xF6, 0xF5};

// Abfrage-Antwort
const byte Seeed_HSP24::frameAskStart[Seeed_HSP24::FRAME_START_SIZE] = {0xFD, 0xFC, 0xFB, 0xFA};
const byte Seeed_HSP24::frameAskEnd[Seeed_HSP24::FRAME_END_SIZE] = {0x04, 0x03, 0x02, 0x01};

Seeed_HSP24::RadarStatus Seeed_HSP24::getStatus()
{
    bufferIndex_hsp24 = 0; // Pufferindex zurücksetzen
    Seeed_HSP24::RadarStatus radarStatus;
    const int MIN_FRAME_LENGTH_BASE = 23;        // Mindest-Framelänge, Basismodus
    const int MIN_FRAME_LENGTH_ENGINEERING = 45; // Mindest-Framelänge, Ingenieurbetrieb

    // Daten vom seriellen Port lesen und im Puffer speichern
    while (_serial->available() && bufferIndex_hsp24 < BUFFER_SIZE)
    {
        buffer_hsp24[bufferIndex_hsp24] = _serial->read();
        bufferIndex_hsp24++;

        // Prüfen ob vollständiger Frame empfangen wurde
        int startIndex = findSequence(buffer_hsp24, bufferIndex_hsp24, frameStart, 4);
        int endIndex = findSequence(buffer_hsp24, bufferIndex_hsp24, frameEnd, 4);

        // Prüfen ob vollständiger Abfrage-Frame empfangen wurde
        int startAskIndex = findSequence(buffer_hsp24, bufferIndex_hsp24, frameAskStart, 4);
        int endAskIndex = findSequence(buffer_hsp24, bufferIndex_hsp24, frameAskEnd, 4);

        uint8_t finalBuffer[(endIndex + 4) - startIndex];
        // Vollständigen Frame empfangen
        if (startIndex != -1 && endIndex != -1 && endIndex > startIndex)
        {
            uint8_t tmp_buffer[(endIndex + 4) - startIndex];
            for (int i = startIndex; i < endIndex + 4; i++)
            {
                tmp_buffer[i] = buffer_hsp24[i];
            }
            // Array auswerten
            int lastFrameStart = -1;
            int lastFrameEnd = -1;
            int tmp_bufferSize = sizeof(tmp_buffer);
            for (int i = tmp_bufferSize - 4; i >= 0; i--)
            {
                if (tmp_buffer[i] == 0xF4 && tmp_buffer[i + 1] == 0xF3 && tmp_buffer[i + 2] == 0xF2 && tmp_buffer[i + 3] == 0xF1 && lastFrameStart == -1)
                {
                    lastFrameStart = i;
                }
                if (tmp_buffer[i] == 0xF8 && tmp_buffer[i + 1] == 0xF7 && tmp_buffer[i + 2] == 0xF6 && tmp_buffer[i + 3] == 0xF5 && lastFrameEnd == -1)
                {
                    lastFrameEnd = i;
                }
            }
            if (lastFrameStart != -1 && lastFrameEnd != -1 && lastFrameStart < lastFrameEnd && (lastFrameEnd - lastFrameStart + 4) >= MIN_FRAME_LENGTH_BASE)
            {
                int j = 0;
                for (int i = lastFrameStart; i <= lastFrameEnd + 3; i++)
                {
                    finalBuffer[j++] = tmp_buffer[i];
                }
            }
            else
            {
                // Frame unvollständig oder zu kurz: finalBuffer leeren
                memset(finalBuffer, 0, sizeof(finalBuffer));
                return radarStatus;
            }
            // Array-Auswertung abgeschlossen
            // Arraylänge ermitteln
            int finalBufferSize = (lastFrameEnd + 4) - lastFrameStart;

            // Bit 7 bestimmt ob Ingenieurbetrieb- oder Basismodus-Daten vorliegen
            int mode = finalBuffer[6];
            radarStatus.radarMode = mode;
            if (mode == 1) // Ingenieurbetrieb-Meldung
            {
                if (finalBufferSize > 45)
                {
                    return radarStatus;
                }
                else
                {
                    radarStatus.moveSetDistance = finalBuffer[17];   // Weitestes Bewegungs-Entfernungstor
                    radarStatus.staticSetDistance = finalBuffer[18]; // Weitestes Stillstands-Entfernungstor
                    radarStatus.photosensitive = finalBuffer[37];    // Lichtsensor
                    for (int i = 0; i < 9; i++) // Energiewerte je Entfernungstor für Bewegung und Stillstand
                    {
                        radarStatus.radarMovePower.moveGate[i] = finalBuffer[i + 19];
                        radarStatus.radarStaticPower.staticGate[i] = finalBuffer[i + 28];
                    }
                }
            }
            else if (mode == 2) // Basismodus-Meldung
            {
                if (finalBufferSize > 23)
                {
                    return radarStatus;
                }
            }

            // Vom Radar gemeldeten Status auslesen
            if (_debugSerial != nullptr && _debugSerial->available() > 0)
            {
                _debugSerial->println(finalBuffer[8]);
            }
            status = static_cast<Seeed_HSP24::TargetStatus>(finalBuffer[8]);
            radarStatus.targetStatus = static_cast<Seeed_HSP24::TargetStatus>(finalBuffer[8]);

            // Zielentfernung auslesen
            if(radarStatus.targetStatus != Seeed_HSP24::TargetStatus::ErrorFrame)
            {
                int distance = finalBuffer[15] | (finalBuffer[16] << 8); // Little-Endian auswerten
                if (_debugSerial != nullptr && _debugSerial->available() > 0)
                {
                    _debugSerial->println("distance: " + String(distance));
                }
                radarStatus.distance = distance;
            }
            else
            {
                radarStatus.distance = -1;
            }

             // _debugSerial->print("nature: ");
            // for (int i = 0; i < sizeof(tmp_buffer); i++)
            // {
            //     if (tmp_buffer[i] < 0x10)
            //         _debugSerial->print("0");
            //     _debugSerial->print(tmp_buffer[i], HEX);
            //     _debugSerial->print(" ");
            // }
            // _debugSerial->println();

            // _debugSerial->print("Payload: ");
            // for (int i = 0; i < finalBufferSize; i++)
            // {
            //     if (finalBuffer[i] < 0x10)
            //         _debugSerial->print("0");
            //     _debugSerial->print(finalBuffer[i], HEX);
            //     _debugSerial->print(" ");
            // }
            // _debugSerial->println();

            // Puffer bereinigen
            int bytesToMove = bufferIndex_hsp24 - (endIndex + 4);
            for (int i = 0; i < bytesToMove; i++)
            {
                buffer_hsp24[i] = buffer_hsp24[endIndex + 4 + i];
            }
            bufferIndex_hsp24 = bytesToMove;

            return radarStatus;
        }

        else if (startAskIndex != -1 && endAskIndex != -1 && endAskIndex > startAskIndex)
        {
            uint8_t tmp_buffer[(endAskIndex + 4) - startAskIndex];
            for (int i = startAskIndex; i < endAskIndex + 4; i++)
            {
                tmp_buffer[i] = buffer_hsp24[i];
            }

            // Array auswerten
            int lastFrameStart = -1;
            int lastFrameEnd = -1;
            int tmp_bufferSize = sizeof(tmp_buffer);
            for (int i = tmp_bufferSize - 4; i >= 0; i--)
            {
                if (tmp_buffer[i] == 0xFD && tmp_buffer[i + 1] == 0xFC && tmp_buffer[i + 2] == 0xFB && tmp_buffer[i + 3] == 0xFA && lastFrameStart == -1)
                {
                    lastFrameStart = i;
                }
                if (tmp_buffer[i] == 0x04 && tmp_buffer[i + 1] == 0x03 && tmp_buffer[i + 2] == 0x02 && tmp_buffer[i + 3] == 0x01 && lastFrameEnd == -1)
                {
                    lastFrameEnd = i;
                }
            }
            if (lastFrameStart != -1 && lastFrameEnd != -1 && lastFrameStart < lastFrameEnd)
            {
                int j = 0;
                for (int i = lastFrameStart; i <= lastFrameEnd + 3; i++)
                {
                    finalBuffer[j++] = tmp_buffer[i];
                }
            }
            else
            {
                // Frame unvollständig oder zu kurz: finalBuffer leeren
            memset(finalBuffer, 0, sizeof(finalBuffer));
            // return Seeed_HSP24::AskStatus::Error;
            return radarStatus;
        }
            // Array-Auswertung abgeschlossen

            // Puffer bereinigen
           int finalBufferSize = (lastFrameEnd + 4) - lastFrameStart;

            // _debugSerial->print("nature: ");
            // for (int i = 0; i < sizeof(tmp_buffer); i++)
            // {
            //     if (tmp_buffer[i] < 0x10)
            //         _debugSerial->print("0");
            //     _debugSerial->print(tmp_buffer[i], HEX);
            //     _debugSerial->print(" ");
            // }
            // _debugSerial->println();

            // _debugSerial->print("Payload: ");
            // for (int i = 0; i < finalBufferSize; i++)
            // {
            //     if (finalBuffer[i] < 0x10)
            //         _debugSerial->print("0");
            //     _debugSerial->print(finalBuffer[i], HEX);
            //     _debugSerial->print(" ");
            // }
            // _debugSerial->println();

            //Clear Buffer/Flush Buffer
            int bytesToMove = bufferIndex_hsp24 - (endAskIndex + 4);
            for (int i = 0; i < bytesToMove; i++)
            {
                buffer_hsp24[i] = buffer_hsp24[endAskIndex + 4 + i];
            }
            bufferIndex_hsp24 = bytesToMove;

            if (finalBuffer[8] == 0)
            {                
                // return Seeed_HSP24::AskStatus::Success;
                return radarStatus;
            }
            else
            {
                // return Seeed_HSP24::AskStatus::Error;
                return radarStatus;
            }
        }
    }
    return radarStatus;
}

Seeed_HSP24::DataResult Seeed_HSP24::sendCommand(const byte *sendData, int sendDataLength)
{
    bufferIndex_hsp24 = 0; // Pufferindex zurücksetzen
    Seeed_HSP24::DataResult dataResult;

    /*
        Endlosschleife beim Senden eines Befehls: Empfang bis gültige Antwort
        eingetroffen ist, dann Schleife verlassen. Bei keiner Antwort nach 2
        Sekunden erneut senden. Maximal 3 Versuche, dann Schleife verlassen.
    */
    int tryTimes = 0;                        // Anzahl Wiederholungsversuche
    unsigned long startTime = millis();      // Startzeit merken
    unsigned long lastSendTime = 0;          // Zeitpunkt des letzten Sendens
    const unsigned long sendInterval = 1000; // Sendeintervall: 1000 ms = 1 Sekunde
                                             // Puffer leeren
    memset(buffer_hsp24, 0, sizeof(buffer_hsp24));
    bufferIndex_hsp24 = 0;
    while (true)
    {
        if (millis() - lastSendTime >= sendInterval)
        {
            // Aktivierungsbefehl über seriellen Port senden
            _serial->write(sendData, sendDataLength);
            tryTimes++;
            lastSendTime = millis(); // Sendezeitpunkt aktualisieren
            if (_debugSerial != nullptr && _debugSerial->available() > 0)
            {
                _debugSerial->println("times: " + String(tryTimes));
            }
        }

        // Daten vom seriellen Port lesen und im Puffer speichern
        while (_serial->available() && bufferIndex_hsp24 < BUFFER_SIZE)
        {
            buffer_hsp24[bufferIndex_hsp24] = _serial->read();
            bufferIndex_hsp24++;

            // Prüfen ob vollständiger Frame empfangen wurde
            int startIndex = findSequence(buffer_hsp24, bufferIndex_hsp24, frameAskStart, 4);
            int endIndex = findSequence(buffer_hsp24, bufferIndex_hsp24, frameAskEnd, 4);

            // static uint8_t finalBuffer[128];
            uint8_t finalBuffer[(endIndex + 4) - startIndex];
            // Vollständigen Frame empfangen
            if (startIndex != -1 && endIndex != -1 && endIndex > startIndex)
            {
                uint8_t tmp_buffer[(endIndex + 4) - startIndex];
                if (_debugSerial != nullptr && _debugSerial->available() > 0)
                {
                    _debugSerial->println("start: " + String(startIndex) + "  end: " + String(endIndex));
                }
                int j = 0;
                for (int i = startIndex; i < endIndex + 4; i++)
                {
                    tmp_buffer[j++] = buffer_hsp24[i];
                }
                // Array auswerten
                int lastFrameStart = -1;
                int lastFrameEnd = -1;
                int tmp_bufferSize = sizeof(tmp_buffer);
                for (int i = tmp_bufferSize - 4; i >= 0; i--)
                {
                    if (tmp_buffer[i] == 0xFD && tmp_buffer[i + 1] == 0xFC && tmp_buffer[i + 2] == 0xFB && tmp_buffer[i + 3] == 0xFA && lastFrameStart == -1)
                    {
                        lastFrameStart = i;
                    }
                    if (tmp_buffer[i] == 0x04 && tmp_buffer[i + 1] == 0x03 && tmp_buffer[i + 2] == 0x02 && tmp_buffer[i + 3] == 0x01 && lastFrameEnd == -1)
                    {
                        lastFrameEnd = i;
                    }
                }

                if (lastFrameStart != -1 && lastFrameEnd != -1 && lastFrameStart < lastFrameEnd)
                {
                    int j = 0;
                    for (int i = lastFrameStart; i <= lastFrameEnd + 3; i++)
                    {
                        finalBuffer[j++] = tmp_buffer[i];
                    }
                    // _debugSerial->println("finnsh!!!");
                    dataResult.length = (endIndex + 4) - startIndex;
                    dataResult.resultBuffer = finalBuffer;

                    if (_debugSerial != nullptr && _debugSerial->available() > 0)
                    {
                        _debugSerial->println("resultBuffer: ");
                        for (int i = 0; i < dataResult.length; i++)
                        {
                            if (dataResult.resultBuffer[i] < 0x10)
                                _debugSerial->print("0");
                            _debugSerial->print(dataResult.resultBuffer[i], HEX); // Jeden Byte ausgeben
                            _debugSerial->print(" ");
                        }
                        _debugSerial->println("");
                        _debugSerial->println("length: " + String(dataResult.length));
                    }
                    return dataResult;
                }
                else
                {
                    // Frame unvollständig oder zu kurz: finalBuffer mit -1 füllen
                    memset(finalBuffer, -1, sizeof(finalBuffer));
                    if (_debugSerial != nullptr && _debugSerial->available() > 0)
                    {
                        _debugSerial->println("error!!!");
                    }
                    dataResult.length = -1;
                    return dataResult;
                }
                // Array-Auswertung abgeschlossen

                // Arraylänge ermitteln
                int finalBufferSize = (lastFrameEnd + 4) - lastFrameStart;

                // Puffer bereinigen
                int bytesToMove = bufferIndex_hsp24 - (endIndex + 4);
                for (int i = 0; i < bytesToMove; i++)
                {
                    buffer_hsp24[i] = buffer_hsp24[endIndex + 4 + i];
                }
                bufferIndex_hsp24 = bytesToMove;
            }
        }

        if (millis() - startTime > 6000 || tryTimes > 6) // Wartezeit von 6 Sekunden überschritten
        {
            // return Seeed_HSP24::AskStatus::Error; // Zeitüberschreitung, Fehler beim Wechsel in den AT-Modus
            dataResult.length = -1;
            return dataResult;
        }
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::enableConfigMode()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xFF, 0x00, 0x01, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);
    dataResult = sendCommand(sendData, sendDataLength);
    if (dataResult.resultBuffer[8] == 0)
    {
        return Seeed_HSP24::AskStatus::Success;
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::disableConfigMode()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xFE, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);
    dataResult = sendCommand(sendData, sendDataLength);
    if (dataResult.resultBuffer[8] == 0)
    {
        return Seeed_HSP24::AskStatus::Success;
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

String Seeed_HSP24::getVersion()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xA0, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
        {
            char versionBuffer[32];
            sprintf(versionBuffer, "V%d.%02d.%06d",
                    dataResult.resultBuffer[11],
                    dataResult.resultBuffer[12],
                    (dataResult.resultBuffer[13] << 24) | (dataResult.resultBuffer[14] << 16) |
                        (dataResult.resultBuffer[15] << 8) | dataResult.resultBuffer[16]);

            disableConfigMode();

            return versionBuffer;
        }
        else
        {
            disableConfigMode();
            return "Get Version Error!";
        }
    }
    else
    {
        return "Into AT Mode Error!";
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::setDetectionDistance(int distance, int times)
{
    if (distance >= 1 && distance < 9 && times > 0)
    {
        Seeed_HSP24::DataResult dataResult;
        byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x14, 0x00, 0x60, 0x00,
                           0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
                           0x01, 0x00, 0x08, 0x00, 0x00, 0x00,
                           0x02, 0x00, 0x05, 0x00, 0x00, 0x00,
                           0x04, 0x03, 0x02, 0x01};
        sendData[10] = distance;
        sendData[16] = distance;

        // times zuweisen, Little-Endian (niedrigstes Byte zuerst)
        sendData[22] = times & 0xFF;         // Niedrigstes Byte
        sendData[23] = (times >> 8) & 0xFF;  // Zweites Byte
        sendData[24] = (times >> 16) & 0xFF; // Drittes Byte
        sendData[25] = (times >> 24) & 0xFF; // Höchstes Byte

        int sendDataLength = sizeof(sendData);

        if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
        {
            dataResult = sendCommand(sendData, sendDataLength);

            if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
            {
                disableConfigMode();
                return Seeed_HSP24::AskStatus::Success;
            }
            else
            {
                disableConfigMode();
                return Seeed_HSP24::AskStatus::Error;
            }
        }
        else
        {
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::setGatePower(int gate, int movePower, int staticPower)
{
    if (gate >= 1 && gate < 9 && movePower > 0 && staticPower > 0 && movePower <= 100 && staticPower <= 100)
    {
        Seeed_HSP24::DataResult dataResult;
        byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x14, 0x00, 0x64, 0x00,
                           0x00, 0x00,
                           0x08, 0x00, 0x00, 0x00,
                           0x01, 0x00,
                           0x08, 0x00, 0x00, 0x00,
                           0x02, 0x00,
                           0x05, 0x00, 0x00, 0x00,
                           0x04, 0x03, 0x02, 0x01};
        sendData[10] = gate;

        // movePower zuweisen, Little-Endian
        sendData[16] = movePower & 0xFF;
        sendData[17] = (movePower >> 8) & 0xFF;
        sendData[18] = (movePower >> 16) & 0xFF;
        sendData[19] = (movePower >> 24) & 0xFF;

        // staticPower zuweisen, Little-Endian
        sendData[22] = staticPower & 0xFF;
        sendData[23] = (staticPower >> 8) & 0xFF;
        sendData[24] = (staticPower >> 16) & 0xFF;
        sendData[25] = (staticPower >> 24) & 0xFF;

        int sendDataLength = sizeof(sendData);

        if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
        {
            dataResult = sendCommand(sendData, sendDataLength);

            if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
            {
                disableConfigMode();
                return Seeed_HSP24::AskStatus::Success;
            }
            else
            {
                disableConfigMode();
                return Seeed_HSP24::AskStatus::Error;
            }
        }
        else
        {
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::RadarStatus Seeed_HSP24::getConfig()
{
    Seeed_HSP24::DataResult dataResult;
    Seeed_HSP24::RadarStatus radarStatus;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x61, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[7] == 0x01 && dataResult.resultBuffer[8] == 0x00)
        {
            radarStatus.detectionDistance = dataResult.resultBuffer[11];
            radarStatus.moveSetDistance = dataResult.resultBuffer[12];
            radarStatus.staticSetDistance = dataResult.resultBuffer[13];
            int j = 0;
            for (int i = 14; i < 23; i++)
            {
                radarStatus.radarMovePower.moveGate[j++] = dataResult.resultBuffer[i];
            }
            j = 0;
            for (int i = 23; i < 32; i++)
            {
                radarStatus.radarStaticPower.staticGate[j++] = dataResult.resultBuffer[i];
            }

            int noTargrtduration = dataResult.resultBuffer[32] | (dataResult.resultBuffer[33] << 8);
            radarStatus.noTargrtduration = noTargrtduration;

            disableConfigMode();

            return radarStatus;
        }
        else
        {
            disableConfigMode();
            return radarStatus;
        }
    }
    else
    {
        return radarStatus;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::setResolution(int resolution)
{
    if (resolution == 0 || resolution == 1)
    {
        Seeed_HSP24::DataResult dataResult;
        byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x04, 0x03, 0x02, 0x01};
        if (resolution == 1)
        {
            sendData[8] = 1;
        }
        int sendDataLength = sizeof(sendData);

        if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
        {
            dataResult = sendCommand(sendData, sendDataLength);

            if (dataResult.resultBuffer[8] == 0x00)
            {
                rebootRadar();
                return Seeed_HSP24::AskStatus::Success;
            }
            else
            {
                rebootRadar();
                return Seeed_HSP24::AskStatus::Error;
            }
        }
        else
        {
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::RadarStatus Seeed_HSP24::getResolution()
{
    Seeed_HSP24::DataResult dataResult;
    Seeed_HSP24::RadarStatus radarStatus;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xAB, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            radarStatus.resolution = dataResult.resultBuffer[10];
            disableConfigMode();
            return radarStatus;
        }
        else
        {
            disableConfigMode();
            return radarStatus;
        }
    }
    else
    {
        return radarStatus;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::rebootRadar()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xA3, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            return Seeed_HSP24::AskStatus::Success;
        }
        else
        {
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::refactoryRadar()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xA2, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            rebootRadar();
            return Seeed_HSP24::AskStatus::Success;
        }
        else
        {
            rebootRadar();
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::enableEngineeringModel()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x62, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            disableConfigMode();
            return Seeed_HSP24::AskStatus::Success;
        }
        else
        {
            disableConfigMode();
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

Seeed_HSP24::AskStatus Seeed_HSP24::disableEngineeringModel()
{
    Seeed_HSP24::DataResult dataResult;
    byte sendData[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0x63, 0x00, 0x04, 0x03, 0x02, 0x01};
    int sendDataLength = sizeof(sendData);

    if (enableConfigMode() == Seeed_HSP24::AskStatus::Success)
    {
        dataResult = sendCommand(sendData, sendDataLength);

        if (dataResult.resultBuffer[8] == 0x00)
        {
            disableConfigMode();
            return Seeed_HSP24::AskStatus::Success;
        }
        else
        {
            disableConfigMode();
            return Seeed_HSP24::AskStatus::Error;
        }
    }
    else
    {
        return Seeed_HSP24::AskStatus::Error;
    }
}

int Seeed_HSP24::findSequence(byte *arr, int arrLen, const byte *seq, int seqLen)
{
    for (int i = 0; i < arrLen - seqLen + 1; i++)
    {
        bool found = true;
        for (int j = 0; j < seqLen; j++)
        {
            if (arr[i + j] != seq[j])
            {
                found = false;
                break;
            }
        }
        if (found)
            return i;
    }
    return -1;
}