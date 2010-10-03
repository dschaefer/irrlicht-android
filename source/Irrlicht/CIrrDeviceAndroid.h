// Copyright (C) 2010 Doug Schaefer
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IRR_DEVICE_ANDROID_H_INCLUDED__
#define __C_IRR_DEVICE_ANDROID_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_

#include "CIrrDeviceStub.h"
#include "IImagePresenter.h"

namespace irr
{
	class CIrrDeviceAndroid : public CIrrDeviceStub, public video::IImagePresenter
	{
		CIrrDeviceAndroid(const SIrrlichtCreationParameters& param);

		virtual ~CIrrDeviceAndroid();

	private:

		void createDriver();
	};
}

#endif // _IRR_COMPILE_WITH_ANDROID_DEVICE_

#endif // __C_IRR_DEVICE_ANDROID_H_INCLUDED__
