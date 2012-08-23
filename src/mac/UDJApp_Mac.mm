#include "UDJApp_Mac.h"

#import <AppKit/NSApplication.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSError.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSAppleEventManager.h>
#import <Foundation/NSURL.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSNibDeclarations.h>
#import <Sparkle/SUUpdater.h>

#include <QDebug>
#include <QApplication>
#include <QObject>
#include <QMetaObject>


void macMain() {
  [[NSAutoreleasePool alloc] init];
  // Creates and sets the magic global variable so QApplication will find it.
  //[MacApplication sharedApplication];
  [NSApplication sharedApplication];
  // Creates and sets the magic global variable for Sparkle.
  [[SUUpdater sharedUpdater] setDelegate: NSApp];
}

void checkForUpdates() {
  [[SUUpdater sharedUpdater] checkForUpdates: NSApp];
}

