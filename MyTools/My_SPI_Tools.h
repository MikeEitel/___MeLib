// Collection of different BUS tools
///////////////////////////// SPI ////////////////////////

// Signal-IDs für den freien SPI3 (HSPI) Bus des ESP32-S3
#define ESP32_S3_HSPID_OUT_IDX   70  // MOSI Ausgang Signal
#define ESP32_S3_HSPIQ_IN_IDX    71  // MISO Eingang Signal
#define ESP32_S3_HSPICLK_OUT_IDX 69  // SCK Ausgang Signal


void printActiveFS2RegisterPins() {
  Serial.println("\n--- Genuinely Active ESP32-S3 FSPI (SPI2) Hardware Pins ---");
  int hardwareMosi = -1; 
  int hardwareMiso = -1; 
  int hardwareSck  = -1;

  // 1. IOMUX-Direktpfad abfragen (GPIO 7, 8, 9 auf dem XIAO-S3)
  // Wenn der SPI2-Controller aktiv ist und den IOMUX nutzt, sind diese Pins fest verdrahtet.
  // Wir prüfen zusätzlich, ob die GPIO-Matrix für diese Pins auf "Bypass" (Wert 128 / 0x80) steht.
  if ((REG_READ(IO_MUX_GPIO7_REG) & MCU_SEL_M) == 0) hardwareSck  = 7;
  if ((REG_READ(IO_MUX_GPIO8_REG) & MCU_SEL_M) == 0) hardwareMiso = 8;
  if ((REG_READ(IO_MUX_GPIO9_REG) & MCU_SEL_M) == 0) hardwareMosi = 9;

  // 2. Fallback: GPIO-Matrix scannen (Falls TFT_eSPI die Matrix erzwingt; SPI2-Signale: MOSI=64, SCK=63)
  for (int gpio = 0; gpio < 48; gpio++) {
    uint32_t outSig = GPIO.func_out_sel_cfg[gpio].val & 0x1FF;
    if (outSig == 64) hardwareMosi = gpio;
    if (outSig == 63) hardwareSck  = gpio;
  }

  // 3. Matrix-Eingang für SPI2 MISO scannen (Signal ID 65)
  if (GPIO.func_in_sel_cfg[65].sig_in_sel) {
    hardwareMiso = GPIO.func_in_sel_cfg[65].func_sel;
  }

  Serial.print("True Silicon FSPI MOSI Pin: "); Serial.println(hardwareMosi);
  Serial.print("True Silicon FSPI MISO Pin: "); Serial.println(hardwareMiso);
  Serial.print("True Silicon FSPI SCK Pin:  "); Serial.println(hardwareSck);
  Serial.println("-----------------------------------------------------------\n");
}


void forceHardwareSPI3Pins(uint8_t sck, uint8_t miso, uint8_t mosi) {
  Serial.println("-> Erwinke manuelle GPIO-Matrix Register-Einträge...");

  // 1. Pins als Ausgänge/Eingänge im GPIO-Controller vorbereiten
  pinMode(sck, OUTPUT);
  pinMode(mosi, OUTPUT);
  pinMode(miso, INPUT);

  // 2. Ausgänge patchen (Verbinde physikalischen Pin mit dem SPI3-Signal)
  GPIO.func_out_sel_cfg[mosi].val = ESP32_S3_HSPID_OUT_IDX;
  GPIO.func_out_sel_cfg[sck].val  = ESP32_S3_HSPICLK_OUT_IDX;

  // 3. Eingang patchen (Leite das Signal vom physikalischen Pin in die SPI3-MISO-Engine)
  GPIO.func_in_sel_cfg[ESP32_S3_HSPIQ_IN_IDX].val = 0; // Reset
  GPIO.func_in_sel_cfg[ESP32_S3_HSPIQ_IN_IDX].sig_in_sel = 1; // Aktiviere Matrix-Routing
  GPIO.func_in_sel_cfg[ESP32_S3_HSPIQ_IN_IDX].func_sel = miso; // Setze Pin
}

void printActiveHS3RegisterPins() {
  Serial.println("\n--- Genuinely Active ESP32-S3 HSPI (SPI3) Hardware Pins ---");

  int hardwareMosi = -1;
  int hardwareMiso = -1;
  int hardwareSck  = -1;

  for (int gpio = 0; gpio < 48; gpio++) {
    uint32_t registerValue = GPIO.func_out_sel_cfg[gpio].val;
    uint32_t outputSignal = registerValue & 0x1FF; 
    
    if (outputSignal == ESP32_S3_HSPID_OUT_IDX)   hardwareMosi = gpio;
    if (outputSignal == ESP32_S3_HSPICLK_OUT_IDX) hardwareSck  = gpio;

    if (GPIO.func_in_sel_cfg[ESP32_S3_HSPIQ_IN_IDX].sig_in_sel && 
        (GPIO.func_in_sel_cfg[ESP32_S3_HSPIQ_IN_IDX].func_sel == gpio)) {
      hardwareMiso = gpio;
    }
  }

  Serial.print("True Silicon HSPI MOSI Pin: "); Serial.println(hardwareMosi);
  Serial.print("True Silicon HSPI MISO Pin: "); Serial.println(hardwareMiso);
  Serial.print("True Silicon HSPI SCK Pin:  "); Serial.println(hardwareSck);
  Serial.println("-----------------------------------------------------------\n");
}
