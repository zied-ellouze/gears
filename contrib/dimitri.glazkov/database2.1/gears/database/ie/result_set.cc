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

#include "gears/base/common/common.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/stopwatch.h"
#include "gears/base/ie/activex_utils.h"
#include "gears/base/ie/atl_headers.h"
#include "gears/database/common/database_utils.h"
#include "gears/database/ie/database.h"
#include "gears/database/ie/result_set.h"
#include "third_party/sqlite_google/preprocessed/sqlite3.h"

GearsResultSet::GearsResultSet() :
    database_(NULL),
    statement_(NULL),
    column_indexes_built_(false),
    is_valid_row_(false) {
}

GearsResultSet::~GearsResultSet() {
  if (statement_) {
    LOG16((L"~GearsResultSet - was NOT closed by caller\n"));
  }

  Finalize();

  if (database_ != NULL) {
    database_->RemoveResultSet(this);
    database_->Release();
    database_ = NULL;
  }
}

bool GearsResultSet::InitializeResultSet(sqlite3_stmt *statement,
                                         GearsDatabase *db,
                                         std::string16 *error_message) {
  assert(statement);
  assert(db);
  assert(error_message);
  statement_ = statement;
  // convention: call next() when the statement is set
  bool succeeded = NextImpl(error_message);
  if (!succeeded || sqlite3_column_count(statement_) == 0) {
    // Either an error occurred or this was a command that does
    // not return a row, so we can just close automatically
    close();
  } else {
    database_ = db;
    database_->AddRef();
    db->AddResultSet(this);
  }
  return succeeded;
}

bool GearsResultSet::Finalize() {
  if (statement_) {
    sqlite3 *db = sqlite3_db_handle(statement_);
    int sql_status = sqlite3_finalize(statement_);
    sql_status = SqlitePoisonIfCorrupt(db, sql_status);
    statement_ = NULL;

    LOG16((L"DB ResultSet Close: %d", sql_status));

    if (sql_status != SQLITE_OK) {
      return false;
    }
  }
  return true;
}

STDMETHODIMP GearsResultSet::field(int index, VARIANT *retval) {
  LOG16((L"GearsResultSet::field(%d)\n", index));
#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif  // DEBUG

  if (statement_ == NULL) {
    RETURN_EXCEPTION(STRING16(L"SQL statement is NULL."));
  }
  if ((index < 0) || (index >= sqlite3_column_count(statement_))) {
    RETURN_EXCEPTION(STRING16(L"Invalid index."));
  }

  VariantClear(retval);
  int column_type = sqlite3_column_type(statement_, index);
  switch (column_type) {
    case SQLITE_INTEGER: {
      sqlite_int64 i64 = sqlite3_column_int64(statement_, index);
      if ((i64 >= INT_MIN) && (i64 <= INT_MAX)) {
        retval->intVal = static_cast<int>(i64);
        retval->vt = VT_INT;
      } else if ((i64 >= 0) && (i64 <= UINT_MAX)) {
        retval->uintVal = static_cast<unsigned int>(i64);
        retval->vt = VT_UINT;
      } else if ((i64 >= JS_INT_MIN) && (i64 <= JS_INT_MAX)) {
        retval->dblVal = static_cast<double>(i64);
        retval->vt = VT_R8;
      } else {
        RETURN_EXCEPTION(STRING16(L"Integer value is out of range."));
      }
      RETURN_NORMAL();
    }
    case SQLITE_FLOAT:
      retval->dblVal = sqlite3_column_double(statement_, index);
      retval->vt = VT_R8;
      RETURN_NORMAL();
    case SQLITE_TEXT: {
      const void *ptr = sqlite3_column_text16(statement_, index);
      // Even though we're supplying an empty string when sqlite says it's
      // NULL, JavaScript considers the field() result to be equal to null.
      // I don't understand why this is, but the behavior conforms to
      // Firefox.
      CComBSTR field_bstr((ptr != NULL)
                          ? static_cast<const wchar_t *>(ptr)
                          : L"");
      field_bstr.CopyTo(retval);
      RETURN_NORMAL();
    }
    case SQLITE_BLOB:
      // TODO(miket): figure out right way to pass around blobs in variants.
      break;
    case SQLITE_NULL:
      retval->vt = VT_NULL;
      RETURN_NORMAL();
    default:
      break;
  }
  RETURN_EXCEPTION(STRING16(L"Data type not supported."));
}

STDMETHODIMP GearsResultSet::fieldByName(const BSTR field_name_in,
                                         VARIANT *retval) {
  const BSTR field_name = ActiveXUtils::SafeBSTR(field_name_in);

#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif  // DEBUG

  LOG16((L"GearsResultSet::fieldByName\n"));
  if (statement_ == NULL) {
    RETURN_EXCEPTION(STRING16(L"SQL statement is NULL."));
  }

  // TODO(miket): refactor into common code. This turned out to be a little
  // difficult to do in STL (the reasonable choice for a default hashtable
  // implementation) because (a) we have exceptions disabled in our projects,
  // and (b) the Win32 Firefox build was trying to pick up the Microsoft STL,
  // which isn't necessarily a bad thing, but it was causing a bunch of
  // weirdness in that build. So I'd like to do a STL conversion at some point
  // and refactor appropriate code into common code, but not right now.
  if (!column_indexes_built_) {
    column_indexes_built_ = true;
    int n = sqlite3_column_count(statement_);
    for (int i = 0; i < n; i++) {
      const void *s = sqlite3_column_name16(statement_, i);
      if (s != NULL) {
        CStringW column_name(static_cast<const wchar_t *>(s));
        column_indexes_[column_name] = i;
      }
    }
  }

  int index = -1;
  bool found = column_indexes_.Lookup(field_name, index);
  if (!found) {
    RETURN_EXCEPTION(STRING16(L"Field name not found."));
  }
  return field(index, retval);
}

STDMETHODIMP GearsResultSet::fieldName(int index, VARIANT *retval) {
  LOG16((L"GearsResultSet::fieldName(%d)\n", index));
#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif  // DEBUG

  if (statement_ == NULL) {
    RETURN_EXCEPTION(STRING16(L"SQL statement is NULL."));
  }

  if ((index < 0) || (index >= sqlite3_column_count(statement_))) {
    RETURN_EXCEPTION(STRING16(L"Invalid index."));
  }

  const void *ptr = sqlite3_column_name16(statement_, index);
  if (ptr != NULL) {
    CComBSTR field_bstr((const wchar_t *)ptr);
    field_bstr.CopyTo(retval);
  } else {
    retval->vt = VT_EMPTY;
  }

  RETURN_NORMAL();
}

STDMETHODIMP GearsResultSet::fieldCount(int *retval) {
  LOG16((L"GearsResultSet::fieldCount()\n"));
#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif  // DEBUG

  // rs.fieldCount() should never throw. Return 0 if there is no statement.
  if (statement_ == NULL) {
    *retval = 0;
  } else {
    *retval = sqlite3_column_count(statement_);
  }
  RETURN_NORMAL();
}

STDMETHODIMP GearsResultSet::close() {
  LOG16((L"GearsResultSet::close()\n"));
#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif  // DEBUG

  if (!Finalize()) {
    RETURN_EXCEPTION(STRING16(L"SQLite finalize() failed."));
  }

  RETURN_NORMAL();
}

STDMETHODIMP GearsResultSet::next() {
  if (!statement_) {
    RETURN_EXCEPTION(STRING16(L"Called Next() with NULL statement."));
  }
  std::string16 error_message;
  if (!NextImpl(&error_message)) {
    LOG16((error_message.c_str()));
    RETURN_EXCEPTION(error_message.c_str());
  }
  RETURN_NORMAL();
}

bool GearsResultSet::NextImpl(std::string16 *error_message) {
#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif  // DEBUG
  assert(statement_);
  assert(error_message);
  int sql_status = sqlite3_step(statement_);
  sql_status = SqlitePoisonIfCorrupt(sqlite3_db_handle(statement_),
                                     sql_status);
  LOG16((L"GearsResultSet::next() sqlite3_step returned %d\n", sql_status));
  switch (sql_status) {
    case SQLITE_ROW:
      is_valid_row_ = true;
      break;
    case SQLITE_BUSY:
      // If there was a timeout (SQLITE_BUSY) the SQL row cursor did not
      // advance, so we don't reset is_valid_row_. If it was valid prior to
      // this call, it's still valid now.
      break;
    default:
      is_valid_row_ = false;
      break;
  }
  bool succeeded = (sql_status == SQLITE_ROW) ||
                   (sql_status == SQLITE_DONE) ||
                   (sql_status == SQLITE_OK);
  if (!succeeded) {
    BuildSqliteErrorString(STRING16(L"Database operation failed."),
                           sql_status, sqlite3_db_handle(statement_),
                           error_message);
  }
  return succeeded;
}

STDMETHODIMP GearsResultSet::isValidRow(VARIANT_BOOL *retval) {
  LOG16((L"GearsResultSet::isValidRow()\n"));

  // rs.isValidRow() should never throw. Return false if there is no statement.
  bool valid = false;
  if (statement_ != NULL) {
    valid = is_valid_row_;
  }
  *retval = (valid ? VARIANT_TRUE : VARIANT_FALSE);
  RETURN_NORMAL();
}
