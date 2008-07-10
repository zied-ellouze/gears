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

#include "gears/factory/factory_impl.h"

#include <assert.h>
#include <stdlib.h>

#include "gears/base/common/base_class.h"
#include "gears/base/common/detect_version_collision.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/string16.h"
#include "gears/console/console.h"
#include "gears/database/database.h"
#include "gears/database2/manager.h"
#include "gears/desktop/desktop.h"

#ifdef OFFICIAL_BUILD
// The Dummy module is not included in official builds.
#else
#include "gears/dummy/dummy_module.h"
#endif

#include "gears/factory/common/factory_utils.h"
#include "gears/geolocation/geolocation.h"
#include "gears/httprequest/httprequest.h"
#include "gears/localserver/localserver_module.h"
#include "gears/media/audio.h"
#include "gears/media/audio_recorder.h"
#include "gears/timer/timer.h"
#include "gears/workerpool/workerpool.h"
#ifdef WIN32
#include "gears/base/common/process_utils_win32.h"
#include "gears/ui/ie/string_table.h"
#endif
#include "genfiles/product_constants.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

#ifdef USING_CCTESTS
#include "gears/cctests/test.h"
#endif

DECLARE_GEARS_WRAPPER(GearsFactoryImpl);

// static
template <>
void Dispatcher<GearsFactoryImpl>::Init() {
  RegisterProperty("version", &GearsFactoryImpl::GetVersion, NULL);
  RegisterProperty("hasPermission", &GearsFactoryImpl::GetHasPermission, NULL);
  RegisterMethod("create", &GearsFactoryImpl::Create);
  RegisterMethod("getBuildInfo", &GearsFactoryImpl::GetBuildInfo);
  RegisterMethod("getPermission", &GearsFactoryImpl::GetPermission);
}

const std::string GearsFactoryImpl::kModuleName("GearsFactoryImpl");

GearsFactoryImpl::GearsFactoryImpl()
    : ModuleImplBaseClassVirtual(kModuleName),
      is_creation_suspended_(false) {
  SetActiveUserFlag();
}

#if BROWSER_FF
  // TODO(nigeltao): implement version collision UI for Firefox.
#elif defined(WIN32) || defined(BROWSER_WEBKIT)
// Returns NULL if an error occured loading the string.
static const char16 *GetVersionCollisionErrorString() {
#if defined(WIN32)
  // TODO(playmobil): This code isn't threadsafe, refactor.
  const int kMaxStringLength = 256;
  static char16 error_text[kMaxStringLength];
  if (!LoadString(GetGearsModuleHandle(), IDS_VERSION_COLLISION_TEXT, 
                  error_text, kMaxStringLength)) {
    return NULL;
  }
  return error_text;
#elif defined(BROWSER_WEBKIT)
  //TODO(playmobil): Internationalize string.
  static const char16 *error_text = STRING16(L"A " PRODUCT_FRIENDLY_NAME 
                                    L" update has been downloaded.\n"
                                    L"\n"
                                    L"Please close all browser windows"
                                    L" to complete the upgrade process.\n");
  return error_text;
#else
  return NULL;
#endif
}
#endif

void GearsFactoryImpl::Create(JsCallContext *context) {
#if BROWSER_FF
  // TODO(nigeltao): implement version collision UI for Firefox.
#elif defined(WIN32) || defined(BROWSER_WEBKIT)
  if (DetectedVersionCollision()) {
    if (!EnvIsWorker()) {
      MaybeNotifyUserOfVersionCollision();  // only notifies once per process
    }
    
    const char16 *error_text = GetVersionCollisionErrorString();
    if (error_text) {
      context->SetException(error_text);
      return;
    } else {
      context->SetException(STRING16(L"Internal Error"));
      return;
    }
  }
#endif

  std::string16 module_name;
  std::string16 version = STRING16(L"1.0");  // default for this optional param
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &module_name },
    { JSPARAM_OPTIONAL, JSPARAM_STRING16, &version },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  // Check is_creation_suspended_, because the factory can be suspended
  // inside a worker that hasn't yet called allowCrossOrigin().
  if (is_creation_suspended_) {
    context->SetException(kPermissionExceptionString);
  }

  // Check if the module requires local data permission to be created.
  if (RequiresLocalDataPermissionType(module_name)) {
    // Make sure the user gives this site permission to use Gears unless the
    // module can be created without requiring any permissions.
    if (!GetPermissionsManager()->AcquirePermission(
        PermissionsDB::PERMISSION_LOCAL_DATA)) {
      context->SetException(kPermissionExceptionString);
    }
  }

  // Check the version string.
  if (version != kAllowedClassVersion) {
    context->SetException(STRING16(L"Invalid version string. Must be 1.0."));
    return;
  }

  // Create an instance of the object.
  //
  // Do case-sensitive comparisons, which are always better in APIs. They make
  // code consistent across callers, and they are easier to support over time.
  scoped_refptr<ModuleImplBaseClass> object;
  
  if (module_name == STRING16(L"beta.console")) {
    CreateModule<GearsConsole>(module_environment_.get(), context, &object);
  } else if (module_name == STRING16(L"beta.database")) {
    CreateModule<GearsDatabase>(module_environment_.get(), context, &object);
  } else if (module_name == STRING16(L"beta.desktop")) {
    CreateModule<GearsDesktop>(module_environment_.get(), context, &object);
  } else if (module_name == STRING16(L"beta.localserver")) {
    CreateModule<GearsLocalServer>(module_environment_.get(),
                                   context, &object);
  } else if (module_name == STRING16(L"beta.workerpool")) {
    CreateModule<GearsWorkerPool>(module_environment_.get(),
                                  context, &object);
#ifdef OFFICIAL_BUILD
  // The Database2, Geolocation and Media API have not been finalized for
  // official builds.
#else
  } else if (module_name == STRING16(L"beta.databasemanager")) {
    CreateModule<Database2Manager>(module_environment_.get(),
                                   context, &object);
  } else if (module_name == STRING16(L"beta.geolocation")) {
    CreateModule<GearsGeolocation>(module_environment_.get(),
                                   context, &object);
  } else if (module_name == STRING16(L"beta.audio")) {
    CreateModule<GearsAudio>(module_environment_.get(), context, &object);
  } else if (module_name == STRING16(L"beta.audiorecorder")) {
    CreateModule<GearsAudioRecorder>(module_environment_.get(),
                                     context, &object);
#endif  // OFFICIAL_BUILD
#ifdef OFFICIAL_BUILD
  // The Dummy module is not included in official builds.
#else
  } else if (module_name == STRING16(L"beta.dummymodule")) {
    CreateModule<DummyModule>(module_environment_.get(), context, &object);
#endif // OFFICIAL_BUILD
  } else if (module_name == STRING16(L"beta.httprequest")) {
    CreateModule<GearsHttpRequest>(module_environment_.get(),
                                   context, &object);
  } else if (module_name == STRING16(L"beta.timer")) {
    CreateModule<GearsTimer>(module_environment_.get(), context, &object);
  } else if (module_name == STRING16(L"beta.test")) {
#ifdef USING_CCTESTS
    CreateModule<GearsTest>(module_environment_.get(), context, &object);
#else
    context->SetException(STRING16(L"Object is only available in debug build."));
    return;
#endif
  } else {
    context->SetException(STRING16(L"Unknown object."));
    return;
  }

  if (!object) {
    return;  // CreateModule will set the error message.
  }

  context->SetReturnValue(JSPARAM_DISPATCHER_MODULE, object.get());
}

void GearsFactoryImpl::GetBuildInfo(JsCallContext *context) {
  std::string16 build_info;
  AppendBuildInfo(&build_info);
  context->SetReturnValue(JSPARAM_STRING16, &build_info);
}

void GearsFactoryImpl::GetPermission(JsCallContext *context) {
  scoped_ptr<PermissionsDialog::CustomContent> custom_content(
      PermissionsDialog::CreateCustomContent(context));
 
  if (!custom_content.get()) { return; }
 
  bool has_permission = GetPermissionsManager()->AcquirePermission(
      PermissionsDB::PERMISSION_LOCAL_DATA,
      custom_content.get());

  context->SetReturnValue(JSPARAM_BOOL, &has_permission);
}

// Purposely ignores 'is_creation_suspended_'.  The 'hasPermission' property
// indicates whether USER opt-in is still required, not whether DEVELOPER
// methods have been called correctly (e.g. allowCrossOrigin).
void GearsFactoryImpl::GetHasPermission(JsCallContext *context) {
  bool has_permission = GetPermissionsManager()->HasPermission(
      PermissionsDB::PERMISSION_LOCAL_DATA);
  context->SetReturnValue(JSPARAM_BOOL, &has_permission);
}

void GearsFactoryImpl::GetVersion(JsCallContext *context) {
  std::string16 version(STRING16(PRODUCT_VERSION_STRING));
  context->SetReturnValue(JSPARAM_STRING16, &version);
}

// TODO(cprince): See if we can use Suspend/Resume with the opt-in dialog too,
// rather than just the cross-origin worker case.  (Code re-use == good.)
void GearsFactoryImpl::SuspendObjectCreation() {
  is_creation_suspended_ = true;
}

void GearsFactoryImpl::ResumeObjectCreationAndUpdatePermissions() {
  // TODO(cprince): The transition from suspended to resumed is where we should
  // propagate cross-origin opt-in to the permissions DB.
  is_creation_suspended_ = false;
}
