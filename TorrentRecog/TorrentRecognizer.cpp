#include <ImplementationProxy.h>

#include <TorrentRecognizer.h>
#include <f32file.h>

CTorrentRecognizer::CTorrentRecognizer() :
	CApaDataRecognizerType(KUidRecognizer, EHigh)
{
	iCountDataTypes = 1;
}

TUint CTorrentRecognizer::PreferredBufSize()
{
	return 10;
}

TDataType CTorrentRecognizer::SupportedDataTypeL(TInt aIndex) const
{
	switch (aIndex)
	{
		case 0: 
		default: 
			return TDataType(KTorrentType);
	}
}

void CTorrentRecognizer::DoRecognizeL(const TDesC &aName, const TDesC8 &/*aBuffer*/)
{
	iConfidence = ECertain;
	TParse parse;
    parse.Set(aName, NULL, NULL);
    TPtrC ext = parse.Ext();
    if (ext.CompareF(KTorrentExtension) == 0)
    {
        iConfidence = ECertain;
        iDataType = TDataType(KTorrentType);
    }
    else
    {
    	iConfidence = ENotRecognized;
    	iDataType = TDataType();
    }

	/*TFileName name=aName;
	name.LowerCase();

	if (name.Right(KTorrentExtension().Length()) == KTorrentExtension)
	{ 		
		iDataType = TDataType(KTorrentType);
		
		return; 
	}*/
}

const TInt KTorrentRecognizerImplementationUid = 0x20035B25;

const TImplementationProxy ImplementationTable[] =
{
    IMPLEMENTATION_PROXY_ENTRY(KTorrentRecognizerImplementationUid, CTorrentRecognizer::CreateRecognizerL)
};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
{
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
}

CApaDataRecognizerType* CTorrentRecognizer::CreateRecognizerL()
{
	return new (ELeave) CTorrentRecognizer;
}
