/*****************************************************************************
 * Copyright (C) 2006 Imre Kel�nyi
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

#ifndef SYMTORRENT_STGLOBAL
#define SYMTORRENT_STGLOBAL

// Error codes
#define STERROR -9600
const TInt KErrCopyFailed = STERROR - 1;
const TInt KErrParsingFailed = STERROR - 2;
const TInt KErrAlreadyOpened = STERROR - 3;

_LIT(KLitSymTorrent, "SymTorrent");
_LIT(KLitHideButtonText, "Hide");
_LIT(KLitExitButtonText, "Exit");

enum KSymTorrentPanics
{
	ESTPanGetFilePositionFailed = 0
};

#endif
