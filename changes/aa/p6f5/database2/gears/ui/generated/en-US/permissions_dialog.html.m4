m4_changequote(`^',`^')m4_dnl
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
  <title>PRODUCT_FRIENDLY_NAME_UQ Security Warning</title>
  <link rel="stylesheet" href="button.css">
  <link rel="stylesheet" href="html_dialog.css">
  <style type="text/css">
    #content {
      margin:0 1em;
    }

    #yellowbox-right {
      padding-left:1em;
      padding-right:1em;
    }

m4_ifelse(PRODUCT_OS,^wince^,m4_dnl
^
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
^,^^)

    #checkbox-row {
      margin-top:1em;
m4_ifelse(PRODUCT_OS,^wince^,m4_dnl
^
      margin-bottom:0.5em;
^,
^
      margin-bottom:1em;
^)
    }

    #icon {
      margin-right:0.5em;
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
m4_ifelse(PRODUCT_OS,^wince^,m4_dnl
^
      margin-top:2px; 
^,
^
      margin-top:6px;
^)
      display:none;
    }

    #origin-only {
      font-weight:bold;
      font-size:1.1em;
      display:none;
    }

    #yellowbox {
      margin:0 1px;
      border:solid #f1cc1d;
      border-width:1px 0;
      background:#faefb8;
    }

    #yellowbox-inner {
      /* pseudo-rounded corners */
      margin:0 -1px;
      border:solid #f1cc1d;
      border-width:0 1px;
m4_ifelse(PRODUCT_OS,^wince^,^^,m4_dnl
^
      padding:1em 0;
^)
    }

    #yellowbox p {
m4_ifelse(PRODUCT_OS,^wince^,m4_dnl
^
      padding:0 0;
^,
^
      padding:0 1em;
^)
    }

    #permissions-help {
      margin:4px;
      display:none;
    }
  </style>
</head>
<body>

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
     <TRANS_BLOCK desc="Header for basic help section.">
     PRODUCT_FRIENDLY_NAME_UQ is an open source browser extension that enables
     web applications to provide offline functionality using the following
     JavaScript APIs:
     </TRANS_BLOCK>
   </p>
   <ul>
     <li><TRANS_BLOCK desc="Short explanation of localserver">Store and serve application resources locally</TRANS_BLOCK></li>
     <li><TRANS_BLOCK desc="Short explanation of database">Store data locally in a fully-searchable relational database</TRANS_BLOCK></li>
     <li><TRANS_BLOCK desc="Short explanation of workerpool">Run asynchronous JavaScript to improve application responsiveness</TRANS_BLOCK></li>
   </ul>
   <a href="#" onclick="showHelp(false); return false;">Go back</a>
  </div>

m4_ifelse(PRODUCT_OS,^wince^,m4_dnl
^ <div id="permissions-settings">^)
  <div id="head">
    <table width="100%" cellpadding="0" cellspacing="0" border="0">
      <tr>
        <td align="left" valign="top">
          <img id="icon" src="icon_32x32.png" width="32" height="32">
        </td>
        <td width="100%" align="left" valign="middle">
          <TRANS_BLOCK desc="Asks the user if they want to let the site use Gears">
          The website below wants to use PRODUCT_FRIENDLY_NAME_UQ. This site 
          will be able to store and access information on your computer.&nbsp;
          </TRANS_BLOCK>
          <a href="#" onclick="showHelp(true); return false;">
          <TRANS_BLOCK desc="Help link displayed in the installation dialog.">
          What is this?
          </TRANS_BLOCK>
          </a>
        </td>
      </tr>
    </table>
  </div>

  <div id="content">
    <div id="yellowbox">
      <div id="yellowbox-inner">
        <table width="100%" cellpadding="0" cellspacing="0" border="0">
          <tr>
            <td id="yellowbox-left" align="left" valign="top">
              <img id="custom-icon" width="0" height="0">
            </td>
            <td id="yellowbox-right" width="100%" align="left" valign="middle">
              <div id="custom-name"></div>
              <div id="origin"></div>
              <div id="origin-only"></div>
              <div id="custom-message"></div>
            </td>
          </tr>
        </table>
      </div>
    </div>

    <p id="checkbox-row">
      <table cellspacing="0" cellpadding="0" border="0">
        <tr>
          <td valign="middle">
            <input type="checkbox" id="unlock" accesskey="T"
                   onclick="updateAllowButtonEnabledState()">
          </td>
          <td valign="middle">
            <label for="unlock">
          <TRANS_BLOCK desc="Indicates the user lets the site use Gears.">
              &nbsp;I <span class="accesskey">t</span>rust this site. Allow
              it to use PRODUCT_FRIENDLY_NAME_UQ.
          </TRANS_BLOCK>
            </label>
          </td>
        </tr>
      </table>
    </p>
    m4_ifelse(PRODUCT_OS,^wince^,^^,^<br>^)
  </div>

  <div id="foot">
    <div id="text-buttons" style="display:none">
      <TRANS_BLOCK desc="Link that disallows Gears on this site.">
        <div id="text-never-allow">Never allow it</div>
      </TRANS_BLOCK>
      <TRANS_BLOCK desc="Button user can press to allow the use of Gears.">
        <div id="text-allow">Allow</div>
      </TRANS_BLOCK>
      <TRANS_BLOCK desc="Button user can press to disallow the use of Gears.">
        <div id="text-deny">Deny</div>
      </TRANS_BLOCK>
      <TRANS_BLOCK desc="Button user can press to cancel the dialog.">
        <div id="text-cancel">Cancel</div>
      </TRANS_BLOCK>
    </div>
m4_ifelse(PRODUCT_OS,^wince^,m4_dnl
^
    <!--
    On Windows Mobile, we use the softkey bar in addition to HTML buttons.
    On Windows Mobile smartphone, we only display this div and hide the
    button-row containing the HTML buttons (as screen estate is expensive
    and we already have the buttons through the softkey bar)
    -->

    <div id="button-row-smartphone">
      <table cellpadding="0" cellspacing="0" border="0">
      <tr>
        <td align="left" valign="middle">
          <a href="#" onclick="denyAccessPermanently(); return false;">
            <TRANS_BLOCK desc="Link that disallows Gears on this site.">
              Never allow this site
            </TRANS_BLOCK>
          </a>
        </td>
      </tr>
      </table>
    </div>
^,^^)
    <div id="button-row">
      <table cellpadding="0" cellspacing="0" border="0">
        <tr>
m4_ifelse(PRODUCT_OS,^wince^,m4_dnl
^          <!-- 
          We use form input buttons instead of buttons elements 
          as PIE does not support them
          -->

          <td width="50%" align="left" valign="middle">
            <input type="BUTTON" id="never-allow-button" onclick="denyAccessPermanently(); return false;"></input>
          </td>

          <div id="div-buttons">
            <td width="50%" align="right" valign="middle">
              <input disabled type="BUTTON" id="allow-button" onclick="allowAccess();"></input>
              <input type="BUTTON" id="deny-button" onclick="denyAccess(); return false;"></input>
            </td>
          </div>^,m4_dnl
^           
          <td width="100%" align="left" valign="middle">
            <a href="#" onclick="denyAccessPermanently(); return false;">
            <TRANS_BLOCK desc="Link that disallows Gears on this site.">
              Never allow this site
            </TRANS_BLOCK>
            </a>
          </td>
          <td nowrap="true" align="right" valign="middle">
            <!--
            Fancy buttons
            Note: Weird line breaks are on purpose to avoid extra space between
            buttons.
            Note: Outer element is <a> because we want it to focusable and
            behave like an anchor. Inner elements should theoretically be able
            to be <span>, but IE renders incorrectly in this case.
             
            Note: The whitespace in this section is very delicate.  The lack of
            space between the tags and the space between the buttons both
            are important to ensure proper rendering.
            TODO(aa): This results in inconsistent spacing in IE vs Firefox
            between the buttons, but I am reluctant to hack things even further
            to fix that.
            -->
            <a href="#" accesskey="A" id="allow-button" 
                onclick="allowAccess(); return false;"
                class="inline-block custom-button">
              <div class="inline-block custom-button-outer-box">
                <div class="inline-block custom-button-inner-box"
                  ><TRANS_BLOCK desc="Button user can press to allow the use of Gears."><span class="accesskey">A</span>llow</TRANS_BLOCK></div></div></a>
            <a href="#" accesskey="C" id="deny-button"
                onclick="denyAccess(); return false;"
                class="inline-block custom-button">
              <div class="inline-block custom-button-outer-box">
                <div class="inline-block custom-button-inner-box"
                  ><TRANS_BLOCK desc="Button user can press to disallow the use of Gears."><span class="accesskey">C</span>ancel</TRANS_BLOCK></div></div></a></td>^)
        </tr>
      </table>
    </div>
  </div>
m4_ifelse(PRODUCT_OS,^wince^,m4_dnl
^ </div>^)
m4_ifelse(PRODUCT_OS,^wince^,m4_dnl
^<object style="display:none;" classid="clsid:134AB400-1A81-4fc8-85DD-29CD51E9D6DE" id="pie_dialog">
</object>^)
</body>
<!--
 We include all files through m4 as the HTML dialog implementation
 on PocketIE does not support callbacks for loading external javascript files
 TODO: find a better way to include scripts for PIE
-->
<script>
m4_include(third_party/jsonjs/json_noeval.js)
m4_include(ui/common/html_dialog.js)
</script>

<script>
  initDialog();
  if (isPIE) {
    setButtonLabel("text-never-allow", "never-allow-button");
    setButtonLabel("text-allow", "allow-button");
    setButtonLabel("text-deny", "deny-button");
    var allowText = getElementById("text-allow");
    if (allowText) {
      window.pie_dialog.SetButton(allowText.innerText, "allowAccess();");
    }
    var cancelText = getElementById("text-cancel");
    if (cancelText) {
      window.pie_dialog.SetCancelButton(cancelText.innerText);
    }
    updateAllowButtonEnabledState();
  }
  initWarning();

  var disabled = true;

  function setTextContent(elem, content) {
    if (isDefined(typeof document.createTextNode)) {
      elem.appendChild(document.createTextNode(content)); 
    } else {
      elem.innerText = content;
    }
  }

  function showHelp(show) {
    if (isPIE) {
      var elemSettings = getElementById("permissions-settings"); 
      var elemHelp = getElementById("permissions-help"); 
      if (show) {
        elemSettings.style.display = 'none';
        elemHelp.style.display = 'block';
        window.pie_dialog.SetButton("Back", "showHelp(false);");
        window.pie_dialog.SetButtonEnabled(true);
      } else {
        elemSettings.style.display = 'block';
        elemHelp.style.display = 'none';
        window.pie_dialog.SetButton("Allow", "allowAccess();");
        updateAllowButtonEnabledState();
      }
      window.pie_dialog.ResizeDialog();
    } else {
      window.open('http://gears.google.com/?action=help');
    }
  }

  function initWarning() {
    // The arguments to this dialog are a single string, see PermissionsDialog
    var args = getArguments();

    var origin = args['origin'];  // required parameter
    var customIcon = args['customIcon'];
    var customName = args['customName'];
    var customMessage = args['customMessage'];

    var elem;

    if (!customName) {
       elem = getElementById("origin-only");
       elem.style.display = "block";
       setTextContent(elem, origin);

    // When all we have is the origin, we lay it out centered because that
    // looks nicer. This is also what the original dialog did, which did not
    // support the extra name, icon, or message.
    if (!customIcon && !customMessage) {
      elem.setAttribute("align", "center");
    }
    } else {
       elem = getElementById("origin");
       elem.style.display = "block";
       setTextContent(elem, origin);
    }

    if (customIcon) {
      elem = getElementById("custom-icon");
      elem.style.display = "inline";
      elem.src = customIcon;
      elem.height = 32;
      elem.width = 32;
    }

    if (customName) {
      elem = getElementById("custom-name");
      elem.style.display = "block";
      setTextContent(elem, customName);
    }

    if (customMessage) {
      elem = getElementById("custom-message");
      elem.style.display = "block";
      setTextContent(elem, customMessage);
    }

    // Focus deny by default
    getElementById("deny-button").focus();

    // This does not work on PIE...
    if (!isPIE) {
      // Set up the checkbox to toggle the enabledness of the Allow button.
      getElementById("unlock").onclick = updateAllowButtonEnabledState;
    }
    updateAllowButtonEnabledState();

    // This does not work on PIE (no height measurement)
    if (!isPIE) {
      // Resize the window to fit
      var contentDiv = getElementById("content");
      var contentHeightProvided = getContentHeight();
      var contentHeightDesired = contentDiv.offsetHeight;
      if (contentHeightDesired != contentHeightProvided) {
        var dy = contentHeightDesired - contentHeightProvided;
        window.resizeBy(0, dy);
      }
    }  
  }


  function updateAllowButtonEnabledState() {
    var allowButton = getElementById("allow-button");
    var checkbox = getElementById("unlock");

    disabled = !checkbox.checked;

    if (disabled) {
      disableButton(allowButton);
    } else {
      enableButton(allowButton);
    }
  }

  // Note: The structure that the following functions pass are coupled to the
  // code in PermissionsDialog that processes it.
  function allowAccess() {
    if (!disabled) {
      saveAndClose({
        allow: true
      });
    }
  }

  function denyAccess() {
    saveAndClose(null); // default behavior is to deny temporarily
  }

  function denyAccessPermanently() {
    saveAndClose({
      allow: false
    });
  }
</script>
</html>
