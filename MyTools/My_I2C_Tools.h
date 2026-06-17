// Collection of different BUS tools
///////////////////////////// I2C ////////////////////////

// Kleine Hilfsfunktion (nur innerhalb dieser .h), um Register einzeln anzufragen
uint8_t readSingleKlonRegister(uint8_t reg) {
  Wire.beginTransmission(0x19);
  Wire.write(reg);
    // WICHTIG: 'true' sendet ein echtes STOP-Signal. 
  // Das verhindert den SDA-Freeze und den i2cWriteReadNonStop-Fehler!
  if (Wire.endTransmission(true) != 0) {
    return 0; // Bus-Fehler abfangen
  }
    // Kurze Pause, damit der Klon-Chip die Registerdaten intern bereitstellen kann
  delayMicroseconds(50); 
  Wire.requestFrom((uint8_t)0x19, (uint8_t)1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

// Hilfsfunktion: Schreibt ein einzelnes Register direkt in die Hardware
void writeSingleKlonRegister(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(0x19);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

// Helper function to identify I2C devices
void scanI2Cbus(){
  Serial.print("\nI2C devices: \t");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0)
      Serial.printf(" 0x%02X\t", addr);
  }
  Serial.print("\nNo more I2C devices:\n");
}