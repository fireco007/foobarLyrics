#include "stdafx.h"
#include "LyricsWindow.h"
#include "DropSourceImpl.h"
#include "utils.h"
#include "config.h"

HWND CLyricsWindow::m_wnd;
CLyricsWindow CLyricsWindow::g_instance;
string CLyricsWindow::m_strLrc;

//The directory for save lyrics files;
#define LYRICS_PATH ".\\lyrics\\"

//the height of Lyrics display window.
#define LRCWND_HEIGHT 150 

//add by excalibur
GLvoid CLyricsWindow::drawScene() 
{ 
    //清除屏幕和深度缓存
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glLoadIdentity();

    //移动坐标系
    //glTranslatef(/*-1.5f*/-22.5f, 0.0f, -6.0f);
    glTranslatef(-0.0f, 0.0f, -6.0f);

    //todo : draw some effect

    m_lrcFont.Show2DGbkText((char*)m_strLrc.c_str());

    SwapBuffers(m_hDC);
}


GLvoid CLyricsWindow::resizeWindow(GLsizei width, GLsizei height) 
{ 
    GLfloat aspect; 

    glViewport( 0, 0, width, height ); 

    aspect = (GLfloat) width / height; 

    glMatrixMode( GL_PROJECTION ); 
    glLoadIdentity(); 
    gluPerspective( 45.0, aspect, 3.0, 7.0 ); 
    glMatrixMode( GL_MODELVIEW ); 
} 

GLvoid CLyricsWindow::initializeGL(GLsizei width, GLsizei height) 
{ 
    GLfloat aspect;
    //GLfloat     maxObjectSize, aspect; 
    //GLdouble    near_plane, far_plane; 

    //glClearIndex( (GLfloat)BLACK_INDEX); 
    glClearDepth( 1.0 ); 

    glEnable(GL_DEPTH_TEST); 

    glMatrixMode( GL_PROJECTION ); 
    aspect = (GLfloat) width / height; 
    gluPerspective( 45.0, aspect, 3.0, 7.0 ); 
    glMatrixMode( GL_MODELVIEW ); 

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    //m_lrcFont.MyCreateFont("微软雅黑", 100, 80, 0, 0, true);
    //m_lrcFont.MyCreateFont("汉堡包手机字体", 100, 80, 0, 0, true);
    //g_font.CreateFont("叶根友爵宋体", 200, 80, 0, 0, 0);
} 

BOOL CLyricsWindow::bSetupPixelFormat(HDC hdc) 
{ 
    PIXELFORMATDESCRIPTOR pfd, *ppfd; 
    int pixelformat; 

    ppfd = &pfd; 

    ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR); 
    ppfd->nVersion = 1; 
    ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |  PFD_DOUBLEBUFFER; 
    //ppfd->dwLayerMask = PFD_MAIN_PLANE; 
    ppfd->iPixelType = PFD_TYPE_RGBA; 
    ppfd->cColorBits = 32; 
    ppfd->cDepthBits = 32; 
    ppfd->cAccumBits = 0; 
    ppfd->cStencilBits = 8; 
    ppfd->iLayerType = PFD_MAIN_PLANE;

    if ( (pixelformat = ChoosePixelFormat(hdc, ppfd)) == 0 ) 
    { 
        //MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK); 
        return FALSE; 
    } 

    if (SetPixelFormat(hdc, pixelformat, ppfd) == FALSE) 
    { 
        //MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK); 
        return FALSE; 
    } 

    return TRUE; 
} 



void CLyricsWindow::ShowWindow() {
	if (!g_instance.IsWindow()) {
		cfg_enabled = (g_instance.Create(core_api::get_main_window()) != NULL);
	}
}

void CLyricsWindow::HideWindow() {
	// Set window state to disabled.
	cfg_enabled = false;

	// Destroy the window.
	g_instance.Destroy();
}

HWND CLyricsWindow::Create(HWND p_hWndParent) {
    
     //the window's width and height will be changed later, so the value can be 0;
	m_wnd = super::Create(core_api::get_main_window(),
		TEXT(APP_TITLE),
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP | WS_VISIBLE /*| WS_CAPTION | WS_SYSMENU*/,
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST,//set lyrics window always on top
		CW_USEDEFAULT, CW_USEDEFAULT, 0, LRCWND_HEIGHT);

    //set window transparent(windows Vista and above)
    DWM_BLURBEHIND bb = {0};

    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.fEnable = true;
    //bb.hRgnBlur = NULL;
    bb.hRgnBlur = CreateRectRgn(0, 0, -1, -1);

    //see Issue #8
    HRESULT hr = S_OK;
    hr = DwmEnableBlurBehindWindow(m_wnd, &bb);
    if (FAILED(hr)) {
        console::error("foo_gl_lyris : set window transparent failed! please make sure you've enabled Aero.\n");
    }

    return m_wnd;
}

void CLyricsWindow::displayLrcCallback(const string &strLrc)
{
    m_strLrc = strLrc;
    
    InvalidateRect(m_wnd, NULL, TRUE);
}

BOOL CLyricsWindow::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT & lResult) {
	switch (uMsg) {
	case WM_CREATE:
		{
			lResult = OnCreate(reinterpret_cast<LPCREATESTRUCT>(lParam));

            //add by excalibur
            //init OpenGL context with HDC
            m_hDC = GetDC(hWnd); 
            int charSet = GetTextCharset(m_hDC);
            if (!bSetupPixelFormat(m_hDC)) 
                PostQuitMessage (0); 

            m_hRC = wglCreateContext(m_hDC); 
            wglMakeCurrent(m_hDC, m_hRC); 

            int width = GetSystemMetrics(SM_CXSCREEN);
            int height = GetSystemMetrics(SM_CYSCREEN);

            //modify the window size and position
            MoveWindow(hWnd, 0, height - LRCWND_HEIGHT, width, LRCWND_HEIGHT, TRUE);

            initializeGL(width, LRCWND_HEIGHT);

            string strDir = LYRICS_PATH;
            createLRCDir(LYRICS_PATH);
            m_lrcPlayer.setLrcDirectory(strDir);
            m_lrcPlayer.setLrcCB(displayLrcCallback);
            //m_lrcPlayer.startDisplayLrc();
			return TRUE;
		}

        //add by excalibur
    case WM_SIZE:
        {
            RECT rect;
            GetClientRect(hWnd, &rect); 
            resizeWindow(rect.right, rect.bottom); 
        }
        break;

	case WM_DESTROY:
		{
			OnDestroy();
			lResult = 0;

            //add by excalibur
            if (m_hRC) {
                wglDeleteContext(m_hRC);
            }
            
			return TRUE;
		}

	case WM_CLOSE:
		{
			OnClose();
			lResult = 0;

            //add by excalibur
            if (m_hRC) {
                wglDeleteContext(m_hRC);
            }

			return TRUE;
		}

	case WM_KEYDOWN:
		{
			OnKeyDown((TCHAR)wParam, (UINT)lParam & 0xfff, (UINT)((lParam >> 16) & 0xffff));
			lResult = 0;
			return TRUE;
		}

	case WM_LBUTTONDOWN:
		{
			OnLButtonDown(wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			lResult = 0;
			return TRUE;
		}
	case WM_CONTEXTMENU:
		{
			OnContextMenu((HWND)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			lResult = 0;
			return TRUE;
		}

	case WM_SETFOCUS:
		{
			OnSetFocus((HWND)wParam);
			lResult = 0;
			return TRUE;
		}

	case WM_PAINT:
		{
			OnPaint((HDC)wParam);
			lResult = 0;
			return TRUE;
		}

	case WM_PRINTCLIENT:
		{
			OnPrintClient((HDC)wParam, (UINT)lParam);
			lResult = 0;
			return TRUE;
		}
	}

	// The framework will call DefWindowProc() for us.
	return FALSE;
}

LRESULT CLyricsWindow::OnCreate(LPCREATESTRUCT pCreateStruct) {
	if (DefWindowProc(m_hWnd, WM_CREATE, 0, (LPARAM)pCreateStruct) != 0) return -1;

	// If "Remember window positions" is enabled, this will
	// restore the last position of our window. Otherwise it
	// won't do anything.
	cfg_popup_window_placement.on_window_creation(m_hWnd);

	// Initialize the font and font color
	m_font = cfg_font.get_value().create();
    m_lrcFont.SetLrcFont(m_font); //change the lyrics font
    m_lrcFont.SetTextColor(cfg_font_color); //set font's color

	// Acquire a ui_selection_holder that allows us to notify other components
	// of the selected tracks in our window, when it has the focus.
	// Also used for keyboard shortcut processing (see pretranslate_message).
	m_selection = static_api_ptr_t<ui_selection_manager>()->acquire();

	// Register ourselves as message_filter, so we can process keyboard shortcuts
	// and (possibly) dialog messages.
	static_api_ptr_t<message_loop>()->add_message_filter(this);

	// Register ourselves as play_callback to get notified about playback events.
	static_api_ptr_t<play_callback_manager>()->register_callback(this,
		flag_on_playback_new_track |
		flag_on_playback_dynamic_info_track |
		flag_on_playback_stop |
        flag_on_playback_pause |
        flag_on_playback_seek,
		false);

	return 0;
}

void CLyricsWindow::OnDestroy() {
	m_selection.release();

	static_api_ptr_t<message_loop>()->remove_message_filter(this);

	static_api_ptr_t<play_callback_manager>()->unregister_callback(this);

	// Notify the window placement variable that our window
	// was destroyed. This will also update the variables value
	// with the current window position and size.
	cfg_popup_window_placement.on_window_destruction(m_hWnd);
}

void CLyricsWindow::OnClose() {
	// Hide and disable the window.
	HideWindow();
}

void CLyricsWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if (nChar == VK_ESCAPE) {
		// Hide and disable the window.
		HideWindow();
	}
}

void CLyricsWindow::OnLButtonDown(UINT nFlags, CPoint point) {
	// Get currently playing track.
	static_api_ptr_t<play_control> pc;
	metadb_handle_ptr handle;

	// If some track is playing...
	if (pc->get_now_playing(handle)) {
		POINT pt;
		GetCursorPos(&pt);

		// ...detect a drag operation.
		if (DragDetect(m_hWnd, pt)) {

            console::info("drag & drop action test\n");
			metadb_handle_list items;
			items.add_item(handle);

			// Create an IDataObject that contains the dragged track.
			static_api_ptr_t<playlist_incoming_item_filter> piif;
			// create_dataobject_ex() returns a smart pointer unlike create_dataobject()
			// which returns a raw COM pointer. The less chance we have to accidentally
			// get the reference counting wrong, the better.
			pfc::com_ptr_t<IDataObject> pDataObject = piif->create_dataobject_ex(items);

			// Create an IDropSource.
			// The constructor of IDropSource_tutorial1 is hidden by design; we use the
			// provided factory method which returns a smart pointer.
			pfc::com_ptr_t<IDropSource> pDropSource = IDropSource_tutorial1::g_create(m_hWnd);

			DWORD effect;
			// Perform drag&drop operation.
			DoDragDrop(pDataObject.get_ptr(), pDropSource.get_ptr(), DROPEFFECT_COPY, &effect);
		}
	}
}

void CLyricsWindow::OnPaint(HDC hdc) {
	//CPaintDC dc(m_hWnd);
	//PaintContent(dc.m_ps);

    //add by excalibur
    drawScene();
}

void CLyricsWindow::OnPrintClient(HDC hdc, UINT uFlags) {
	PAINTSTRUCT ps = { 0 };
	ps.hdc = hdc;
	GetClientRect(m_hWnd, &ps.rcPaint);
	ps.fErase = FALSE;
	PaintContent(ps);
}

void CLyricsWindow::PaintContent(PAINTSTRUCT &ps) {
	if (GetSystemMetrics(SM_REMOTESESSION)) {
		// Do not use double buffering, if we are running on a Remote Desktop Connection.
		// The system would have to transfer a bitmap everytime our window is painted.
		Draw(ps.hdc, ps.rcPaint);
	} else if (!IsRectEmpty(&ps.rcPaint)) {
		// Use double buffering for local drawing.
		CMemoryDC dc(ps.hdc, ps.rcPaint);
		Draw(dc, ps.rcPaint);
	}
}

void CLyricsWindow::Draw(HDC hdc, CRect rcPaint) {
	// We will paint the background in the default window color.
	HBRUSH hBrush = GetSysColorBrush(COLOR_WINDOW);
	FillRect(hdc, rcPaint, hBrush);

	HICON hIcon = static_api_ptr_t<ui_control>()->get_main_icon();
	if (hIcon != NULL) {
		DrawIconEx(hdc, 2, 2, hIcon, 32, 32, 0, hBrush, DI_NORMAL);
	}

	try
	{
		static_api_ptr_t<play_control> pc;
		metadb_handle_ptr handle;;
		if (pc->get_now_playing(handle)) {
			pfc::string8 format;
			g_advconfig_string_format.get_static_instance().get_state(format);
			service_ptr_t<titleformat_object> script;
			static_api_ptr_t<titleformat_compiler>()->compile_safe(script, format);

			pfc::string_formatter text;
			pc->playback_format_title_ex(handle, NULL, text, script, NULL, play_control::display_level_titles);
			HFONT hFont = m_font;
			if (hFont == NULL)
				hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SelectObject(hdc, hFont);
			SetTextAlign(hdc, TA_TOP | TA_LEFT);
			uExtTextOut(hdc, 32+4, 2, ETO_CLIPPED, rcPaint, text, text.length(), 0);
		}
	}
	catch (const std::exception & exc) {
		console::formatter() << "Exception occurred while drawing " APP_TITLE " window:\n" << exc;
	}
}


void CLyricsWindow::createLRCDir(const char* strDir)
{
    DWORD dwAttr = GetFileAttributesA(strDir);
    if (dwAttr == 0xffffffff) {
        CreateDirectoryA(strDir, NULL);
    }
}

void CLyricsWindow::OnContextMenu(HWND hWnd, CPoint point) {
	// We need some IDs for the context menu.
	enum {
		// ID for "Choose font..."
		ID_FONT = 1,
        ID_EXIT_LYRICS = 2,// exit lyrics display

		// The range ID_CONTEXT_FIRST through ID_CONTEXT_LAST is reserved
		// for menu entries from menu_manager.
		ID_CONTEXT_FIRST,
		ID_CONTEXT_LAST = ID_CONTEXT_FIRST + 1000,
	};

	// Create new popup menu.
	HMENU hMenu = CreatePopupMenu();

	// Add our "Choose font..." and "Choose font color" command.
	AppendMenu(hMenu, MF_STRING, ID_FONT, TEXT("字体"));
    AppendMenu(hMenu, MF_STRING, ID_EXIT_LYRICS, TEXT("退出歌词显示"));

	// Get the currently playing track.
	metadb_handle_list items;
	static_api_ptr_t<play_control> pc;
	metadb_handle_ptr handle;
	if (pc->get_now_playing(handle)) {
		// Insert it into a list.
		items.add_item(handle);
	}

	// Create a menu_manager that will build the context menu.
	service_ptr_t<contextmenu_manager> cmm;
	contextmenu_manager::g_create(cmm);
	// Query setting for showing keyboard shortcuts.
	const bool show_shortcuts = config_object::g_get_data_bool_simple(standard_config_objects::bool_show_keyboard_shortcuts_in_menus, false);
	// Set up flags for contextmenu_manager::init_context.
	unsigned flags = show_shortcuts ? contextmenu_manager::FLAG_SHOW_SHORTCUTS : 0;
	// Initialize menu_manager for using a context menu.
	cmm->init_context(items, flags);
	// If the menu_manager has found any applicable commands,
	// add them to our menu (after a separator).
	if (cmm->get_root()) {
		uAppendMenu(hMenu, MF_SEPARATOR, 0, 0);
		cmm->win32_build_menu(hMenu, ID_CONTEXT_FIRST, ID_CONTEXT_LAST);
	}

	// Use menu helper to gnereate mnemonics.
	menu_helpers::win32_auto_mnemonics(hMenu);

	// Get the location of the mouse pointer.
	// WM_CONTEXTMENU provides position of mouse pointer in argument lp,
	// but this isn't reliable (for example when the user pressed the
	// context menu key on the keyboard).
	CPoint pt;
	GetCursorPos(pt);
	// Show the context menu.
	int cmd = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON, 
		pt.x, pt.y, 0, m_hWnd, 0);

	// Check what command has been chosen. If cmd == 0, then no command
	// was chosen.
	if (cmd == ID_FONT) {
		// Show font configuration.
		t_font_description font = cfg_font;

        LOGFONTW lf;
        memset(&lf, 0, sizeof(LOGFONTW));
        lf.lfCharSet = font.m_charset;

        //copy font facename (utf8 to unicode-16)
        int stringSize = MultiByteToWideChar(CP_UTF8, 0, font.m_facename, -1, NULL, 0);
        WCHAR *wfaceName = new WCHAR[stringSize + 1];
        memset(wfaceName, 0, stringSize + 1);
        MultiByteToWideChar(CP_UTF8, 0, font.m_facename, -1, wfaceName, stringSize + 1);
        wcscpy(lf.lfFaceName, wfaceName);
        delete wfaceName;

        lf.lfHeight = font.m_height;
        lf.lfWeight = font.m_weight;
        lf.lfItalic = font.m_italic;

        CHOOSEFONTW chooseFont;
        memset(&chooseFont, 0, sizeof(CHOOSEFONTW));
        chooseFont.rgbColors = cfg_font_color;
        //chooseFont.hwndOwner = hWnd;
        chooseFont.lStructSize = sizeof(CHOOSEFONTW);
        chooseFont.Flags = CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_TTONLY | CF_NOSIZESEL;
        chooseFont.lpLogFont = &lf;
        if (ChooseFontW(&chooseFont)) {

            //set color of the font
            cfg_font_color = chooseFont.rgbColors;
            m_lrcFont.SetTextColor(chooseFont.rgbColors);

            //didn't support to change the font's size
            font.m_height = 100;//chooseFont.lpLogFont->lfHeight;
            font.m_weight = 100;//chooseFont.lpLogFont->lfWeight;
            font.m_italic = chooseFont.lpLogFont->lfItalic;

            stringSize = WideCharToMultiByte(CP_UTF8, 0, chooseFont.lpLogFont->lfFaceName, -1, NULL, 0, NULL, NULL);
            char *faceName = new char[stringSize];
            memset(faceName, 0, (stringSize) * sizeof(char));
            WideCharToMultiByte(CP_UTF8, 0, chooseFont.lpLogFont->lfFaceName, -1, faceName, stringSize, NULL, NULL);
            strcpy(font.m_facename, faceName);
            delete faceName;

            cfg_font = font;
            m_font = font.create();
            m_lrcFont.SetLrcFont(m_font);
            ::RedrawWindow(m_hWnd, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW);
        }

	} 
    if (cmd == ID_EXIT_LYRICS) {
        ::SendMessage(m_wnd, WM_CLOSE, NULL, NULL);
    } else if (cmd >= ID_CONTEXT_FIRST && cmd <= ID_CONTEXT_LAST ) {
		// Let the menu_manager execute the chosen command.
		cmm->execute_by_id(cmd - ID_CONTEXT_FIRST);
	}

	// contextmenu_manager instance is released automatically, as is the metadb_handle we used.

	// Finally, destroy the popup menu.
	DestroyMenu(hMenu);
}

void CLyricsWindow::OnSetFocus(HWND hWndOld) {
	metadb_handle_list items;
	metadb_handle_ptr track;
	if (static_api_ptr_t<playback_control>()->get_now_playing(track)) {
		items.add_item(track);
	}
	set_selection(items);
}

void CLyricsWindow::set_selection(metadb_handle_list_cref p_items) {
	// Only notify other components about changes in our selection,
	// if our window is the active one.
	if (::GetFocus() == m_hWnd && m_selection.is_valid()) {
		m_selection->set_selection_ex(p_items, contextmenu_item::caller_now_playing);
	}
}

bool CLyricsWindow::pretranslate_message(MSG * p_msg) {
	// Process keyboard shortcuts
	if (static_api_ptr_t<keyboard_shortcut_manager_v2>()->pretranslate_message(p_msg, m_hWnd)) return true;

	// If you use a dialog or a window with child controls, you can uncomment the following line.
	//if (::IsDialogMessage(m_hWnd, p_msg)) return true;

	return false;
}

void CLyricsWindow::on_playback_new_track(metadb_handle_ptr p_track) {

    long long startTime = gl_lyrics_utils::getCurTime();
	RedrawWindow();
	set_selection(pfc::list_single_ref_t<metadb_handle_ptr>(p_track));

    //add by excalibur to get the playing song information
    pfc::string8 title;
    
    pfc::string8 artist;
    pfc::string8 album;
    static_api_ptr_t<titleformat_compiler> titleService;
    titleformat_object::ptr fmt;

    titleService->compile(fmt, "%TITLE%");
    p_track->format_title_nonlocking(NULL, title, fmt, NULL);

    titleService->compile(fmt, "%ARTIST%");
    p_track->format_title_nonlocking(NULL, artist, fmt, NULL);

    titleService->compile(fmt, "%ALBUM%");
    p_track->format_title_nonlocking(NULL, album, fmt, NULL);

    m_lrcPlayer.setPlayingSong(title.get_ptr(), album.get_ptr(), artist.get_ptr(), startTime);
}

void CLyricsWindow::on_playback_seek(double p_time)
{
    unsigned int timePos = (unsigned int)(p_time * 1000);
    m_lrcPlayer.startPlayFromAnyTime(timePos);
}

void CLyricsWindow::on_playback_pause(bool p_state)
{
    m_lrcPlayer.pauseDisplayLrc(p_state);
}

void CLyricsWindow::on_playback_stop(play_control::t_stop_reason reason) {
	RedrawWindow();
	set_selection(metadb_handle_list());
    m_lrcPlayer.stopDisplayLrc();
}

void CLyricsWindow::on_playback_dynamic_info_track(const file_info & p_info) {
	RedrawWindow();
}
