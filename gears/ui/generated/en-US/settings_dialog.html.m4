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
  <title>PRODUCT_FRIENDLY_NAME_UQ Settings</title>
  <link rel="stylesheet" href="button.css">
  <link rel="stylesheet" href="html_dialog.css">
  <style type="text/css">
    h1 {
      font-size:1.2em;
      margin:0;
    }

    h2 {
      font-size:1.1em;
      margin:0;
    }

    /* TODO(aa): Move into standard stylesheet if used in all dialogs? */
    #icon {
      margin-right:0.5em;
    }

    #content {
      overflow-x:hidden;
      overflow-y:auto;
      margin:0 1em;
    }

    #content table {
      border:1px #ccc;
      border-style:none solid;
      border-collapse:collapse;
      /* A bug in Mozilla with border-collapse:collapse along with the overflow
      settings we have on the scroll element causes the left border to be
      placed 1px to the left, making it invisible. */
      margin-left:1px;
    }

    #content td {
      border:1px #ccc;
      border-style:solid none;
      padding:0.5em;
    }

    #version {
      font-size:0.8em;
      font-style:italic;
    }

    .left {
      width:100%;
    }
  </style>
</head>
<body>
  <div id="head">
    <table width="100%" cellpadding="0" cellspacing="0" border="0">
      <tr>
        <td align="left" valign="middle">
          <img id="icon" src="icon_32x32.png" width="32" height="32">
        </td>
        <td width="100%" align="left" valign="middle">
          <h1>PRODUCT_FRIENDLY_NAME_UQ Settings</h1>
        </td>
      </tr>
    </table>
  </div>
  <div id="content">
    <h2>Allowed Sites</h2>
    <p>These sites are always allowed to access PRODUCT_FRIENDLY_NAME_UQ.</p>
    <div>
      <table>
        <tbody id="allowed-list">
          <tr>
            <td class="left"><em>No allowed sites.</em></td>
            <td></td>
          </tr>
        </tbody>
      </table>
    </div>
    <br>
    <br>
    <h2>Denied Sites</h2>
    <p>These sites are never allowed to access PRODUCT_FRIENDLY_NAME_UQ.</p>
    <div>
      <table>
        <tbody id="denied-list">
          <tr>
            <td class="left"><em>No denied sites.</em></td>
            <td></td>
          </tr>
        </tbody>
      </table>
    </div>
    <br>
    <br>
    <div id="version">
      PRODUCT_FRIENDLY_NAME_UQ version PRODUCT_VERSION
    </div>
  </div>
  <div id="foot">
    <div id="button-row">
      <table width="100%" cellpadding="0" cellspacing="0" border="0">
        <tr>
          <td nowrap="true" align="right" valign="middle">
            <!--
            Fancy buttons
            Note: Weird line breaks are on purpose to avoid extra space between
            buttons.
            Note: Outer element is <a> because we want it to focusable and
            behave like an anchor. Inner elements should theoretically be able
            to be <span>, but IE renders incorrectly in this case.
            -->
            <a href="#" accesskey="S" id="confirm-button" 
                onclick="saveAndClose(g_dialogResult); return false;"
                class="inline-block custom-button">
              <div class="inline-block custom-button-outer-box">
                <div class="inline-block custom-button-inner-box"
                  ><span class="accesskey">S</span>ave</div></div></a>
            <!--
            Note: There must be whitespace here or Firefox messes up the
            rendering.
            TODO(aa): This results in inconsistent spacing in IE vs Firefox
            between the buttons, but I am reluctant to hack things even further
            to fix that.
            -->
            <a href="#" accesskey="C" id="cancel-button"
                onclick="saveAndClose(null); return false;"
                class="inline-block custom-button">
              <div class="inline-block custom-button-outer-box">
                <div class="inline-block custom-button-inner-box"
                  ><span class="accesskey">C</span>ancel</div></div></a>
          </td>
        </tr>
      </table>
    </div>
  </div>
</body>
<script src="json_noeval.js"></script>
<script src="html_dialog.js"></script>
<script>
  var g_dialogResult = [];

  initDialog();
  initCustomLayout(layoutSettings);
  initSettings();
  
  function initSettings() {
    var args = getArguments();
    initList("allowed-list", args.allowed);
    initList("denied-list", args.denied);
  }
  
  function initList(tableId, sites) {
    var table = document.getElementById(tableId);
    if (!sites.length) {
      return;
    }
    
    // Hide the empty message
    table.rows[0].style.display = "none";
    
    // Add rows for each child    
    for (var site, i = 0; site = sites[i]; i++) {
      initSite(table, site);
    }
  }
  
  function initSite(table, siteName) {
    var row = document.createElement("TR");
    var left = document.createElement("TD");
    var right = document.createElement("TD");
    var link = document.createElement("A");
    left.className = "left";
    right.className = "right";
    left.appendChild(document.createTextNode(siteName));
    link.appendChild(document.createTextNode("Remove"));
    link.onclick = handleRemoveClick;
    link.row = row;
    link.siteName = siteName;
    link.href = "#";
    right.appendChild(link);
    row.appendChild(left);
    row.appendChild(right);
    table.appendChild(row);
  }
  
  function handleRemoveClick() {
    // Add to the list of things to be removed from database.
    g_dialogResult.push(this.siteName);

    // Remove the row visually.
    var table = this.row.parentNode;
    table.removeChild(this.row);
    
    // If we have removed all the rows, show the empty message again.
    if (table.rows.length == 1) {
      table.rows[0].style.display = "";
    }
    
    // Enable the save button since we have now made a change.
    document.getElementById("confirm-button").disabled = false;

    // Return false to prevent the default link action (scrolling up to top of
    // page in this case).    
    return false;
  }

  function layoutSettings(contentHeight) {
    var content = document.getElementById("content");

    content.style.height = Math.max(contentHeight, 0) + "px";

    // If a scrollbar is going to be visible, then add some padding between it
    // and the inner right edge of the content. We don't want to have this in
    // all the time or else there will be double-padding when the scrollbar
    // isn't visible.
    if (content.scrollHeight > contentHeight) {
      content.style.paddingRight = "1em";
    } else {
      content.style.paddingRight = "";
    }
  }
</script>
</html>
