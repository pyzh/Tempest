#include "appdelegate.h"

#ifdef __OSX__
#include <OpenGL/gl.h>

#include "osxapi.h"

@implementation AppDelegate

@synthesize window;
@synthesize openGLContext;

- (void) draw {
  Tempest::OsxAPI::swapContext();
  }

- (id)createWindow {
  id menubar = [[NSMenu new] autorelease];
  id appMenuItem = [[NSMenuItem new] autorelease];
  [menubar addItem:appMenuItem];
  [NSApp setMainMenu:menubar];
  id appMenu = [[NSMenu new] autorelease];
  id appName = [[NSProcessInfo processInfo] processName];
  id quitTitle = [@"Quit " stringByAppendingString:appName];
  id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
  action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
  [appMenu addItem:quitMenuItem];
  [appMenuItem setSubmenu:appMenu];

  id wnd = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 800, 600)
                 styleMask:NSTitledWindowMask
                 backing:NSBackingStoreBuffered
                 defer:NO]
               autorelease];
  [wnd cascadeTopLeftFromPoint:NSMakePoint(20,20)];
  [wnd setTitle:appName];
  [wnd setStyleMask:[wnd styleMask] | NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask];

  return wnd;
  }

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  (void)aNotification;
  self.timer =
    [NSTimer
        scheduledTimerWithTimeInterval:0.001
        target:self
        selector:@selector(draw)
        userInfo:nil
        repeats:YES];
  Tempest::OsxAPI::swapContext();
  }

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)_app {
  (void)_app;
  [self.timer invalidate];
  Tempest::OsxAPI::finish();
  return YES;
  }

@end
#endif
