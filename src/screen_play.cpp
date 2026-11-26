
#include "platform.h"
#include "keys.h"
#include "rom.h"
#include "memory.h"
#include "cgb.h"
#include "snd/snd.h"
#include "cgb_bootstrap.h"
#include "ptune2_simple/Ptune2_direct.h"

#include "screen_play.h"

void screen_play::setup() {
	loadedROM[0] = 0;
}

void screen_play::select() {
	// Reuse an already loaded ROM when possible.
	if (loadedROM[0]) {
		if (strcmp(loadedROM, emulator.settings.selectedRom)) {
			unloadROM();
			loadedROM[0] = 0;
		} else {
			play();
			return;
		}
	}

	// F6 is the Play action: load and launch immediately.
	if (initRom()) {
		play();
	}
}

void screen_play::deselect() {
}

void screen_play::handleSelect() {
	if (mbc.romFile) play();
}

bool screen_play::initRom() {
	ResolveBG(bg_menu);
	reset_printf();
	printf("Loading file \"%s\"...\n", emulator.settings.selectedRom);

	if (!loadROM(emulator.settings.selectedRom) || !mbc.romFile) {
		loadedROM[0] = 0;
		ResolveBG(bg_menu);
		Print(14, 8, "PRIZMGBC", false, COLOR_WHITE);
		Print(14, 27, "GAME COULD NOT START", false, COLOR_CYAN);
		Print(18, 82, "ROM LOAD FAILED", false, COLOR_WHITE);
		Print(18, 103, "Check the ROM file and format.", false, COLOR_SILVER);
		Print(18, 119, "Supported: .GB, .GBC, .GBZ", false, COLOR_SILVER);
		display_fill footer;
		footer.x1 = 0; footer.x2 = 383; footer.y1 = 192; footer.y2 = 215; footer.mode = 1;
		Bdisp_AreaClr(&footer, 1, COLOR_BLACK);
		Print(10, 198, "F1 LIBRARY   F2 SETTINGS", false, COLOR_SILVER);
		DrawFrame(0);
		Bdisp_PutDisp_DD();
		return false;
	}

	sndStartup();
	strcpy(loadedROM, emulator.settings.selectedRom);
	return true;
}

void screen_play::drawPlayBG() {
	if (emulator.settings.scaleMode != emu_scale::NONE) {
		DrawBGEmbedded((unsigned char*)bg_fit);
	} else {
		DrawBGEmbedded((unsigned char*)bg_1x1);
	}
	EnableStatusArea(3);
	DrawFrame(0);
	Bdisp_PutDisp_DD();
}

void screen_play::postStateChange() {
	drawPlayBG();
}

bool bSoundEnabled = false;

void screen_play::play() {
	if (!mbc.romFile) return;
	mbcFileUpdate();

	drawPlayBG();

	bool soundInitted = false;
	
	if (emulator.settings.sound) {
		soundInitted = sndInit();
		bSoundEnabled = soundInitted;
	}

	// Apply settings
	bool doOverclock = Ptune2_GetSetting() == PT2_DEFAULT;
	if (doOverclock) {
		if (emulator.settings.overclock) {
			// clock up to double unless not scaling/sound/CGB
			if (emulator.settings.scaleMode != emu_scale::NONE || emulator.settings.sound || cgb.isCGB) {
				Ptune2_LoadSetting(PT2_DOUBLE);
			} else {
				Ptune2_LoadSetting(PT2_HALFINC);
			}
		} else {
			Ptune2_LoadSetting(PT2_NOMEMWAIT);
		}
	}

	// look for gameboy color palette override
	if (!cgb.isCGB) {
		if (!emulator.settings.useCGBColors || !getCGBTableEntry(&memoryMap[0][ROM_OFFSET_NAME], &ppuPalette[12])) {
			colorpalette_type pal;
			emulator.getPalette(emulator.settings.bgColorPalette, pal);
			for (int i = 0; i < 4; i++) {
				ppuPalette[i + 12] = pal.colors[i] | (pal.colors[i] << 16);
			}

			emulator.getPalette(emulator.settings.obj1ColorPalette, pal);
			for (int i = 0; i < 4; i++) {
				ppuPalette[i + 16] = pal.colors[i] | (pal.colors[i] << 16);
			}

			emulator.getPalette(emulator.settings.obj2ColorPalette, pal);
			for (int i = 0; i < 4; i++) {
				ppuPalette[i + 20] = pal.colors[i] | (pal.colors[i] << 16);
			}
		}

		SetupDisplayPalette();
	}

	SetupDisplayDriver(emulator.settings.frameSkip);

#if DEBUG
	// init debug timing system
	ScopeTimer::InitSystem();
#endif

	// wait for exit "key" to exit (This actually renders a preview screen first)
	keys.exit = false;
	while (keys.exit == false) {
		cpuStep();
	}

	// make sure menu isn't pressed
	while (keyDown_fast(48) == true);

	// save sram
	saveRAM();

	// undo overclock
	if (doOverclock) {
		Ptune2_LoadSetting(PT2_DEFAULT);
	}

	if (soundInitted) {
		sndCleanup();
	}

#if DEBUG
	ScopeTimer::Shutdown();
#endif

	// Return to the game library after MENU exits gameplay.
	emulator.tryScreenChange(1);
}