#pragma once

#include "emulator.h"

struct foundFile {
	char path[32];
};

struct screen_rom : public emulator_screen {
	foundFile* files;
	int numFiles;
	int selectedFile;
	int loadedFile;
	int curScroll;
	bool gridMode;
	bool detailsMode;

	virtual void setup() override;
	virtual void select() override;
	virtual void deselect() override;

	// key press handles
	virtual void handleUp() override;
	virtual void handleDown() override;
	virtual void handleLeft() override;
	virtual void handleRight() override;
	virtual void handleSelect() override;
	virtual void handleF3() override;
	virtual void handleF4() override;

private:
	void discoverFiles();
	void sortFiles();
	void changeSort(int direction);
	void drawFiles();
	void drawGameDetails();
	bool checkScroll();
};