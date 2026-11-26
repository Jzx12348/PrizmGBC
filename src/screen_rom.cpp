
#include "platform.h"
#include "screen_rom.h"

typedef struct
{
	unsigned short id, type;
	unsigned long fsize, dsize;
	unsigned int property;
	unsigned long address;
} file_type_t;

static void FindFiles(const char* path, foundFile* toArray, int& numFound) {
	unsigned short filter[0x100], found[0x100];
	int ret, handle;
	file_type_t info; // See Bfile_FindFirst for the definition of this struct

	Bfile_StrToName_ncpy(filter, path, 0x50); // Overkill

	ret = Bfile_FindFirst((const char*)filter, &handle, (char*)found, &info);

	while (ret == 0 && numFound < 64) {
		Bfile_NameToStr_ncpy(toArray[numFound++].path, found, 32);
		ret = Bfile_FindNext(handle, (char*)found, (char*)&info);
	};

	Bfile_FindClose(handle);
}

static int findFavorite(const char* filename) {
	int count = emulator.settings.favoriteCount;
	if (count > 8) count = 8;
	for (int i = 0; i < count; i++) {
		if (!strcmp(emulator.settings.favoriteRoms[i], filename)) return i;
	}
	return -1;
}

void screen_rom::discoverFiles() {
	files = new foundFile[64];
	numFiles = 0;
	FindFiles("\\\\fls0\\*.gbz", files, numFiles);			// compressed gameboy ROM with utility
	FindFiles("\\\\fls0\\*.gbc", files, numFiles);			// gameboy color
	FindFiles("\\\\fls0\\*.gb", files, numFiles);			// gameboy
	// TODO : These don't work on Prizm?
	// FindFiles("\\\\fls0\\Games\\*.gb", files, numFiles);
	// FindFiles("\\\\fls0\\ROMS\\*.gb", files, numFiles);
}

void screen_rom::drawFiles() {
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
	Print(300, 5, "v1.0.0", false, COLOR_LIGHTGREEN);
	Print(300, 24, countText, false, COLOR_SILVER);

	// Main library and game information panels.
	display_fill list;
	list.x1 = 7; list.x2 = 254; list.y1 = 52; list.y2 = 187; list.mode = 1;
	Bdisp_AreaClr(&list, 1, COLOR_DARKGRAY);
	display_fill detail;
	detail.x1 = 261; detail.x2 = 376; detail.y1 = 52; detail.y2 = 187; detail.mode = 1;
	Bdisp_AreaClr(&detail, 1, COLOR_BLACK);

	if (!numFiles) {
		Print(20, 82, "LIBRARY EMPTY", true, COLOR_WHITE);
		Print(20, 103, "Add game files to the", false, COLOR_SILVER);
		Print(20, 117, "calculator root folder.", false, COLOR_SILVER);
		Print(20, 145, "GB  /  GBC  /  GBZ", false, COLOR_LIGHTGREEN);
	} else {
		Print(16, 56, "YOUR GAMES", false, COLOR_LIGHTGREEN);
		for (int i = 0; i < numFiles; i++) {
			int curY = i * 16 - curScroll + 73;
			if (curY >= 72 && curY < 184) {
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
				if (findFavorite(files[i].path) >= 0) Print(232, curY, "*", true, COLOR_YELLOW);
				Print(43, curY, files[i].path, i == selectedFile, COLOR_WHITE);
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
		Print(270, 151, "STATUS", false, COLOR_SILVER);
		Print(270, 165, selectedFile == loadedFile ? "LOADED" : "READY", false, COLOR_LIGHTGREEN);
		Print(270, 180, findFavorite(files[selectedFile].path) >= 0 ? "FAVORITE *" : "F3 TO FAVORITE", false, COLOR_YELLOW);
	}

	// Consistent footer navigation.
	display_fill footer;
	footer.x1 = 0; footer.x2 = 383; footer.y1 = 192; footer.y2 = 215; footer.mode = 1;
	Bdisp_AreaClr(&footer, 1, COLOR_BLACK);
	Print(10, 198, "UP / DOWN  BROWSE", false, COLOR_SILVER);
	Print(145, 198, "F3  FAV", false, COLOR_YELLOW);
	Print(220, 198, "F2 SETTINGS", false, COLOR_SILVER);
	Print(316, 198, "F6 PLAY", false, COLOR_LIGHTGREEN);
	DrawFrame(0);
}
void screen_rom::setup() {
	files = NULL;
	numFiles = 0;
	selectedFile = 0;
	loadedFile = -1;
	curScroll = 0;
}

void screen_rom::select() {
	// Keep the library viewport deterministic every time the screen is opened.
	curScroll = 0;

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
		selectedFile = (selectedFile + numFiles - 1) % numFiles;

		checkScroll();
		drawFiles();
	}
}

void screen_rom::handleDown() {
	if (numFiles) {
		selectedFile = (selectedFile + 1) % numFiles;

		checkScroll();
		drawFiles();
	}
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