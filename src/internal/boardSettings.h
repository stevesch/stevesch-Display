#ifndef _BOARDSETTINGS_H_
#define _BOARDSETTINGS_H_

#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>

#define stringOf(x) #x
#define XSTR(x) stringOf(x)
#define MSG(x) _Pragma (stringOf(message (x)))

#ifndef BOARD_DEFINED
#pragma message "Board not specified-- must be specified in platformio.ini.  Use"
#pragma message "-DBOARD_DEFINED=1"
#pragma message "-DBOARD_<board-specific-name>"
#pragma message "Board not specified-- using value from header"
#define BOARD_T_DISPLAY			1
#endif

#ifdef BOARD_T_DISPLAY
	#pragma message "Board specified: T-Display"
  #define BOARD_NAME stringOf(BOARD_T_DISPLAY)
	#define WAKE_GPIO				GPIO_SEL_35	// wake gpio (should correspond to sleep button)
	// TFT_WIDTH x TFT_HEIGHT expected to be 135x240
#elif defined(BOARD_TS)
	#pragma message "Board specified: TS"
  // TFT_WIDTH x TFT_HEIGHT expected to be 128x160

	#define USE_IMU		1
	// #define I2C_SDA     21
	// #define I2C_SCL     22
	#define IMU_ADDRESS 0x68
	#define IMU_ORIENTATION_E_N_D_GYRO_REVERSED
	#define I2C_SDA     19
	#define I2C_SCL     18
#elif defined(BOARD_T7)
	#pragma message "Board specified: T7"
	#define BOARD_NAME stringOf(BOARD_T7)
	// TTGO T7:
	// 35=vbat monitor
	// 34=low?
	// 36=low?
	// 39=low?
	// 33: OK
	// 27: OK
	// 32: ?
	// 0: 
	// TFT_WIDTH x TFT_HEIGHT expected to be 240x320

#elif defined(BOARD_M5STICK_C) || defined(BOARD_M5STICK_C_PLUS)
	#pragma message "Board specified: M5StickC [plus]"
  #if defined(BOARD_M5STICK_C)
    #define BOARD_NAME stringOf(BOARD_M5STICK_C)
  #elif defined(BOARD_M5STICK_C_PLUS)
    #define BOARD_NAME stringOf(BOARD_M5STICK_C_PLUS)
  #endif
	MSG("TFT Width: " XSTR(TFT_WIDTH))
	MSG("TFT Height: " XSTR(TFT_HEIGHT))
	#define USE_IMU		1
	#define IMU_ORIENTATION_E_N_U
	#define IMU_NO_MAG								// no compass on MPU6886
	#define IMU_ADDRESS 0x68
	#define AXP192_ADDRESS 0x34				// for backlight/power control
	#define I2C_SDA     21
	#define I2C_SCL     22
#elif defined(BOARD_ATOM_MATRIX) || defined(BOARD_ATOM_LITE)
	#pragma message "Board specified: Atom"
	#if defined(BOARD_ATOM_MATRIX)
		#define BOARD_NAME stringOf(BOARD_ATOM_MATRIX)
	#elif defined(BOARD_ATOM_LITE)
		#define BOARD_NAME stringOf(BOARD_ATOM_LITE)
	#endif
	// no TFT display
#elif defined(BOARD_ESP32_SJS)
	#pragma message "Board specified: ESP32_SJS"
  #define BOARD_NAME stringOf(BOARD_ESP32_SJS)
#else
	#error Please specify BOARD
#endif

// #ifndef ICACHE_FLASH_ATTR
// #define ICACHE_FLASH_ATTR 
// #endif

#endif // _BOARDSETTINGS_H_
