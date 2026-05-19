// For XIAO_ESP32-C3 andwaveshare Dual Eye
#define USER_SETUP_ID 500

#define XIAO_GC9D01_DRIVER

/*
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_SCLK 10
#define TFT_CS    9  // Chip select control pin
#define TFT_DC    8  // Data Command control pin
#define TFT_RST   14  // Reset pin (could connect to RST pin)
#define TFT_BL 2
*/

// Software reset & init
#define TFT_SWRST      0x01
#define TFT_INIT_DELAY 0x80   // High bit flag used in command lists

// Column/Row address set (standard MIPI commands)
#define TFT_CASET      0x2A   // Column Address Set
#define TFT_PASET      0x2B   // Page (Row) Address Set
#define TFT_RAMWR      0x2C   // Memory Write
#define TFT_RAMRD      0x2E   // Memory Read

// Common display commands
#define TFT_MADCTL     0x36   // Memory Data Access Control
#define TFT_IDXRD      0xDD   // ILI9341 only, but needed as placeholder





#define TFT_BACKLIGHT_ON HIGH

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts
#define SMOOTH_FONT

#define TFT_WIDTH 160
#define TFT_HEIGHT 160

#define USE_HSPI_PORT          // Force HSPI for esp32 >= v3.0

#define SPI_FREQUENCY  40000000

#define SPI_READ_FREQUENCY  20000000

#define SPI_TOUCH_FREQUENCY  2500000

// #define SUPPORT_TRANSACTIONS