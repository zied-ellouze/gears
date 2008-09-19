// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// This file is intended to be included in another C++ file where the character
// types are defined. This allows us to write mostly generic code, but not have
// templace bloat because everything is inlined when anybody calls any of our
// functions.

#ifndef GOOGLEURL_SRC_URL_CANON_INTERNAL_H__
#define GOOGLEURL_SRC_URL_CANON_INTERNAL_H__
#ifdef OS_SYMBIAN
#include <assert.h>
#include <e32std.h> // for TLex
#endif
#include <stdlib.h>
#include <unicode/utf.h>

#include "googleurl/src/url_canon.h"

namespace url_canon {

// Bits that identify different character types. These types identify different
// bits that are set for each 8-bit character in the kSharedCharTypeTable.
enum SharedCharTypes {
  // Characters that do not require escaping in queries. Characters that do
  // not have this flag will be escaped, see url_canon_query.cc
  CHAR_QUERY = 1,

  // Valid in a IPv4 address (digits plus dot and 'x' for hex).
  CHAR_IPV4 = 2,

  // Valid in an ASCII-representation of a hex digit (as in %-escaped).
  CHAR_HEX = 4,

  // Valid in an ASCII-representation of a decimal digit.
  CHAR_DEC = 8,

  // Valid in an ASCII-representation of an octal digit.
  CHAR_OCT = 16,
};

// This table contains the flags in SharedCharTypes for each 8-bit character.
// Some canonicalization functions have their own specialized lookup table.
// For those with simple requirements, we have collected the flags in one
// place so there are fewer lookup tables to load into the CPU cache.
//
// Using an unsigned char type has a small but measurable performance benefit
// over using a 32-bit number.
extern const unsigned char kSharedCharTypeTable[0x100];

// More readable wrappers around the character type lookup table.
inline bool IsCharOfType(unsigned char c, SharedCharTypes type) {
  return !!(kSharedCharTypeTable[c] & type);
}
inline bool IsQueryChar(unsigned char c) {
  return IsCharOfType(c, CHAR_QUERY);
}
inline bool IsIPv4Char(unsigned char c) {
  return IsCharOfType(c, CHAR_IPV4);
}
inline bool IsHexChar(unsigned char c) {
  return IsCharOfType(c, CHAR_HEX);
}

// Maps the hex numerical values 0x0 to 0xf to the corresponding ASCII digit
// that will be used to represent it.
extern const char kHexCharLookup[0x10];

// This lookup table allows fast conversion between ASCII hex letters and their
// corresponding numerical value. The 8-bit range is divided up into 8
// regions of 0x20 characters each. Each of the three character types (numbers,
// uppercase, lowercase) falls into different regions of this range. The table
// contains the amount to subtract from characters in that range to get at
// the corresponding numerical value.
//
// See HexDigitToValue for the lookup.
extern const char kCharToHexLookup[8];

// Assumes the input is a valid hex digit! Call IsHexChar before using this.
inline unsigned char HexCharToValue(unsigned char c) {
  return c - kCharToHexLookup[c / 0x20];
}

// Indicates if the given character is a dot or dot equivalent, returning the
// number of characters taken by it. This will be one for a literal dot, 3 for
// an escaped dot. If the character is not a dot, this will return 0.
template<typename CHAR>
inline int IsDot(const CHAR* spec, int offset, int end) {
  if (spec[offset] == '.') {
    return 1;
  } else if (spec[offset] == '%' && offset + 3 <= end &&
             spec[offset + 1] == '2' &&
             (spec[offset + 2] == 'e' || spec[offset + 2] == 'E')) {
    // Found "%2e"
    return 3;
  }
  return 0;
}

// Returns the canonicalized version of the input character according to scheme
// rules. This is implemented alongside the scheme canonicalizer, and is
// required for relative URL resolving to test for scheme equality.
//
// Returns 0 if the input character is not a valid scheme character.
char CanonicalSchemeChar(UTF16Char ch);

// Write a single character, escaped, to the output. This always escapes: it
// does no checking that thee character requires escaping.
inline void AppendEscapedChar(unsigned char ch,
                              CanonOutput* output) {
  output->push_back('%');
  output->push_back(kHexCharLookup[ch >> 4]);
  output->push_back(kHexCharLookup[ch & 0xf]);
}

// The character we'll substitute for undecodable or invalid characters.
extern const UTF16Char kUnicodeReplacementCharacter;

// UTF-8 functions ------------------------------------------------------------

// Reads one character in UTF-8 starting at |*begin| in |str| and places
// the decoded value into |*code_point|. If the character is valid, we will
// return true. If invalid, we'll return false and put the
// kUnicodeReplacementCharacter into |*code_point|.
//
// |*begin| will be updated to point to the last character consumed so it
// can be incremented in a loop and will be ready for the next character.
// (for a single-byte ASCII character, it will not be changed).
inline bool ReadUTFChar(const char* str, int* begin, int length,
                        unsigned* code_point) {
  U8_NEXT(str, *begin, length, *code_point);

  // The ICU macro above moves to the next char, we want to point to the last
  // char consumed.
  (*begin)--;

  // Validate the decoded value.
  if (U_IS_UNICODE_CHAR(*code_point))
    return true;
  *code_point = kUnicodeReplacementCharacter;
  return false;
}

// Generic To-UTF-8 converter. This will call the given append method for each
// character that should be appended, with the given output method. Wrappers
// are provided below for escaped and non-escaped versions of this.
template<class Output, void Appender(unsigned char, Output*)>
inline void DoAppendUTF8(unsigned char_value, Output* output) {
  if (char_value <= 0x7f) {
    Appender(static_cast<unsigned char>(char_value), output);
  } else if (char_value <= 0x7ff) {
    // 110xxxxx 10xxxxxx
    Appender(static_cast<unsigned char>(0xC0 | (char_value >> 6)),
             output);
    Appender(static_cast<unsigned char>(0x80 | (char_value & 0x3f)),
             output);
  } else if (char_value <= 0xffff) {
    // 1110xxxx 10xxxxxx 10xxxxxx
    Appender(static_cast<unsigned char>(0xe0 | (char_value >> 12)),
             output);
    Appender(static_cast<unsigned char>(0x80 | ((char_value >> 6) & 0x3f)),
             output);
    Appender(static_cast<unsigned char>(0x80 | (char_value & 0x3f)),
             output);
  } else if (char_value <= 0x1fffff) {
    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    Appender(static_cast<unsigned char>(0xf0 | (char_value >> 18)),
             output);
    Appender(static_cast<unsigned char>(0x80 | ((char_value >> 12) & 0x3f)),
             output);
    Appender(static_cast<unsigned char>(0x80 | ((char_value >> 6) & 0x3f)),
             output);
    Appender(static_cast<unsigned char>(0x80 | (char_value & 0x3f)),
             output);
  } else if (char_value <= 0x10FFFF) {  // Max unicode code point.
    // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    Appender(static_cast<unsigned char>(0xf8 | (char_value >> 24)),
             output);
    Appender(static_cast<unsigned char>(0x80 | ((char_value >> 18) & 0x3f)),
             output);
    Appender(static_cast<unsigned char>(0x80 | ((char_value >> 12) & 0x3f)),
             output);
    Appender(static_cast<unsigned char>(0x80 | ((char_value >> 6) & 0x3f)),
             output);
    Appender(static_cast<unsigned char>(0x80 | (char_value & 0x3f)),
             output);
  } else {
    // Invalid UTF-8 character (>20 bits)
  }
}

// Helper used by AppendUTF8Value below. We use an unsigned parameter so there
// are no funny sign problems with the input, but then have to convert it to
// a regular char for appending.
inline void AppendCharToOutput(unsigned char ch, CanonOutput* output) {
  output->push_back(static_cast<char>(ch));
}

// Writes the given character to the output as UTF-8. This does NO checking
// of the validity of the unicode characters; the caller should ensure that
// the value it is appending is valid to append.
inline void AppendUTF8Value(unsigned char_value, CanonOutput* output) {
  DoAppendUTF8<CanonOutput, AppendCharToOutput>(char_value, output);
}

// Writes the given character to the output as UTF-8, escaping ALL
// characters (even when they are ASCII). This does NO checking of the
// validity of the unicode characters; the caller should ensure that the value
// it is appending is valid to append.
inline void AppendUTF8EscapedValue(unsigned char_value, CanonOutput* output) {
  DoAppendUTF8<CanonOutput, AppendEscapedChar>(char_value, output);
}

// UTF-16 functions -----------------------------------------------------------

// Reads one character in UTF-16 starting at |*begin| in |str| and places
// the decoded value into |*code_point|. If the character is valid, we will
// return true. If invalid, we'll return false and put the
// kUnicodeReplacementCharacter into |*code_point|.
//
// |*begin| will be updated to point to the last character consumed so it
// can be incremented in a loop and will be ready for the next character.
// (for a single-16-bit-word character, it will not be changed).
inline bool ReadUTFChar(const UTF16Char* str, int* begin, int length,
                        unsigned* code_point) {
  if (U16_IS_SURROGATE(str[*begin])) {
    if (!U16_IS_SURROGATE_LEAD(str[*begin]) || *begin + 1 >= length ||
        !U16_IS_TRAIL(str[*begin + 1])) {
      // Invalid surrogate pair.
      *code_point = kUnicodeReplacementCharacter;
      return false;
    } else {
      // Valid surrogate pair.
      *code_point = U16_GET_SUPPLEMENTARY(str[*begin], str[*begin + 1]);
      (*begin)++;
    }
  } else {
    // Not a surrogate, just one 16-bit word.
    *code_point = str[*begin];
  }

  if (U_IS_UNICODE_CHAR(*code_point))
    return true;

  // Invalid code point.
  *code_point = kUnicodeReplacementCharacter;
  return false;
}

// Equivalent to U16_APPEND_UNSAFE in ICU but uses our output method.
inline void AppendUTF16Value(unsigned code_point,
                             CanonOutputT<UTF16Char>* output) {
  if (code_point > 0xffff) {
    output->push_back(static_cast<UTF16Char>((code_point >> 10) + 0xd7c0));
    output->push_back(static_cast<UTF16Char>((code_point & 0x3ff) | 0xdc00));
  } else {
    output->push_back(static_cast<UTF16Char>(code_point));
  }
}

// Escaping functions ---------------------------------------------------------

// Writes the given character to the output as UTF-8, escaped. Call this
// function only when the input is wide. Returns true on success. Failure
// means there was some problem with the encoding, we'll still try to
// update the |*begin| pointer and add a placeholder character to the
// output so processing can continue.
//
// We will append the character starting at ch[begin] with the buffer ch
// being |length|. |*begin| will be updated to point to the last character
// consumed (we may consume more than one for UTF-16) so that if called in
// a loop, incrementing the pointer will move to the next character.
//
// Every single output character will be escaped. This means that if you
// give it an ASCII character as input, it will be escaped. Some code uses
// this when it knows that a character is invalid according to its rules
// for validity. If you don't want escaping for ASCII characters, you will
// have to filter them out prior to calling this function.
//
// Assumes that ch[begin] is within range in the array, but does not assume
// that any following characters are.
inline bool AppendUTF8EscapedChar(const UTF16Char* str, int* begin, int length,
                                  CanonOutput* output) {
  // UTF-16 input. ReadUTF16Char will handle invalid characters for us and give
  // us the kUnicodeReplacementCharacter, so we don't have to do special
  // checking after failure, just pass through the failure to the caller.
  unsigned char_value;
  bool success = ReadUTFChar(str, begin, length, &char_value);
  AppendUTF8EscapedValue(char_value, output);
  return success;
}

// Handles UTF-8 input. See the wide version above for usage.
inline bool AppendUTF8EscapedChar(const char* str, int* begin, int length,
                                  CanonOutput* output) {
  // ReadUTF8Char will handle invalid characters for us and give us the
  // kUnicodeReplacementCharacter, so we don't have to do special checking
  // after failure, just pass through the failure to the caller.
  unsigned ch;
  bool success = ReadUTFChar(str, begin, length, &ch);
  AppendUTF8EscapedValue(ch, output);
  return success;
}

// Given a '%' character at |*begin| in the string |spec|, this will decode
// the escaped value and put it into |*unescaped_value| on success (returns
// true). On failure, this will return false, and will not write into
// |*unescaped_value|.
//
// |*begin| will be updated to point to the last character of the escape
// sequence so that when called with the index of a for loop, the next time
// through it will point to the next character to be considered. On failure,
// |*begin| will be unchanged.
inline bool Is8BitChar(char c) {
  return true;  // this case is specialized to avoid a warning
}
inline bool Is8BitChar(UTF16Char c) {
  return c <= 255;
}

template<typename CHAR>
inline bool DecodeEscaped(const CHAR* spec, int* begin, int end,
                          char* unescaped_value) {
  if (*begin + 3 > end ||
      !Is8BitChar(spec[*begin + 1]) || !Is8BitChar(spec[*begin + 2])) {
    // Invalid escape sequence because there's not enough room, or the
    // digits are not ASCII.
    return false;
  }

  unsigned char first = static_cast<unsigned char>(spec[*begin + 1]);
  unsigned char second = static_cast<unsigned char>(spec[*begin + 2]);
  if (!IsHexChar(first) || !IsHexChar(second)) {
    // Invalid hex digits, fail.
    return false;
  }

  // Valid escape sequence.
  *unescaped_value = (HexCharToValue(first) << 4) + HexCharToValue(second);
  *begin += 2;
  return true;
}

// Given a '%' character at |*begin| in the string |spec|, this will copy
// the appropriate characters to the output for it to be in canonical form.
//
// |*begin| will be updated to point to the last character of the escape
// sequence so that when called with the index of a for loop, the next time
// through it will point to the next character to be considered.
//
// On failure (return false) this will just do a literal copy of the percent
// sign and, if possible, the following two characters, which would be
// otherwise interpreted as the values. If the characters are wide, we will
// stop copying since they we don't know the proper conversion rules.
//
// On failure, the caller should NOT accept the URL as valid, as it could be
// a security problem. Imagine if the caller could be coaxed to produce a
// valid escape sequence out of characters this function does not consider
// to be vaild (maybe the caller is canonicalizing fullwidth to ASCII).
// A URL could be constructed that the canonicalizer would treat differently
// than the server, which could potentially be bad.
bool CanonicalizeEscaped(const char* spec, int* begin, int end,
                         CanonOutput* output);
bool CanonicalizeEscaped(const UTF16Char* spec, int* begin, int end,
                         CanonOutput* output);

// Appends the given substring to the output, escaping "some" characters that
// it feels may not be safe. It assumes the input values are all contained in
// 8-bit although it allows any type.
//
// This is used in error cases to append invalid output so that it looks
// approximately correct. Non-error cases should not call this function since
// the escaping rules are not guaranteed!
void AppendInvalidNarrowString(const char* spec, int begin, int end,
                               CanonOutput* output);
void AppendInvalidNarrowString(const UTF16Char* spec, int begin, int end,
                               CanonOutput* output);

// Misc canonicalization helpers ----------------------------------------------

// Converts between UTF-8 and UTF-16, returning true on successful conversion.
// The output will be appended to the given canonicalizer output (so make sure
// it's empty if you want to replace).
//
// On invalid input, this will still write as much output as possible,
// replacing the invalid characters with the "invalid character". It will
// return false in the failure case, and the caller should not continue as
// normal.
bool ConvertUTF16ToUTF8(const UTF16Char* input, int input_len,
                        CanonOutput* output);
bool ConvertUTF8ToUTF16(const char* input, int input_len,
                        CanonOutputT<UTF16Char>* output);

// Converts from UTF-16 to 8-bit using the character set converter. If the
// converter is NULL, this will use UTF-8.
void ConvertUTF16ToQueryEncoding(const UTF16Char* input,
                                 const url_parse::Component& query,
                                 CharsetConverter* converter,
                                 CanonOutput* output);

// Applies the replacements to the given component source. The component source
// should be pre-initialized to the "old" base. That is, all pointers will
// point to the spec of the old URL, and all of the Parsed components will
// be indices into that string.
//
// The pointers and components in the |source| for all non-NULL strings in the
// |repl| (replacements) will be updated to reference those strings.
// Canonicalizing with the new |source| and |parsed| can then combine URL
// components from many different strings.
void SetupOverrideComponents(const char* base,
                             const Replacements<char>& repl,
                             URLComponentSource<char>* source,
                             url_parse::Parsed* parsed);

// Like the above 8-bit version, except that it additionally converts the
// UTF-16 input to UTF-8 before doing the overrides.
//
// The given utf8_buffer is used to store the converted components. They will
// be appended one after another, with the parsed structure identifying the
// appropriate substrings. This buffer is a parameter because the source has
// no storage, so the buffer must have the same lifetime as the source
// parameter owned by the caller.
//
// Returns true on success. Fales means that the input was not valid UTF-16,
// although we will have still done the override with "invalid characters" in
// place of errors.
bool SetupUTF16OverrideComponents(const char* base,
                                  const Replacements<UTF16Char>& repl,
                                  CanonOutput* utf8_buffer,
                                  URLComponentSource<char>* source,
                                  url_parse::Parsed* parsed);

// Implemented in url_canon_path.cc, these are required by the relative URL
// resolver as well, so we declare them here.
bool CanonicalizePartialPath(const char* spec,
                             const url_parse::Component& path,
                             int path_begin_in_output,
                             CanonOutput* output);
bool CanonicalizePartialPath(const UTF16Char* spec,
                             const url_parse::Component& path,
                             int path_begin_in_output,
                             CanonOutput* output);

#if !defined(WIN32) || defined(WINCE)

// Implementations of Windows' int-to-string conversions
int _itoa_s(int value, char* buffer, size_t size_in_chars, int radix);
int _itow_s(int value, UTF16Char* buffer, size_t size_in_chars, int radix);

// Secure template overloads for these functions
template<size_t N>
inline int _itoa_s(int value, char (&buffer)[N], int radix) {
  return _itoa_s(value, buffer, N, radix);
}

template<size_t N>
inline int _itow_s(int value, UTF16Char (&buffer)[N], int radix) {
  return _itow_s(value, buffer, N, radix);
}

#ifdef OS_SYMBIAN
inline unsigned long long _strtoui64(const char* nptr,
                                     char** endptr, 
                                     int base) {
  assert(endptr == NULL);  // TODO(marcogelmi): endptr not supported,
                           // but googleurl passes NULL.
  TInt64 val;
  TRadix radix;
  TLex8 lex(reinterpret_cast<const unsigned char*>(nptr));
  
  switch (base) {
    case 2:
      radix = EBinary;
      break;
    case 8:
      radix = EOctal;
      break;
    case 10:
      radix = EDecimal;
      break;
    case 16:
      radix = EHex;
      break;
    default:
      // TODO(marcogelmi): other radixes are not supported yet.
      assert(false);
  }
  return KErrNone == lex.Val(val, radix) ? val : 0;
}
#elif !defined(WINCE)
// _strtoui64 and strtoull behave the same
inline unsigned long long _strtoui64(const char* nptr,
                                     char** endptr, int base) {
  return strtoull(nptr, endptr, base);
}
#endif

#endif  // WIN32

}  // namespace url_canon

#endif  // GOOGLEURL_SRC_URL_CANON_INTERNAL_H__
