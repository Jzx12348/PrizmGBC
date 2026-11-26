#include "platform.h"
#include "keys.h"

#include "screen_settings.h"
#include "ptune2_simple/Ptune2_direct.h"

enum settings_category {
	CATEGORY_PERFORMANCE = 0,
	CATEGORY_DISPLAY,
	CATEGORY_CONTROLS,
	CATEGORY_AUDIO,
	CATEGORY_SYSTEM,
	CATEGORY_COUNT
};

static const char* categoryNames[CATEGORY_COUNT] = {
	"PERF", "DISPLAY", "CTRL", "AUDIO", "SYSTEM"
};

struct option_type {
	const char* name;
	int type; // 0 = toggle, 1 = keys, 2 = frameskip, 3 = color, 4 = scale mode
	int category;
	void* addr;
	bool disabled;
	bool noCGB;
};

static option_type options[] = {
	{ "Overclock", 0, CATEGORY_PERFORMANCE, &emulator.settings.overclock, false, false },
	{ "Clamp Speed", 0, CATEGORY_PERFORMANCE, &emulator.settings.clampSpeed, false, false },
	{ "Frameskip", 2, CATEGORY_PERFORMANCE, &emulator.settings.frameSkip, false, false },
	{ "Fit To Screen", 4, CATEGORY_DISPLAY, &emulator.settings.scaleMode, false, false },
	{ "CGB Colors", 0, CATEGORY_DISPLAY, &emulator.settings.useCGBColors, false, false },
	{ "BG Colors", 3, CATEGORY_DISPLAY, &emulator.settings.bgColorPalette, false, false },
	{ "Sprite 1 Colors", 3, CATEGORY_DISPLAY, &emulator.settings.obj1ColorPalette, false, false },
	{ "Sprite 2 Colors", 3, CATEGORY_DISPLAY, &emulator.settings.obj2ColorPalette, false, false },
	{ "Map Keys", 1, CATEGORY_CONTROLS, NULL, false, false },
	{ "Sound", 0, CATEGORY_AUDIO, &emulator.settings.sound, false, false },
	{ "FPS Counter", 0, CATEGORY_SYSTEM, &emulator.settings.fpsCounter, false, false },
	{ "Save State Slot", 5, CATEGORY_SYSTEM, &emulator.settings.saveSlot, false, false },
	{ "Save Game Profile", 6, CATEGORY_SYSTEM, NULL, false, false },
	{ "Apply Game Profile", 7, CATEGORY_SYSTEM, NULL, false, false },
	{ "Save State Manager", 8, CATEGORY_SYSTEM, NULL, false, false },
};

static inline int NumOptions() {
	return sizeof(options) / sizeof(option_type);
}

static int categoryOptionCount(int category) {
	int count = 0;
	for (int i = 0; i < NumOptions(); i++) {
		if (options[i].category == category && !options[i].noCGB) count++;
	}
	return count;
}

static int categoryOptionIndex(int category, int row) {
	for (int i = 0; i < NumOptions(); i++) {
		if (options[i].category == category && !options[i].noCGB) {
			if (row == 0) return i;
			row--;
		}
	}
	return -1;
}

static int firstEnabledOption(int category) {
	int count = categoryOptionCount(category);
	for (int row = 0; row < count; row++) {
		int index = categoryOptionIndex(category, row);
		if (index >= 0 && !options[index].disabled) return row;
	}
	return 0;
}

void screen_settings::setup() {
	curOption = 0;
	curCategory = CATEGORY_PERFORMANCE;
}

void screen_settings::select() {
	ResolveBG(bg_menu);
	DrawPausePreview();

	for (int i = 0; i < NumOptions(); i++) {
		options[i].disabled = false;
		options[i].noCGB = false;
	}

	// Overclock is only available on supported CG-20 hardware when Ptune2 is inactive.
	int currentPtuneSetting = Ptune2_GetSetting();
	options[0].disabled = getDeviceType() != DT_CG20 || currentPtuneSetting != PT2_DEFAULT;
	int profileCount = emulator.settings.profileCount > 8 ? 8 : emulator.settings.profileCount;
	options[12].disabled = !emulator.settings.selectedRom[0] ||
		(!emulator.hasGameProfile(emulator.settings.selectedRom) && profileCount >= 8);
	options[13].disabled = !emulator.hasGameProfile(emulator.settings.selectedRom);
	options[14].disabled = !emulator.settings.selectedRom[0];

	if (emulator.isCGBRomLoaded()) {
		options[4].noCGB = true;
		options[5].noCGB = true;
		options[6].noCGB = true;
		options[7].noCGB = true;
	}

	curOption = firstEnabledOption(curCategory);
	drawOptions();
}

void screen_settings::deselect() {
	emulator.saveSettings();
}

void screen_settings::drawOptions() {
	ResolveBG(bg_menu);
	EnableStatusArea(3);

	display_fill header;
	header.x1 = 0; header.x2 = 383; header.y1 = 0; header.y2 = 39; header.mode = 1;
	Bdisp_AreaClr(&header, 1, COLOR_BLACK);
	display_fill accent;
	accent.x1 = 0; accent.x2 = 383; accent.y1 = 38; accent.y2 = 41; accent.mode = 1;
	Bdisp_AreaClr(&accent, 1, COLOR_LIGHTGREEN);
	Print(12, 4, "PRIZMGBC", true, COLOR_WHITE);
	Print(12, 22, "SETTINGS", false, COLOR_LIGHTGREEN);
	Print(310, 13, "v1.3.0 BETA", false, COLOR_LIGHTGREEN);

	const int tabX[CATEGORY_COUNT] = { 8, 84, 160, 236, 312 };
	const int tabW[CATEGORY_COUNT] = { 64, 64, 64, 64, 64 };
	for (int i = 0; i < CATEGORY_COUNT; i++) {
		if (i == curCategory) {
			display_fill tab;
			tab.x1 = tabX[i]; tab.x2 = tabX[i] + tabW[i];
			tab.y1 = 48; tab.y2 = 68; tab.mode = 1;
			Bdisp_AreaClr(&tab, 1, COLOR_DARKGREEN);
			Print(tabX[i] + (tabW[i] - PrintWidth(categoryNames[i])) / 2, 53, categoryNames[i], true, COLOR_LIGHTGREEN);
		} else {
			Print(tabX[i] + (tabW[i] - PrintWidth(categoryNames[i])) / 2, 53, categoryNames[i], false, COLOR_SILVER);
		}
	}

	display_fill panel;
	panel.x1 = 7; panel.x2 = 376; panel.y1 = 76; panel.y2 = 186; panel.mode = 1;
	Bdisp_AreaClr(&panel, 1, COLOR_DARKGRAY);
	Print(17, 81, categoryNames[curCategory], false, COLOR_LIGHTGREEN);

	int count = categoryOptionCount(curCategory);
	for (int row = 0; row < count; row++) {
		int index = categoryOptionIndex(curCategory, row);
		if (index < 0) continue;
		// Keep the five-row System category comfortably inside the grey panel.
		int rowSpacing = curCategory == CATEGORY_SYSTEM ? 15 : 17;
		int y = 99 + row * rowSpacing;
		bool selected = row == curOption;

		if (selected) {
			display_fill highlight;
			highlight.x1 = 11; highlight.x2 = 372;
			highlight.y1 = y - 3; highlight.y2 = y + 13; highlight.mode = 1;
			Bdisp_AreaClr(&highlight, 1, COLOR_BLACK);
			display_fill marker;
			marker.x1 = 11; marker.x2 = 14;
			marker.y1 = y - 3; marker.y2 = y + 13; marker.mode = 1;
			Bdisp_AreaClr(&marker, 1, COLOR_LIGHTGREEN);
		}

		Print(21, y, options[index].name, selected,
			options[index].disabled ? COLOR_SILVER : COLOR_WHITE);

		if (options[index].type == 0) {
			unsigned char isOn = *((unsigned char*)options[index].addr);
			const char* toggleText = (index == 0 && options[index].disabled)
				? "CG20 ONLY" : (isOn ? "ON" : "OFF");
			Print(294, y, toggleText, selected,
				options[index].disabled ? COLOR_SILVER : (isOn ? COLOR_LIGHTGREEN : COLOR_SILVER));
		} else if (options[index].type == 1) {
			char bindText[16];
			sprintf(bindText, "%d BOUND", emu_button::MAX);
			Print(286, y, bindText, selected, COLOR_LIGHTGREEN);
		} else if (options[index].type == 2) {
			char skip = *((char*)options[index].addr);
			if (skip == -1) Print(315, y, "AUTO", selected, COLOR_LIGHTGREEN);
			else {
				char buffer[8];
				sprintf(buffer, "%d", skip);
				Print(315, y, buffer, selected, COLOR_LIGHTGREEN);
			}
		} else if (options[index].type == 3) {
			colorpalette_type pal;
			emulator.getPalette(*((unsigned char*)options[index].addr), pal);
			for (int j = 0; j < 4; j++) {
				display_fill swatch;
				swatch.x1 = 306 + 16 * j; swatch.x2 = swatch.x1 + 12;
				swatch.y1 = y - 1; swatch.y2 = y + 11; swatch.mode = 1;
				Bdisp_AreaClr(&swatch, 1, pal.colors[j]);
			}
		} else if (options[index].type == 4) {
			unsigned char opt = *((unsigned char*)options[index].addr);
			const char* modes[] = { "1X", "FIT 1.5X", "2X CROP" };
			Print(286, y, modes[opt < emu_scale::MAX ? opt : 0], selected, COLOR_LIGHTGREEN);
		} else if (options[index].type == 5) {
			unsigned char slot = *((unsigned char*)options[index].addr);
			char slotText[12];
			sprintf(slotText, "SLOT %d", (slot < 3 ? slot : 0) + 1);
			Print(300, y, slotText, selected, COLOR_LIGHTGREEN);
		}
		else if (options[index].type == 6) {
			Print(300, y, "SAVE", selected, COLOR_LIGHTGREEN);
		} else if (options[index].type == 7) {
			Print(270, y, emulator.hasGameProfile(emulator.settings.selectedRom) ? "AVAILABLE" : "NONE",
				selected, options[index].disabled ? COLOR_SILVER : COLOR_LIGHTGREEN);
		} else if (options[index].type == 8) {
			Print(300, y, "OPEN", selected, COLOR_LIGHTGREEN);
		}
	}

	display_fill footer;
	footer.x1 = 0; footer.x2 = 383; footer.y1 = 192; footer.y2 = 215; footer.mode = 1;
	Bdisp_AreaClr(&footer, 1, COLOR_BLACK);
	Print(9, 198, "LEFT/RIGHT: CATEGORY", false, COLOR_SILVER);
	Print(250, 198, "UP/DOWN: OPTION", false, COLOR_LIGHTGREEN);
	DrawFrame(0);
}

static const char* keyName[] = {
	"A", "B", "Select", "Start", "Right", "Left", "Up", "Down", "Save State", "Load State"
};

unsigned char getCurrentKey(bool wait = true) {
	unsigned char keyPressed = 0;
	do {
		for (unsigned int c = 25; c <= 79; c++) {
			if (c % 10 == 0 || c == 34) continue;
			if (keyDown_fast(c)) {
				keyPressed = c;
				while (keyDown_fast(keyPressed)) {}
				break;
			}
		}
	} while (keyPressed == 0 && wait);
	OS_InnerWait_ms(10);
	return keyPressed;
}

static const char* getMappedKeyLabel(unsigned char code) {
	switch (code) {
		case 78: return "SHIFT";
		case 68: return "OPTN";
		case 59: return "F3";
		case 49: return "F4";
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
	Bdisp_AreaClr(&accent, 1, COLOR_LIGHTGREEN);
	Print(12, 5, "PRIZMGBC", true, COLOR_WHITE);
	Print(12, 24, "CONTROLS / KEY MAPPING", false, COLOR_LIGHTGREEN);
	Print(300, 15, "v1.3.0 BETA", false, COLOR_LIGHTGREEN);

	for (int i = 0; i < emu_button::MAX; i++) {
		int column = i / 5;
		int row = i % 5;
		int x = column ? 202 : 14;
		int y = 57 + row * 25;
		if (i == selected) {
			display_fill highlight;
			highlight.x1 = x - 4; highlight.x2 = x + 175;
			highlight.y1 = y - 3; highlight.y2 = y + 16; highlight.mode = 1;
			Bdisp_AreaClr(&highlight, 1, COLOR_DARKGREEN);
		}
		Print(x, y, keyName[i], i == selected, COLOR_WHITE);
		const char* keyLabel = getMappedKeyLabel(emulator.settings.keyMap[i]);
		char keyText[20];
		if (keyLabel) strcpy(keyText, keyLabel);
		else sprintf(keyText, "KEY %02u", emulator.settings.keyMap[i]);
		Print(x + 112, y, keyText, true, COLOR_LIGHTGREEN);
	}
	display_fill footer;
	footer.x1 = 0; footer.x2 = 383; footer.y1 = 192; footer.y2 = 215; footer.mode = 1;
	Bdisp_AreaClr(&footer, 1, COLOR_BLACK);
	Print(8, 198, "UP/DN SELECT", false, COLOR_SILVER);
	Print(130, 198, "EXE CHANGE", false, COLOR_LIGHTGREEN);
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
			static const unsigned char defaults[emu_button::MAX] = {
				78, 68, 39, 29, 27, 38, 28, 37, 59, 49
			};
			memcpy(emulator.settings.keyMap, defaults, sizeof(defaults));
			emulator.saveSettings();
		} else if (key == KEY_CTRL_EXE || key == KEY_CTRL_SHIFT) {
			getCurrentKey(false);
			display_fill prompt;
			prompt.x1 = 55; prompt.x2 = 328; prompt.y1 = 82; prompt.y2 = 137; prompt.mode = 1;
			Bdisp_AreaClr(&prompt, 1, COLOR_WHITE);
			const char* promptText = "PRESS A NEW KEY";
			Print(192 - PrintWidth(promptText) / 2, 94, promptText, true, COLOR_BLACK);
			Print(192 - PrintWidth(keyName[selected]) / 2, 114, keyName[selected], true, COLOR_DARKCYAN);
			Bdisp_PutDisp_DD();
			unsigned char newKey = getCurrentKey();
			if (newKey != 48) {
				emulator.settings.keyMap[selected] = newKey;
				emulator.saveSettings();
			}
		}
	}
}

void screen_settings::moveCategory(int direction) {
	curCategory = (curCategory + CATEGORY_COUNT + direction) % CATEGORY_COUNT;
	curOption = firstEnabledOption(curCategory);
	ResolveBG(bg_menu);
	DrawPausePreview();
	drawOptions();
}

void screen_settings::handleLeft() {
	moveCategory(-1);
}

void screen_settings::handleRight() {
	moveCategory(1);
}

void screen_settings::handleUp() {
	int count = categoryOptionCount(curCategory);
	if (!count) return;
	for (int attempt = 0; attempt < count; attempt++) {
		curOption = (curOption + count - 1) % count;
		int index = categoryOptionIndex(curCategory, curOption);
		if (index >= 0 && !options[index].disabled) break;
	}
	ResolveBG(bg_menu);
	DrawPausePreview();
	drawOptions();
}

void screen_settings::handleDown() {
	int count = categoryOptionCount(curCategory);
	if (!count) return;
	for (int attempt = 0; attempt < count; attempt++) {
		curOption = (curOption + 1) % count;
		int index = categoryOptionIndex(curCategory, curOption);
		if (index >= 0 && !options[index].disabled) break;
	}
	ResolveBG(bg_menu);
	DrawPausePreview();
	drawOptions();
}

void screen_settings::handleSelect() {
	int index = categoryOptionIndex(curCategory, curOption);
	if (index < 0 || options[index].disabled) return;

	switch (options[index].type) {
		case 0: {
			unsigned char* isOn = (unsigned char*)options[index].addr;
			*isOn = 1 - *isOn;
			break;
		}
		case 1:
			editKeys();
			break;
		case 2: {
			char* skip = (char*)options[index].addr;
			(*skip)++;
			if (*skip == 5) *skip = -1;
			break;
		}
		case 3: {
			unsigned char* palette = (unsigned char*)options[index].addr;
			*palette = ((*palette) + 1) % emulator.numPalettes();
			emulator.settings.useCGBColors = false;
			break;
		}
		case 4: {
			unsigned char* opt = (unsigned char*)options[index].addr;
			*opt = ((*opt) + 1) % emu_scale::MAX;
			break;
		}
		case 5: {
			unsigned char* slot = (unsigned char*)options[index].addr;
			*slot = ((*slot) + 1) % 3;
			break;
		}
		case 6:
			if (emulator.saveGameProfile(emulator.settings.selectedRom)) options[13].disabled = false;
			break;
		case 7:
			emulator.applyGameProfile(emulator.settings.selectedRom);
			break;
		case 8:
			emulator.tryScreenChange(4);
			return;
	}

	ResolveBG(bg_menu);
	DrawPausePreview();
	drawOptions();
}
