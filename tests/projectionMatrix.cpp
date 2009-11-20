// Copyright (C) 2008-2009 Christian Stehno, Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

//! Tests projection matrices
static bool runTestWithDriver(E_DRIVER_TYPE driverType)
{
	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	IVideoDriver* driver = device->getVideoDriver();

	bool result = true;

	driver->beginScene(true, false, SColor(255,0,0,0));
	                
	SMaterial mat;
	mat.MaterialType = EMT_SOLID;
	mat.Lighting = false;
	mat.ZBuffer = false;
	mat.ZWriteEnable = false;
	mat.Thickness = 1;

	driver->setMaterial(mat);

	core::dimension2d<u32> dims = driver->getCurrentRenderTargetSize();
	//apply custom projection, no offset
	core::matrix4 pmtx = matrix4().buildProjectionMatrixOrthoLH(dims.Width, dims.Height, 0, 100);
	driver->setTransform(ETS_PROJECTION, pmtx);
	driver->setTransform(ETS_VIEW, matrix4());
	driver->setTransform(ETS_WORLD, matrix4());

	//the red cross appears at center
	for (u32 i=0; i<10; ++i)
	{
		driver->draw3DLine(vector3df(0+i,-50,1), vector3df(0+i,50,1), SColor(255,255,0,0));
		driver->draw3DLine(vector3df(-50,0+i,1), vector3df(50,0+i,1), SColor(255,255,0,0));
	}

	//apply custom projection, offset to right-top
	pmtx.setTranslation(vector3df(0.7, 0.7, 0));
	driver->setTransform(ETS_PROJECTION, pmtx);
	driver->setTransform(ETS_VIEW, matrix4());
	driver->setTransform(ETS_WORLD, matrix4());

	//The green cross must be in right-top corner. But for OpenGL driver it is in left-top corner
	for (u32 i=0; i<10; ++i)
	{
		driver->draw3DLine(vector3df(0+i,-50,1), vector3df(0+i,50,1), SColor(255,0,255,0));
		driver->draw3DLine(vector3df(-50,0+i,1), vector3df(50,0+i,1), SColor(255,0,255,0));
	}

	driver->endScene();

	result = takeScreenshotAndCompareAgainstReference(driver, "-projMat.png");

	device->drop();

	return result;
}


bool projectionMatrix(void)
{
	bool passed = true;

	passed &= runTestWithDriver(EDT_SOFTWARE);
	passed &= runTestWithDriver(EDT_BURNINGSVIDEO);
	passed &= runTestWithDriver(EDT_DIRECT3D9);
	passed &= runTestWithDriver(EDT_DIRECT3D8);
	passed &= runTestWithDriver(EDT_OPENGL);

	return passed;
}

