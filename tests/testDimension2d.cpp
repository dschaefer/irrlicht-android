// Copyright (C) 2008-2009 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"
#include "irrlicht.h"
#include <assert.h>

using namespace irr;
using namespace core;

/** Some very basic testing of dimension2df:
	operator+=
	operator!= (and thus implicitly operator==) */
bool testDimension2d(void)
{
	dimension2df dimension(100.f, 100.f);
	const dimension2df addDimension(200.f, -200.f);

	(void)(dimension += addDimension);

	if(dimension != dimension2df(300.f, -100.f))
	{
		logTestString("dimension2df != produced unexpected result.\n");
		assert(false);
		return false;
	}

	(void)(dimension -= addDimension);
	if(dimension != dimension2df(100.f, 100.f))
	{
		logTestString("dimension2df -= produced unexpected result.\n");
		assert(false);
		return false;
	}

	return true;
}

