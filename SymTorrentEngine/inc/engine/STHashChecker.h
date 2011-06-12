/*****************************************************************************
 * Copyright (C) 2006-2007 Imre Kelényi
 *-------------------------------------------------------------------
 * This file is part of SymTorrent
 *
 * SymTorrent is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SymTorrent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Symella; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#ifndef ST_HASH_CHECKER_H_
#define ST_HASH_CHECKER_H_

// INCLUDES
#include <e32base.h>
#include <aknprogressdialog.h> 

class CSTTorrent;
class CAknProgressDialog;
class CSTHashChecker;
class CSTBitField;

class MSTHashCheckerObserver
{
public:

	virtual void HashCheckFinishedL(CSTHashChecker* aHashChecker) = 0;
};

class CSTHashChecker: public CActive, public MProgressDialogCallback
{
public:

	CSTHashChecker(CSTTorrent* aTorrent, CAknProgressDialog* aProgressDialog, MSTHashCheckerObserver* aHashCheckerObserver);

//	void ConstructL();

	~CSTHashChecker();
	
	/**
	 * Starts reading the files and checking the hash values
	 */
	void StartL();

protected: // from CActive

	void RunL();

	void DoCancel();
	
private: // from MProgressDialogCallback

	void DialogDismissedL(TInt  aButtonId);

private:

	CSTTorrent* iTorrent;
	
	CAknProgressDialog* iProgressDialog;
	
	TInt iCurrentPieceIndex;
	
	TInt iPieceCount;
	
	MSTHashCheckerObserver* iHashCheckerObserver;
	
	CSTBitField* iOriginalBitfield;
};

#endif
