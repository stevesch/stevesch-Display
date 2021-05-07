#ifndef STEVESCH_DISPLAY_EXAMPLES_MINIMAL_BOARDSETTINGS_H_
#define STEVESCH_DISPLAY_EXAMPLES_MINIMAL_BOARDSETTINGS_H_

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
