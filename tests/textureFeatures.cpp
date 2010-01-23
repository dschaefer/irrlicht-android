// Copyright (C) 2008-2009 Christian Stehno, Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

//! Tests locking miplevels
static bool lockAllMipLevels(E_DRIVER_TYPE driverType)
{
	bool result=true;

	IrrlichtDevice *device = createDevice( driverType, dimension2d<u32>(160, 120), 32);
	if (!device)
		return result; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager * smgr = device->getSceneManager();

	scene::ISceneNode* n = smgr->addCubeSceneNode();

	if (n)
	{
		// create the texture and miplevels with distinct colors
		u32 texData[16*16];
		for (u32 i=0; i<16*16; ++i)
			texData[i]=0xff0000ff;
		video::IImage* image = driver->createImageFromData(video::ECF_A8R8G8B8, core::dimension2du(16,16), texData, false);
		u32 mipdata[8*16];
		u32 index=0;
		for (u32 j=8; j>0; j/=2)
		{
			u32 val=(j==8?0x00ff00ff:(j==4?0x0000ffff:(j==2?0xc2c200ff:0x001212ff)));
			for (u32 i=0; i<j; ++i)
			{
				for (u32 k=0; k<j; ++k)
					mipdata[index++]=val;
			}
		}

		ITexture* tex = driver->addTexture("miptest", image, mipdata);
		if (!tex)
			// is probably an error in the mipdata handling
			return false;
		else
			n->setMaterialTexture(0, tex);
		image->drop();
	}

	(void)smgr->addCameraSceneNode();

	driver->beginScene(true, true, SColor(255,100,101,140));
	smgr->drawAll();
	driver->endScene();

	video::ITexture* tex = driver->findTexture("miptest");
	video::SColor* bits = (video::SColor*)tex->lock(true, 0);
	result |= (bits[0].color==0xff0000ff);
	tex->unlock();
	bits = (video::SColor*)tex->lock(true, 1);
	result |= (bits[0].color==0x00ff00ff);
	tex->unlock();
	bits = (video::SColor*)tex->lock(true, 2);
	result |= (bits[0].color==0x0000ffff);
	tex->unlock();
	bits = (video::SColor*)tex->lock(true, 3);
	result |= (bits[0].color==0xc2c200ff);
	tex->unlock();
	bits = (video::SColor*)tex->lock(true, 4);
	result |= (bits[0].color==0x001212ff);
	tex->unlock();
	
	device->drop();

	return result;
}


bool textureFeatures(void)
{
	bool passed = true;

	passed &= lockAllMipLevels(EDT_OPENGL);
	passed &= lockAllMipLevels(EDT_SOFTWARE);
	passed &= lockAllMipLevels(EDT_BURNINGSVIDEO);
	passed &= lockAllMipLevels(EDT_DIRECT3D9);
	passed &= lockAllMipLevels(EDT_DIRECT3D8);

	return passed;
}

