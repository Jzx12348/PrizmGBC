
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

	// PrizmGBC identity bar: high contrast and calculator-friendly.
	display_fill header;
	header.x1 = 0; header.x2 = 383; header.y1 = 0; header.y2 = 38; header.mode = 1;
	Bdisp_AreaClr(&header, 1, COLOR_BLACK);
	display_fill accent;
	accent.x1 = 0; accent.x2 = 383; accent.y1 = 37; accent.y2 = 39; accent.mode = 1;
	Bdisp_AreaClr(&accent, 1, COLOR_CYAN);

	Print(10, 5, "PRIZMGBC", false, COLOR_WHITE);
	Print(10, 21, "GAME LIBRARY", false, COLOR_CYAN);
	Print(278, 21, "GB / GBC", false, COLOR_SILVER);

	for (int i = 0; i < numFiles; i++) {
		int curY = i * 16 - curScroll + 44;

		if (curY >= 42 && curY < 188) {
			char buffer[16];
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer, "%02d/%02d", i + 1, numFiles);
			int width = PrintWidth(buffer);
			if (i == selectedFile) {
				display_fill marker;
				marker.x1 = 5; marker.x2 = 8; marker.y1 = curY + 1; marker.y2 = curY + 12; marker.mode = 1;
				Bdisp_AreaClr(&marker, 1, COLOR_CYAN);
			}
			Print(14, curY, buffer, i == selectedFile, COLOR_SILVER);
			Print(14 + width + 10, curY, files[i].path, i == selectedFile, COLOR_WHITE);
		}
	}

	display_fill footer;
	footer.x1 = 0; footer.x2 = 383; footer.y1 = 192; footer.y2 = 215; footer.mode = 1;
	Bdisp_AreaClr(&footer, 1, COLOR_BLACK);
	Print(10, 198, "UP/DOWN  BROWSE", false, COLOR_SILVER);
	Print(238, 198, "F6 LOAD / EXE START", false, COLOR_CYAN);

	EnableStatusArea(3);
	DrawFrame(0);
}

void screen_rom::setup() {
	loadedFile = -1;
}

void screen_rom::select() {
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
	
	if (numFiles) {
		checkScroll();
		drawFiles();
	} else {
		int x = 10;
		int y = 10;
		PrintMini(&x, &y, "No .gb or .gbc files found", 0x42, 0xffffffff, 0, 0, COLOR_RED, COLOR_BLACK, 1, 0);
	}
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