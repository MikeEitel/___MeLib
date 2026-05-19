#ifndef Seeed_HSP24_h
#define Seeed_HSP24_h

#include <SoftwareSerial.h> // Softwareserielle Bibliothek einbinden
#include <Arduino.h>

#define BUFFER_SIZE 256     // Größe des seriellen Puffers

class Seeed_HSP24
{
    public:
         // Seeed_HSP24(SoftwareSerial &serial, HardwareSerial &debugSerial); //
        Seeed_HSP24(Stream &serial);                                        // Konstruktor für direkte Kommunikation mit dem Modul
        explicit Seeed_HSP24(Stream &serial, Stream &debugSerial);          // Konstruktor mit zusätzlichem Debug-Port

        int enterATMode();                                                   // Modul in AT-Modus versetzen; Erfolg: 1, Fehler: 0
        int exitATMode();                                                    // Modul aus AT-Modus beenden; Erfolg: 1, Fehler: 0
        int getVer();                                                        // Versionsnummer auslesen
        int setNetwork(String ssid, String password);                        // Modul mit WLAN verbinden (STA-Modus); Erfolg: 1, Fehler: 0

        static const int FRAME_START_SIZE = 4;
        static const int FRAME_END_SIZE = 4;

        static const byte frameStart[FRAME_START_SIZE];
        static const byte frameEnd[FRAME_END_SIZE];
        static const byte frameAskStart[FRAME_START_SIZE];
        static const byte frameAskEnd[FRAME_END_SIZE];

        struct RadarMovePower // Energiewerte je Bewegungs-Entfernungstor
        {
            int moveGate[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
        };

        struct RadarStaticPower // Energiewerte je Stillstands-Entfernungstor
        {
            int staticGate[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
        };

        // Enum-Klasse AskStatus
        enum class AskStatus : byte
        {
            Success = 0x00, // Erfolg
            Error = 0x01,   // Fehler
        };

        // Enum-Klasse TargetStatus
        enum class TargetStatus : byte
        {
            NoTarget = 0x00,     // Kein Ziel erkannt
            MovingTarget = 0x01, // Bewegtes Ziel erkannt
            StaticTarget = 0x02, // Statisches Ziel erkannt
            BothTargets = 0x03,  // Bewegtes und statisches Ziel erkannt
            ErrorFrame = 0x04    // Statusabfrage fehlgeschlagen
        };

        // Struktur RadarStatus
        struct RadarStatus
        {
            TargetStatus targetStatus = TargetStatus::ErrorFrame; // Zielstatus des Radars
            int distance = -1;           // Zielentfernung in mm
            int moveSetDistance = -1;    // Anzahl der Bewegungserkennungs-Entfernungstore (normalerweise nicht konfigurieren)
            int staticSetDistance = -1;  // Anzahl der Stillstandserkennungs-Entfernungstore (normalerweise nicht konfigurieren)
            int detectionDistance = -1;  // Maximales Erkennungs-Entfernungstor
            int resolution = -1;         // Auflösung der Entfernungstore
            int noTargrtduration = -1;   // Dauer ohne Zielerkennung
            int radarMode = -1;          // Unterscheidet Basismodus (2) von Ingenieurbetrieb (1)
            RadarMovePower radarMovePower;    // Energiewerte für Bewegungserkennung
            RadarStaticPower radarStaticPower; // Energiewerte für Stillstandserkennung
            int photosensitive = -1;     // Lichtsensor-Wert (0–255)
        };

        // Rückgabestruktur für gesendete Befehle
        struct DataResult
        {
            byte *resultBuffer; // Zeiger auf dynamisch allokiertes Array
            int length;         // Länge des Arrays
        };

        // Radar-Erkennungsstatus abrufen
        RadarStatus getStatus();
        // Befehl senden
        DataResult sendCommand(const byte* sendData, int sendDataLength);
        // Konfigurationsmodus aktivieren
        AskStatus enableConfigMode();
        // Konfigurationsmodus beenden
        AskStatus disableConfigMode();
        // Firmware-Version des Radars auslesen
        String getVersion();
        // Maximales Entfernungstor und Verzögerung bei Abwesenheit konfigurieren
        AskStatus setDetectionDistance(int distance, int times);
        // Empfindlichkeit der Entfernungstore konfigurieren
        AskStatus setGatePower(int gate, int movePower, int staticPower);
        // Parameter auslesen
        RadarStatus getConfig();
        // Auflösung der Entfernungstore setzen: 1 = 0,25 m, 0 = 0,75 m (Standard: 0,75 m)
        AskStatus setResolution(int resolution);
        // Auflösung der Entfernungstore auslesen: 1 = 0,25 m, 0 = 0,75 m (Standard: 0,75 m)
        RadarStatus getResolution();
        // Radar neu starten
        AskStatus rebootRadar();
        // Radar auf Werkseinstellungen zurücksetzen
        AskStatus refactoryRadar();
        // Ingenieurbetrieb aktivieren
        AskStatus enableEngineeringModel();
        // Ingenieurbetrieb beenden
        AskStatus disableEngineeringModel();

    private:
        // HardwareSerial &_serial;
        Stream* _serial;
        Stream* _debugSerial;
        char buffer[BUFFER_SIZE];              // Puffer als Member-Variable
        unsigned long receiveStartTime;
        const unsigned long RECEIVE_TIMEOUT = 100; // Empfangs-Timeout in Millisekunden
        int bufferIndex;
        int isInATMode;    // Gibt an, ob sich das Modul im AT-Modus befindet
        int checkBuffer();
        int sendATCommand(String command);
        int sendATCommandWithExit(String command);

        byte buffer_hsp24[BUFFER_SIZE];
        int bufferIndex_hsp24;
        int findSequence(byte *arr, int arrLen, const byte *seq, int seqLen);

    protected:
        
};
#endif