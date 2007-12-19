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

#ifndef GEARS_LOCALSERVER_FIREFOX_ASYNC_TASK_FF_H__
#define GEARS_LOCALSERVER_FIREFOX_ASYNC_TASK_FF_H__

#include <vector>
#include "gears/base/common/common.h"
#include "gears/localserver/common/critical_section.h"
#include "gears/localserver/common/http_request.h"
#include "gears/localserver/common/localserver_db.h"

class nsIEventQueue;

//------------------------------------------------------------------------------
// AsyncTask
//------------------------------------------------------------------------------
class AsyncTask : protected HttpRequest::ReadyStateListener {
 public:
  // Starts a worker thread which will call the Run method
  bool Start();

  // Gracefully aborts a worker thread that was previously started
  void Abort();

  // Instructs the AsyncTask to delete itself when the worker thread terminates
  void DeleteWhenDone();

  // Firefox specific API
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

  AsyncTask();
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
               const char16 *if_mod_since_date,
               const char16 *required_cookie,
               WebCacheDB::PayloadInfo *payload,
               bool *was_redirected,
               std::string16 *full_redirect_url,
               std::string16 *error_message);

  CriticalSection lock_;
  bool is_aborted_;
  bool is_initialized_;

 private:
  struct HttpRequestParameters;

  static const int kStartHttpGetMessageCode = -1;
  static const int kAbortHttpGetMessageCode = -2;

  // worker thread methods

  static void ThreadEntry(void *self);
  nsresult CallAsync(nsIEventQueue *event_queue, int msg_code, void *msg_param);

  // listener and main thread methods

  void OnAsyncCall(int msg_code, void *msg_param);
  void OnAbortHttpGet();
  bool OnStartHttpGet();
  void OnListenerEvent(int msg_code, int msg_param);
  void ReadyStateChanged(HttpRequest *source);

  // Returns true if the currently executing thread is our listener thread
  bool IsListenerThread() { 
    return listener_thread_ == PR_GetCurrentThread();
  }

  // Returns true if the currently executing thread is our task thread
  bool IsTaskThread() {
    return thread_ == PR_GetCurrentThread();
  }

  void AddReference();
  void RemoveReference();

  bool delete_when_done_;
  Listener *listener_;
  PRThread *thread_;
  PRThread *listener_thread_;
  nsCOMPtr<nsIEventQueue> listener_event_queue_;
  nsCOMPtr<nsIEventQueue> ui_event_queue_;
  ScopedHttpRequestPtr http_request_;
  HttpRequestParameters *params_;
  int refcount_;

  struct AsyncCallEvent;
  static void *PR_CALLBACK AsyncCall_EventHandlerFunc(AsyncCallEvent*);
  static void  PR_CALLBACK AsyncCall_EventCleanupFunc(AsyncCallEvent*);
};

#endif  // GEARS_LOCALSERVER_FIREFOX_ASYNC_TASK_FF_H__
