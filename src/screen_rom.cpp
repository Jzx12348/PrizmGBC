
#include "platform.h"
#include "screen_rom.h"
#include "rom.h"

typedef struct
{
	unsigned short id, type;
	unsigned long fsize, dsize;
	unsigned int property;
	unsigned long address;
} file_type_t;

static void FindFiles(const char* path, foundFile* toArray, int& numFound) {
	unsigned short filter[0x100], found[0x100];
	int ret, handle = -1;
	file_type_t info; // See Bfile_FindFirst for the definition of this struct

	Bfile_StrToName_ncpy(filter, path, 0x50); // Overkill

	ret = Bfile_FindFirst((const char*)filter, &handle, (char*)found, &info);
	if (ret != 0) return;

	while (ret == 0 && numFound < 64) {
		Bfile_NameToStr_ncpy(toArray[numFound++].path, found, 32);
		ret = Bfile_FindNext(handle, (char*)found, (char*)&info);
	};

	if (handle >= 0) Bfile_FindClose(handle);
}


static const char* cartridgeTypeName(unsigned char type) {
	switch (type) {
		case 0x00: return "ROM ONLY";
		case 0x01: return "MBC1";
		case 0x02: return "MBC1 + RAM";
		case 0x03: return "MBC1 + RAM + BATTERY";
		case 0x05: return "MBC2";
		case 0x06: return "MBC2 + BATTERY";
		case 0x08: return "ROM + RAM";
		case 0x09: return "ROM + RAM + BATTERY";
		case 0x0F: return "MBC3 + TIMER + BATTERY";
		case 0x10: return "MBC3 + TIMER + RAM";
		case 0x11: return "MBC3";
		case 0x12: return "MBC3 + RAM";
		case 0x13: return "MBC3 + RAM + BATTERY";
		case 0x19: return "MBC5";
		case 0x1A: return "MBC5 + RAM";
		case 0x1B: return "MBC5 + RAM + BATTERY";
		case 0x1C: return "MBC5 + RUMBLE";
		case 0x1D: return "MBC5 + RUMBLE + RAM";
		case 0x1E: return "MBC5 + RUMBLE + RAM + BATTERY";
		case 0x22: return "MBC7 + SENSOR";
		case 0xFC: return "POCKET CAMERA";
		case 0xFD: return "BANDAI TAMA5";
		case 0xFE: return "HuC3";
		case 0xFF: return "HuC1 + RAM + BATTERY";
		default: return "OTHER / UNKNOWN";
	}
}

static const char* romSizeName(unsigned char code) {
	static char label[20];
	if (code <= 8) {
		sprintf(label, "%lu KB", (unsigned long)(32UL << code));
		return label;
	}
	if (code == 0x52) return "1152 KB";
	if (code == 0x53) return "1280 KB";
	if (code == 0x54) return "1536 KB";
	return "UNKNOWN";
}

static const char* ramSizeName(unsigned char code) {
	switch (code) {
		case 0: return "NONE";
		case 1: return "2 KB";
		case 2: return "8 KB";
		case 3: return "32 KB";
		case 4: return "128 KB";
		case 5: return "64 KB";
		default: return "UNKNOWN";
	}
}

static bool readGameHeader(const char* filename, unsigned char* header, int& fileSize) {
	char path[64];
	unsigned short pFile[256];
	strcpy(path, "\\\\fls0\\");
	strcat(path, filename);
	Bfile_StrToName_ncpy(pFile, (const char*)path, strlen(path) + 2);
	int handle = Bfile_OpenFile_OS(pFile, READ, 0);
	if (handle < 0) return false;
	fileSize = Bfile_GetFileSize_OS(handle);
	if (fileSize < 0x150) {
		Bfile_CloseFile_OS(handle);
		return false;
	}
	Bfile_ReadFile_OS(handle, header, 0x150, 0);
	Bfile_CloseFile_OS(handle);
	return true;
}

static int findFavorite(const char* filename) {
	int count = emulator.settings.favoriteCount;
	if (count > 8) count = 8;
	for (int i = 0; i < count; i++) {
		if (!strcmp(emulator.settings.favoriteRoms[i], filename)) return i;
	}
	return -1;
}

static int findRecent(const char* filename) {
	int count = emulator.settings.recentCount;
	if (count > 5) count = 5;
	for (int i = 0; i < count; i++) {
		if (!strcmp(emulator.settings.recentRoms[i], filename)) return i;
	}
	return -1;
}

static unsigned int findPlaytime(const char* filename) {
	int count = emulator.settings.playtimeCount;
	if (count > 16) count = 16;
	for (int i = 0; i < count; i++) {
		if (!strcmp(emulator.settings.playtimeRoms[i], filename)) {
			return emulator.settings.playtimeSeconds[i];
		}
	}
	return 0;
}

static int compareNames(const char* left, const char* right) {
	while (*left && *right) {
		char a = *left++;
		char b = *right++;
		if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
		if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
		if (a != b) return a < b ? -1 : 1;
	}
	if (*left) return 1;
	if (*right) return -1;
	return 0;
}

void screen_rom::sortFiles() {
	for (int i = 1; i < numFiles; i++) {
		foundFile current = files[i];
		int currentRank = 0;
		if (emulator.settings.sortMode == 1) {
			currentRank = findRecent(current.path);
			if (currentRank < 0) currentRank = 100;
		} else if (emulator.settings.sortMode == 2) {
			currentRank = findFavorite(current.path) >= 0 ? 0 : 1;
		} else if (emulator.settings.sortMode == 3) {
			currentRank = (int)findPlaytime(current.path);
		}

		int j = i - 1;
		while (j >= 0) {
			int previousRank = 0;
			if (emulator.settings.sortMode == 1) {
				previousRank = findRecent(files[j].path);
				if (previousRank < 0) previousRank = 100;
			} else if (emulator.settings.sortMode == 2) {
				previousRank = findFavorite(files[j].path) >= 0 ? 0 : 1;
			} else if (emulator.settings.sortMode == 3) {
				previousRank = (int)findPlaytime(files[j].path);
			}

			bool before = emulator.settings.sortMode == 3 ?
				currentRank > previousRank : currentRank < previousRank;
			if (currentRank == previousRank) before = compareNames(current.path, files[j].path) < 0;
			if (!before) break;
			files[j + 1] = files[j];
			j--;
		}
		files[j + 1] = current;
	}
}

void screen_rom::discoverFiles() {
	files = new foundFile[64];
	numFiles = 0;
	FindFiles("\\\\fls0\\*.gbz", files, numFiles);			// compressed gameboy ROM with utility
	FindFiles("\\\\fls0\\*.gbc", files, numFiles);			// gameboy color
	FindFiles("\\\\fls0\\*.gb", files, numFiles);			// gameboy
	sortFiles();
	// TODO : These don't work on Prizm?
	// FindFiles("\\\\fls0\\Games\\*.gb", files, numFiles);
	// FindFiles("\\\\fls0\\ROMS\\*.gb", files, numFiles);
}


void screen_rom::drawGameDetails() {
	ResolveBG(bg_menu);
	EnableStatusArea(3);

	display_fill headerBar;
	headerBar.x1 = 0; headerBar.x2 = 383; headerBar.y1 = 0; headerBar.y2 = 43; headerBar.mode = 1;
	Bdisp_AreaClr(&headerBar, 1, COLOR_BLACK);
	display_fill accent;
	accent.x1 = 0; accent.x2 = 383; accent.y1 = 42; accent.y2 = 45; accent.mode = 1;
	Bdisp_AreaClr(&accent, 1, COLOR_LIGHTGREEN);
	Print(12, 5, "PRIZMGBC", true, COLOR_WHITE);
	Print(12, 24, "GAME INFORMATION", false, COLOR_LIGHTGREEN);
	Print(300, 5, "v1.3.1", false, COLOR_LIGHTGREEN);

	display_fill leftPanel;
	leftPanel.x1 = 8; leftPanel.x2 = 187; leftPanel.y1 = 53; leftPanel.y2 = 185; leftPanel.mode = 1;
	Bdisp_AreaClr(&leftPanel, 1, COLOR_DARKGRAY);
	display_fill rightPanel;
	rightPanel.x1 = 194; rightPanel.x2 = 375; rightPanel.y1 = 53; rightPanel.y2 = 185; rightPanel.mode = 1;
	Bdisp_AreaClr(&rightPanel, 1, COLOR_BLACK);

	unsigned char header[0x150];
	int fileSize = 0;
	bool valid = readGameHeader(files[selectedFile].path, header, fileSize);
	Print(18, 60, "FILE", false, COLOR_LIGHTGREEN);
	Print(18, 78, files[selectedFile].path, true, COLOR_WHITE);
	if (!valid) {
		Print(18, 111, "HEADER UNAVAILABLE", true, COLOR_WHITE);
		Print(18, 132, "The file could not be", false, COLOR_SILVER);
		Print(18, 146, "read as a valid ROM.", false, COLOR_SILVER);
	} else {
		char title[17];
		int titleLength = 0;
		for (int i = 0; i < 16; i++) {
			unsigned char ch = header[ROM_OFFSET_NAME + i];
			if (ch == 0 || ch == 0x80 || ch == 0xC0) break;
			if (ch < 32 || ch > 126) ch = '?';
			title[titleLength++] = (char)ch;
		}
		title[titleLength] = 0;
		if (!titleLength) strcpy(title, "UNTITLED");
		Print(18, 105, "INTERNAL TITLE", false, COLOR_SILVER);
		Print(18, 120, title, true, COLOR_WHITE);
		Print(18, 148, "FILE SIZE", false, COLOR_SILVER);
		char sizeText[24];
		sprintf(sizeText, "%d KB", fileSize / 1024);
		Print(18, 164, sizeText, false, COLOR_WHITE);

		Print(204, 60, "CARTRIDGE", false, COLOR_LIGHTGREEN);
		Print(204, 78, cartridgeTypeName(header[ROM_OFFSET_TYPE]), true, COLOR_WHITE);
		Print(204, 101, "ROM CAPACITY", false, COLOR_SILVER);
		Print(204, 116, romSizeName(header[ROM_OFFSET_ROM_SIZE]), false, COLOR_WHITE);
		Print(204, 139, "RAM CAPACITY", false, COLOR_SILVER);
		Print(204, 154, ramSizeName(header[ROM_OFFSET_RAM_SIZE]), false, COLOR_WHITE);
		Print(204, 174, (header[ROM_OFFSET_GBC] == 0x80 || header[ROM_OFFSET_GBC] == 0xC0) ?
			"GAME BOY COLOR" : "GAME BOY / DMG", false, COLOR_LIGHTGREEN);

		unsigned int playedSeconds = findPlaytime(files[selectedFile].path);
		unsigned int playedMinutes = playedSeconds / 60;
		char playtimeText[32];
		if (playedMinutes >= 60) {
			sprintf(playtimeText, "PLAYED: %uH %uM", playedMinutes / 60, playedMinutes % 60);
		} else if (playedMinutes > 0) {
			sprintf(playtimeText, "PLAYED: %u MIN", playedMinutes);
		} else {
			strcpy(playtimeText, playedSeconds ? "PLAYED: UNDER 1 MIN" : "NOT PLAYED YET");
		}
		Print(18, 178, playtimeText, false, COLOR_YELLOW);
	}

	display_fill footer;
	footer.x1 = 0; footer.x2 = 383; footer.y1 = 192; footer.y2 = 215; footer.mode = 1;
	Bdisp_AreaClr(&footer, 1, COLOR_BLACK);
	Print(10, 198, "EXE BACK TO LIBRARY", false, COLOR_SILVER);
	Print(178, 198, findFavorite(files[selectedFile].path) >= 0 ? "F3 UNFAVOURITE" : "F3 FAVOURITE", false, COLOR_YELLOW);
	Print(292, 198, "F6 PLAY", false, COLOR_LIGHTGREEN);
	DrawFrame(0);
}

void screen_rom::drawFiles() {
	if (detailsMode) { drawGameDetails(); return; }
	ResolveBG(bg_menu);
	EnableStatusArea(3);

	// Clean, high-contrast application header.
	display_fill header;
	header.x1 = 0; header.x2 = 383; header.y1 = 0; header.y2 = 43; header.mode = 1;
	Bdisp_AreaClr(&header, 1, COLOR_BLACK);
	display_fill accent;
	accent.x1 = 0; accent.x2 = 383; accent.y1 = 42; accent.y2 = 45; accent.mode = 1;
	Bdisp_AreaClr(&accent, 1, COLOR_LIGHTGREEN);
	Print(12, 5, "PRIZMGBC", true, COLOR_WHITE);
	Print(12, 24, "GAME LIBRARY", false, COLOR_LIGHTGREEN);
	char countText[24];
	sprintf(countText, "%d GAMES", numFiles);
	Print(286, 5, "v1.3.1", false, COLOR_LIGHTGREEN);
	Print(300, 24, countText, false, COLOR_SILVER);

	// Main library and game information panels.
	display_fill list;
	list.x1 = 7; list.x2 = 254; list.y1 = 52; list.y2 = 181; list.mode = 1;
	Bdisp_AreaClr(&list, 1, COLOR_DARKGRAY);
	display_fill detail;
	detail.x1 = 261; detail.x2 = 376; detail.y1 = 52; detail.y2 = 181; detail.mode = 1;
	Bdisp_AreaClr(&detail, 1, COLOR_BLACK);

	if (!numFiles) {
		Print(20, 82, "LIBRARY EMPTY", true, COLOR_WHITE);
		Print(20, 103, "Add game files to the", false, COLOR_SILVER);
		Print(20, 117, "calculator root folder.", false, COLOR_SILVER);
		Print(20, 145, "GB  /  GBC  /  GBZ", false, COLOR_LIGHTGREEN);
	} else {
		const char* sortNames[] = { "SORT: NAME", "SORT: RECENT", "SORT: FAVOURITES", "SORT: PLAYTIME" };
		Print(16, 56, sortNames[emulator.settings.sortMode < 4 ? emulator.settings.sortMode : 0], false, COLOR_LIGHTGREEN);
		if (!gridMode) {
			for (int i = 0; i < numFiles; i++) {
				int curY = i * 16 - curScroll + 73;
				if (curY >= 72 && curY < 176) {
					if (i == selectedFile) {
						display_fill selected;
						selected.x1 = 10; selected.x2 = 251;
						selected.y1 = curY - 2; selected.y2 = curY + 12;
						selected.mode = 1;
						Bdisp_AreaClr(&selected, 1, COLOR_DARKGREEN);
						display_fill marker;
						marker.x1 = 10; marker.x2 = 13;
						marker.y1 = curY - 2; marker.y2 = curY + 12;
						marker.mode = 1;
						Bdisp_AreaClr(&marker, 1, COLOR_LIGHTGREEN);
					}
					char index[8];
					sprintf(index, "%02d", i + 1);
					Print(18, curY, index, false, COLOR_SILVER);
					Print(43, curY, files[i].path, i == selectedFile, COLOR_WHITE);
					if (findRecent(files[i].path) >= 0) Print(216, curY, "R", false, COLOR_LIGHTGREEN);
					if (findFavorite(files[i].path) >= 0) Print(232, curY, "*", true, COLOR_YELLOW);
				}
			}
		} else {
			for (int slot = 0; slot < 6; slot++) {
				int fileIndex = curScroll + slot;
				if (fileIndex >= numFiles) break;
				int column = slot % 2;
				int row = slot / 2;
				int x = 12 + column * 120;
				int y = 78 + row * 35;
				display_fill tile;
				tile.x1 = x; tile.x2 = x + 113;
				tile.y1 = y; tile.y2 = y + 29; tile.mode = 1;
				Bdisp_AreaClr(&tile, 1, fileIndex == selectedFile ? COLOR_DARKGREEN : COLOR_BLACK);
				if (fileIndex == selectedFile) {
					display_fill marker;
					marker.x1 = x; marker.x2 = x + 3;
					marker.y1 = y; marker.y2 = y + 29; marker.mode = 1;
					Bdisp_AreaClr(&marker, 1, COLOR_LIGHTGREEN);
				}
				char label[20];
				int length = strlen(files[fileIndex].path);
				if (length > 15) {
					memcpy(label, files[fileIndex].path, 12);
					strcpy(label + 12, "...");
				} else {
					strcpy(label, files[fileIndex].path);
				}
				Print(x + 8, y + 6, label, fileIndex == selectedFile, COLOR_WHITE);
				Print(x + 8, y + 19, findFavorite(files[fileIndex].path) >= 0 ? "FAVOURITE" :
					(findRecent(files[fileIndex].path) >= 0 ? "RECENTLY PLAYED" : "GAME FILE"),
					false, COLOR_SILVER);
			}
		}

		// Selected game details.
		Print(270, 59, "SELECTED", false, COLOR_LIGHTGREEN);
		Print(270, 82, files[selectedFile].path, true, COLOR_WHITE);
		Print(270, 112, "FORMAT", false, COLOR_SILVER);
		const char* format = "GAME BOY";
		const char* filename = files[selectedFile].path;
		int length = strlen(filename);
		if (length >= 4 && (filename[length - 3] == 'g' || filename[length - 3] == 'G') &&
			(filename[length - 2] == 'b' || filename[length - 2] == 'B') &&
			(filename[length - 1] == 'c' || filename[length - 1] == 'C')) format = "GAME BOY COLOR";
		else if (length >= 4 && (filename[length - 3] == 'g' || filename[length - 3] == 'G') &&
			(filename[length - 2] == 'b' || filename[length - 2] == 'B') &&
			(filename[length - 1] == 'z' || filename[length - 1] == 'Z')) format = "COMPRESSED";
		Print(270, 126, format, false, COLOR_WHITE);
		Print(270, 151, "PLAYTIME", false, COLOR_SILVER);
		unsigned int playedSeconds = findPlaytime(files[selectedFile].path);
		char playedText[20];
		if (playedSeconds >= 3600) sprintf(playedText, "%uH %uM", playedSeconds / 3600, (playedSeconds / 60) % 60);
		else if (playedSeconds >= 60) sprintf(playedText, "%u MIN", playedSeconds / 60);
		else strcpy(playedText, playedSeconds ? "UNDER 1 MIN" : "NOT PLAYED");
		Print(270, 165, playedText, false, COLOR_WHITE);
		Print(270, 180, findFavorite(files[selectedFile].path) >= 0 ? "FAVOURITE *" : "F3 TO FAVOURITE", false, COLOR_YELLOW);
	}

	// Consistent footer navigation.
	display_fill footer;
	footer.x1 = 0; footer.x2 = 383; footer.y1 = 184; footer.y2 = 215; footer.mode = 1;
	Bdisp_AreaClr(&footer, 1, COLOR_BLACK);

	// Place each action directly above its physical function-key position.
	const int actionCenters[] = { 32, 96, 160, 224, 288, 352 };
	const char* actionLabels[] = { "MENU", "SETTINGS", "FAV", gridMode ? "LIST" : "GRID", "NOTES", "PLAY" };
	const char* keyLabels[] = { "F1", "F2", "F3", "F4", "F5", "F6" };
	const unsigned short actionColors[] = { COLOR_SILVER, COLOR_SILVER, COLOR_YELLOW, COLOR_LIGHTGREEN, COLOR_LIGHTGREEN, COLOR_LIGHTGREEN };
	for (int i = 0; i < 6; i++) {
		Print(actionCenters[i] - PrintWidth(actionLabels[i]) / 2, 187,
			actionLabels[i], false, actionColors[i]);
		Print(actionCenters[i] - PrintWidth(keyLabels[i]) / 2, 201,
			keyLabels[i], true, COLOR_WHITE);
	}
	DrawFrame(0);
}
void screen_rom::setup() {
	files = NULL;
	numFiles = 0;
	selectedFile = 0;
	loadedFile = -1;
	curScroll = 0;
	gridMode = emulator.settings.libraryView == 1;
	detailsMode = false;
}

void screen_rom::select() {
	// Keep the library viewport deterministic every time the screen is opened.
	curScroll = 0;
	detailsMode = false;

	// file discovery doesn't work well when rom file is currently active, so we close it
	loadedFile = -1;
	bool wasLoaded = mbc.romFile != 0;
	if (wasLoaded) {
		Bfile_CloseFile_OS(mbc.romFile);
		mbc.romFile = 0;
	}

	discoverFiles();

	ResolveBG(bg_menu);
	DrawPausePreview();

	// if our selected file from settings is in the rom list
	selectedFile = 0;
	for (int i = 0; i < numFiles; i++) {
		if (!strcmp(files[i].path, emulator.settings.selectedRom)) {
			selectedFile = i;
			if (wasLoaded) {
				loadedFile = i;
			}
			break;
		}
	}
	
	if (numFiles) checkScroll();
	drawFiles();
}

void screen_rom::deselect() {
	if (numFiles) {
		strcpy(emulator.settings.selectedRom, files[selectedFile].path);
		if (selectedFile == loadedFile) {
			char romFile[64];
			unsigned short pFile[256];
			strcpy(romFile, "\\\\fls0\\");
			strcat(romFile, files[selectedFile].path);
			Bfile_StrToName_ncpy(pFile, (const char*) romFile, strlen(romFile) + 2);
			mbc.romFile = Bfile_OpenFile_OS(pFile, READ, 0);
		}
	}

	delete[] files;
	files = NULL;

	emulator.saveSettings();

	emulator.pausePreviewValid = false;
}

bool screen_rom::checkScroll() {
	if (gridMode) {
		int page = (selectedFile / 6) * 6;
		bool changed = curScroll != page;
		curScroll = page;
		return changed;
	}
	int selectY = selectedFile * 16 - curScroll + 73;

	if (selectY < 72) {
		curScroll = 16 * selectedFile;
		return true;
	}
	if (selectY > 180) {
		curScroll = 16 * selectedFile - 107;
		return true;
	}
	return false;
}

void screen_rom::handleUp() {
	if (numFiles) {
		selectedFile = gridMode ? (selectedFile >= 2 ? selectedFile - 2 : selectedFile) :
			(selectedFile + numFiles - 1) % numFiles;
		checkScroll();
		drawFiles();
	}
}

void screen_rom::handleDown() {
	if (numFiles) {
		if (gridMode) {
			selectedFile = selectedFile + 2 < numFiles ? selectedFile + 2 : selectedFile;
		} else {
			selectedFile = (selectedFile + 1) % numFiles;
		}
		checkScroll();
		drawFiles();
	}
}

void screen_rom::changeSort(int direction) {
	char selectedName[32] = { 0 };
	if (numFiles && selectedFile >= 0 && selectedFile < numFiles) {
		strcpy(selectedName, files[selectedFile].path);
	}
	emulator.settings.sortMode = (emulator.settings.sortMode + 4 + direction) % 4;
	sortFiles();
	selectedFile = 0;
	for (int i = 0; i < numFiles; i++) {
		if (!strcmp(files[i].path, selectedName)) {
			selectedFile = i;
			break;
		}
	}
	checkScroll();
	emulator.saveSettings();
	drawFiles();
}

void screen_rom::handleLeft() {
	if (gridMode) {
		if (numFiles && selectedFile % 2 == 1) selectedFile--;
		checkScroll();
		drawFiles();
	} else {
		changeSort(-1);
	}
}

void screen_rom::handleRight() {
	if (gridMode) {
		if (numFiles && selectedFile % 2 == 0 && selectedFile + 1 < numFiles) selectedFile++;
		checkScroll();
		drawFiles();
	} else {
		changeSort(1);
	}
}

void screen_rom::handleSelect() {
	if (!numFiles) return;
	detailsMode = !detailsMode;
	drawFiles();
}

void screen_rom::handleF4() {
	if (detailsMode) return;
	gridMode = !gridMode;
	emulator.settings.libraryView = gridMode ? 1 : 0;
	emulator.saveSettings();
	curScroll = 0;
	checkScroll();
	drawFiles();
}

void screen_rom::handleF3() {
	if (!numFiles) return;

	const char* filename = files[selectedFile].path;
	int favorite = findFavorite(filename);
	int count = emulator.settings.favoriteCount;
	if (count > 8) count = 8;

	if (favorite >= 0) {
		for (int i = favorite; i < count - 1; i++) {
			strcpy(emulator.settings.favoriteRoms[i], emulator.settings.favoriteRoms[i + 1]);
		}
		if (count > 0) {
			emulator.settings.favoriteRoms[count - 1][0] = 0;
			emulator.settings.favoriteCount = count - 1;
		}
	} else if (count < 8) {
		strcpy(emulator.settings.favoriteRoms[count], filename);
		emulator.settings.favoriteCount = count + 1;
	}

	emulator.saveSettings();
	drawFiles();
}