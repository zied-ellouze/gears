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

#ifndef GEARS_LOCALSERVER_IE_ASYNC_TASK_IE_H__
#define GEARS_LOCALSERVER_IE_ASYNC_TASK_IE_H__

// This is a hack because something in the ATL headers prevents the gecko SDK
// from defining int32, so we include it first.
#include "gears/base/common/basictypes.h"

#include <atlsync.h>
#include <vector>
#include "gears/base/common/browsing_context.h"
#include "gears/base/common/message_queue.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/string16.h"
#include "gears/base/ie/atl_headers.h" // include this before other ATL headers
#include "gears/localserver/common/critical_section.h"
#include "gears/localserver/common/http_request.h"
#include "gears/localserver/common/resource_store.h"

class BlobInterface;

// TODO(mpcomplete): this should use a cross-platform thread abstraction, when
// we have it.

//------------------------------------------------------------------------------
// AsyncTask
//------------------------------------------------------------------------------
class AsyncTask : protected HttpRequest::HttpListener,
                  private RefCounted {
 public:
  // Starts a worker thread which will call the Run method
  bool Start();

  // Gracefully aborts a worker thread that was previously started
  void Abort();

  // Instructs the AsyncTask to delete itself when the worker thread terminates
  void DeleteWhenDone();

  // NPAPI specific API
  // Sets where notification messages will be sent. Notifications will be
  // delivered on thread of control that the AsyncTask is initialized on.
  // Firefox specific API utilized by clients of derived classes
  class Listener {
   public:
    virtual void HandleEvent(int msg_code,
                             int msg_param,
                             AsyncTask *source) = 0;
  };
  void SetListener(Listener* listener);

 protected:
  // Members that derived classes in common code depend on
  // TODO(michaeln): perhaps define these in a interface IAsyncTask

  static const char16 *kCookieRequiredErrorMessage;

  static const int kAbortMessageCode = -1;

  AsyncTask(BrowsingContext *browsing_context);
  virtual ~AsyncTask();

  bool Init();
  virtual void Run() = 0;

  // Posts a message to our listener
  void NotifyListener(int code, int param);

  // Synchronously fetches a url on the worker thread.  This should only
  // be called on the Run method's thread of control.
  //
  // If 'is_capturing' is true, the custom "X-Gears-Google" header is
  // added, and redirects will not be followed, rather the 30x response
  // will be directly returned.
  //
  // If 'if_mode_since_date' is not null and not an empty string, the
  // request will be a conditional HTTP GET.
  //
  // If 'required_cookie' is not null or empty, the presense of the
  // cookie is tested prior to issueing the request. If the test fails
  // the return value is false and 'error_message' will contain
  // kCookieRequiredErrorMessage.
  //
  // Upon successful return, the 'payload' structure will contain the
  // response. If redirection was followed, 'was_redirected' will be set
  // to true, 'full_redirect_url' will contain the final location in
  // the chain of redirects, and the 'payload' will contain the response
  // for the final location. The 'was_redirected', 'full_redirect_url',
  // and 'error_message' parameters may be null.
  //
  // If Abort() is called from another thread of control while a request
  // is pending, the request is cancelled and HttpGet will return
  // shortly thereafter.
  bool HttpGet(const char16 *full_url,
               bool is_capturing,
               const char16 *reason_header_value,
               const char16 *if_mod_since_date,
               const char16 *required_cookie,
               WebCacheDB::PayloadInfo *payload,
               bool *was_redirected,
               std::string16 *full_redirect_url,
               std::string16 *error_message);

  #ifdef OFFICIAL_BUILD
  // The Blob API has not yet been finalized for official builds.
#else
  // As HttpGet, but for POST.
  bool HttpPost(const char16 *full_url,
                bool is_capturing,
                const char16 *reason_header_value,
                const char16 *if_mod_since_date,
                const char16 *required_cookie,
                BlobInterface *post_body,
                WebCacheDB::PayloadInfo *payload,
                bool *was_redirected,
                std::string16 *full_redirect_url,
                std::string16 *error_message);
#endif

  CriticalSection lock_;
  bool is_aborted_;
  bool is_initialized_;
  scoped_refptr<BrowsingContext> browsing_context_;

 private:
  // Implementation of HttpGet and HttpPost.
  bool MakeHttpRequest(const char16 *method,
                       const char16 *full_url,
                       bool is_capturing,
                       const char16 *reason_header_value,
                       const char16 *if_mod_since_date,
                       const char16 *required_cookie,
#ifdef OFFICIAL_BUILD
                       // The Blob API has not yet been finalized for official
                       // builds.
#else
                       BlobInterface *post_body,
#endif
                       WebCacheDB::PayloadInfo *payload,
                       bool *was_redirected,
                       std::string16 *full_redirect_url,
                       std::string16 *error_message);

  friend struct AsyncTaskFunctor;

  // An HttpRequest listener callback
  void ReadyStateChanged(HttpRequest *source);

  static unsigned int _stdcall ThreadMain(void *self);
  void CallAsync(ThreadId thread_id, int code, int param);
  void HandleAsync(int code, int param);

  // Returns true if the currently executing thread is our listener thread
  bool IsListenerThread() { 
    return listener_thread_id_ ==
      ThreadMessageQueue::GetInstance()->GetCurrentThreadId();
  }

  // Returns true if the currently executing thread is our task thread
  bool IsTaskThread() {
    return task_thread_id_ ==
      ThreadMessageQueue::GetInstance()->GetCurrentThreadId();
  }

  bool delete_when_done_;
  Listener *listener_;
  HANDLE thread_;
  bool ready_state_changed_signalled_;
  bool abort_signalled_;
  ThreadId listener_thread_id_;
  ThreadId task_thread_id_;
};

#endif  // GEARS_LOCALSERVER_IE_ASYNC_TASK_IE_H__