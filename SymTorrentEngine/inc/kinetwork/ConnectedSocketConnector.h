/*****************************************************************************
 * Copyright (C) 2006-2007 Imre Kelényi
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

#ifndef TCPSOCKETCONNECTOR_H
#define TCPSOCKETCONNECTOR_H

#include <e32base.h>
#include <in_sock.h>

class CKiConnectedSocket;


class CKiConnectedSocketConnector : public CActive
{
public:

	CKiConnectedSocketConnector(CKiConnectedSocket& aSocket);
	
	void ConstructL();

	~CKiConnectedSocketConnector();

	void ConnectL(const TSockAddr& aAddress);

protected: // from CActive

	void RunL();

	void DoCancel();

private:

	TSockAddr 		iAddress;

	CKiConnectedSocket&	iSocket;
};

#endif
