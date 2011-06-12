/*
 ============================================================================
 Name		: MeasurementLog.h
 Author	  : Imre Kelényi
 Version	 : 1.0
 Copyright   : 2008
 Description : CMeasurementLog declaration
 ============================================================================
 */

#ifndef MEASUREMENTLOG_H
#define MEASUREMENTLOG_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>

// CLASS DECLARATION

class CSTTorrent;

/**
 *  CMeasurementLog
 * 
 */
class CMeasurementLog : public CBase
{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CMeasurementLog();

	/**
	 * Two-phased constructor.
	 */
	static CMeasurementLog* NewL();

	/**
	 * Two-phased constructor.
	 */
	static CMeasurementLog* NewLC();
	
	IMPORT_C void SaveFinalLogL(CSTTorrent* aTorrent);
	
	IMPORT_C void SavePreliminaryLogL(CSTTorrent* aTorrent);

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CMeasurementLog();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
	void SaveLogL(CSTTorrent* aTorrent, const TDesC& aFileName);
	
private:
	
/*	RFs iFs;
	RFileWriteStream iStream;*/

};

#endif // MEASUREMENTLOG_H
