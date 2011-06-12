/*
============================================================================
 Name        : SymTorrentLaunch.cpp
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Application entry point
============================================================================
*/

// INCLUDE FILES
#include <eikstart.h>
#include "SymTorrentLaunchApplication.h"


LOCAL_C CApaApplication* NewApplication()
	{
	return new CSymTorrentLaunchApplication;
	}

GLDEF_C TInt E32Main()
	{
	return EikStart::RunApplication( NewApplication );
	}
