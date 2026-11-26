#pragma once

#include "emulator.h"

struct screen_settings : public emulator_screen {
	int curOption;
	int curCategory;

	virtual void setup() override;
	virtual void select() override;
	virtual void deselect() override;

	// key press handles
	virtual void handleUp() override;
	virtual void handleDown() override;
	virtual void handleLeft() override;
	virtual void handleRight() override;
	virtual void handleSelect() override;

private:
	void drawOptions();
	void drawCurrentKeyMap(int selected);
	void editKeys();
	void moveCategory(int direction);
};
