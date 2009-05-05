// Copyright (C) 2008-2009 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"
#include "irrlicht.h"
#include <assert.h>

using namespace irr;
using namespace scene;
using namespace video;

bool enumerateImageManipulators(void)
{
    IrrlichtDevice *device = createDevice(video::EDT_NULL);
    if (!device)
        return false;

    IVideoDriver* driver = device->getVideoDriver();

	const char filenames[][8] = 
	{
		"foo.bmp",
		"foo.jpg",
		"foo.pcx",
		"foo.png",
		"foo.ppm",
		"foo.psd",
		"foo.tga",
		"foo.wal"
	};
	const u32 numberOfFilenames = sizeof(filenames) / sizeof(filenames[0]);
	bool loaderForFilename[numberOfFilenames] = { false }; // and the rest get 0 == false
	bool writerForFilename[numberOfFilenames] = { false }; // and the rest get 0 == false

	bool result = true;

	u32 i;
	const u32 loaders = driver->getImageLoaderCount();
	for (i = 0; i < loaders; ++i)
	{
		IImageLoader * loader = driver->getImageLoader(i);
		
		if(!loader)
		{
			logTestString("Failed to get image loader %d\n", i);
			assert(false);
			result = false;
		}

		for(u32 filename = 0; filename < numberOfFilenames; ++filename)
		{
			if(loader->isALoadableFileExtension(filenames[filename]))
			{
				loaderForFilename[filename] = true;
				break;
			}
		}
	}

	IImageLoader * loader = driver->getImageLoader(i);
	assert(loader == 0);
	if(loader)
	{
		logTestString("Got a loader when none was expected (%d)\n", i);
		result = false;
	}

	for(u32 filename = 0; filename < numberOfFilenames; ++filename)
	{
		if(!loaderForFilename[filename])
		{
			logTestString("File type '%s' doesn't have a loader\n", filenames[filename]);
			assert(false);
			result = false;
		}
	}

	const u32 writers = driver->getImageWriterCount();
	for (i = 0; i < writers; ++i)
	{
		IImageWriter * writer = driver->getImageWriter(i);
		
		if(!writer)
		{
			logTestString("Failed to get image writer %d\n", i);
			assert(false);
			result = false;
		}

		for(u32 filename = 0; filename < numberOfFilenames; ++filename)
		{
			if(writer->isAWriteableFileExtension(filenames[filename]))
			{
				writerForFilename[filename] = true;
				break;
			}
		}
	}

	IImageWriter * writer = driver->getImageWriter(i);
	assert(writer == 0);
	if(writer)
	{
		logTestString("Got a writer when none was expected (%d)\n", i);
		result = false;
	}

	for(u32 filename = 0; filename < numberOfFilenames; ++filename)
	{
		// There's no writer for the .WAL file type.
		if(!writerForFilename[filename] && 0 != strcmp(filenames[filename], "foo.wal"))
		{
			logTestString("File type '%s' doesn't have a writer\n", filenames[filename]);
			assert(false);
			result = false;
		}
	}

    device->drop();

    return result;
}
