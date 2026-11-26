#pragma once

#include "emulator.h"

struct screen_savestate : public emulator_screen {
	int selectedSlot;

	virtual void setup() override;
	virtual void select() override;
	virtual void postStateChange() override;
	virtual void handleUp() override;
	virtual void handleDown() override;
	virtual void handleSelect() override;
	virtual void handleF3() override;
	virtual void handleF4() override;

private:
	bool hasActiveGame();
	void draw();
};
