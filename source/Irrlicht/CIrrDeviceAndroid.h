// Copyright (C) 2010 Doug Schaefer
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IRR_DEVICE_ANDROID_H_INCLUDED__
#define __C_IRR_DEVICE_ANDROID_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_

#include "CIrrDeviceStub.h"
#include "IImagePresenter.h"
#include "os.h"

namespace irr
{
	class CIrrDeviceAndroid : public CIrrDeviceStub, public video::IImagePresenter
	{
	public:
		CIrrDeviceAndroid(const SIrrlichtCreationParameters& param);

		virtual ~CIrrDeviceAndroid();

		virtual bool run();
		virtual void yield();
		virtual void sleep(u32 timeMs, bool pauseTimer=false);
		virtual void setWindowCaption(const wchar_t* text);
		virtual bool isWindowActive() const;
		virtual bool isWindowFocused() const;
		virtual bool isWindowMinimized() const;
		virtual void closeDevice();
		virtual void setResizable(bool resize=false);
		virtual void minimizeWindow();
		virtual void maximizeWindow();
		virtual void restoreWindow();
		virtual E_DEVICE_TYPE getType() const;
		virtual bool present(video::IImage* surface, void* windowId=0, core::rect<s32>* src=0 );

	private:
		void createDriver();
	};
}

#endif // _IRR_COMPILE_WITH_ANDROID_DEVICE_

#endif // __C_IRR_DEVICE_ANDROID_H_INCLUDED__
