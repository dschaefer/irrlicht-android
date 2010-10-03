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

}
