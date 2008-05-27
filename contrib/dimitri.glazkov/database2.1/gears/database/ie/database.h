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

#ifndef GEARS_DATABASE_IE_DATABASE_H__
#define GEARS_DATABASE_IE_DATABASE_H__

#include <set>
#include "third_party/scoped_ptr/scoped_ptr.h"

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/stopwatch.h"
#include "genfiles/interfaces.h"

struct sqlite3;
struct sqlite3_stmt;

class ATL_NO_VTABLE GearsDatabase
    : public ModuleImplBaseClass,
      public CComObjectRootEx<CComMultiThreadModel>,
      public CComCoClass<GearsDatabase>,
      public IDispatchImpl<GearsDatabaseInterface>,
      public JsEventHandlerInterface {
 public:
  BEGIN_COM_MAP(GearsDatabase)
    COM_INTERFACE_ENTRY(GearsDatabaseInterface)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()

  DECLARE_NOT_AGGREGATABLE(GearsDatabase)
  DECLARE_PROTECT_FINAL_CONSTRUCT()
  // End boilerplate code. Begin interface.

  // need a default constructor to CreateInstance objects in IE
  GearsDatabase();
  ~GearsDatabase();

  STDMETHOD(open)(const VARIANT *database_name);
  STDMETHOD(execute)(const BSTR expr, const VARIANT *arg_array,
                     GearsResultSetInterface **retval);
  STDMETHOD(close)();

  STDMETHOD(get_lastInsertRowId)(VARIANT *retval);
  STDMETHOD(get_rowsAffected)(VARIANT *retval);

  void HandleEvent(JsEventType event_type);

// Right now this is just used for testing perf. If we ever want to make it a
// real feature of Gears, then it will need to keep separate stopwatches for
// each database file, not a single stopwatch for the entire process as it does
// now.
#ifdef DEBUG
  STDMETHOD(get_executeMsec)(int *retval);
  static Stopwatch g_stopwatch_;
#endif

  friend class GearsResultSet;

 private:
  HRESULT BindArgsToStatement(const VARIANT *args, sqlite3_stmt *stmt);
  HRESULT BindArg(const CComVariant &arg, int arg_index, sqlite3_stmt *stmt);

  void AddResultSet(GearsResultSet *rs);
  void RemoveResultSet(GearsResultSet *rs);
  bool CloseInternal();

  sqlite3 *db_;
  std::set<GearsResultSet *> result_sets_;
  scoped_ptr<JsEventMonitor> unload_monitor_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsDatabase);
};


#endif  // GEARS_DATABASE_IE_DATABASE_H__
