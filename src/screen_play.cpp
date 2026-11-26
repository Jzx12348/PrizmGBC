
#include "platform.h"
#include "keys.h"
#include "rom.h"
#include "memory.h"
#include "cgb.h"
#include "snd/snd.h"
#include "cgb_bootstrap.h"
#include "ptune2_simple/Ptune2_direct.h"

#include "screen_play.h"

// Ptune2 changes are only supported on CG20 hardware. Keep track of changes
// so the OS clock configuration can be restored on both normal and forced exit.
static bool ptuneSettingChanged = false;

void restorePtuneSetting() {
	if (!ptuneSettingChanged) return;
	Ptune2_LoadSetting(PT2_DEFAULT);
	ptuneSettingChanged = false;
}

static void recordPlaytime(const char* filename, unsigned int elapsedSeconds) {
	if (!filename || !filename[0] || elapsedSeconds == 0) return;

	int count = emulator.settings.playtimeCount;
	if (count > 16) count = 16;
	int existing = -1;
	for (int i = 0; i < count; i++) {
		if (!strcmp(emulator.settings.playtimeRoms[i], filename)) {
			existing = i;
			break;
		}
	}

	unsigned int totalSeconds = existing >= 0 ? emulator.settings.playtimeSeconds[existing] : 0;
	if (existing >= 0) {
		for (int i = existing; i > 0; i--) {
			strcpy(emulator.settings.playtimeRoms[i], emulator.settings.playtimeRoms[i - 1]);
			emulator.settings.playtimeSeconds[i] = emulator.settings.playtimeSeconds[i - 1];
		}
	} else {
		if (count < 16) count++;
		for (int i = count - 1; i > 0; i--) {
			strcpy(emulator.settings.playtimeRoms[i], emulator.settings.playtimeRoms[i - 1]);
			emulator.settings.playtimeSeconds[i] = emulator.settings.playtimeSeconds[i - 1];
		}
	}

	strcpy(emulator.settings.playtimeRoms[0], filename);
	emulator.settings.playtimeSeconds[0] = totalSeconds + elapsedSeconds;
	emulator.settings.playtimeCount = count;
	emulator.saveSettings();
}

static void rememberRecentRom(const char* filename) {
	int count = emulator.settings.recentCount;
	if (count > 5) count = 5;
	int existing = -1;
	for (int i = 0; i < count; i++) {
		if (!strcmp(emulator.settings.recentRoms[i], filename)) {
			existing = i;
			break;
		}
	}

	int last = existing >= 0 ? existing : (count < 5 ? count : 4);
	for (int i = last; i > 0; i--) {
		strcpy(emulator.settings.recentRoms[i], emulator.settings.recentRoms[i - 1]);
	}
	strcpy(emulator.settings.recentRoms[0], filename);
	if (existing < 0 && count < 5) emulator.settings.recentCount = count + 1;
	emulator.saveSettings();
}

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
	rememberRecentRom(loadedROM);
	return true;
}

void screen_play::drawPlayBG() {
	if (emulator.settings.scaleMode != emu_scale::NONE) {
		DrawBGEmbedded((unsigned char*)bg_fit);
	} else {
		DrawBGEmbedded((unsigned char*)bg_1x1);
	}
	EnableStatusArea(3);

	// Clear the unused side margins once per gameplay session. The renderer
	// updates only the game image, so rewriting these pixels per scanline is wasteful.
	if (emulator.settings.scaleMode == emu_scale::FIT_150 ||
		emulator.settings.scaleMode == emu_scale::CROP_200) {
		unsigned short* screen = (unsigned short*)GetVRAMAddress();
		const int margin = emulator.settings.scaleMode == emu_scale::FIT_150 ? 72 : 32;
		const int rightStart = LCD_WIDTH_PX - margin;
		for (int y = 0; y < LCD_HEIGHT_PX; y++) {
			unsigned short* row = screen + y * LCD_WIDTH_PX;
			for (int x = 0; x < margin; x++) {
				row[x] = COLOR_BLACK;
				row[rightStart + x] = COLOR_BLACK;
			}
		}
	}

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
	// The bundled Ptune2 library explicitly warns against use on CG50 devices.
	// Never call it there, including for the otherwise-default no-memory-wait mode.
	bool doOverclock = getDeviceType() == DT_CG20 && Ptune2_GetSetting() == PT2_DEFAULT;
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
		ptuneSettingChanged = true;
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

	// Track active gameplay time using the calculator RTC tick counter.
	unsigned int playtimeStart = (unsigned int)RTC_GetTicks();

	// wait for exit "key" to exit (This actually renders a preview screen first)
	keys.exit = false;
	while (keys.exit == false) {
		cpuStep();
	}

	// make sure menu isn't pressed
	while (keyDown_fast(48) == true);

	// Persist the elapsed playtime for this ROM.
	unsigned int elapsedTicks = (unsigned int)RTC_GetTicks() - playtimeStart;
	recordPlaytime(loadedROM, elapsedTicks / 128U);

	// save sram
	saveRAM();

	// Restore any Ptune2 setting changed for this session.
	restorePtuneSetting();

	if (soundInitted) {
		sndCleanup();
	}

#if DEBUG
	ScopeTimer::Shutdown();
#endif

	// Return to the game library after MENU exits gameplay.
	emulator.tryScreenChange(1);
}