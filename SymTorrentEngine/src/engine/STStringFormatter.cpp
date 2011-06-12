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

#include "STStringFormatter.h"

/*EXPORT_C void TSTStringFormatter::AppendFileLength(TInt aLength, TDes &aLengthStr) 
{ 
        AppendFileLength64(TInt64(aLength), aLengthStr); 
} */

EXPORT_C void TSTStringFormatter::AppendFileLength(TInt64 aLength, TDes &aLengthStr) 
{ 
        if (aLength<0) return; 

        TInt range; 
        for (range=0;range<3;++range) 
        { 
                if (aLength<1000*1000) break; // keep some digits from fraction 
                aLength/=1000; 
        } 

#ifdef EKA2 
        TReal len=aLength; 
#else 
        TReal len=aLength.GetTInt(); 
#endif 

        if (len>1000) { len/=1000; ++range; } 

        if (range==0) 
                aLengthStr.AppendFormat(_L("%.0f"),len); 
        else 
                aLengthStr.AppendFormat(_L("%.1f"),len); 

        switch (range) 
        { 
                case 0: aLengthStr.Append(_L(" B")); break; 
                case 1: aLengthStr.Append(_L(" kB")); break; 
                case 2: aLengthStr.Append(_L(" MB")); break; 
                case 3: aLengthStr.Append(_L(" GB")); break; 
                case 4: aLengthStr.Append(_L(" TB")); break; 
                default: __ASSERT_DEBUG(false,User::Panic(_L("#TStringFormatter"),0)); 
        } 
} 

