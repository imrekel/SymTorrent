# SymTorrent 1.51 release notes

<https://github.com/kenley/SymTorrent>

<http://symtorrent.aut.bme.hu>

## General info

SymTorrent is a BitTorrent client for Symbian based smartphones built on the S60 platform. You can use SymTorrent to download torrents right to your mobile phone. Torrents can be started from the browser by clicking on torrent links. If the torrent contains multiple files, you can select which files you want to download. 

SymTorrent has not been actively developed since 2011. Currently its main shortcoming is the lack of DHT support (trackerless torrents) and UDP trackers. Since the DHT support was partially implemented, there is slight chance that it will be added in the future.

This version supports Symbian^3 or later OSs (Symbian Anna, Belle, etc.)

## Building SymTorrent

SymTorrent is written in Symbian C++. To build it, the following tools are needed:

- Symbian^3 SDK 1.0 (theoretically it can be built using S60 5th SDK or any newer SDK, but I have tested this version with Symbian^3 1.0 only)
- Carbide C++ 3.2 (or 3.3)

After installing carbide, you might have to manually add the SDK to it. To do so, go to Windows/Preferences/Carbide C++/Symbian SDKs, click "Add" and enter "Nokia_Symbian3_SDK_v1.0:com.nokia.symbian" as the SDK ID and "C:\Nokia\devices\Nokia_Symbian3_SDK_v1.0\" (or where you have installed the SDK) as the SDK location.

SymTorrent consists of 4 projects, which you have to import to Carbide and build in a specific order. You should select the repository's root folder as the workspace when Carbide launches. To import a project use File/Import/Symbian OS/Symbian OS Bld.inf file, pointing to the group/bld.inf file in each project's folder. The projects must be built in the following order:

1. **KiLogger**: Project for file based debug logging. Logging is disabled by default.

2. **SymTorrentEngine**: A separate DLL which is responsible for all the UI independent client functions, such as connecting               the tracker, communicating with the peers, etc. If you are thinking on creating your own BitTorrent-based                      application then you will need this project. It also includes the communication/network related code.

3. **SymTorrentLaucher**: Separate dummy application for launching SymTorrent when a recognizer finds an associated file (.torrent)

4. **SymTorrent**: The UI and the other "high level" components of the application excluding the BitTorrent engine

During compilation, you may get an `instantiated from here` error from gcc for one of the SDK's files (s32stor.inl). This is actually only a warning falsely treated as an error by Carbide. After rebuilding the project, the error should disappear, so you can ignore it. More info about this [here](http://www.developer.nokia.com/Community/Discussion/showthread.php?237748-GCC-4-4-1-Carbide-2-7-quot-instantiated-from-here-quot-compilation-error-FIX).

You may also get a couple of `extra qualification` errors on a set of SDK files. This is a bug in the SDK, and can be easily fixed by editing each affected SDK header file and removing the extra qualifiers (e.g. CAknCcpuSupport::) from the method declarations.

To create an installable SIS file, go to Project/Properties/Carbide C++/Build Configurations/SIS Builder, click Add and select SymTorrent.pkg.

You can test the running application by going to <http://www.clearbits.net/music> with the phone's browser, clicking on a torrent then select the "Torrent file" link. After the file has been downloaded, tap it in the download queue so that SymTorrent can launch and start downloading it.

## Known issues

- SymTorrent rarely crashes during download (the application exits and needs to be restarted to continue downloading). 
- Poor performance with large (larger than 2 GByte) torrents.
- Some downloaded files are not accessible while other parts of the associated torrent are still being downloaded.
- Lack of DHT and UDP tracker support,

## Version history

### 1.51  -- 2012.10.23.
- Updated project to Symbian^3 SDK 1.0
- Fixed the download folder selection dialog so that now all drives are displayed

### 1.50  -- 2011.03.11.
- Changed UIDs back to unprotected in order to reenable self signing.
	
### 1.50  -- 2010.12.14. 
#### General:
- First OVI Store release, had to completely restructure projects, change UIDs, etc.
- Fixed startup crash caused by missing/deleted saved torrent file
- Files larger than 4GB are skipped by default (FAT32 limitation) and a warning is displayed to the user when the torrent is loaded.
	
#### BitTorrent engine:
- If a peer sends a particular piece incorrectly (hash check fails) two times, that piece is not downloaded again from the peer.
- Torrent tries to connect to all trackers simultaneously not to only one
- Maximum number of peers allowed per torrent is raised to 200. Maximum number of parallel TCP connections per torrent is raised to 25.
- 50 most recent peers are saved when exiting
- BUG FIXED: USER 130 panic if connecting to network fails while some sockets are open
- BUG FIXED: USER 11 panic at random intervals while downloading (caused by an undersized TBuf used for logging some debug info)
						
#### UI changes:
- Fixed piece bar width when screen orientation is changed (bar fits screen width)
- Fixed piece bar pieces colors
- Fixed torrent properties view size and background when layout is changed
- Enabled touch scrolling for torrent properties view
- Enabled marquee effect for torrent and files lists
- Added touch friendly tab bars
- Changed "Checking hash" progress dialog text to "Looking for already downloaded files" and renamed "Cancel" softkey to "Skip"
- If no torrents has been added, tapping the screen adds a torrent
	
### 1.42 -- 2009.07.17. 
- SVN version control integration
- 2008.12.13. - When a torrent is removed, the user is asked if they want to have incomplete files of the torrent deleted
    
### 1.41 -- 2008.11.26. 
- BUG[SymTorrent crashes after entering endgame] fixed

### 1.40 -- 2008.05.01.
- BUG[Green upload icon is displayed correctly (only when there is an active upload in progress)

### 2008.05.15. -- 2008.11.24. 
- Peers from tracker response are processed correctly (compact response bug)
- Option to automatically close network connection after downloads has been added
- Text colors fixed (obtained from theme)
- Changed line spacing in status and details views
- Changed the colors in the download status bar: yellow means partially download, green is fully downloaded
- BUG[Download path setting dialog pops up two times] FIXED
- BUG[Network connection selection dialog popped up multiple times when cancelled] FIXED
- Error popups are displayed when starting the network connection fails or the network is disconnected
- Torrents no longer become "failed" when the user cancels the network selection dialog
- BUG[Application crashes with KERN-EXEC 0 during download] POSSIBLY FIXED (?)
- BUG[Receiving HAVE message with piece index 0 causes that the peer disconnects] FIXED
- Piece queuing added/updated
- Selecting which files of a torrent to download is now possible (in the files view select Download selection)
- BUG[The download status view is not refreshed after rotating the screen] FIXED
- S60 5th UI compatibility problems solved
- Text size on status and torrent properties view changed            
- BUG[IP address is not displayed in status view] FIXED
- Added warning popups when writing to disk fails or no peers left
 - BUG[Some of the downloaded data is lost after exiting] FIXED (partially downloaded pieces are saved)

### 1.32

- A memory leak in the tracker has been fixed
- Some misspellings has been corrected
- The networking and logging has been moved to separate libraries (KiNetwork is responsible for handling network connections, sockets, etc.
- KiLogger is a general logging library which enables logging into multiple files with timestamps).

### 1.30 
- End game algorithm implemented, downloading shouldn't slow down at the end. When a torrent enters end game, an "EG" label will appear next to the download percentage in the torrent's status view.
- Network address of the devices is displayed correctly in the Status panel.
- In the torrent status view, the total number of peers is displayed (in brackets) along with the number of connected peers for each torrent.
- Backup & restore functionality for S60 3rd edition is now working for SymTorrent.
- The settings menu has been divided into a couple of sub-parts (separate page for general, proxy, and tracker settings).
- Piece hashes are checked when the application loads (can be enabled / disabled in the settings). 
- Piece hashes are checked when a new torrent is opened. If matching files are found in the target directory then those parts of torrent appears as downloaded.
- Settings and download status are automatically saved in every 60 seconds (in previous versions the interval was 2 minutes).
- "SymTorrent disconnects good peers after handshake" MAJOR BUG fixed (apparently it caused that SymTorrent disconnected from around the third of the peers right after handshake)
- "Application won't start after changing the incoming port" bug fixed
- "Application crashes in the Torrent Properties view if the 'Comment' or 'Created by' fields in a torrent are longer than 200 characters" bug fixed
- "Loading torrents with empty files (0 bytes) crashes" bug fixed
- "Wrong IP address is sent to the tracker if the device is behind a router (NAT)" bug fixed (IP address is now sent only if proxy connection is used)
- "SymTorrent cannot open sound files in the Files View" bug fixed (the file opening mechanism is completely replaced, files are now opened embedded in SymTorrent)         
- Tracker is moved to a separate DLL. From now on, SymTorrent will be released in two different packages: one with and one without the built-in tracker (it saves memory if you use the version which doesn't have the tracker)

### 1.26 
- Major bugfix ("downloads slow down at the end" problem fixed), status of tracker is displayed in the tracker view. On the protocol level, piece requests are sent in bursts (it may also speed up transfer a bit).

### 1.21 
- Added multi-tracker support (SymTorrent now parses "announce-list" entries from the torrent files)
- Added proxy support (you can run a proxy on a separate desktop computer to provide public IP address for your mobile device)
- Added tracker (a basic tracker service is available inside SymTorrent)
- Some fixes in the UI (hopefully, the flickering in the download state view is gone)
- Pieces are downloaded in random order
- For developers: SymTorrent is divided into several separate projects, e.g. the engine is available in a separate DLL.

### 1.10 
- Client can accept incoming connections.
- Some bugs have been fixed.
- The state of torrents is saved in every 2 minutes. 
- Both S60 3rd and 2nd edition phones are supported.

### 1.0  
-  Initial release

   
## Credits

### Active developers:
   
- Imre Kelényi (imre.kelenyi@gmail.com)
	
### Former developers (initial version):
   
- Péter Ekler (parts of the SymTorrent UI, tracker and proxy)
			
- Zsolt Pszota (parts of SymTorrent UI, tracker and proxy)	
Special thanks Dorottya Takács for the SymTorrent logo!