/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM /builds/slave/mozilla-1.9.2-macosx-xulrunner/build/netwerk/mime/public/nsIMIMEHeaderParam.idl
 */

#ifndef __gen_nsIMIMEHeaderParam_h__
#define __gen_nsIMIMEHeaderParam_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIMIMEHeaderParam */
#define NS_IMIMEHEADERPARAM_IID_STR "ddbbdfb8-a1c0-4dd5-a31b-5d2a7a3bb6ec"

#define NS_IMIMEHEADERPARAM_IID \
  {0xddbbdfb8, 0xa1c0, 0x4dd5, \
    { 0xa3, 0x1b, 0x5d, 0x2a, 0x7a, 0x3b, 0xb6, 0xec }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIMIMEHeaderParam : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IMIMEHEADERPARAM_IID)

  /** 
   * Given the value of a single header field  (such as
   * Content-Disposition and Content-Type) and the name of a parameter
   * (e.g. filename, name, charset), returns the value of the parameter.
   * The value is obtained by decoding RFC 2231-style encoding,
   * RFC 2047-style encoding, and converting to UniChar(UTF-16)
   * from charset specified in RFC 2231/2047 encoding, UTF-8, 
   * <code>aFallbackCharset</code>, the locale charset as fallback if
   * <code>TryLocaleCharset</code> is set, and null-padding as last resort
   * if all else fails.
   *
   * <p> 
   * This method internally invokes <code>getParameterInternal</code>, 
   * However, it does not stop at decoding RFC 2231 (the task for
   * <code>getParameterInternal</code> but tries to cope
   * with several non-standard-compliant cases mentioned below.
   *
   * <p>
   * Note that a lot of MUAs and HTTP servers put RFC 2047-encoded parameters 
   * in mail headers and HTTP headers. Unfortunately, this includes Mozilla 
   * as of 2003-05-30. Even more standard-ignorant MUAs, web servers and 
   * application servers put 'raw 8bit characters'. This will try to cope 
   * with all these cases as gracefully as possible. Additionally, it 
   * returns the language tag if the parameter is encoded per RFC 2231 and 
   * includes lang.
   *
   *
   *
   * @param  aHeaderVal        a header string to get the value of a parameter 
   *                           from.
   * @param  aParamName        the name of a MIME header parameter (e.g. 
   *                           filename, name, charset). If empty,  returns 
   *                           the first (possibly) _unnamed_ 'parameter'.
   * @param  aFallbackCharset  fallback charset to try if  the string after
   *                           RFC 2231/2047 decoding or the raw 8bit 
   *                           string is not UTF-8
   * @param  aTryLocaleCharset If set, makes yet another attempt 
   *                           with the locale charset.
   * @param  aLang             If non-null, assigns it to a pointer 
   *                           to a string containing the value of language 
   *                           obtained from RFC 2231 parsing. Caller has to 
   *                           nsMemory::Free it.
   * @return the value of <code>aParamName</code> in Unichar(UTF-16).
   */
  /* AString getParameter (in ACString aHeaderVal, in string aParamName, in ACString aFallbackCharset, in boolean aTryLocaleCharset, out string aLang); */
  NS_SCRIPTABLE NS_IMETHOD GetParameter(const nsACString & aHeaderVal, const char *aParamName, const nsACString & aFallbackCharset, PRBool aTryLocaleCharset, char **aLang NS_OUTPARAM, nsAString & _retval NS_OUTPARAM) = 0;

  /** 
   * Given the value of a single header field  (such as
   * Content-Disposition and Content-Type) and the name of a parameter
   * (e.g. filename, name, charset), returns the value of the parameter 
   * after decoding RFC 2231-style encoding. 
   * <p>
   * For <strong>internal use only</strong>. The only other place where 
   * this needs to be  invoked  is  |MimeHeaders_get_parameter| in 
   * mailnews/mime/src/mimehdrs.cpp defined as 
   * char * MimeHeaders_get_parameter (const char *header_value, 
   *                                   const char *parm_name,
   *                                   char **charset, char **language)
   *
   * Otherwise, this method would have been made static.
   *
   * @param  aHeaderVal  a header string to get the value of a parameter from.
   * @param  aParamName  the name of a MIME header parameter (e.g. 
   *                     filename, name, charset). If empty,  returns 
   *                     the first (possibly) _unnamed_ 'parameter'.
   * @param  aCharset    If non-null, it gets assigned a new pointer
   *                     to a string containing the value of charset obtained
   *                     from RFC 2231 parsing. Caller has to nsMemory::Free it.
   * @param  aLang       If non-null, it gets assigned a new pointer
   *                     to a string containing the value of language obtained
   *                     from RFC 2231 parsing. Caller has to nsMemory::Free it.
   * @return             the value of <code>aParamName</code> after
   *                     RFC 2231 decoding but without charset conversion.
   */
  /* [noscript] string getParameterInternal (in string aHeaderVal, in string aParamName, out string aCharset, out string aLang); */
  NS_IMETHOD GetParameterInternal(const char *aHeaderVal, const char *aParamName, char **aCharset NS_OUTPARAM, char **aLang NS_OUTPARAM, char **_retval NS_OUTPARAM) = 0;

  /** 
   * Given a header value, decodes RFC 2047-style encoding and
   * returns the decoded header value in UTF-8 if either it's
   * RFC-2047-encoded or aDefaultCharset is given. Otherwise,
   * returns the input header value (in whatever encoding) 
   * as it is except that  RFC 822 (using backslash) quotation and 
   * CRLF (if aEatContinuation is set) are stripped away
   * <p>
   * For internal use only. The only other place where this needs to be 
   * invoked  is  <code>MIME_DecodeMimeHeader</code> in 
   * mailnews/mime/src/mimehdrs.cpp defined as
   * char * Mime_DecodeMimeHeader(char *header_val, const char *charset, 
   *                              PRBool override, PRBool eatcontinuation)
   *
   * @param aHeaderVal       a header value to decode
   * @param aDefaultCharset  MIME charset to use in place of MIME charset
   *                         specified in RFC 2047 style encoding
   *                         when <code>aOverrideCharset</code> is set.
   * @param aOverrideCharset When set, overrides MIME charset specified 
   *                         in RFC 2047 style encoding with <code>aDefaultCharset</code>
   * @param aEatContinuation When set, removes CR/LF
   * @return                 decoded header value
   */
  /* [noscript] ACString decodeRFC2047Header (in string aHeaderVal, in string aDefaultCharset, in boolean aOverrideCharset, in boolean aEatContinuation); */
  NS_IMETHOD DecodeRFC2047Header(const char *aHeaderVal, const char *aDefaultCharset, PRBool aOverrideCharset, PRBool aEatContinuation, nsACString & _retval NS_OUTPARAM) = 0;

  /** 
   * Given a header parameter, decodes RFC 2047 style encoding (if it's 
   * not obtained from RFC 2231 encoding),  converts it to
   * UTF-8 and returns the result in UTF-8 if an attempt to extract 
   * charset info. from a few different sources succeeds.
   * Otherwise,  returns the input header value (in whatever encoding) 
   * as it is except that  RFC 822 (using backslash) quotation is
   * stripped off.
   * <p>
   * For internal use only. The only other place where this needs to be 
   * invoked  is  <code>mime_decode_filename</code> in 
   * mailnews/mime/src/mimehdrs.cpp defined as
   * char * mime_decode_filename(char *name, const char *charset, 
   *                             MimeDisplayOptions *opt) 
   *
   * @param aParamValue      the value of a parameter to decode and convert
   * @param aCharset         charset obtained from RFC 2231 decoding  in which 
   *                         <code>aParamValue</code> is encoded. If null,
   *                         indicates that it needs to try RFC 2047, instead. 
   * @param aDefaultCharset  MIME charset to use when aCharset is null and
   *                         cannot be obtained per RFC 2047 (most likely 
   *                         because 'bare' string is  used.)  Besides, it 
   *                         overrides aCharset/MIME charset obtained from 
   *                         RFC 2047 if <code>aOverrideCharset</code>  is set.
   * @param aOverrideCharset When set, overrides MIME charset specified 
   *                         in RFC 2047 style encoding with 
   *                         <code>aDefaultCharset</code>
   * @return                 decoded parameter 
   */
  /* [noscript] ACString decodeParameter (in ACString aParamValue, in string aCharset, in string aDefaultCharset, in boolean aOverrideCharset); */
  NS_IMETHOD DecodeParameter(const nsACString & aParamValue, const char *aCharset, const char *aDefaultCharset, PRBool aOverrideCharset, nsACString & _retval NS_OUTPARAM) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIMIMEHeaderParam, NS_IMIMEHEADERPARAM_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIMIMEHEADERPARAM \
  NS_SCRIPTABLE NS_IMETHOD GetParameter(const nsACString & aHeaderVal, const char *aParamName, const nsACString & aFallbackCharset, PRBool aTryLocaleCharset, char **aLang NS_OUTPARAM, nsAString & _retval NS_OUTPARAM); \
  NS_IMETHOD GetParameterInternal(const char *aHeaderVal, const char *aParamName, char **aCharset NS_OUTPARAM, char **aLang NS_OUTPARAM, char **_retval NS_OUTPARAM); \
  NS_IMETHOD DecodeRFC2047Header(const char *aHeaderVal, const char *aDefaultCharset, PRBool aOverrideCharset, PRBool aEatContinuation, nsACString & _retval NS_OUTPARAM); \
  NS_IMETHOD DecodeParameter(const nsACString & aParamValue, const char *aCharset, const char *aDefaultCharset, PRBool aOverrideCharset, nsACString & _retval NS_OUTPARAM); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIMIMEHEADERPARAM(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetParameter(const nsACString & aHeaderVal, const char *aParamName, const nsACString & aFallbackCharset, PRBool aTryLocaleCharset, char **aLang NS_OUTPARAM, nsAString & _retval NS_OUTPARAM) { return _to GetParameter(aHeaderVal, aParamName, aFallbackCharset, aTryLocaleCharset, aLang, _retval); } \
  NS_IMETHOD GetParameterInternal(const char *aHeaderVal, const char *aParamName, char **aCharset NS_OUTPARAM, char **aLang NS_OUTPARAM, char **_retval NS_OUTPARAM) { return _to GetParameterInternal(aHeaderVal, aParamName, aCharset, aLang, _retval); } \
  NS_IMETHOD DecodeRFC2047Header(const char *aHeaderVal, const char *aDefaultCharset, PRBool aOverrideCharset, PRBool aEatContinuation, nsACString & _retval NS_OUTPARAM) { return _to DecodeRFC2047Header(aHeaderVal, aDefaultCharset, aOverrideCharset, aEatContinuation, _retval); } \
  NS_IMETHOD DecodeParameter(const nsACString & aParamValue, const char *aCharset, const char *aDefaultCharset, PRBool aOverrideCharset, nsACString & _retval NS_OUTPARAM) { return _to DecodeParameter(aParamValue, aCharset, aDefaultCharset, aOverrideCharset, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIMIMEHEADERPARAM(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetParameter(const nsACString & aHeaderVal, const char *aParamName, const nsACString & aFallbackCharset, PRBool aTryLocaleCharset, char **aLang NS_OUTPARAM, nsAString & _retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetParameter(aHeaderVal, aParamName, aFallbackCharset, aTryLocaleCharset, aLang, _retval); } \
  NS_IMETHOD GetParameterInternal(const char *aHeaderVal, const char *aParamName, char **aCharset NS_OUTPARAM, char **aLang NS_OUTPARAM, char **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetParameterInternal(aHeaderVal, aParamName, aCharset, aLang, _retval); } \
  NS_IMETHOD DecodeRFC2047Header(const char *aHeaderVal, const char *aDefaultCharset, PRBool aOverrideCharset, PRBool aEatContinuation, nsACString & _retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->DecodeRFC2047Header(aHeaderVal, aDefaultCharset, aOverrideCharset, aEatContinuation, _retval); } \
  NS_IMETHOD DecodeParameter(const nsACString & aParamValue, const char *aCharset, const char *aDefaultCharset, PRBool aOverrideCharset, nsACString & _retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->DecodeParameter(aParamValue, aCharset, aDefaultCharset, aOverrideCharset, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsMIMEHeaderParam : public nsIMIMEHeaderParam
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIMIMEHEADERPARAM

  nsMIMEHeaderParam();

private:
  ~nsMIMEHeaderParam();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsMIMEHeaderParam, nsIMIMEHeaderParam)

nsMIMEHeaderParam::nsMIMEHeaderParam()
{
  /* member initializers and constructor code */
}

nsMIMEHeaderParam::~nsMIMEHeaderParam()
{
  /* destructor code */
}

/* AString getParameter (in ACString aHeaderVal, in string aParamName, in ACString aFallbackCharset, in boolean aTryLocaleCharset, out string aLang); */
NS_IMETHODIMP nsMIMEHeaderParam::GetParameter(const nsACString & aHeaderVal, const char *aParamName, const nsACString & aFallbackCharset, PRBool aTryLocaleCharset, char **aLang NS_OUTPARAM, nsAString & _retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] string getParameterInternal (in string aHeaderVal, in string aParamName, out string aCharset, out string aLang); */
NS_IMETHODIMP nsMIMEHeaderParam::GetParameterInternal(const char *aHeaderVal, const char *aParamName, char **aCharset NS_OUTPARAM, char **aLang NS_OUTPARAM, char **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] ACString decodeRFC2047Header (in string aHeaderVal, in string aDefaultCharset, in boolean aOverrideCharset, in boolean aEatContinuation); */
NS_IMETHODIMP nsMIMEHeaderParam::DecodeRFC2047Header(const char *aHeaderVal, const char *aDefaultCharset, PRBool aOverrideCharset, PRBool aEatContinuation, nsACString & _retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] ACString decodeParameter (in ACString aParamValue, in string aCharset, in string aDefaultCharset, in boolean aOverrideCharset); */
NS_IMETHODIMP nsMIMEHeaderParam::DecodeParameter(const nsACString & aParamValue, const char *aCharset, const char *aDefaultCharset, PRBool aOverrideCharset, nsACString & _retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIMIMEHeaderParam_h__ */
