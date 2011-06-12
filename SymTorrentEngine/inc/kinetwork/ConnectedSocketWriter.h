/*****************************************************************************
 * Copyright (C) 2006-2008 Imre Kelényi
 *-------------------------------------------------------------------
 * This file is part of KiNetwork
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

#ifndef KICONNECTEDSOCKETWRITER_H
#define KICONNECTEDSOCKETWRITER_H

// INCLUDES
#include "WriteBuffer.h"
#include <in_sock.h>

// FORWARD DECLARATIONS
class CKiSocketBase;

const TUint KWriteBufferSize = 16384; // 16 KB

class MKiConnectedSocketWriterObserver
{
public:
	/**
	 * Called when the issued write operation has been completed
	 */
	virtual void OnSocketWriteFinishedL() = 0;
};

/**
 * Active object for handling write operations to a socket
 */
class CKiConnectedSocketWriter : public CActive
{
private:
	
	class TKiSocketWriterObserverEntry
	{
	public:
		TKiSocketWriterObserverEntry(MKiConnectedSocketWriterObserver* aObserver, TInt aBufferIndex) 
		 : iObserver(aObserver), iBufferIndex(aBufferIndex) {}
		
	public:
		MKiConnectedSocketWriterObserver* iObserver;
		TInt iBufferIndex;
	};
public:

	CKiConnectedSocketWriter(	RSocket& aSocket, 
								CKiSocketBase& aSocketBase, 
								TInt aWriteBufferSize = KWriteBufferSize);

	void ConstructL();

	~CKiConnectedSocketWriter();
	
	inline void ResetLongBuffer();
	
	IMPORT_C void WriteL(const TDesC8& aBuf, MKiConnectedSocketWriterObserver* aObserver = NULL);
	
	IMPORT_C void WriteWithoutSendingL(const TDesC8& aBuf, MKiConnectedSocketWriterObserver* aObserver = NULL);
	
	IMPORT_C void SendNow();

protected:

	void IssueWrite();

protected: // from CActive

	void RunL();

	void DoCancel();

private:

    RSocket&					iSocket;

	CKiSocketBase&				iSocketBase;

	CWriteBuffer*				iLongBuffer;
	
	TInt 						iShortBufferSize;

	RBuf8						iShortBuffer;
	
	RArray<TKiSocketWriterObserverEntry> iObservers;

	//friend class CSocketBase;
};

inline void CKiConnectedSocketWriter::ResetLongBuffer() {
	iLongBuffer->Reset();	
}

#endif
