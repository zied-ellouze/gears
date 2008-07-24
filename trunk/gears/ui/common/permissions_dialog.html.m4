﻿m4_changequote(`~',`~')m4_dnl
<!DOCTYPE html>

<!--
Copyright 2007, Google Inc.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
 3. Neither the name of Google Inc. nor the names of its contributors may be
    used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-->

<html>
<head>
  <style type="text/css">

m4_ifelse(PRODUCT_OS,~android~,~~,~m4_dnl
m4_include(ui/common/button.css)~)
m4_include(ui/common/html_dialog.css)
    #content {
      margin:0 1em;
    }

    #yellowbox-right {
      padding-left:1em;
      padding-right:1em;
    }

m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
    /* 
     * On Windows Mobile, we hide the div containing the buttons
     * by default, to only show the correct one (ie smartphone or not)
     */

    #button-row {
      display:none;
    }

    #button-row-smartphone {
      margin-left:2em;
      display:none;
    }
~,~~)

    #checkbox-row {
      margin-top:1em;
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
      margin-bottom:0.5em;
~,
~
      margin-bottom:1em;
~)
    }

    .header-icon {
      margin-right:0.5em;
      display:none;
    }

    #custom-icon {
      display:none;
      margin-left:1em;
    }

    #custom-name {
      font-weight:bold;
      font-size:1.1em;
      display:none;
    }

    #origin {
      display:none;
    }

    #custom-message {
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
      margin-top:2px; 
~,
~
      margin-top:6px;
~)
      display:none;
    }

    #origin-only {
      font-weight:bold;
      font-size:1.1em;
      display:none;
    }

    #yellowbox {
      margin:4px 0 4px 0;
      border:solid #f1cc1d;
      border-width:1px 0;
      background:#faefb8;
    }

    #yellowbox-inner {
      /* pseudo-rounded corners */
      margin:0 -1px;
      border:solid #f1cc1d;
      border-width:0 1px;
m4_ifelse(PRODUCT_OS,~wince~,~~,m4_dnl
~
      padding:1em 0;
~)
    }

    #yellowbox p {
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
      padding:0 0;
~,
~
      padding:0 1em;
~)
    }

    #permissions-help {
      margin:4px;
      display:none;
    }
  </style>
</head>
<body>
  <div id="strings" style="display:none">
    <div id="string-allow"></div>
    <div id="string-allow-accesskey"></div>
    <div id="string-deny"></div>
    <div id="string-deny-accesskey"></div>
    <div id="string-cancel"></div>
    <div id="string-trust-site-accesskey"></div>
  </div>

  <!--
  PIE only works with one window, and we are in a modal dialog.
  Using window.open(this.href) replaces the content of the current dialog,
  which is annoying when no back button is displayed...
  The workaround is to embed directly a short explanation text, and
  hide/show the div container for the help and the settings dialog.
  -->

  <div id="permissions-help">
   <h2>Information</h2>
   <p>
     <span id="string-description"></span>
   </p>
   <ul>
     <li><span id="string-localserver-desc"></span></li>
     <li><span id="string-database-desc"></span></li>
     <li><span id="string-workerpool-desc"></span></li>
   </ul>
   <a href="#" onclick="showHelp(false); return false;">Go back</a>
  </div>

m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~ <div id="permissions-settings">~)
  <div id="head">
    <table width="100%" cellpadding="0" cellspacing="0" border="0">
      <tr>
        <td valign="top">
m4_ifelse(PRODUCT_OS,~android~,m4_dnl
~
          <img id="icon-local-data" class="header-icon"
           width="32" height="32" src="data:image/png;base64,
m4_include(genfiles/local_data.png.base64)m4_dnl
           ">
~,~
          <img id="icon-local-data" class="header-icon"
           width="32" height="32" src="local_data.png">
~)
m4_ifelse(PRODUCT_OS,~android~,m4_dnl
~
          <img id="icon-locationdata" class="header-icon"
           width="32" height="32" src="data:image/png;base64,
m4_include(genfiles/location_data.png.base64)m4_dnl
           ">
~,~
          <img id="icon-locationdata" class="header-icon"
           width="32" height="32" src="location_data.png">
~)
          <!-- Some browsers automatically focus the first focusable item. We
          don't want anything focused, so we add this fake item. -->
          <a href="#" id="focus-thief"></a>
        </td>
        <td width="100%" valign="middle">
          <div id="local-data" style="display:none">
            <span id="string-query-data"></span>
          </div>
          <div id="location-data" style="display:none">
            <span id="string-query-location"></span>
          </div>
        </td>
      </tr>
    </table>
  </div>

  <div id="content">
    <div id="yellowbox">
      <div id="yellowbox-inner">
        <table width="100%" cellpadding="0" cellspacing="0" border="0">
          <tr>
            <td id="yellowbox-left" valign="top">
              <img id="custom-icon" width="0" height="0" src="blank.gif">
            </td>
            <td id="yellowbox-right" width="100%" valign="middle">
              <div id="custom-name"></div>
              <div id="origin"></div>
              <div id="origin-only"></div>
              <div id="custom-message"></div>
            </td>
          </tr>
        </table>
      </div>
    </div>

    <div id="string-location-privacy-statement" style="display:none; padding-top:3px;"></div>

    <p id="checkbox-row" style="display:none">
      <table width="100%" cellspacing="0" cellpadding="0" border="0">
        <tr>
          <td valign="middle">
            <input type="checkbox" id="unlock" accesskey="T"
                   onclick="updateAllowButtonEnabledState()">
          </td>
          <td valign="middle">
            <label for="unlock">
              &nbsp;<span id="string-trust-site"></span>
            </label>
          </td>
        </tr>
      </table>
    </p>
    m4_ifelse(PRODUCT_OS,~wince~,~~,m4_ifelse(PRODUCT_OS,~android~,~~,~<br>~))
  </div>

  <div id="foot">
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
    <!--
    On Windows Mobile, we use the softkey bar in addition to HTML buttons.
    On Windows Mobile smartphone, we only display this div and hide the
    button-row containing the HTML buttons (as screen estate is expensive
    and we already have the buttons through the softkey bar)
    -->

    <div id="button-row-smartphone">
      <table cellpadding="0" cellspacing="0" border="0">
      <tr>
        <td valign="middle">
          <a href="#" onclick="denyAccessPermanently(); return false;">
            <span id="string-never-allow-link"></span></a>
        </td>
      </tr>
      </table>
    </div>
~,~~)
    <div id="button-row">
      <table cellpadding="0" cellspacing="0" border="0">
        <tr>

m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
          <td width="50%" valign="middle">
~,~
          <td width="100%" valign="middle">
~)
            <a href="#" onclick="denyAccessPermanently(); return false;">
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~
            <span id="string-never-allow-link-wince"></span></a>
~,~
            <span id="string-never-allow-link"></span></a>
~)
          </td>

m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~         <!-- 
          We use form input buttons instead of button elements as PIE
          does not support them.
          -->

          <td width="50%" align="right" valign="middle">
            <form>
              <input type="BUTTON" id="allow-button" onclick="allowAccessPermanently(); return false;"></input>
              <input type="BUTTON" id="deny-button" onclick="denyAccessTemporarily(); return false;"></input>
            </form>
          </td>
~,~
          <td nowrap="true" align="right" valign="middle">
            <button id="allow-button" class="custom"
              onclick="allowAccessPermanently(); return false;"></button
            ><button id="deny-button" class="custom"
              onclick="denyAccessTemporarily(); return false;"></button>
          </td>~)
        </tr>
      </table>
    </div>
  </div>
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~ </div>~)
m4_ifelse(PRODUCT_OS,~wince~,m4_dnl
~<object style="display:none;" classid="clsid:134AB400-1A81-4fc8-85DD-29CD51E9D6DE" id="pie_dialog">
</object>~)
</body>
<!--
We include all files through m4 as the HTML dialog implementation on
PocketIE does not support callbacks for loading external JavaScript files.
TODO: find a better way to include scripts for PIE
-->
<script>
m4_include(../third_party/jsonjs/json_noeval.js)
m4_include(ui/common/base.js)
m4_include(ui/common/dom.js)
m4_include(ui/common/html_dialog.js)
m4_include(ui/common/button.js)
m4_include(genfiles/permissions_dialog.js)
</script>

<script>
  var debug = false;
  initDialog();

  initWarning();

  setButtonLabel("string-allow", "allow-button", "string-allow-accesskey");
  setButtonLabel("string-deny", "deny-button", "string-deny-accesskey");

  // Highlight the access key in the 'trust' text.
  // TODO(zork): Cause this to be based on the localized string, and figure out
  // how to programmatically set the access key for the checkbox.
  var trustTextElement = dom.getElementById("string-trust-site");
  text = trustTextElement.innerHTML;
  text = text.replace(/t/, "<span class='accesskey'>t</span>");
  trustTextElement.innerHTML = text;

  if (!browser.ie_mobile) {
    CustomButton.initializeAll();
  }

  if (browser.ie_mobile) {
    var allowText = dom.getElementById("string-allow");
    if (allowText) {
      window.pie_dialog.SetButton(allowText.innerText, "allowAccessPermanently();");
      window.pie_dialog.SetButtonEnabled(true);
    }
    var cancelText = dom.getElementById("string-cancel");
    if (cancelText) {
      window.pie_dialog.SetCancelButton(cancelText.innerText);
    }
    // Focus allow button by default.
    dom.getElementById('allow-button').focus();
  }

  function setTextContent(elem, content) {
    if (isDefined(typeof document.createTextNode)) {
      elem.appendChild(document.createTextNode(content)); 
    } else {
      elem.innerText = content;
    }
  }

  function showHelp(show) {
    if (browser.ie_mobile) {
      var elemSettings = dom.getElementById("permissions-settings"); 
      var elemHelp = dom.getElementById("permissions-help"); 
      if (show) {
        elemSettings.style.display = 'none';
        elemHelp.style.display = 'block';
        window.pie_dialog.SetButton("Back", "showHelp(false);");
        window.pie_dialog.SetButtonEnabled(true);
      } else {
        elemSettings.style.display = 'block';
        elemHelp.style.display = 'none';
        window.pie_dialog.SetButton("Allow", "allowAccessPermanently();");
      }
      window.pie_dialog.ResizeDialog();
    } else {
      window.open('http://gears.google.com/?action=help');
    }
  }

  function initWarning() {
    var args;
    if (debug) {
      // Handy for debugging layout:
      args = {
        locale: "en-US",
        origin: "http://www.google.com",
        dialogType: "localData",
        customIcon: "http://google-gears.googlecode.com/svn/trunk/gears/test/manual/shortcuts/32.png",
        customName: "My Application",
        customMessage: "Press the button to enable my application to run offline!"
      };
    } else {
      // The arguments to this dialog are a single string, see PermissionsDialog
      args = getArguments();
    }

    loadI18nStrings(args.locale);
    var origin = args['origin'];  // required parameter
    var dialogType = args['dialogType'];  // required parameter
    var customIcon = args['customIcon'];
    var customName = args['customName'];
    var customMessage = args['customMessage'];

    var elem;

    elem = dom.getElementById("icon");
    if (dialogType == "localData") {
      elem = dom.getElementById("icon-local-data");
      elem.style.display = "block";
      elem = dom.getElementById("local-data");
      elem.style.display = "block";
    } else if (dialogType == "locationData") {
      elem = dom.getElementById("icon-locationdata");
      elem.style.display = "block";
      elem = dom.getElementById("location-data");
      elem.style.display = "block";
      elem = dom.getElementById("string-location-privacy-statement");
      elem.style.display = "block";
    }

    if (!customName) {
      elem = dom.getElementById("origin-only");
      elem.style.display = "block";
      if (browser.ie_mobile) {
        elem.innerHTML = wrapDomain(origin);
      } else {
        setTextContent(elem, origin);
      }

      // When all we have is the origin, we lay it out centered because that
      // looks nicer. This is also what the original dialog did, which did not
      // support the extra name, icon, or message.
      if (!browser.ie_mobile && !customIcon && !customMessage) {
        elem.setAttribute("align", "center");
      }
    } else {
      elem = dom.getElementById("origin");
      elem.style.display = "block";
      setTextContent(elem, origin);
    }

    if (customIcon) {
      elem = dom.getElementById("custom-icon");
      elem.style.display = "inline";
      elem.height = 32;
      elem.width = 32;
      loadImage(elem, customIcon);
    }

    if (customName) {
      elem = dom.getElementById("custom-name");
      elem.style.display = "block";
      setTextContent(elem, customName);
    }

    if (customMessage) {
      elem = dom.getElementById("custom-message");
      elem.style.display = "block";
      setTextContent(elem, customMessage);
    }

    if (!browser.ie_mobile) {
      // Set up the checkbox to toggle the enabledness of the Allow button.
      dom.getElementById("unlock").onclick = updateAllowButtonEnabledState;
      dom.getElementById("checkbox-row").style.display = 'block';
      updateAllowButtonEnabledState();
    }
    resizeDialogToFitContent();
  }


  function updateAllowButtonEnabledState() {
    var allowButton = dom.getElementById("allow-button");
    var unlockCheckbox = dom.getElementById("unlock");

    if (unlockCheckbox.checked) {
      enableButton(allowButton);
    } else {
      disableButton(allowButton);
    }
  }

  // Note: The structure that the following functions pass is coupled to the
  // code in PermissionsDialog that processes it.
  function allowAccessPermanently() {
      saveAndClose({"allow": true, "permanently": true});
  }

  function denyAccessTemporarily() {
    saveAndClose({"allow": false, "permanently": false});
  }

  function denyAccessPermanently() {
    saveAndClose({"allow": false, "permanently": true });
  }
</script>
</html>