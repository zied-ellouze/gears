// Copyright 2008, Google Inc.
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

#ifndef GEARS_DESKTOP_DROP_TARGET_BASE_H__
#define GEARS_DESKTOP_DROP_TARGET_BASE_H__
#ifdef OFFICIAL_BUILD
// The Drag-and-Drop API has not been finalized for official builds.
#else

#if BROWSER_FF || (BROWSER_IE && !defined(OS_WINCE)) || BROWSER_SAFARI

// TODO(nigeltao): Before Drag and Drop is made an official Gears API, we have
// to (1) implement DnD on Chrome, and (2) decide what to do about
// mobile platforms like Android and WinCE.
#define GEARS_DRAG_AND_DROP_API_IS_SUPPORTED_FOR_THIS_PLATFORM 1

#include "gears/base/common/base_class.h"
#include "gears/base/common/js_dom_element.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/scoped_refptr.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class DropTargetBase
    : public JsEventHandlerInterface {
 protected:
  scoped_refptr<ModuleEnvironment> module_environment_;
  scoped_ptr<JsEventMonitor> unload_monitor_;
  scoped_ptr<JsRootedCallback> on_drag_enter_;
  scoped_ptr<JsRootedCallback> on_drag_over_;
  scoped_ptr<JsRootedCallback> on_drag_leave_;
  scoped_ptr<JsRootedCallback> on_drop_;

#ifdef DEBUG
  bool is_debugging_;
#endif

  DropTargetBase(ModuleEnvironment *module_environment,
                 JsObject *options,
                 std::string16 *error_out);

 private:
  DISALLOW_EVIL_CONSTRUCTORS(DropTargetBase);
};

#endif

#endif  // OFFICIAL_BUILD
#endif  // GEARS_DESKTOP_DROP_TARGET_BASE_H__
