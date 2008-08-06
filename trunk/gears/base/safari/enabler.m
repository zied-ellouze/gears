// Copyright 2007, Google Inc.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import "gears/base/safari/enabler.h"
#import "gears/base/safari/loader.h"
#import "gears/base/safari/browser_load_hook.h"

@implementation GearsEnabler
//------------------------------------------------------------------------------
- (id)init {
  if ((self = [super init])) {
    NSString *bundleID = [[NSBundle mainBundle] bundleIdentifier];
    
    // Setup things if we're running in Safari
    if ([bundleID isEqualToString:@"com.apple.Safari"]) {
      if (![[self class] loadGears]) {
        [self release];
        self = nil;
      }
    }
  }
  
  return self;
}

+ (BOOL)loadGears {
  BOOL result = NO;

  // Check if Gears can be loaded into this version of WebKit.
  if ([GearsLoader canLoadGears]) {
    if ([GearsLoader loadGearsBundle]) {
      Class gearsBrowserLoadHook = 
                 NSClassFromString(@"GearsBrowserLoadHook"); 
      result = [gearsBrowserLoadHook installHook];
      if (!result) {
        NSLog(@"gearsBrowserLoadHook installHook failed");
      }
    }
  } else {
    NSLog(@"canLoadGears Returned False");
  }
  
  return result;
}

@end
