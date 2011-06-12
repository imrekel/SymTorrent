#include "STUtils.h"

namespace NSTUtils
{

TUint ReadInt32(const TDesC8& aData)
{	
	TUint value = aData[0] << 24;
	value += (aData[1] << 16);
	value += (aData[2] << 8);
	value += aData[3];
	
	return value;	
}

void WriteInt32(TDes8& aDes, TUint aInteger)
{
	aDes.SetLength(4);
	
	aDes[3] = aInteger & 0xFF;
	aDes[2] = ((aInteger & (0xFF << 8)) >> 8);
	aDes[1] = ((aInteger & (0xFF << 16)) >> 16);
	aDes[0] = ((aInteger & (0xFF << 24)) >> 24);
}

} // namespace NSTUtils
