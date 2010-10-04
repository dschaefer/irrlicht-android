#include "CIrrDeviceAndroid.h"

#include "os.h"
#include "COSOperator.h"

namespace irr
{
	namespace video
	{
		IVideoDriver* createOGLES2Driver(const SIrrlichtCreationParameters& params,
				video::SExposedVideoData& data,
				io::IFileSystem* io);
	}
}

namespace irr
{

CIrrDeviceAndroid::CIrrDeviceAndroid(const SIrrlichtCreationParameters& param)
	: CIrrDeviceStub(param)
{
	#ifdef _DEBUG
	setDebugName("CIrrDeviceAndroid");
	#endif

	Operator = new COSOperator("Android");

	createDriver();

	createGUIAndScene();
}

void CIrrDeviceAndroid::createDriver()
{
	switch (CreationParams.DriverType)
	{
	case video::EDT_OGLES2:
#ifdef _IRR_COMPILE_WITH_OGLES2_
		{
			video::SExposedVideoData data;
			VideoDriver = video::createOGLES2Driver(CreationParams, data, FileSystem);
		}
#else
		os::Printer::log("No OpenGL-ES2 support compiled in.", ELL_ERROR);
#endif

	default:
		os::Printer::log("Unsupported video driver type.", ELL_ERROR);
	}
}

CIrrDeviceAndroid::~CIrrDeviceAndroid()
{
}

bool CIrrDeviceAndroid::run()
{
	os::Timer::tick();
	return true;
}

void CIrrDeviceAndroid::yield()
{
	// We yeild all the time
}

void CIrrDeviceAndroid::sleep(u32 timeMs, bool pauseTimer)
{
	const bool wasStopped = Timer ? Timer->isStopped() : true;

	struct timespec ts;
	ts.tv_sec = (time_t) (timeMs / 1000);
	ts.tv_nsec = (long) (timeMs % 1000) * 1000000;

	if (pauseTimer && !wasStopped)
		Timer->stop();

	nanosleep(&ts, NULL);

	if (pauseTimer && !wasStopped)
		Timer->start();
}

void CIrrDeviceAndroid::setWindowCaption(const wchar_t* text)
{
	// not supported
}

bool CIrrDeviceAndroid::isWindowActive() const
{
	// TODO need to find out from java side if we're really active
	return true;
}

bool CIrrDeviceAndroid::isWindowFocused() const
{
	return isWindowActive();
}

bool CIrrDeviceAndroid::isWindowMinimized() const
{
	return !isWindowActive();
}

void CIrrDeviceAndroid::closeDevice()
{
	// nothing to do to close device
}

void CIrrDeviceAndroid::setResizable(bool resize)
{
	// not applicable
}

void CIrrDeviceAndroid::minimizeWindow()
{
	// not applicable
}

void CIrrDeviceAndroid::maximizeWindow()
{
	// not applicable
}

void CIrrDeviceAndroid::restoreWindow()
{
	// not applicable
}

E_DEVICE_TYPE CIrrDeviceAndroid::getType() const
{
	return EIDT_ANDROID;
}

bool CIrrDeviceAndroid::present(video::IImage* surface, void* windowId, core::rect<s32>* src)
{
	// Not sure we want to support this
	return true;
}

}
