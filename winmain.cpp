/*
 * Copyright (c) 2010-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//////////////////////////////////////////////////////////////////////////
//
// spivlcvideoplay sample
//
// This sample demonstrates how to use the vlc library for simple video
// playback.
//
// 2014march26, spi, creation from microsoft sample
// 2014march27, spi, todo, modify to get the IMFPMediaPlayer::GetDuration()
//				           so the spivideoplay terminates upon media duration
// 2016march25, spi, copied spivideoplay's winmain.cpp into spivlcvideoplay's
//					 winmain.cpp and replacing MFPlay API functions and vars
//					 with vlc library functions and var
////////////////////////////////////////////////////////////////////////// 

#define WINVER _WIN32_WINNT_WIN7

#include <new>
#include <windows.h>
#include <windowsx.h>
//#include <mfplay.h>
//#include <mferror.h>
//#include <shobjidl.h>   // defines IFileOpenDialog
#include <strsafe.h>

#include <vlc/vlc.h>

#include "resource.h"

// Include the v6 common controls in the manifest
#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")


BOOL    InitializeWindow(HWND *pHwnd);
HRESULT PlayMediaFile(HWND hwnd, const WCHAR *sURL);
void    ShowErrorMessage(PCWSTR format, HRESULT hr);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Window message handlers
void    OnClose(HWND hwnd);
void    OnPaint(HWND hwnd);
void    OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void    OnSize(HWND hwnd, UINT state, int cx, int cy);
void    OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);

// Menu handlers
void    OnFileOpen(HWND hwnd);

/*
// MFPlay event handler functions.
void OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT *pEvent);
void OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT *pEvent);
*/

libvlc_instance_t* global_libvlc_instance;
libvlc_media_player_t* global_libvlc_mediaplayer;
libvlc_media_t* global_libvlc_media;

#include <string>
using namespace std;

// Constants 
WCHAR CLASS_NAME[1024]  = {L"SPIVLCVIDEOPLAY"};
WCHAR WINDOW_NAME[1024] = {L"spivlcvideoplay"};

char global_filename[1024]={"baraka_african-ritual.avi"};
float global_duration_sec=-1.0; //smaller than 0 for playback until end of file
int global_x=200;
int global_y=200;
int global_xwidth=400;
int global_yheight=400;
int global_alpha=220;
int global_titlebardisplay=1;
int global_menubardisplay=0;
int global_acceleratoractive=0;

HWND global_hwnd=NULL;
MMRESULT global_timer=0;
DWORD global_startstamp_ms;

string global_begin="begin.ahk";
string global_end="end.ahk";

//-------------------------------------------------------------------
//
// MediaPlayerCallback class
// 
// Implements the callback interface for MFPlay events.
//
//-------------------------------------------------------------------

#include <Shlwapi.h>

/*
class MediaPlayerCallback : public IMFPMediaPlayerCallback
{
    long m_cRef; // Reference count

public:

    MediaPlayerCallback() : m_cRef(1)
    {
    }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] = 
        {
            QITABENT(MediaPlayerCallback, IMFPMediaPlayerCallback),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }
    STDMETHODIMP_(ULONG) AddRef() 
    {
            return InterlockedIncrement(&m_cRef); 
    }
    STDMETHODIMP_(ULONG) Release()
    {
        ULONG count = InterlockedDecrement(&m_cRef);
        if (count == 0)
        {
            delete this;
            return 0;
        }
        return count;
    }

    // IMFPMediaPlayerCallback methods
    void STDMETHODCALLTYPE OnMediaPlayerEvent(MFP_EVENT_HEADER *pEventHeader);
};
*/


// Global variables

/*
IMFPMediaPlayer         *g_pPlayer = NULL;      // The MFPlay player object.
MediaPlayerCallback     *g_pPlayerCB = NULL;    // Application callback object.
BOOL                    g_bHasVideo = FALSE;
*/


/////////////////////////////////////////////////////////////////////

//spi, begin

void CALLBACK StartGlobalProcess(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	bool bcalled = false;
	PROPVARIANT myPROPVARIANT = {0};
	float mediaduration_s = -1.0f;

	//PostMessage(global_hwnd, WM_COMMAND, ID_FILE_OPEN, 0);
	DWORD nowstamp_ms = GetTickCount();
	//while( (global_duration_sec<0.0f) || ((nowstamp_ms-global_startstamp_ms)/1000.0f)<global_duration_sec )
	while( ((nowstamp_ms-global_startstamp_ms)/1000.0f)<global_duration_sec || (mediaduration_s<0.0f) || ((global_duration_sec<0.0f)&&(mediaduration_s>0.0f)&&(((nowstamp_ms-global_startstamp_ms)/1000.0f)<mediaduration_s)) )
	{
		nowstamp_ms = GetTickCount();
		//Sleep(250); //media not playing yet
		Sleep(500); //long enough so media is playing
		if(bcalled==false)
		//if(1)
		{
			/*
			if(g_pPlayer)
			{
				bcalled=true;
				g_pPlayer->GetDuration(MFP_POSITIONTYPE_100NS, &myPROPVARIANT);
				//int i=myPROPVARIANT.uhVal.; //cannot /10000000
				mediaduration_s=myPROPVARIANT.uhVal.QuadPart/10000000.0f;
				int ii=1;
			}
			*/
			libvlc_time_t length_ms = libvlc_media_get_duration(global_libvlc_media); //can only get media duration while playing
			mediaduration_s = (float) length_ms/1000.0;
			bcalled=true;
		}
	}
	PostMessage(global_hwnd, WM_COMMAND, ID_FILE_EXIT, 0);	
}

#include <string>
// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

PCHAR*
    CommandLineToArgvA(
        PCHAR CmdLine,
        int* _argc
        )
    {
        PCHAR* argv;
        PCHAR  _argv;
        ULONG   len;
        ULONG   argc;
        CHAR   a;
        ULONG   i, j;

        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;

        len = strlen(CmdLine);
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

        argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
            i + (len+2)*sizeof(CHAR));

        _argv = (PCHAR)(((PUCHAR)argv)+i);

        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;

        while( a = CmdLine[i] ) {
            if(in_QM) {
                if(a == '\"') {
                    in_QM = FALSE;
                } else {
                    _argv[j] = a;
                    j++;
                }
            } else {
                switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = '\0';
        argv[argc] = NULL;

        (*_argc) = argc;
        return argv;
    }

//spi, end


//INT WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,INT)
INT WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPWSTR lpCmdLine,INT nShowCmd)
{
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	//int nShowCmd = false;
	//ShellExecuteA(NULL, "open", "begin.bat", "", NULL, nShowCmd);
	//ShellExecuteA(NULL, "open", "begin.bat", "", NULL, false);

	//spi, begin
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nShowCmd);

	global_startstamp_ms = GetTickCount();

	//LPWSTR *szArgList;
	LPSTR *szArgList;
	int nArgs;
	//szArgList = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}

	LPWSTR *szArgListW;
	int nArgsW;
	szArgListW = CommandLineToArgvW(GetCommandLineW(), &nArgsW);
	if( NULL == szArgListW )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}

	if(nArgs>1)
	{
		strcpy(global_filename, szArgList[1]);
	}
	if(nArgs>2)
	{
		global_duration_sec = atof(szArgList[2]);
	}
	if(nArgs>3)
	{
		global_x = atoi(szArgList[3]);
	}
	if(nArgs>4)
	{
		global_y = atoi(szArgList[4]);
	}
	if(nArgs>5)
	{
		global_xwidth = atoi(szArgList[5]);
	}
	if(nArgs>6)
	{
		global_yheight = atoi(szArgList[6]);
	}
	if(nArgs>7)
	{
		global_alpha = atoi(szArgList[7]);
	}
	if(nArgs>8)
	{
		global_titlebardisplay = atoi(szArgList[8]);
	}
	if(nArgs>9)
	{
		global_menubardisplay = atoi(szArgList[9]);
	}
	if(nArgs>10)
	{
		global_acceleratoractive = atoi(szArgList[10]);
	}

	if(nArgs>11)
	{
		wcscpy(CLASS_NAME, szArgListW[11]); 
	}
	if(nArgs>12)
	{
		wcscpy(WINDOW_NAME, szArgListW[12]); 
	}
	if(nArgs>13)
	{
		global_begin = szArgList[13]; 
	}
	if(nArgs>14)
	{
		global_end = szArgList[14]; 
	}

	LocalFree(szArgList);
	LocalFree(szArgListW);
	//spi, end

	ShellExecuteA(NULL, "open", global_begin.c_str(), "", NULL, false);

    //Load the VLC engine
    global_libvlc_instance = libvlc_new(0, NULL);

	//create new item
    //global_libvlc_media = libvlc_media_new_location(global_libvlc_instance, "http://mycool.movie.com/test.mov");
	if(PathIsURLA(global_filename))
	{
		global_libvlc_media = libvlc_media_new_location(global_libvlc_instance, global_filename); //url
	}
	else
	{
		global_libvlc_media = libvlc_media_new_path(global_libvlc_instance, global_filename); //local filename
	}

    //Create a media player playing environement 
    global_libvlc_mediaplayer = libvlc_media_player_new_from_media(global_libvlc_media);


    HWND hwnd = 0;
    MSG msg = {0};

    if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
    {
        return 0;
    }

    if (!InitializeWindow(&hwnd))
    {
        return 0;
    }

	//set the media player window
	libvlc_media_player_set_hwnd(global_libvlc_mediaplayer, hwnd);
    //play the media_player
    libvlc_media_player_play(global_libvlc_mediaplayer);

    // Message loop
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(hwnd);
    CoUninitialize();

    return 0;
}


//-------------------------------------------------------------------
// WindowProc
//
// Main window procedure.
//-------------------------------------------------------------------

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        //HANDLE_MSG(hwnd, WM_CLOSE,   OnClose);
        HANDLE_MSG(hwnd, WM_DESTROY,   OnClose); //spi
        HANDLE_MSG(hwnd, WM_KEYDOWN, OnKeyDown);
        HANDLE_MSG(hwnd, WM_PAINT,   OnPaint);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_SIZE,    OnSize);

    case WM_ERASEBKGND:
        return 1;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}


//-------------------------------------------------------------------
// InitializeWindow
//
// Creates the main application window.
//-------------------------------------------------------------------

BOOL InitializeWindow(HWND *pHwnd)
{
    WNDCLASS wc = {0};

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon		 = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAIN_ICON)); //spi, added
    wc.lpszClassName = CLASS_NAME;
	if(global_menubardisplay)
	{
		//wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
		wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU2); //menu bar
		//wc.lpszMenuName  = NULL;
	}
	else
	{
		wc.lpszMenuName = NULL; //no menu
	}

    if (!RegisterClass(&wc))
    {
        return FALSE;
    }

    HWND hwnd;
	if(global_titlebardisplay)
	{
		hwnd = CreateWindow( CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW,
			//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			global_x, global_y, global_xwidth, global_yheight,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL
			);
	}
	else
	{
		//hwnd = CreateWindow( CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW,
		hwnd = CreateWindow( CLASS_NAME, WINDOW_NAME, WS_POPUP | WS_VISIBLE, //no WS_CAPTION etc.
			//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			global_x, global_y, global_xwidth, global_yheight,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			NULL
			);
	}

    if (!hwnd)
    {
        return FALSE;
    }

	global_hwnd = hwnd;
	SetWindowLong(global_hwnd, GWL_EXSTYLE, GetWindowLong(global_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(global_hwnd, 0, global_alpha, LWA_ALPHA);

	ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    *pHwnd = hwnd;
	//global_timer=timeSetEvent(500,25,(LPTIMECALLBACK)&StartGlobalProcess,0,TIME_ONESHOT);
	global_timer=timeSetEvent(100,25,(LPTIMECALLBACK)&StartGlobalProcess,0,TIME_ONESHOT);
    return TRUE;
}

//-------------------------------------------------------------------
// OnClose
//
// Handles the WM_CLOSE message.
//-------------------------------------------------------------------

void OnClose(HWND /*hwnd*/)
{
	/*
    if (g_pPlayer)
    {
        g_pPlayer->Shutdown();
        g_pPlayer->Release();
        g_pPlayer = NULL;
    }

    if (g_pPlayerCB)
    {
        g_pPlayerCB->Release();
        g_pPlayerCB = NULL;
    }
	*/
	if(global_timer) timeKillEvent(global_timer);
	//Sleep(1000);
    //Stop playing 
    if(global_libvlc_mediaplayer) libvlc_media_player_stop(global_libvlc_mediaplayer);
	//Sleep(1000); //spi test
    //Free the media_player
    if(global_libvlc_mediaplayer) libvlc_media_player_release(global_libvlc_mediaplayer);
	if(global_libvlc_media) libvlc_media_release(global_libvlc_media);
    if(global_libvlc_instance) libvlc_release(global_libvlc_instance);


	int nShowCmd = false;
	//ShellExecuteA(NULL, "open", "end.bat", "", NULL, nShowCmd);
	ShellExecuteA(NULL, "open", global_end.c_str(), "", NULL, nShowCmd);
    PostQuitMessage(0);
}


//-------------------------------------------------------------------
// OnPaint
//
// Handles the WM_PAINT message.
//-------------------------------------------------------------------

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = 0;

    hdc = BeginPaint(hwnd, &ps);
    
    //if (g_pPlayer && g_bHasVideo)
	if(global_libvlc_mediaplayer && libvlc_media_player_is_playing(global_libvlc_mediaplayer))
    {
		/*
        // Playback has started and there is video. 

        // Do not draw the window background, because the video 
        // frame fills the entire client area.

        g_pPlayer->UpdateVideo();
		*/
    }
    else
    {
        // There is no video stream, or playback has not started.
        // Paint the entire client area.

        //FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
        FillRect(hdc, &ps.rcPaint, (HBRUSH) GetStockObject(BLACK_BRUSH));
    }

    EndPaint(hwnd, &ps);
}


//-------------------------------------------------------------------
// OnSize
//
// Handles the WM_SIZE message.
//-------------------------------------------------------------------

void OnSize(HWND /*hwnd*/, UINT state, int /*cx*/, int /*cy*/)
{
    if (state == SIZE_RESTORED)
    {
		/*
        if (g_pPlayer)
        {
            // Resize the video.
            g_pPlayer->UpdateVideo();
        }
		*/
    }
}


//-------------------------------------------------------------------
// OnKeyDown
//
// Handles the WM_KEYDOWN message.
//-------------------------------------------------------------------

void OnKeyDown(HWND /*hwnd*/, UINT vk, BOOL /*fDown*/, int /*cRepeat*/, UINT /*flags*/)
{
    HRESULT hr = S_OK;

    switch (vk)
    {
    case VK_SPACE:

		/*
        // Toggle between playback and paused/stopped.
        if (g_pPlayer)
        {
            MFP_MEDIAPLAYER_STATE state = MFP_MEDIAPLAYER_STATE_EMPTY;
            
            hr = g_pPlayer->GetState(&state);

            if (SUCCEEDED(hr))
            {
                if (state == MFP_MEDIAPLAYER_STATE_PAUSED || state == MFP_MEDIAPLAYER_STATE_STOPPED)
                {
                    hr = g_pPlayer->Play();
                }
                else if (state == MFP_MEDIAPLAYER_STATE_PLAYING)
                {
                    hr = g_pPlayer->Pause();
                }
            }
        }
		*/
        break;
    }

    if (FAILED(hr))
    {
        ShowErrorMessage(TEXT("Playback Error"), hr);
    }
}


//-------------------------------------------------------------------
// OnCommand
// 
// Handles the WM_COMMAND message.
//-------------------------------------------------------------------

void OnCommand(HWND hwnd, int id, HWND /*hwndCtl*/, UINT /*codeNotify*/)
{
    switch (id)
    {
        case ID_FILE_OPEN:
            OnFileOpen(hwnd);
            break;

        case ID_FILE_EXIT:
            OnClose(hwnd);
            break;
    }
}


//-------------------------------------------------------------------
// OnFileOpen
//
// Handles the "File Open" command.
//-------------------------------------------------------------------
/*
void OnFileOpen(HWND hwnd)
{    
    HRESULT hr = S_OK;

    IFileOpenDialog *pFileOpen = NULL;
    IShellItem *pItem = NULL;

    PWSTR pwszFilePath = NULL;

    // Create the FileOpenDialog object.
    hr = CoCreateInstance(
        __uuidof(FileOpenDialog), 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        IID_PPV_ARGS(&pFileOpen)
        );

    if (FAILED(hr)) { goto done; }


    hr = pFileOpen->SetTitle(L"Select a File to Play");

    if (FAILED(hr)) { goto done; }


    // Show the file-open dialog.
    hr = pFileOpen->Show(hwnd);

    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        // User cancelled.
        hr = S_OK;
        goto done;
    }
    if (FAILED(hr)) { goto done; }


    // Get the file name from the dialog.
    hr = pFileOpen->GetResult(&pItem);

    if (FAILED(hr)) { goto done; }


    hr = pItem->GetDisplayName(SIGDN_URL, &pwszFilePath);

    if (FAILED(hr)) { goto done; }


    // Open the media file.
    hr = PlayMediaFile(hwnd, pwszFilePath);

    if (FAILED(hr)) { goto done; }

done:
    if (FAILED(hr))
    {
        ShowErrorMessage(L"Could not open file.", hr);
    }

    CoTaskMemFree(pwszFilePath);

    if (pItem)
    {
        pItem->Release();
    }
    if (pFileOpen)
    {
        pFileOpen->Release();
    }
}
*/


void OnFileOpen(HWND hwnd)
{    
    HRESULT hr = S_OK;

    // Open the media file.
	std::string mystring = global_filename;
	std::wstring mywstring = utf8_decode(mystring);
    /*
	hr = PlayMediaFile(hwnd, mywstring.c_str());
	*/

    if (FAILED(hr)) { goto done; }

done:
    if (FAILED(hr))
    {
        ShowErrorMessage(L"Could not open file.", hr);
    }
}


/*
//-------------------------------------------------------------------
// PlayMediaFile
//
// Plays a media file, using the IMFPMediaPlayer interface.
//-------------------------------------------------------------------

HRESULT PlayMediaFile(HWND hwnd, const WCHAR *sURL)
{
    HRESULT hr = S_OK;

    // Create the MFPlayer object.
    if (g_pPlayer == NULL)
    {
        g_pPlayerCB = new (std::nothrow) MediaPlayerCallback();

        if (g_pPlayerCB == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        hr = MFPCreateMediaPlayer(
            NULL,
            FALSE,          // Start playback automatically?
            0,              // Flags
            g_pPlayerCB,    // Callback pointer
            hwnd,           // Video window
            &g_pPlayer
            );

        if (FAILED(hr)) { goto done; }
    }

    // Create a new media item for this URL.
    hr = g_pPlayer->CreateMediaItemFromURL(sURL, FALSE, 0, NULL);

    // The CreateMediaItemFromURL method completes asynchronously. 
    // The application will receive an MFP_EVENT_TYPE_MEDIAITEM_CREATED 
    // event. See MediaPlayerCallback::OnMediaPlayerEvent().


done:
    return hr;
}
*/

/*
//-------------------------------------------------------------------
// OnMediaPlayerEvent
// 
// Implements IMFPMediaPlayerCallback::OnMediaPlayerEvent.
// This callback method handles events from the MFPlay object.
//-------------------------------------------------------------------

void MediaPlayerCallback::OnMediaPlayerEvent(MFP_EVENT_HEADER * pEventHeader)
{
    if (FAILED(pEventHeader->hrEvent))
    {
        ShowErrorMessage(L"Playback error", pEventHeader->hrEvent);
        return;
    }

    switch (pEventHeader->eEventType)
    {
    case MFP_EVENT_TYPE_MEDIAITEM_CREATED:
        OnMediaItemCreated(MFP_GET_MEDIAITEM_CREATED_EVENT(pEventHeader));
        break;

    case MFP_EVENT_TYPE_MEDIAITEM_SET:
        OnMediaItemSet(MFP_GET_MEDIAITEM_SET_EVENT(pEventHeader));
        break;
    }
}
*/

/*
//-------------------------------------------------------------------
// OnMediaItemCreated
//
// Called when the IMFPMediaPlayer::CreateMediaItemFromURL method
// completes.
//-------------------------------------------------------------------

void OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT *pEvent)
{
    HRESULT hr = S_OK;

    // The media item was created successfully.

    if (g_pPlayer)
    {
        BOOL bHasVideo = FALSE, bIsSelected = FALSE;

        // Check if the media item contains video.
        hr = pEvent->pMediaItem->HasVideo(&bHasVideo, &bIsSelected);

        if (FAILED(hr)) { goto done; }

        g_bHasVideo = bHasVideo && bIsSelected;

        // Set the media item on the player. This method completes asynchronously.
        hr = g_pPlayer->SetMediaItem(pEvent->pMediaItem);
    }

done:
    if (FAILED(hr))
    {
        ShowErrorMessage(L"Error playing this file.", hr);
    }
}
*/

/*
//-------------------------------------------------------------------
// OnMediaItemSet
//
// Called when the IMFPMediaPlayer::SetMediaItem method completes.
//-------------------------------------------------------------------

void OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT * ) 
{
    HRESULT hr = S_OK;

    hr = g_pPlayer->Play();

    if (FAILED(hr))
    {
        ShowErrorMessage(L"IMFPMediaPlayer::Play failed.", hr);
    }
}
*/

void ShowErrorMessage(PCWSTR format, HRESULT hrErr)
{
    HRESULT hr = S_OK;
    WCHAR msg[MAX_PATH];

    hr = StringCbPrintf(msg, sizeof(msg), L"%s (hr=0x%X)", format, hrErr);

    if (SUCCEEDED(hr))
    {
        MessageBox(NULL, msg, L"Error", MB_ICONERROR);
    }
}


