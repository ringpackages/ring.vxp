#pragma once

#include "vmchset.h"
#include "vm4res.h"
#include "vmgraph.h"
#include "vmio.h"
#include "vmmm.h"
#include "vmstdlib.h"
#include "vmsys.h"
#include "vmtimer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "ring.h"
#include "darr.h"
#include "util.h"
#include "menu.h"

typedef enum State {
	ST_MENU,
	ST_RUNNING,
	ST_EDITOR,
} State;

#define RING_VXP_VERSION "1.3"

#define RING_DIR "e:\\ring\\"

#define HINT_BAR_HEIGHT 18
#define TITLE_HEIGHT 18
#define SOFTKEY_HEIGHT 18

#define BG_COLOR MRE_RGB(18, 24, 31)
#define ACCENT_COLOR MRE_RGB(59, 130, 246)
#define MAIN_COLOR ACCENT_COLOR
#define TEXT_COLOR MRE_RGB(236, 242, 248)
#define TEXT_DIM_COLOR MRE_RGB(166, 178, 190)
#define ERROR_COLOR MRE_RGB(225, 95, 95)
#define SEPARATOR_COLOR MRE_RGB(58, 70, 82)

#define MRE_RGB(r, g, b)                                                       \
	(VMUINT16)((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))

#define SCREEN_WIDTH vm_graphic_get_screen_width()
#define SCREEN_HEIGHT vm_graphic_get_screen_height()

extern State current_state;
extern char currentScript[FILENAME_MAX];
extern char currentDir[FILENAME_MAX];
extern VMWCHAR ucs2_str[128];
extern vm_graphic_color color;
extern VMINT layer_hdl[1];
extern char **menu_list;
extern Menu menu;

void set_state(State new_state);
void handle_sysevt(VMINT message, VMINT param);
void handle_keyevt(VMINT event, VMINT keycode);

static void draw_running_state(void);
static void cleanup_ring_state(void);
static VMBOOL execute_script(const char *script_path);