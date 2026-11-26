#include "platform.h"
#include "screen_savestate.h"
#include "screen_play.h"

void screen_savestate::setup() {
	selectedSlot = 0;
}

bool screen_savestate::hasActiveGame() {
	screen_play* play = (screen_play*)emulator.screens[2];
	return play && play->loadedROM[0] &&
		!strcmp(play->loadedROM, emulator.settings.selectedRom) &&
		mbc.romFile != 0;
}

void screen_savestate::select() {
	selectedSlot = emulator.settings.saveSlot < 3 ? emulator.settings.saveSlot : 0;
	draw();
}

void screen_savestate::postStateChange() {
	draw();
}

void screen_savestate::draw() {
	ResolveBG(bg_menu);
	EnableStatusArea(3);

	display_fill header;
	header.x1 = 0; header.x2 = 383; header.y1 = 0; header.y2 = 43; header.mode = 1;
	Bdisp_AreaClr(&header, 1, COLOR_BLACK);
	display_fill accent;
	accent.x1 = 0; accent.x2 = 383; accent.y1 = 42; accent.y2 = 45; accent.mode = 1;
	Bdisp_AreaClr(&accent, 1, COLOR_LIGHTGREEN);
	Print(12, 5, "PRIZMGBC", true, COLOR_WHITE);
	Print(12, 24, "SAVE STATE MANAGER", false, COLOR_LIGHTGREEN);
	Print(300, 5, "v1.3.0 BETA", false, COLOR_LIGHTGREEN);

	display_fill panel;
	panel.x1 = 8; panel.x2 = 375; panel.y1 = 54; panel.y2 = 184; panel.mode = 1;
	Bdisp_AreaClr(&panel, 1, COLOR_DARKGRAY);

	Print(20, 62, "CURRENT GAME", false, COLOR_LIGHTGREEN);
	Print(20, 80, emulator.settings.selectedRom[0] ? emulator.settings.selectedRom : "NO GAME SELECTED", true, COLOR_WHITE);
	bool active = hasActiveGame();
	Print(20, 99, active ? "GAME STATE IS LOADED" : "LOAD A GAME BEFORE USING SLOTS", false,
		active ? COLOR_LIGHTGREEN : COLOR_YELLOW);

	for (int i = 0; i < 3; i++) {
		int y = 119 + i * 19;
		if (i == selectedSlot) {
			display_fill highlight;
			highlight.x1 = 15; highlight.x2 = 368; highlight.y1 = y - 3; highlight.y2 = y + 13; highlight.mode = 1;
			Bdisp_AreaClr(&highlight, 1, COLOR_BLACK);
			display_fill marker;
			marker.x1 = 15; marker.x2 = 18; marker.y1 = y - 3; marker.y2 = y + 13; marker.mode = 1;
			Bdisp_AreaClr(&marker, 1, COLOR_LIGHTGREEN);
		}
		char slotText[24];
		sprintf(slotText, "SLOT %d", i + 1);
		Print(25, y, slotText, i == selectedSlot, COLOR_WHITE);
		bool saved = active && emulator.hasSaveState(i);
		Print(285, y, saved ? "SAVED" : (active ? "EMPTY" : "N/A"), false,
			saved ? COLOR_LIGHTGREEN : COLOR_SILVER);
	}

	display_fill footer;
	footer.x1 = 0; footer.x2 = 383; footer.y1 = 192; footer.y2 = 215; footer.mode = 1;
	Bdisp_AreaClr(&footer, 1, COLOR_BLACK);
	Print(8, 198, "UP/DN SLOT", false, COLOR_SILVER);
	Print(105, 198, "EXE SELECT", false, COLOR_LIGHTGREEN);
	Print(207, 198, "F3 SAVE", false, COLOR_YELLOW);
	Print(278, 198, "F4 LOAD", false, COLOR_LIGHTGREEN);
	Print(344, 198, "F1", false, COLOR_SILVER);
	DrawFrame(0);
}

void screen_savestate::handleUp() {
	selectedSlot = (selectedSlot + 2) % 3;
	draw();
}

void screen_savestate::handleDown() {
	selectedSlot = (selectedSlot + 1) % 3;
	draw();
}

void screen_savestate::handleSelect() {
	emulator.settings.saveSlot = (unsigned char)selectedSlot;
	emulator.saveSettings();
	draw();
}

void screen_savestate::handleF3() {
	if (!hasActiveGame()) {
		draw();
		return;
	}
	emulator.settings.saveSlot = (unsigned char)selectedSlot;
	emulator.saveSettings();
	emulator.saveState();
	draw();
}

void screen_savestate::handleF4() {
	if (!hasActiveGame() || !emulator.hasSaveState(selectedSlot)) {
		draw();
		return;
	}
	emulator.settings.saveSlot = (unsigned char)selectedSlot;
	emulator.saveSettings();
	emulator.loadState();
	draw();
}
