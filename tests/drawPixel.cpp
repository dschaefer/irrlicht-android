// Copyright (C) 2008-2009 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

//! Tests IVideoDriver::drawPixel().
/** Expect to see two diagonal lines overlaying a wall texture cube.
	One line should run from green at the top left to red at the bottom right.
	The other should run from cyan 100% transparent at the bottom left to
	cyan 100% opaque at the top right. */
static bool runTestWithDriver(E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager * smgr = device->getSceneManager();

	// Draw a cube background so that we can check that the pixels' alpha is working.
	ISceneNode * cube = smgr->addCubeSceneNode(50.f, 0, -1, vector3df(0, 0, 60));
	cube->setMaterialTexture(0, driver->getTexture("../media/wall.bmp"));
	cube->setMaterialFlag(video::EMF_LIGHTING, false);
	(void)smgr->addCameraSceneNode();

	driver->beginScene(true, true, SColor(255,100,101,140));
	smgr->drawAll();

	// Test for benign handling of offscreen pixel values as well as onscreen ones.
	for(s32 x = -10; x < 170; ++x)
	{
		s32 y = 120 * x / 160;
		driver->drawPixel((u32)x, (u32)y, SColor(255, 255 * x / 640, 255 * (640 - x) / 640, 0));
		y = 120 - y;
		driver->drawPixel((u32)x, (u32)y, SColor(255 * x / 640, 0, 255, 255));
	}

	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-drawPixel.png");

	device->drop();

	return result;
}


bool drawPixel(void)
{
	bool passed = true;

	logTestString("Check Software driver\n");
	passed &= runTestWithDriver(EDT_SOFTWARE);
	logTestString("Check Burning's Video driver\n");
	passed &= runTestWithDriver(EDT_BURNINGSVIDEO);
	logTestString("Check Direct3D9 driver\n");
	passed &= runTestWithDriver(EDT_DIRECT3D9);
	logTestString("Check Direct3D8 driver\n");
	passed &= runTestWithDriver(EDT_DIRECT3D8);
	logTestString("Check OpenGL driver\n");
	passed &= runTestWithDriver(EDT_OPENGL);

	return passed;
}

