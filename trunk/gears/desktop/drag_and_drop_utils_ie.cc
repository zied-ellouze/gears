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

#include <mshtmdid.h>
#include <shlobj.h>
#include <windows.h>

#include "gears/desktop/drag_and_drop_utils_ie.h"

#include "gears/base/ie/activex_utils.h"
#include "gears/desktop/file_dialog.h"


// This function overwrites buffer in-place.
static HRESULT ResolveShortcut(TCHAR *buffer) {
  static CComPtr<IShellLink> link;
  if (!link && FAILED(link.CoCreateInstance(CLSID_ShellLink))) {
    return E_FAIL;
  }

  CComQIPtr<IPersistFile> file(link);
  if (!file) return E_FAIL;

  // Open the shortcut file, load its content.
  HRESULT hr = file->Load(buffer, STGM_READ);
  if (FAILED(hr)) return hr;

  // Resolve the target of the shortcut.
  hr = link->Resolve(NULL, SLR_NO_UI);
  if (FAILED(hr)) return hr;

  // Read the target path.
  hr = link->GetPath(buffer, MAX_PATH, 0, SLGP_UNCPRIORITY);
  if (FAILED(hr)) return hr;

  return S_OK;
}


HRESULT GetHtmlDataTransfer(
    IHTMLWindow2 *html_window_2,
    CComPtr<IHTMLEventObj> &html_event_obj,
    CComPtr<IHTMLDataTransfer> &html_data_transfer)
{
  HRESULT hr;
  hr = html_window_2->get_event(&html_event_obj);
  if (FAILED(hr)) return hr;
  CComQIPtr<IHTMLEventObj2> html_event_obj_2(html_event_obj);
  if (!html_event_obj_2) return E_FAIL;
  hr = html_event_obj_2->get_dataTransfer(&html_data_transfer);
  return hr;
}


bool GetDroppedFiles(ModuleEnvironment *module_environment,
                     JsArray *files_out,
                     std::string16 *error_out) {
  CComPtr<IHTMLWindow2> html_window_2;
  if (FAILED(ActiveXUtils::GetHtmlWindow2(
          module_environment->iunknown_site_, &html_window_2))) {
    *error_out = STRING16(L"Could not access the IHtmlWindow2.");
    return false;
  }

  CComPtr<IHTMLEventObj> html_event_obj;
  CComPtr<IHTMLDataTransfer> html_data_transfer;
  HRESULT hr = GetHtmlDataTransfer(
      html_window_2, html_event_obj, html_data_transfer);
  if (FAILED(hr)) return false;

  CComPtr<IServiceProvider> service_provider;
  hr = html_data_transfer->QueryInterface(&service_provider);
  if (FAILED(hr)) return false;
  CComPtr<IDataObject> data_object;
  hr = service_provider->QueryService<IDataObject>(
      IID_IDataObject, &data_object);
  if (FAILED(hr)) return false;

  FORMATETC desired_format_etc =
    { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  STGMEDIUM stg_medium;
  hr = data_object->GetData(&desired_format_etc, &stg_medium);
  if (FAILED(hr)) return false;

  HDROP hdrop = static_cast<HDROP>(GlobalLock(stg_medium.hGlobal));
  if (hdrop == NULL) {
    ReleaseStgMedium(&stg_medium);
    return false;
  }

  std::vector<std::string16> filenames;
  UINT num_files = DragQueryFile(hdrop, -1, 0, 0);
  TCHAR buffer[MAX_PATH + 1];
  for (UINT i = 0; i < num_files; i++) {
    DragQueryFile(hdrop, i, buffer, sizeof(buffer));
    SHFILEINFO sh_file_info;
    if (!SHGetFileInfo(buffer, 0, &sh_file_info, sizeof(sh_file_info),
                       SHGFI_ATTRIBUTES) ||
        !(sh_file_info.dwAttributes & SFGAO_FILESYSTEM) ||
        (sh_file_info.dwAttributes & SFGAO_FOLDER)) {
      continue;
    }
    if ((sh_file_info.dwAttributes & SFGAO_LINK) &&
        FAILED(ResolveShortcut(buffer))) {
      continue;
    }
    filenames.push_back(std::string16(buffer));
  }

  GlobalUnlock(stg_medium.hGlobal);
  ReleaseStgMedium(&stg_medium);

  return FileDialog::FilesToJsObjectArray(
      filenames, module_environment, files_out, error_out);
}


void AcceptDrag(ModuleEnvironment *module_environment,
                JsObject *event,
                std::string16 *error_out) {
  // TODO(nigeltao): port the following JavaScript to C++.
  // if (isDragEnterOrDragOver) {
  //   evt.returnValue = false;
  //   evt.cancelBubble = true;
  // }
}


void GetDragData(ModuleEnvironment *module_environment,
                 JsObject *event_as_js_object,
                 JsObject *data_out,
                 std::string16 *error_out) {
  // We ignore event_as_js_object, since Gears can access the IHTMLWindow2
  // and its IHTMLEventObj directly, via COM, and that seems more trustworthy
  // than event_as_js_object, which is supplied by (potentially malicious)
  // JavaScript. Note that, even if the script passes the genuine window.event
  // object, it appears to be different than the one we get from
  // IHTMLWindow2::get_event (different in that, querying both for their
  // IUnknown's gives different pointers).
  CComPtr<IHTMLWindow2> window;
  if (FAILED(ActiveXUtils::GetHtmlWindow2(
          module_environment->iunknown_site_, &window))) {
    *error_out = STRING16(L"Could not access the IHtmlWindow2.");
    return;
  }

  CComPtr<IHTMLEventObj> window_event;
  if (FAILED(window->get_event(&window_event))) {
    *error_out = STRING16(L"Could not access the IHtmlEventObj.");
    return;
  }

  if (!window_event) {
    // If we get here, then there is no window.event, so we are not in
    // the browser's event dispatch.
    *error_out = STRING16(L"The drag-and-drop event is invalid.");
    return;
  }

  CComBSTR type;
  if (FAILED(window_event->get_type(&type))) {
    *error_out = STRING16(L"Could not access the event type.");
    return;
  }

  if (type == CComBSTR(L"drop")) {
    scoped_ptr<JsArray> file_array(
        module_environment->js_runner_->NewArray());
    if (!GetDroppedFiles(module_environment, file_array.get(), error_out)) {
      return;
    }
    data_out->SetPropertyArray(STRING16(L"files"), file_array.get());
    return;

  } else if (type == CComBSTR(L"dragover") ||
             type == CComBSTR(L"dragenter") ||
             type == CComBSTR(L"dragleave")) {
    // TODO(nigeltao): For all DnD events (including the non-drop events
    // dragenter and dragover), we still should provide (1) MIME types,
    // (2) file extensions, (3) total file size, and (4) file count.
    return;

  } else {
    *error_out = STRING16(L"The drag-and-drop event is invalid.");
    return;
  }
}