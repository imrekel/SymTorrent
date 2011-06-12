/*
 * STUtils.h
 *
 *  Created on: 2009.01.02.
 *  Author: Imre Kelényi
 */

#ifndef STUTILS_H_
#define STUTILS_H_

#include <e32base.h>

namespace NSTUtils
{
	TUint ReadInt32(const TDesC8& aData);
	
	void WriteInt32(TDes8& aDes, TUint aInteger);
}

#endif /* STUTILS_H_ */
