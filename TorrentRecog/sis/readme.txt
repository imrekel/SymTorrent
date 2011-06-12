======================================
=             TorrentRecog           =
======================================
Document version:	1.00, 4th August 2008

SECTION 1 - RELEASE NOTES
=========================

Application version: TorrentRecog 1.0

Phone(s) concerned:	All
This a recognizer that is used to identify "torrent" files, which are used by the BitTorrent protocol.
Files that has the extension ".torrent" are recognized as MIME type "application/x-bittorrent".
The CApaDataRecognizerType class is used to implement the recognizer.

SECTION 2 - USER GUIDE
======================

The application does not have a user interface. Since it is a recognizer, it is being called by the
framework when a file is needed to be recognized.