#pragma once
#include "olcPixelGameEngine.cpp"

class GUI : public olc::PixelGameEngine
{
public:
	GUI();

public:
	bool OnUserCreate() override;
	bool OnUserUpdate(float fElapsedTime) override;
};