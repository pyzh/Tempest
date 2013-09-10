#include "windowsapi.h"

#ifdef __WIN32__
#include <windows.h>

#include <Tempest/Window>
#include <Tempest/Event>
#include <Tempest/DisplaySettings>

#include <unordered_map>
#include <fstream>
#include <Tempest/Assert>

#include <iostream>

#include <IL/il.h>
#include "core/wrappers/atomic.h"

using namespace Tempest;

static std::unordered_map<WindowsAPI::Window*, Tempest::Window*> wndWx;

static LRESULT CALLBACK WindowProc( HWND   hWnd,
                                    UINT   msg,
                                    WPARAM wParam,
                                    LPARAM lParam );

WindowsAPI::WindowsAPI() {
  TranslateKeyPair k[] = {
    { VK_LEFT,   Event::K_Left   },
    { VK_RIGHT,  Event::K_Right  },
    { VK_UP,     Event::K_Up     },
    { VK_DOWN,   Event::K_Down   },

    { VK_ESCAPE, Event::K_ESCAPE },
    { VK_BACK,   Event::K_Back   },
    { VK_DELETE, Event::K_Delete },
    { VK_INSERT, Event::K_Insert },
    { VK_HOME,   Event::K_Home   },
    { VK_END,    Event::K_End    },
    { VK_PAUSE,  Event::K_Pause  },
    { VK_RETURN, Event::K_Return },

    { VK_F1,     Event::K_F1 },
    { 0x30,      Event::K_0  },
    { 0x41,      Event::K_A  },

    { 0,         Event::K_NoKey }
    };

  setupKeyTranslate(k);
  setFuncKeysCount(24);
  }

WindowsAPI::~WindowsAPI() {
  }

bool WindowsAPI::testDisplaySettings( const DisplaySettings & s ) {
  DEVMODE mode;                   // Device Mode
  memset(&mode,0,sizeof(mode));
  mode.dmSize=sizeof(mode);

  mode.dmPelsWidth    = s.width;
  mode.dmPelsHeight   = s.height;
  mode.dmBitsPerPel   = s.bits;
  mode.dmFields       = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

  DWORD flg = CDS_TEST;
  if( s.fullScreen )
    flg |= CDS_FULLSCREEN;

  return ChangeDisplaySettings(&mode,flg)==DISP_CHANGE_SUCCESSFUL;
  }

bool WindowsAPI::setDisplaySettings( const DisplaySettings &s ) {
  DEVMODE mode;                   // Device Mode
  memset(&mode,0,sizeof(mode));
  mode.dmSize=sizeof(mode);

  mode.dmPelsWidth    = s.width;
  mode.dmPelsHeight   = s.height;
  mode.dmBitsPerPel   = s.bits;
  mode.dmFields       = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

  DWORD flg = 0;
  if( s.fullScreen )
    flg |= CDS_FULLSCREEN;

  if( ChangeDisplaySettings(&mode,flg)==DISP_CHANGE_SUCCESSFUL ){
    //appMode       = mode;
    //appDevModeFlg = flg;
    return 1;
    }

  return 0;
  }

Size WindowsAPI::implScreenSize() {
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  int w = mode.dmPelsWidth;
  int h = mode.dmPelsHeight;

  return Size(w,h);
  }

void WindowsAPI::startApplication(ApplicationInitArgs *) {
  //EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &defaultMode );
  //appMode = defaultMode;

  WNDCLASSEX winClass;

  winClass.lpszClassName = L"Tempest_Window_Class";
  winClass.cbSize        = sizeof(WNDCLASSEX);
  winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  winClass.lpfnWndProc   = WindowProc;
  winClass.hInstance     = GetModuleHandle(0);
  winClass.hIcon         = LoadIcon( GetModuleHandle(0), (LPCTSTR)MAKEINTRESOURCE(32512) );
  winClass.hIconSm       = LoadIcon( GetModuleHandle(0), (LPCTSTR)MAKEINTRESOURCE(32512) );
  winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  winClass.lpszMenuName  = NULL;
  winClass.cbClsExtra    = 0;
  winClass.cbWndExtra    = 0;

  T_ASSERT_X( RegisterClassEx(&winClass), "window not initalized" );
  }

void WindowsAPI::endApplication() {
  UnregisterClass( L"Tempest_Window_Class", GetModuleHandle(0) );
  }

int WindowsAPI::nextEvent(bool &quit) {
  MSG uMsg;
  memset(&uMsg,0,sizeof(uMsg));

  if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) ){
    if( uMsg.message==WM_QUIT )
      quit = 1;

    TranslateMessage( &uMsg );
    DispatchMessage ( &uMsg );
    Sleep(0);
    return uMsg.wParam;
    } else {
    for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
      i->second->render();

    return 0;
    }
  }

int WindowsAPI::nextEvents(bool &quit) {
  MSG uMsg;
  memset(&uMsg,0,sizeof(uMsg));
  int r = 0;

  while( !quit ){
    if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) ){
      if( uMsg.message==WM_QUIT )
        quit = 1;

      TranslateMessage( &uMsg );
      DispatchMessage ( &uMsg );
      Sleep(0);
      r = uMsg.wParam;
      } else {
      for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
        i->second->render();

      return r;
      }
    }

  return r;
  }

WindowsAPI::Window *WindowsAPI::createWindow(int w, int h) {
  DWORD wflags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  RECT r = {0,0,w,h};

  AdjustWindowRect(&r, wflags, false);

  HWND hwnd = CreateWindowEx( 0,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              wflags,
                              0, 0,
                              r.right-r.left,
                              r.bottom-r.top,
                              NULL, NULL,
                              GetModuleHandle(0), NULL );

  return (Window*)hwnd;
  }

SystemAPI::Window *WindowsAPI::createWindowMaximized() {
  int w = GetSystemMetrics(SM_CXFULLSCREEN),
      h = GetSystemMetrics(SM_CYFULLSCREEN);
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  w = mode.dmPelsWidth;
  h = mode.dmPelsHeight;

  DWORD wflags    = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;
  DWORD dwExStyle = WS_EX_APPWINDOW;

  wflags |= WS_CLIPSIBLINGS |	WS_CLIPCHILDREN;

  HWND hwnd = CreateWindowEx( dwExStyle,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              wflags,
                              0, 0,
                              w, h,
                              NULL,
                              NULL,
                              GetModuleHandle(0), NULL );

  ShowWindow( hwnd, SW_MAXIMIZE );
  UpdateWindow( hwnd );
  return (Window*)hwnd;
  }

SystemAPI::Window *WindowsAPI::createWindowMinimized() {
  int w = GetSystemMetrics(SM_CXFULLSCREEN),
      h = GetSystemMetrics(SM_CYFULLSCREEN);
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  w = mode.dmPelsWidth;
  h = mode.dmPelsHeight;

  DWORD wflags = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;
  HWND hwnd = CreateWindowEx( 0,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              wflags,
                              0, 0,
                              w, h,
                              NULL, NULL,
                              GetModuleHandle(0), NULL );

  return (Window*)hwnd;
  }

SystemAPI::Window *WindowsAPI::createWindowFullScr() {
  int w = GetSystemMetrics(SM_CXFULLSCREEN),
      h = GetSystemMetrics(SM_CYFULLSCREEN);
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  w = mode.dmPelsWidth;
  h = mode.dmPelsHeight;

  DWORD wflags = WS_POPUP;
  HWND hwnd = CreateWindowEx( 0,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              wflags,
                              0, 0,
                              w, h,
                              NULL, NULL,
                              GetModuleHandle(0),
                              NULL );

  ShowWindow( hwnd, SW_NORMAL );
  UpdateWindow( hwnd );
  return (Window*)hwnd;
  }

Size WindowsAPI::windowClientRect( SystemAPI::Window * hWnd ) {
  RECT rectWindow;
  GetClientRect( HWND(hWnd), &rectWindow);
  int cW = rectWindow.right  - rectWindow.left;
  int cH = rectWindow.bottom - rectWindow.top;

  return Size(cW,cH);
  }

void WindowsAPI::deleteWindow( Window *w ) {
  DestroyWindow( (HWND)w );
  wndWx.erase(w);
  }

void WindowsAPI::show(Window *hWnd) {
  Tempest::Window* w = 0;
  std::unordered_map<WindowsAPI::Window*, Tempest::Window*>::iterator i
      = wndWx.find( (WindowsAPI::Window*)hWnd );

  if( i!= wndWx.end() )
    w = i->second;

  if( !w )
    return;

  if( w->showMode()==Tempest::Window::FullScreen )
    return;

  HWND hwnd = (HWND)hWnd;

  switch( w->showMode() ){
    case Tempest::Window::Normal:
    case Tempest::Window::FullScreen:
      ShowWindow( hwnd, SW_NORMAL );
      break;

    case Tempest::Window::Minimized:
      ShowWindow( hwnd, SW_MINIMIZE );
      break;

    default:
      ShowWindow( hwnd, SW_MAXIMIZE );
      break;
    }

  UpdateWindow( hwnd );
  }

void WindowsAPI::setGeometry( Window *hw, int x, int y, int w, int h ) {
  DWORD wflags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  RECT r = {0,0,w,h};
  AdjustWindowRect(&r, wflags, false);

  LONG lStyles = GetWindowLong( (HWND)hw, GWL_STYLE );

  if( lStyles & WS_MINIMIZE )
    return;

  if( lStyles & WS_MAXIMIZE )
    return;

  MoveWindow( (HWND)hw,
              x,
              y,
              r.right-r.left,
              r.bottom-r.top,
              false );
  }

void WindowsAPI::bind( Window *w, Tempest::Window *wx ) {
  wndWx[w] = wx;
  }

std::string WindowsAPI::loadTextImpl(const char *file) {
  std::ifstream is( file, std::ifstream::binary );
  if( !is )
    return "";

  is.seekg (0, is.end);
  int length = is.tellg();
  is.seekg (0, is.beg);

  std::string src;
  src.resize( length );
  is.read ( &src[0], length );

  if( !is )
    return "";
  is.close();

  return src;
  }

std::string WindowsAPI::loadTextImpl(const wchar_t *file) {
  HANDLE hTextFile = CreateFile( file, GENERIC_READ,
                                 0, NULL, OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL, NULL);

  DWORD dwFileSize = GetFileSize(hTextFile, &dwFileSize);
  DWORD dwBytesRead;

  if( dwFileSize==DWORD(-1) )
    return std::string();

  std::string str;
  str.resize(dwFileSize);
  ReadFile(hTextFile, &str[0], dwFileSize, &dwBytesRead, NULL);
  CloseHandle(hTextFile);

  return str;
  }

std::vector<char> WindowsAPI::loadBytesImpl(const char *file) {
  std::vector<char> src;

  std::ifstream is( file, std::ifstream::binary );
  if(!is)
    return src;

  is.seekg (0, is.end);
  int length = is.tellg();
  is.seekg (0, is.beg);

  src.resize( length );
  is.read ( &src[0], length );

  if(!is)
    return src;
  is.close();

  return src;
  }

std::vector<char> WindowsAPI::loadBytesImpl(const wchar_t *file) {
  HANDLE hTextFile = CreateFile( file, GENERIC_READ,
                                 0, NULL, OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL, NULL);

  DWORD dwFileSize = GetFileSize(hTextFile, &dwFileSize);
  DWORD dwBytesRead;

  std::vector<char> str;  
  if( dwFileSize==DWORD(-1) )
    return str;

  str.resize(dwFileSize);
  ReadFile(hTextFile, &str[0], dwFileSize, &dwBytesRead, NULL);
  CloseHandle(hTextFile);

  return str;
  }

void WindowsAPI::initImgLib() {
  Tempest::Detail::Atomic::begin();

  static bool wasInit = false;
  if( !wasInit )
    ilInit();
  ilEnable(IL_FILE_OVERWRITE);

  wasInit = true;

  Tempest::Detail::Atomic::end();
  }

template< class ChanelType, int mul >
void WindowsAPI::initRawData( std::vector<unsigned char> &d,
                              void * input,
                              int bpp,
                              int w,
                              int h,
                              int * ix ){
  ChanelType * img = reinterpret_cast<ChanelType*>(input);

  d.resize( bpp*w*h );

  for( int i=0; i<w; ++i )
    for( int r=0; r<h; ++r ){
      ChanelType * raw = &img[ bpp*(i+r*w) ];
      for( int q=0; q<bpp; ++q ){
        d[ bpp*(i+r*w)+q ] = ( raw[ ix[q] ]*mul );
        }

      }
  }

bool WindowsAPI::loadImageImpl( const wchar_t *file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char> &out ) {
  initImgLib();
  bool ok = true;
  std::vector<char> imgBytes = loadBytesImpl(file);

  if( loadS3TCImpl(imgBytes,w,h,bpp,out) )
    return true;

  if( loadPngImpl(imgBytes,w,h,bpp,out) )
    return true;

  ILuint	id;
  ilGenImages ( 1, &id );
  ilBindImage ( id );

  //if( ilLoadImage( file ) ){
  if( ilLoadL( IL_TYPE_UNKNOWN, &imgBytes[0], imgBytes.size() ) ){
    int format = ilGetInteger( IL_IMAGE_FORMAT );

    int size_of_pixel = 1;
    bpp = 3;

    int idx[5][4] = {
      { 0, 1, 2, 3 }, //IL_RGB, IL_RGBA
      { 0, 0, 0, 0 }, //IL_ALPHA
      { 2, 1, 0, 3 },
      { 2, 1, 0, 3 },
      { 0, 0, 0, 1 }
      };
    int *ix = idx[0];


    switch( format ){
      case IL_RGB:   size_of_pixel = 3; break;
      case IL_RGBA:  size_of_pixel = 4; bpp = 4; break;
      case IL_ALPHA: size_of_pixel = 1; ix = idx[1]; break;

      case IL_BGR:   size_of_pixel = 3; ix = idx[2]; break;
      case IL_BGRA:  size_of_pixel = 4; ix = idx[3]; break;
      case IL_LUMINANCE_ALPHA:
        size_of_pixel = 4; ix = idx[4];
        break;
      default: ok = false;
      }

    if( ok ){
      ILubyte * data = ilGetData();
      //Data * image = new Data();

      w = ilGetInteger ( IL_IMAGE_WIDTH  );
      h = ilGetInteger ( IL_IMAGE_HEIGHT );

      if( ilGetInteger ( IL_IMAGE_TYPE ) == IL_UNSIGNED_BYTE )
        initRawData<ILubyte,   1> ( out, data, size_of_pixel, w, h, ix ); else
        initRawData<ILdouble, 255>( out, data, size_of_pixel, w, h, ix );
      }
    } else {
    ok = false;
    }

  ilDeleteImages( 1, &id );

  return ok;
  }

bool WindowsAPI::saveImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& in ){
  initImgLib();

  ILuint	id;
  ilGenImages ( 1, &id );
  ilBindImage ( id );

  ilTexImage( w, h, 1,
              bpp,
              (bpp==4)? IL_RGBA : IL_RGB,
              IL_UNSIGNED_BYTE,
              in.data() );

  ILubyte * img = ilGetData();

  int h2 = h/2;
  for( int i=0; i<w; ++i )
    for( int r=0; r<h2; ++r ){
      ILubyte * raw1 = &img[ bpp*(i+r*w) ];
      ILubyte * raw2 = &img[ bpp*(i+(h-r-1)*w) ];

      for( int q=0; q<bpp; ++q )
        std::swap( raw1[q], raw2[q] );
      }

  bool ok = ilSaveImage( file );
  ilDeleteImages( 1, &id );

  return ok;
  }

bool WindowsAPI::saveImageImpl( const wchar_t *file,
                                int &w, int &h,
                                int &bpp,
                                std::vector<unsigned char> &out) {
  //TODO: save image file
  return 0;
  }

static Event::MouseButton toButton( UINT msg ){
  if( msg==WM_LBUTTONDOWN ||
      msg==WM_LBUTTONUP )
    return Event::ButtonLeft;

  if( msg==WM_RBUTTONDOWN  ||
      msg==WM_RBUTTONUP)
    return Event::ButtonRight;

  if( msg==WM_MBUTTONDOWN ||
      msg==WM_MBUTTONUP )
    return Event::ButtonMid;

  return Event::ButtonNone;
  }

static Tempest::KeyEvent makeKeyEvent( WPARAM k,
                                       bool scut = false ){
  Tempest::KeyEvent::KeyType e = SystemAPI::translateKey(k);

  if( !scut ){
    if( Event::K_0<=e && e<= Event::K_9 )
      e = Tempest::KeyEvent::K_NoKey;

    if( Event::K_A<=e && e<= Event::K_Z )
      e = Tempest::KeyEvent::K_NoKey;
    }

  return Tempest::KeyEvent( e );
  }

LRESULT CALLBACK WindowProc( HWND   hWnd,
                             UINT   msg,
                             WPARAM wParam,
                             LPARAM lParam ) {
    //return DefWindowProc( hWnd, msg, wParam, lParam );

    Tempest::Window* w = 0;
    std::unordered_map<WindowsAPI::Window*, Tempest::Window*>::iterator i
        = wndWx.find( (WindowsAPI::Window*)hWnd );

    if( i!= wndWx.end() )
      w = i->second;

    if( !w )
      return DefWindowProc( hWnd, msg, wParam, lParam );

    switch( msg ) {
      case WM_CHAR:
      {
         Tempest::KeyEvent e = Tempest::KeyEvent( uint16_t(wParam) );

         DWORD wrd[3] = {
           VK_RETURN,
           VK_BACK,
           0
           };

         if( 0 == *std::find( wrd, wrd+2, wParam) ){
           SystemAPI::mkKeyEvent(w, e, Event::KeyDown);
           SystemAPI::mkKeyEvent(w, e, Event::KeyUp);
           }
      }
      break;

      case WM_KEYDOWN:
      {
         Tempest::KeyEvent sce = makeKeyEvent(wParam, true);
         SystemAPI::mkKeyEvent(w, sce, Event::Shortcut);

         if( !sce.isAccepted() ){
           Tempest::KeyEvent e =  makeKeyEvent(wParam);
           if( e.key!=Tempest::KeyEvent::K_NoKey )
             SystemAPI::mkKeyEvent(w, e, Event::KeyDown);
           }
      }
      break;

      case WM_KEYUP:
      {
         Tempest::KeyEvent e =  makeKeyEvent(wParam);
         SystemAPI::mkKeyEvent(w, e, Event::KeyUp);
      }
      break;


      case WM_LBUTTONDOWN:
      case WM_MBUTTONDOWN:
      case WM_RBUTTONDOWN: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      toButton(msg) );
        SystemAPI::mkMouseEvent(w, e, Event::MouseDown);
        }
        break;

      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      toButton(msg) );
        //w->mouseUpEvent(e);
        SystemAPI::mkMouseEvent(w, e, Event::MouseUp);
        }
        break;

      case WM_MOUSEMOVE: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      Event::ButtonNone );
        SystemAPI::mkMouseEvent(w, e, Event::MouseMove);
        }
        break;

       case WM_MOUSEWHEEL:{
          POINT p;
          p.x = LOWORD (lParam);
          p.y = HIWORD (lParam);

          ScreenToClient(hWnd, &p);

          Tempest::MouseEvent e( p.x, p.y,
                                 Tempest::Event::ButtonNone,
                                 GET_WHEEL_DELTA_WPARAM(wParam) );
          SystemAPI::mkMouseEvent(w, e, Event::MouseWheel);
          //w->mouseWheelEvent(e);
          }
        break;

      case WM_SIZE:{
          RECT rectWindow;
          GetClientRect( HWND(hWnd), &rectWindow);
          int cW = rectWindow.right-rectWindow.left;
          int cH = rectWindow.bottom-rectWindow.top;

          if( w )
            SystemAPI::sizeEvent( w, cW, cH );
          }
        break;

      case WM_ACTIVATEAPP:
      {
          bool a = (wParam==TRUE);
          SystemAPI::activateEvent(w,a);

          if( !a && w->isFullScreenMode() ){
            ShowWindow( hWnd, SW_MINIMIZE );
            }

          if( !a ){
            //ChangeDisplaySettings(&defaultMode, 0);
            } else {
            //ChangeDisplaySettings(&appMode, appDevModeFlg);
            }
      }
      break;

      case WM_CLOSE:
      case WM_DESTROY: {
        PostQuitMessage(0);
        }
        break;

      default: {
        return DefWindowProc( hWnd, msg, wParam, lParam );
        }
      }

    return 0;
  }

#endif
