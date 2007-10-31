class TestRunner:
  """ Run browser tests.
  
  This class manages execution in-browser JavaScript tests. Uses webserver
  and list of browser launcher objects to run the test on 
  all specified browsers.

  Args:
    browser_launchers: list of BrowserLauncher types
    test_server: instance of testwebserver
  """
  
  TEST_URL = "http://localhost:8001/tester/gui.html"
  TIMEOUT = 60
    
  def __init__(self, browser_launchers, test_server):
    if not browser_launchers or len(browser_launchers) < 1:
      raise ValueError("Please provide browser launchers")
    self.__verifyBrowserLauncherTypesUnique(browser_launchers)
    self.browser_lauchers = browser_launchers
    self.test_server = test_server


  def runTests(self, automated=True):
    """ Launch tests on the test webserver for each launcher.

    Returns:
      results object keyed by browser.
    """
    test_results = {}
    self.test_server.startServing()
    try:
      try:
        for browser_launcher in self.browser_lauchers:
          self.test_server.startTest(TestRunner.TIMEOUT)
          try:
            browser_launcher.launch(TestRunner.TEST_URL)
          finally:
            browser_type = browser_launcher.type()
            test_results[browser_type] = self.test_server.testResults()
          if automated:
            browser_launcher.kill()
      except:
        print 'error in test_server.startTest'
    finally:
      self.test_server.shutdown()
      return test_results


  def __verifyBrowserLauncherTypesUnique(self, browser_launchers):
    """ Check that the given launchers represent unique browser type.

    Args:
      browser_launchers: list of BrowserLauncher objects.
    """
    browser_launchers_by_name = {}
    for browser_launcher in browser_launchers:
      browser_type = browser_launcher.type()
      if not browser_launchers_by_name.has_key(browser_type):
        browser_launchers_by_name[browser_type] = []
      browser_launchers_by_name[browser_type].append(browser_launcher)
        
    for launchers in browser_launchers_by_name.values():
      if len(launchers) > 1:
        raise ValueError("Browser launchers all must have unique type values")


if __name__ == '__main__':
  """ If run as main, launch tests on current system. """
  import sys
  import os
  from testwebserver import TestWebserver
  import browser_launchers as launcher
  import osutils

  from config import Config
  sys.path.extend(Config.ADDITIONAL_PYTHON_LIBRARY_PATHS)

  def server_root_dir():
      return os.path.join(os.path.dirname(__file__), '../')

  test_server = TestWebserver(server_root_dir())
  installers = []

  if osutils.osIsWin():
    launchers = []
    launchers.append(launcher.IExploreWin32Launcher(automated=False))
    launchers.append(launcher.FirefoxWin32Launcher('ffprofile-win', 
                                                   automated=False))
  
  elif osutils.osIsNix():
    if osutils.osIsMac():
      launchers = [launcher.FirefoxMacLauncher('gears', automated=False)]

    else: #is linux
      launchers = [launcher.FirefoxLinuxLauncher('gears', automated=False)]

  testrunner = TestRunner(launchers, test_server)
  testrunner.runTests(automated=False)
