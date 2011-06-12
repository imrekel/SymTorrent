#ifndef __TORRENT_RECOGNIZER_H__
#define __TORRENT_RECOGNIZER_H__

#include <apmrec.h>

const TUid KUidRecognizer = {0x20035B25};

_LIT8(KTorrentType, "application/x-bittorrent");

_LIT(KTorrentExtension, ".torrent");

class CTorrentRecognizer : public CApaDataRecognizerType
{
public:
	CTorrentRecognizer();

	TUint PreferredBufSize();
	TDataType SupportedDataTypeL(TInt aIndex) const;

    static CApaDataRecognizerType* CreateRecognizerL();
    
private:
	void DoRecognizeL(const TDesC &aName,const TDesC8 &aBuffer);
};

#endif
