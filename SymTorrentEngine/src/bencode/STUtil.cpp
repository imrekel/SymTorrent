/*****************************************************************************
 * Copyright (C) 2006 Imre Kelényi
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

/* ============================================================================
 *  Name     : NSTUtil namespace members
 *  Part of  : SymTorrent Engine
 *  Created  : 13.02.2006 by Imre Kelényi
 * ============================================================================
 */

// INCLUDE FILES
#include "STUtil.h"

namespace NSTUtils
{

// ================= MEMBER FUNCTIONS =======================
	
TInt NumberOfDigits(TInt64 aNumber)
{
	if (aNumber == 0) 
		return 1;
	
	TReal result;
	TReal number(Abs(aNumber));
	User::LeaveIfError(Math::Log(result, number));
			
	return	TInt(result) + 1 + ((aNumber < 0) ? 1 : 0);
}

} // NSTUtil
