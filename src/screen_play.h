#pragma once

#include "emulator.h"

// Restore the CPU setting if the application is closed while a game is running.
void restorePtuneSetting();

struct screen_play : public emulator_screen {
	char loadedROM[32];

	virtual void setup() override;
	virtual void select() override;
	virtual void deselect() override;
	virtual void handleSelect() override;
	virtual void postStateChange() override;

private:
	bool initRom();
	void play();
	void drawPlayBG();
};