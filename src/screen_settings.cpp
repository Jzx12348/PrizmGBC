
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
	{ "FPS Counter", 0, &emulator.settings.fpsCounter, false, false },
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

	// Shared PrizmGBC visual language: dark header, cyan accent, clear panels.
	display_fill header;
	header.x1 = 0; header.x2 = 383; header.y1 = 0; header.y2 = 43; header.mode = 1;
	Bdisp_AreaClr(&header, 1, COLOR_BLACK);
	display_fill accent;
	accent.x1 = 0; accent.x2 = 383; accent.y1 = 42; accent.y2 = 45; accent.mode = 1;
	Bdisp_AreaClr(&accent, 1, COLOR_CYAN);
	Print(12, 5, "PRIZMGBC", true, COLOR_WHITE);
	Print(12, 24, "SYSTEM SETTINGS", false, COLOR_CYAN);
	Print(300, 15, "v1.0.0", false, COLOR_CYAN);

	display_fill panel;
	panel.x1 = 7; panel.x2 = 376; panel.y1 = 51; panel.y2 = 187; panel.mode = 1;
	Bdisp_AreaClr(&panel, 1, COLOR_DARKGRAY);

	int row = 0;
	for (int i = 0; i < NumOptions(); i++) {
		if (options[i].noCGB) continue;
		bool selected = curOption == i;
		int y = row * 12 + 55;
		row++;

		if (selected) {
			display_fill highlight;
			highlight.x1 = 10; highlight.x2 = 373;
			highlight.y1 = y - 2; highlight.y2 = y + 11;
			highlight.mode = 1;
			Bdisp_AreaClr(&highlight, 1, COLOR_MEDIUMBLUE);
			display_fill marker;
			marker.x1 = 10; marker.x2 = 13;
			marker.y1 = y - 2; marker.y2 = y + 11;
			marker.mode = 1;
			Bdisp_AreaClr(&marker, 1, COLOR_CYAN);
		}

		Print(19, y, options[i].name, selected,
			options[i].disabled ? COLOR_SILVER : COLOR_WHITE);

		if (options[i].type == 0) {
			unsigned char isOn = *((unsigned char*)options[i].addr);
			Print(302, y, isOn ? "ON" : "OFF", selected,
				options[i].disabled ? COLOR_SILVER : (isOn ? COLOR_CYAN : COLOR_SILVER));
		} else if (options[i].type == 1) {
			char bindText[16];
			sprintf(bindText, "%d BOUND", emu_button::MAX);
			Print(286, y, bindText, selected, COLOR_CYAN);
		} else if (options[i].type == 2) {
			char skip = *((char*)options[i].addr);
			if (skip == -1) Print(318, y, "AUTO", selected, COLOR_CYAN);
			else {
				char buffer[8];
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "%d", skip);
				Print(318, y, buffer, selected, COLOR_CYAN);
			}
		} else if (options[i].type == 3) {
			colorpalette_type pal;
			emulator.getPalette(*((unsigned char*)options[i].addr), pal);
			for (int j = 0; j < 4; j++) {
				display_fill swatch;
				swatch.x1 = 300 + 17 * j; swatch.x2 = swatch.x1 + 13;
				swatch.y1 = y - 1; swatch.y2 = y + 10; swatch.mode = 1;
				Bdisp_AreaClr(&swatch, 1, pal.colors[j]);
			}
		} else if (options[i].type == 4) {
			unsigned char opt = *((unsigned char*)options[i].addr);
			const char* modes[] = { "1X", "FIT 1.5X", "2X CROP" };
			Print(286, y, modes[opt < emu_scale::MAX ? opt : 0], selected, COLOR_CYAN);
		}
	}

	display_fill footer;
	footer.x1 = 0; footer.x2 = 383; footer.y1 = 192; footer.y2 = 215; footer.mode = 1;
	Bdisp_AreaClr(&footer, 1, COLOR_BLACK);
	Print(10, 198, "UP / DOWN  NAVIGATE", false, COLOR_SILVER);
	Print(235, 198, "EXE  CHANGE", false, COLOR_CYAN);
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

static const char* getMappedKeyLabel(unsigned char code) {
	switch (code) {
		case 78: return "SHIFT";
		case 68: return "OPTN";
		case 39: return "F5";
		case 29: return "F6";
		case 27: return "RIGHT";
		case 38: return "LEFT";
		case 28: return "UP";
		case 37: return "DOWN";
		case 43: return "S";
		case 25: return "L";
		case 48: return "MENU";
		default: return NULL;
	}
}

void screen_settings::drawCurrentKeyMap(int selected) {
	ResolveBG(bg_menu);
	EnableStatusArea(3);
	display_fill header;
	header.x1 = 0; header.x2 = 383; header.y1 = 0; header.y2 = 43; header.mode = 1;
	Bdisp_AreaClr(&header, 1, COLOR_BLACK);
	display_fill accent;
	accent.x1 = 0; accent.x2 = 383; accent.y1 = 42; accent.y2 = 45; accent.mode = 1;
	Bdisp_AreaClr(&accent, 1, COLOR_CYAN);
	Print(12, 5, "PRIZMGBC", true, COLOR_WHITE);
	Print(12, 24, "CONTROLS / KEY MAPPING", false, COLOR_CYAN);
	Print(300, 15, "v1.0.0", false, COLOR_CYAN);

	for (int i = 0; i < emu_button::MAX; i++) {
		int column = i / 5;
		int row = i % 5;
		int x = column ? 202 : 14;
		int y = 57 + row * 25;
		if (i == selected) {
			display_fill highlight;
			highlight.x1 = x - 4; highlight.x2 = x + 175;
			highlight.y1 = y - 3; highlight.y2 = y + 16; highlight.mode = 1;
			Bdisp_AreaClr(&highlight, 1, COLOR_MEDIUMBLUE);
		}
		Print(x, y, keyName[i], i == selected, COLOR_WHITE);
		const char* keyLabel = getMappedKeyLabel(emulator.settings.keyMap[i]);
		char keyText[20];
		if (keyLabel) strcpy(keyText, keyLabel);
		else sprintf(keyText, "KEY %02u", emulator.settings.keyMap[i]);
		Print(x + 112, y, keyText, true, COLOR_CYAN);
	}
	display_fill footer;
	footer.x1 = 0; footer.x2 = 383; footer.y1 = 192; footer.y2 = 215; footer.mode = 1;
	Bdisp_AreaClr(&footer, 1, COLOR_BLACK);
	Print(8, 198, "UP/DN SELECT", false, COLOR_SILVER);
	Print(130, 198, "EXE CHANGE", false, COLOR_CYAN);
	Print(237, 198, "F1 RESET", false, COLOR_YELLOW);
	Print(320, 198, "MENU BACK", false, COLOR_SILVER);
	DrawFrame(0);
}

void screen_settings::editKeys() {
	int selected = 0;
	for (;;) {
		drawCurrentKeyMap(selected);
		int key = 0;
		GetKey(&key);

		if (key == KEY_CTRL_UP) {
			selected = (selected + emu_button::MAX - 1) % emu_button::MAX;
		} else if (key == KEY_CTRL_DOWN) {
			selected = (selected + 1) % emu_button::MAX;
		} else if (key == KEY_CTRL_MENU || key == KEY_CTRL_EXIT) {
			return;
		} else if (key == KEY_CTRL_F1) {
			// Restore only the controls, leaving all other user settings intact.
			static const unsigned char defaults[emu_button::MAX] = {
				78, 68, 39, 29, 27, 38, 28, 37, 43, 25
			};
			memcpy(emulator.settings.keyMap, defaults, sizeof(defaults));
			emulator.saveSettings();
		} else if (key == KEY_CTRL_EXE || key == KEY_CTRL_SHIFT) {
			// Let the confirmation key lift before waiting for the replacement.
			getCurrentKey(false);
			display_fill prompt;
			prompt.x1 = 55; prompt.x2 = 328; prompt.y1 = 82; prompt.y2 = 137; prompt.mode = 1;
			Bdisp_AreaClr(&prompt, 1, COLOR_WHITE);
			const char* promptText = "PRESS A NEW KEY";
			Print(192 - PrintWidth(promptText) / 2, 94, promptText, true, COLOR_BLACK);
			Print(192 - PrintWidth(keyName[selected]) / 2, 114, keyName[selected], true, COLOR_DARKCYAN);
			Bdisp_PutDisp_DD();
			unsigned char newKey = getCurrentKey();
			// MENU cancels this one change.
			if (newKey != 48) {
				emulator.settings.keyMap[selected] = newKey;
				emulator.saveSettings();
			}
		}
	}
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
			drawCurrentKeyMap();
			int key = 0;
			GetKey(&key);
			if (key == KEY_CTRL_EXE || key == KEY_CTRL_SHIFT) {
				selectKeys();
			}
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

