
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

	// PRIZMGBC / LIBRARY
	display_fill top;
	top.x1 = 0; top.x2 = 383; top.y1 = 0; top.y2 = 43; top.mode = 1;
	Bdisp_AreaClr(&top, 1, COLOR_BLACK);
	display_fill accent;
	accent.x1 = 0; accent.x2 = 383; accent.y1 = 42; accent.y2 = 45; accent.mode = 1;
	Bdisp_AreaClr(&accent, 1, COLOR_CYAN);

	Print(12, 5, "PRIZMGBC", false, COLOR_WHITE);
	Print(12, 23, "GAME LIBRARY", false, COLOR_CYAN);
	Print(300, 23, "GB / GBC", false, COLOR_SILVER);

	display_fill list;
	list.x1 = 6; list.x2 = 377; list.y1 = 51; list.y2 = 187; list.mode = 1;
	Bdisp_AreaClr(&list, 1, COLOR_DARKGRAY);

	if (!numFiles) {
		Print(18, 83, "NO GAMES FOUND", false, COLOR_WHITE);
		Print(18, 103, "Place .GB, .GBC or .GBZ files", false, COLOR_SILVER);
		Print(18, 117, "in the calculator root folder.", false, COLOR_SILVER);
	} else {
		for (int i = 0; i < numFiles; i++) {
			int curY = i * 16 - curScroll + 58;
			if (curY >= 54 && curY < 184) {
				if (i == selectedFile) {
					display_fill selected;
					selected.x1 = 9; selected.x2 = 374;
					selected.y1 = curY - 2; selected.y2 = curY + 13;
					selected.mode = 1;
					Bdisp_AreaClr(&selected, 1, COLOR_MEDIUMBLUE);
					display_fill marker;
					marker.x1 = 9; marker.x2 = 12;
					marker.y1 = curY - 2; marker.y2 = curY + 13;
					marker.mode = 1;
					Bdisp_AreaClr(&marker, 1, COLOR_CYAN);
				}
				char index[12];
				memset(index, 0, sizeof(index));
				sprintf(index, "%02d", i + 1);
				Print(19, curY, index, i == selectedFile, COLOR_SILVER);
				Print(48, curY, files[i].path, i == selectedFile, COLOR_WHITE);
				if (i == loadedFile) Print(325, curY, "LOADED", false, COLOR_CYAN);
			}
		}
	}

	display_fill footer;
	footer.x1 = 0; footer.x2 = 383; footer.y1 = 192; footer.y2 = 215; footer.mode = 1;
	Bdisp_AreaClr(&footer, 1, COLOR_BLACK);
	Print(10, 198, "F1 LIBRARY   F2 SETTINGS", false, COLOR_SILVER);
	Print(265, 198, "F6 PLAY", false, COLOR_CYAN);
	EnableStatusArea(3);
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
	int selectY = selectedFile * 16 - curScroll + 44;

	if (selectY < 44) {
		curScroll = 16 * selectedFile;
		return true;
	}
	if (selectY > 178) {
		curScroll = 16 * selectedFile - 134;
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