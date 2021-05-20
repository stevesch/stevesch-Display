#ifndef STEVESCH_DISPLAY_EXAMPLES_MINIMAL_BOARDSETTINGS_H_
#define STEVESCH_DISPLAY_EXAMPLES_MINIMAL_BOARDSETTINGS_H_
/**
 * @file boardSettings.h
 * @author Stephen Schlueter (https://github.com/stevesch)
 * @brief 
 * @version 0.1
 * @date 2021-05-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>

#ifndef BOARD_NAME
#pragma message "Board not specified-- must be specified in platformio.ini.  Use"
#pragma message "  build_flags = "
#pragma message "    -include ${src_dir}/boardSetups/<your setup file>.h"
#define BOARD_NAME "Unspecified-board"
#endif

#endif
