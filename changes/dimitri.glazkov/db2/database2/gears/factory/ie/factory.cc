// Copyright 2006, Google Inc.
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

#include <assert.h>

#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/string16.h"
#include "gears/base/ie/activex_utils.h"
#include "gears/base/ie/atl_headers.h"
#include "gears/base/ie/detect_version_collision.h"
#include "gears/console/ie/console_ie.h"
#include "gears/database/ie/database.h"
#include "gears/database2/manager.h"
#include "gears/desktop/desktop_ie.h"
#include "gears/factory/common/factory_utils.h"
#include "gears/factory/ie/factory.h"
#include "gears/httprequest/ie/httprequest_ie.h"
#ifdef OFFICIAL_BUILD
// The Image API has not been finalized for official builds
#else
#include "gears/image/ie/image_loader_ie.h"
#endif
#include "gears/localserver/ie/localserver_ie.h"
#include "gears/timer/timer.h"
#include "gears/workerpool/ie/workerpool.h"

#ifdef DEBUG
#include "gears/cctests/test.h"
#endif

GearsFactory::GearsFactory()
    : is_creation_suspended_(false),
      is_permission_granted_(false),
      is_permission_value_from_user_(false) {
  SetActiveUserFlag();
}


STDMETHODIMP GearsFactory::create(const BSTR object_name_bstr_in,
                                  const VARIANT *version_variant,
                                  IDispatch **retval) {
  const BSTR object_name_bstr = ActiveXUtils::SafeBSTR(object_name_bstr_in);

  LOG16((L"GearsFactory::create(%s)\n", object_name_bstr));

  if (DetectedVersionCollision()) {
    if (!EnvIsWorker()) {
      MaybeNotifyUserOfVersionCollision();  // only notifies once per process
    }
    RETURN_EXCEPTION(kVersionCollisionErrorMessage);
  }

  // Make sure the user gives this origin permission to use Gears.

  if (!HasPermissionToUseGears(this, NULL, NULL, NULL)) {
    RETURN_EXCEPTION(STRING16(L"Page does not have permission to use "
                              PRODUCT_FRIENDLY_NAME L"."));
  }

  // Check the version string.

  std::string16 version;
  if (!ActiveXUtils::OptionalVariantIsPresent(version_variant)) {
    version = STRING16(L"1.0");  // default value for this optional param
  } else {
    if (version_variant->vt != VT_BSTR) {
      RETURN_EXCEPTION(STRING16(L"Invalid parameter."));
    }
    version = version_variant->bstrVal;
  }

  if (version != kAllowedClassVersion) {
    RETURN_EXCEPTION(STRING16(L"Invalid version string. Must be 1.0."));
  }

  // Create an instance of the object.
  //
  // Do case-sensitive comparisons, which are always better in APIs. They make
  // code consistent across callers, and they are easier to support over time.

  std::string16 object_name(object_name_bstr);
  std::string16 error;
  bool success = false;

  // Try creating a dispatcher-based module first.
  success = CreateDispatcherModule(object_name, retval, &error);
  if (success) {
    RETURN_NORMAL();
  } else if (error.length() > 0) {
    RETURN_EXCEPTION(error.c_str());
  }

  success = CreateComModule(object_name, retval, &error);
  if (success) {
    RETURN_NORMAL();
  } else if (error.length() > 0) {
    RETURN_EXCEPTION(error.c_str());
  } else {
    RETURN_EXCEPTION(STRING16(L"Unknown object."));
  }
}

bool GearsFactory::CreateDispatcherModule(const std::string16 &object_name,
                                          IDispatch **retval,
                                          std::string16 *error) {
  GComPtr<ModuleImplBaseClass> object(NULL);

  if (object_name == STRING16(L"beta.test")) {
#ifdef DEBUG
    object.reset(CreateModule<GearsTest>(GetJsRunner()));
#else
    *error = STRING16(L"Object is only available in debug build.");
    return false;
#endif
	} else if (object_name == STRING16(L"beta.databasemanager")) {
		object.reset(CreateModule<Database2Manager>(GetJsRunner()));
  } else {
    // Don't return an error here. Caller handles reporting unknown modules.
    error->clear();
    return false;
  }

  // setup the ModuleImplBaseClass (copy settings from this factory)
  if (!object->InitBaseFromSibling(this)) {
    *error = STRING16(L"Error initializing base class.");
    return false;
  }

  *retval = object->GetWrapperToken().pdispVal;
  object.ReleaseNewObjectToScript();
  return true;
}

bool GearsFactory::CreateComModule(const std::string16 &object_name,
                                   IDispatch **retval, std::string16 *error) {
  ModuleImplBaseClass *base_class = NULL;
  CComQIPtr<IDispatch> idispatch;

  HRESULT hr = E_FAIL;
  if (0) {  // dummy Database2Statement to support mixed "#ifdef" and "else if" below
#ifdef WINCE
  // TODO(oshlack): Implement console for WinCE.
#else
  } else if (object_name == STRING16(L"beta.console")) {
    CComObject<GearsConsole> *obj;
    hr = CComObject<GearsConsole>::CreateInstance(&obj);
    base_class = obj;
    idispatch = obj;
#endif
  } else if (object_name == STRING16(L"beta.database")) {
    CComObject<GearsDatabase> *obj;
    hr = CComObject<GearsDatabase>::CreateInstance(&obj);
    base_class = obj;
    idispatch = obj;
#ifdef WINCE
  // TODO(steveblock): Implement desktop for WinCE.
#else
  } else if (object_name == STRING16(L"beta.desktop")) {
    CComObject<GearsDesktop> *obj;
    hr = CComObject<GearsDesktop>::CreateInstance(&obj);
    base_class = obj;
    idispatch = obj;
#endif
  } else if (object_name == STRING16(L"beta.httprequest")) {
    CComObject<GearsHttpRequest> *obj;
    hr = CComObject<GearsHttpRequest>::CreateInstance(&obj);
    base_class = obj;
    idispatch = obj;
#ifdef OFFICIAL_BUILD
// The Image API has not been finalized for official builds
#else
  } else if (object_name == STRING16(L"beta.imageloader")) {
    CComObject<GearsImageLoader> *obj;
    hr = CComObject<GearsImageLoader>::CreateInstance(&obj);
    base_class = obj;
    idispatch = obj;
#endif
  } else if (object_name == STRING16(L"beta.localserver")) {
    CComObject<GearsLocalServer> *obj;
    hr = CComObject<GearsLocalServer>::CreateInstance(&obj);
    base_class = obj;
    idispatch = obj;
  } else if (object_name == STRING16(L"beta.timer")) {
    CComObject<GearsTimer> *obj;
    hr = CComObject<GearsTimer>::CreateInstance(&obj);
    base_class = obj;
    idispatch = obj;
  } else if (object_name == STRING16(L"beta.workerpool")) {
    CComObject<GearsWorkerPool> *obj;
    hr = CComObject<GearsWorkerPool>::CreateInstance(&obj);
    base_class = obj;
    idispatch = obj;
  } else {
    // Don't return an error here. Caller handles reporting unknown modules.
    error->clear();
    return false;
  }

  if (FAILED(hr) || !base_class || !idispatch) {
    *error = STRING16(L"Failed to create requested object.");
    return false;
  }

  // setup the ModuleImplBaseClass (copy settings from this factory)
  if (!base_class->InitBaseFromSibling(this)) {
    *error = STRING16(L"Error initializing base class.");
    return false;
  }

  // Our factory-created Gears objects no longer need to be sited, since we get
  // the domain and location_url from ModuleImplBaseClass.  But we may someday
  // later need an object to be sited, in which case we should enable this code.
  //
  //// SetSite any object that supports IObjectWithSite
  //if (SUCCEEDED(hr)) {
  //  CComQIPtr<IObjectWithSite> object_with_site = idispatch;
  //  if (object_with_site) {
  //    hr = object_with_site->SetSite(m_spUnkSite);
  //  }
  //}

  *retval = idispatch;
  (*retval)->AddRef();  // ~CComQIPtr will Release, so must AddRef here
  assert((*retval)->AddRef() == 3 &&
         (*retval)->Release() == 2);
  return true;
}


STDMETHODIMP GearsFactory::getBuildInfo(BSTR *retval) {
  LOG16((L"GearsFactory::getBuildInfo()\n"));
  std::string16 build_info;
  AppendBuildInfo(&build_info);
  if (DetectedVersionCollision()) {
    build_info += L" (pending browser restart)";
  }

  *retval = SysAllocString(build_info.c_str());
  if (*retval == NULL) {
    RETURN_EXCEPTION(STRING16(L"Internal error."));
  }
  RETURN_NORMAL();
}


STDMETHODIMP GearsFactory::get_version(BSTR *retval) {
  *retval = SysAllocString(STRING16(PRODUCT_VERSION_STRING));
  if (*retval == NULL) {
    RETURN_EXCEPTION(STRING16(L"Internal error."));
  }
  RETURN_NORMAL();
}


#ifdef WINCE
// Hold WinCE feature set at version 0.2 for now.
#else
STDMETHODIMP GearsFactory::getPermission(const VARIANT *site_name_in,
                                         const VARIANT *image_url_in,
                                         const VARIANT *extra_message_in,
                                         VARIANT_BOOL *retval) {
  // Guard against NULL BSTRs.
  // TODO(cprince): Do this automatically in JsParamFetcher for IE.
  std::string16 site_name;
  std::string16 image_url;
  std::string16 extra_message;

  if (ActiveXUtils::OptionalVariantIsPresent(site_name_in)) {
    if (site_name_in->vt == VT_BSTR) {
      site_name.assign(site_name_in->bstrVal);
    } else {
      RETURN_EXCEPTION(STRING16(L"siteName must be a string."));
    }
  }

  if (ActiveXUtils::OptionalVariantIsPresent(image_url_in)) {
    if (image_url_in->vt == VT_BSTR) {
      image_url.assign(image_url_in->bstrVal);
    } else {
      RETURN_EXCEPTION(STRING16(L"imageUrl must be a string."));
    }
  }

  if (ActiveXUtils::OptionalVariantIsPresent(extra_message_in)) {
    if (extra_message_in->vt == VT_BSTR) {
      extra_message.assign(extra_message_in->bstrVal);
    } else {
      RETURN_EXCEPTION(STRING16(L"extraMessage must be a string."));
    }
  }

  if (HasPermissionToUseGears(this, image_url.c_str(), site_name.c_str(),
                              extra_message.c_str())) {
    *retval = VARIANT_TRUE;
  } else {
    *retval = VARIANT_FALSE;
  }
  RETURN_NORMAL();
}


// Purposely ignores 'is_creation_suspended_'.  The 'hasPermission' property
// indicates whether USER opt-in is still required, not whether DEVELOPER
// methods have been called correctly (e.g. allowCrossOrigin).
STDMETHODIMP GearsFactory::get_hasPermission(VARIANT_BOOL *retval) {
  *retval = is_permission_granted_ ? VARIANT_TRUE : VARIANT_FALSE;
  RETURN_NORMAL();
}
#endif


// InitBaseFromDOM needs the object to be sited.  We override SetSite just to
// know when this happens, so we can do some one-time setup afterward.
STDMETHODIMP GearsFactory::SetSite(IUnknown *site) {
  HRESULT hr = IObjectWithSiteImpl<GearsFactory>::SetSite(site);
#ifdef WINCE
  // We are unable to get IWebBrowser2 from this pointer. Instead, the user must
  // call privateSetGlobalObject from JavaScript.
#else
  InitBaseFromDOM(m_spUnkSite);
#endif
  return hr;
}

#ifdef WINCE
STDMETHODIMP GearsFactory::privateSetGlobalObject(IDispatch *js_dispatch) {
  return InitBaseFromDOM(js_dispatch);
}
#endif

// TODO(cprince): See if we can use Suspend/Resume with the opt-in dialog too,
// rather than just the cross-origin worker case.  (Code re-use == good.)
void GearsFactory::SuspendObjectCreation() {
  is_creation_suspended_ = true;
}

void GearsFactory::ResumeObjectCreationAndUpdatePermissions() {
  // TODO(cprince): The transition from suspended to resumed is where we should
  // propagate cross-origin opt-in to the permissions DB.
  is_creation_suspended_ = false;
}
