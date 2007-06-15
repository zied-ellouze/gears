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
#include "gears/base/common/common.h"
#include "gears/base/ie/atl_headers.h"


//------------------------------------------------------------------------------
// GetCurrentTimeMillis
//------------------------------------------------------------------------------
int64 GetCurrentTimeMillis() {
  // The FILETIME structure is a 64-bit value representing the 
  // number of 100-nanosecond intervals since January 1, 1601 (UTC).
  // We offset to our epoch (January 1, 1970 GMT) and convert to msecs.
  const int64 kOffset = 116444736000000000i64;
  assert(sizeof(int64) == sizeof(FILETIME));
  int64 filetime;
  GetSystemTimeAsFileTime(reinterpret_cast<LPFILETIME>(&filetime));
  return (filetime - kOffset) / 10000i64;
}

//------------------------------------------------------------------------------
// ResolveRelativeUrl
//------------------------------------------------------------------------------
bool ResolveRelativeUrl(const char16 *base, const char16 *url,
                        std::string16 *resolved) {
  CComPtr<IMoniker> base_moniker;
  HRESULT hr = CreateURLMonikerEx(NULL, base, &base_moniker, URL_MK_UNIFORM);
  if (FAILED(hr)) {
    return false;
  }

  CComPtr<IMoniker> url_moniker;
  hr = CreateURLMonikerEx(base_moniker, url, &url_moniker, URL_MK_UNIFORM);
  if (FAILED(hr)) {
    return false;
  }

  LPOLESTR displayname;
  hr = url_moniker->GetDisplayName(NULL, NULL, &displayname);
  if (FAILED(hr)) {
    return false;
  }

  wchar_t *hash = wcschr(displayname, L'#');
  if (hash) {
    *hash = 0;
  }

  *resolved = displayname;
  CComAllocator::Free(displayname);
  return true;
}
