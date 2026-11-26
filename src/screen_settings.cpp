
#include "platform.h"
#include "keys.h"

#include "screen_settings.h"
#include "ptune2_simple/Ptune2_direct.h"

struct option_type {
	const char* name;
	int type;		// 0 = toggle, 1 = keys, 2 = frameskip, 3 = color, 4 = scale mode
	void* addr;
	bool disabled;
	bool noCGB;
};

static option_type options[] = {
	{ "Overclock", 0, &emulator.settings.overclock, false, false },
	{ "Fit To Screen", 4, &emulator.settings.scaleMode, false, false },
	{ "CGB Colors", 0, &emulator.settings.useCGBColors, false, false },
	{ "Clamp Speed", 0, &emulator.settings.clampSpeed, false, false },
	{ "Frameskip", 2, &emulator.settings.frameSkip, false, false },
	{ "BG Colors", 3, &emulator.settings.bgColorPalette, false, false },
	{ "Sprite 1 Colors", 3, &emulator.settings.obj1ColorPalette, false, false },
	{ "Sprite 2 Colors", 3, &emulator.settings.obj2ColorPalette, false, false },
	{ "Map Keys", 1, NULL, false, false },
	{ "Sound", 0, &emulator.settings.sound, false, false },
};

static inline int NumOptions() {
	return sizeof(options) / sizeof(option_type);
}

void screen_settings::setup() {
	curOption = 0;
}

void screen_settings::select() {
	ResolveBG(bg_menu);
	DrawPausePreview();

	for (int i = 0; i < NumOptions(); i++) {
		options[i].disabled = false;
		options[i].noCGB = false;
	}

	// disable overclock on non CG-20 devices and when already overclocked (such as when using Ptune2)
	int currentPtuneSetting = Ptune2_GetSetting();
	options[0].disabled = getDeviceType() != DT_CG20 || currentPtuneSetting != PT2_DEFAULT;

	if (emulator.isCGBRomLoaded()) {
		options[2].noCGB = true;
		options[5].noCGB = true;
		options[6].noCGB = true;
		options[7].noCGB = true;
	}

	for (int i = 0; i < NumOptions(); i++) {
		if (options[i].noCGB) options[i].disabled = true;
	}

	// can't select disabled option
	while (options[curOption].disabled) curOption = (curOption + 1) % NumOptions();

	drawOptions();
}

void screen_settings::deselect() {
	emulator.saveSettings();
}

void screen_settings::drawOptions() {
	ResolveBG(bg_menu);
	EnableStatusArea(3);

	display_fill top;
	top.x1 = 0; top.x2 = 383; top.y1 = 0; top.y2 = 43; top.mode = 1;
	Bdisp_AreaClr(&top, 1, COLOR_BLACK);
	display_fill accent;
	accent.x1 = 0; accent.x2 = 383; accent.y1 = 42; accent.y2 = 45; accent.mode = 1;
	Bdisp_AreaClr(&accent, 1, COLOR_CYAN);
	Print(12, 5, "PRIZMGBC", false, COLOR_WHITE);
	Print(12, 23, "SYSTEM SETTINGS", false, COLOR_CYAN);

	int row = 0;
	for (int i = 0; i < NumOptions(); i++) {
		bool selected = curOption == i;
		int y = row * 14 + 50;
		if (options[i].noCGB) continue;
		row++;

		if (selected) {
			display_fill highlight;
			highlight.x1 = 8; highlight.x2 = 375;
			highlight.y1 = y - 2; highlight.y2 = y + 12;
			highlight.mode = 1;
			Bdisp_AreaClr(&highlight, 1, COLOR_MEDIUMBLUE);
		}
		Print(15, y, options[i].name, selected,
			options[i].disabled ? COLOR_DARKGRAY : COLOR_WHITE);

		if (options[i].type == 0) {
			unsigned char isOn = *((unsigned char*)options[i].addr);
			Print(230, y, isOn ? "[ ON ]" : "[ OFF ]", selected,
				options[i].disabled ? COLOR_DARKGRAY : (isOn ? COLOR_CYAN : COLOR_SILVER));
		} else if (options[i].type == 1) {
			Print(230, y, "CONFIGURE", selected, COLOR_CYAN);
		} else if (options[i].type == 2) {
			char skip = *((char*)options[i].addr);
			if (skip == -1) Print(230, y, "AUTO", selected, COLOR_CYAN);
			else {
				char buffer[8];
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "%d", skip);
				Print(230, y, buffer, selected, COLOR_CYAN);
			}
		} else if (options[i].type == 3) {
			colorpalette_type pal;
			emulator.getPalette(*((unsigned char*)options[i].addr), pal);
			for (int j = 0; j < 4; j++) {
				display_fill area;
				area.x1 = 230 + 18 * j; area.x2 = area.x1 + 15;
				area.y1 = y - 1; area.y2 = y + 11; area.mode = 1;
				Bdisp_AreaClr(&area, 1, pal.colors[j]);
			}
		} else if (options[i].type == 4) {
			unsigned char opt = *((unsigned char*)options[i].addr);
			const char* modes[] = { "1X", "FIT 1.5X", "2X CROP" };
			Print(230, y, modes[opt], selected,
				!options[i].disabled ? COLOR_CYAN : COLOR_DARKGRAY);
		}
	}

	display_fill footer;
	footer.x1 = 0; footer.x2 = 383; footer.y1 = 192; footer.y2 = 215; footer.mode = 1;
	Bdisp_AreaClr(&footer, 1, COLOR_BLACK);
	Print(10, 198, "UP/DOWN SELECT OPTION", false, COLOR_SILVER);
	Print(280, 198, "F1 LIBRARY", false, COLOR_CYAN);
	DrawFrame(0);
}

static const char* keyName[] = {
	"A", "B", "Select", "Start", "Right", "Left", "Up", "Down", "Save State", "Load State"
};

unsigned char getCurrentKey(bool wait = true) {
	unsigned char keyPressed = 0;
	do {
		for (unsigned int c = 25; c <= 79; c++) {
			// skip invalid keys
			if (c % 10 == 0 || c == 34)
				continue;

			if (keyDown_fast(c)) {
				keyPressed = c;
				// wait for key too lift
				while (keyDown_fast(keyPressed)) {}
				break;
			}
		}
	} while (keyPressed == 0 && wait);

	// wait a moment
	OS_InnerWait_ms(10);

	return keyPressed;
}

void screen_settings::selectKeys() {
	unsigned char newMap[emu_button::MAX];

	// wait for key release
	getCurrentKey(false);

	for (int i = 0; i < emu_button::MAX; i++) {
		display_fill area;
		area.x1 = 65;
		area.x2 = 318;
		area.y1 = 86;
		area.y2 = 132;
		area.mode = 1;
		display_fill temp = area;
		Bdisp_AreaClr(&temp, 1, COLOR_BLACK);

		area.x1 += 2;
		area.x2 -= 2;
		area.y1 += 2;
		area.y2 -= 2;
		Bdisp_AreaClr(&area, 1, COLOR_WHITE);

		const char* pressKey = "Press key for ";
		const char* exitPrompt = "Press MENU to cancel";

		int line1WidthA = PrintWidth(pressKey);
		int line1WidthB = PrintWidth(keyName[i]);
		int x = 192 - (line1WidthA + line1WidthB) / 2;

		Print(x, 90, pressKey, false, COLOR_BLACK);
		Print(x + line1WidthA, 90, keyName[i], true, COLOR_YELLOW);

		int line2Width = PrintWidth(exitPrompt);
		Print(192 - line2Width / 2, 110, exitPrompt, false, COLOR_DARKRED);

		Bdisp_PutDisp_DD();

		newMap[i] = getCurrentKey();

		// MENU = cancel
		if (newMap[i] == 48)
			return;
	}

	// copy to settings
	memcpy(emulator.settings.keyMap, newMap, sizeof(newMap));
}

void screen_settings::handleUp() {
	do {
		curOption = (curOption + NumOptions() - 1) % NumOptions();
	} while (options[curOption].disabled);

	ResolveBG(bg_menu);
	DrawPausePreview();
	drawOptions();
}

void screen_settings::handleDown() {
	do {
		curOption = (curOption + 1) % NumOptions();
	} while (options[curOption].disabled);

	ResolveBG(bg_menu);
	DrawPausePreview();
	drawOptions();
}

void screen_settings::handleSelect() {
	switch (options[curOption].type) {
		case 0:
		{
			unsigned char* isOn = ((unsigned char*)options[curOption].addr);
			*isOn = 1 - *isOn;
			break;
		}
		case 1:
		{
			selectKeys();
			break;
		}
		case 2:
		{
			char* skip = ((char*)options[curOption].addr);
			(*skip)++;
			if (*skip == 5) {
				*skip = -1;
			}
			break;
		}
		case 3:
		{
			unsigned char* palette = ((unsigned char*)options[curOption].addr);
			*palette = ((*palette) + 1) % emulator.numPalettes();

			// disable cgb colors
			emulator.settings.useCGBColors = false;

			break;
		}
		case 4:
		{
			unsigned char* opt = ((unsigned char*)options[curOption].addr);
			*opt = ((*opt) + 1) % emu_scale::MAX;
			break;
		}
	}

	ResolveBG(bg_menu);
	DrawPausePreview();
	drawOptions();
}

