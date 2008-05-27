import os
import shutil
import stat
import zipfile
import time
import subprocess

# Windows imports.
if os.name == 'nt':
  import win32api
  import win32con
  import win32file
  import wmi

# Workaround permission, stat.I_WRITE not allowing delete on some systems
DELETABLE = int('777', 8)

class BaseInstaller:
  """ Handle extension installation and browser profile adjustments. """

  GUID = '{000a9d1c-beef-4f90-9363-039d445309b8}'
  BUILDS = 'output/installers'

  def _prepareProfiles(self):
    """ Unzip profiles. """
    profile_dir = 'profiles'
    profiles = os.listdir(profile_dir)
    for profile in profiles:
      profile_path = os.path.join(profile_dir, profile)
      target_path = profile[:profile.find('.')]
      if os.path.exists(target_path):
        os.chmod(target_path, DELETABLE)
        shutil.rmtree(target_path, onerror=self._handleRmError)
      profile_zip = open(profile_path, 'rb')
      self._unzip(profile_zip, target_path)
      profile_zip.close()

  def _installExtension(self, build):
    """ Locate the desired ff profile, overwrite it, and unzip build to it. 
    
    Args:
      build: local filename for the build
    """
    if os.path.exists('xpi'):
      shutil.rmtree('xpi', onerror=self._handleRmError)
    xpi = open(build, 'rb')
    self._unzip(xpi, 'xpi')
    xpi.close()
    self._copyProfileAndInstall('xpi')

  def _copyProfileAndInstall(self, extension):
    """ Profile and extension placement for mac/linux.
    
    Args:
      extension: path to folder containing extension to install
    """
    profile_folder = self._findProfileFolderIn(self.ffprofile_path)

    if profile_folder:
      print 'copying over profile...'
    else:
      print 'failed to find profile folder'
      return

    shutil.rmtree(profile_folder, onerror=self._handleRmError)
    self._copyAndChmod(self.ffprofile, profile_folder)
    ext = os.path.join(profile_folder, 'extensions')
    if not os.path.exists(ext):
      os.mkdir(ext)
    gears = os.path.join(ext, BaseInstaller.GUID)
    self._copyAndChmod(extension, gears)

  def _findProfileFolderIn(self, path):
    """ Find and return the right profile folder in directory at path.
    
    Args:
      path: path of the directory to search in
    
    Returns:
      String name of the folder for the desired profile, or False
    """
    dir = os.listdir(path)
    for folder in dir:
      if folder.find('.' + self.profile) > 0:
        profile_folder = os.path.join(path, folder)
        return profile_folder
    return False

  def _findBuildPath(self, type, directory):
    """ Find the path to the build of the given type.
  
    Args:
      type: os string eg 'win32' 'osx' or 'linux'
    
    Returns:
      String path to correct build for the type, else throw
    """
    dir_list = os.listdir(directory)
    for item in dir_list:
      if item.find(type) > -1:
        build = os.path.join(directory, item)
        build = build.replace('/', os.sep).replace('\\', os.sep)
        return build
    raise "Can't locate build of type: %s" % type

  def _saveInstalledBuild(self):
    """ Copies given build to the "current build" location. """
    if os.path.exists(self.current_build):
      shutil.rmtree(self.current_build, onerror=self._handleRmError)
    shutil.copytree(BaseInstaller.BUILDS, self.current_build)

  def _copyProfile(self, src, dst_folder, profile_name):
    """ Copy profile to correct location. 
    
    Args:
      src: Location of profile to copy
      dst_folder: Path of folder to copy into
      profile_name: String name of the destination profile
    """
    dst_path = os.path.join(dst_folder, profile_name)
    if not os.path.exists(dst_folder):
      os.mkdir(dst_folder)
    if os.path.exists(dst_path):
      os.chmod(dst_path, DELETABLE)
      shutil.rmtree(dst_path, onerror=self._handleRmError)
    self._copyAndChmod(src, dst_path)

  def _copyAndChmod(self, src, targ):
    shutil.copytree(src, targ)
    os.chmod(targ, DELETABLE)
  
  def _handleRmError(self, func, path, exc_info):
    """ Handle errors removing files with long names on nt systems.

    Args:
      func: function call that caused exception
      path: path to function call argument
      exc_info: string info about the exception
    """
    # On nt, try using win32file to delete if os.remove fails
    if os.name == 'nt':
      # DeleteFileW can only operate on an absolute path
      if not os.path.isabs(path):
        path = os.path.join(os.getcwd(), path)
      unicode_path = '\\\\?\\' + path
      # Throws an exception on error
      win32file.DeleteFileW(unicode_path)
    else:
      raise StandardError(exc_info)

  def _unzip(self, file, target):
    """ Unzip file to target dir.

    Args: 
      file: file pointer to archive
      target: path for folder to unzip to

    Returns:
      True if successful, else False
    """
    if not target:
      print 'invalid path'
      return False

    os.mkdir(target)
    try:
      zf = zipfile.ZipFile(file)
    except zipfile.BadZipfile:
      print 'invalid zip archive'
      return False

    archive = zf.namelist()
    for thing in archive:
      fullpath = thing.split('/')
      path = target
      for p in fullpath[:-1]:
        try:
          os.mkdir(os.path.join(path, p))
        except OSError:
          pass
        path = os.path.join(path, p)
      if thing[-1:] != '/':
        bytes = zf.read(thing)
        nf = open(os.path.join(target, thing), 'wb')
        nf.write(bytes)
        nf.close()
    return True


class BaseWin32Installer(BaseInstaller):
  """ Installer for Win32 machines, extends Installer. """

  def __init__(self):
    self._prepareProfiles()
    self._DestroyOldSlaveProcesses()
    
  def install(self):
    """ Do installation.  """
    # First, uninstall current installed build, if any exists
    if os.path.exists(self.current_build):
      build_path = self._buildPath(self.current_build)
      print 'Uninstalling build %s' % build_path
      c = ['msiexec', '/passive', '/uninstall', build_path]
      p = subprocess.Popen(c)
      p.wait()

    # Now install new build
    build_path = self._buildPath(BaseInstaller.BUILDS)
    print 'Installing build %s' % build_path
    c = ['msiexec', '/passive', '/i', build_path]
    p = subprocess.Popen(c)
    p.wait()
    google_path = os.path.join(self.appdata_path, 'Google')
    self._copyProfile(self.ieprofile, google_path, 
                      'Google Gears for Internet Explorer')

    # Save new build as current installed build
    self._saveInstalledBuild()
  
  def _DestroyOldSlaveProcesses(self):
    """ Check for and kill any existing gears slave processes.

    Gears internal tests create some slave processes while running.
    Here we check to see if any did not shut down properly and
    destroy any that remain.
    """
    process_list = wmi.WMI().Win32_Process(Name='rundll32.exe')
    for process in process_list:
      pid = process.ProcessID
      if process.CommandLine.rfind('gears.dll') > 0:
        print 'Killing deadlocked slave process: %d' % pid
        handle = win32api.OpenProcess(win32con.PROCESS_TERMINATE, 0, pid)
        win32api.TerminateProcess(handle, 0)
        win32api.CloseHandle(handle)    
  

class WinXpInstaller(BaseWin32Installer):
  """ Installer for WinXP, extends Win32Installer. """

  def __init__(self):
    BaseWin32Installer.__init__(self)
    home = os.getenv('USERPROFILE')
    self.current_build = os.path.join(home, 'current_gears_build')
    self.appdata_path = os.path.join(home, 'Local Settings\\Application Data')
    self.ieprofile = 'permissions'

  def type(self):
    return 'WinXpInstaller'

  def _buildPath(self, directory):
    return self._findBuildPath('msi', directory)


class WinVistaInstaller(BaseWin32Installer):
  """ Installer for Vista, extends Win32Installer. """

  def __init__(self):
    BaseWin32Installer.__init__(self)
    home = os.getenv('USERPROFILE')
    self.current_build = os.path.join(home, 'current_gears_build')
    self.appdata_path = os.path.join(home, 'AppData\\LocalLow')
    self.ieprofile = 'permissions'

  def type(self):
    return 'WinVistaInstaller'

  def _buildPath(self, directory):
    return self._findBuildPath('msi', directory)


class WinCeInstaller(BaseInstaller):
  """ Installer for WinCE, extends Installer. """

  def __init__(self, host):
    self.ieprofile = 'permissions'
    self.host = host
  
  def type(self):
    return 'WinCeInstaller' 
  
  def install(self):
    """ Do installation. """
    print 'Installing and copying permissions.'
    self.__installCab()
    self.__copyPermissions()
  
  def _buildPath(self, directory):
    return self._findBuildPath('cab', directory)
  
  def __installCab(self):
    """ Copy installer to device and install.  """
    build_path = self._buildPath(BaseInstaller.BUILDS)
    
    # Requires cecopy.exe in path.
    copy_cmd = ['cecopy.exe', build_path, 'dev:\\windows\\gears.cab']
    p = subprocess.Popen(copy_cmd)
    p.wait()

    # Requires rapistart.exe in path.  Option /noui for silent install.
    install_cmd = ['rapistart.exe', '\\windows\\wceload.exe', '/noui', 
                   '\\windows\\gears.cab']

    # TODO(ace): Find a more robust solution than waiting a set timeout
    # for installation to finish.
    gears_installer_timeout = 45 #seconds
    subprocess.Popen(install_cmd)
    time.sleep(gears_installer_timeout)

    # Requires pkill.exe in path.
    kill_cmd = ['pkill.exe', 'iexplore.exe']
    p = subprocess.Popen(kill_cmd)
    p.wait()
  
  def __copyPermissions(self):
    """ Modify permissions file to include host address and copy to device. """
    perm_path = os.path.join(self.ieprofile, 'permissions.db')
    perm_path.replace('/', '\\')

    # Requires sqlite3.exe in path to modify permissions db file.
    modify_cmd = ['sqlite3', perm_path]
    p = subprocess.Popen(modify_cmd, stdin=subprocess.PIPE)

    # Sql command to add gears permissions to the address of the host server.
    sql_cmd = 'INSERT INTO "Access" VALUES(\'http://%s:8001\',1);' % self.host
    p.stdin.write(sql_cmd)
    p.stdin.close()
    p.wait()

    # Requires cecopy.exe in path to copy permissions to device.
    copy_cmd = ['cecopy.exe', self.ieprofile, 
                'dev:\\application data\\google\\'
                'Google Gears for Internet Explorer']
    p = subprocess.Popen(copy_cmd)
    p.wait()


class BaseMacFirefoxInstaller(BaseInstaller):
  """ Base class for Mac installers. """
  
  def __init__(self, profile_name, firefox_bin, profile_loc):
    self.profile = profile_name
    self._prepareProfiles()
    home = os.getenv('HOME')
    ffprofile = 'Library/Application Support/Firefox/Profiles'
    ffcache = 'Library/Caches/Firefox/Profiles'
    self.current_build = os.path.join(home, 'current_gears_build')
    self.ffprofile_path = os.path.join(home, ffprofile)
    self.ffcache_path = os.path.join(home, ffcache)
    self.firefox_bin = firefox_bin
    self.ffprofile = profile_loc
    self.profile_arg = '-CreateProfile %s' % self.profile

  def _buildPath(self, directory):
    return self._findBuildPath('xpi', directory)
  
  def install(self):
    """ Do installation. """
    print 'Creating test profile and inserting extension'
    build_path = self._buildPath(BaseInstaller.BUILDS)
    os.system('%s %s' % (self.firefox_bin, self.profile_arg))
    self._installExtension(build_path)
    self._copyProfileCacheMac()
    self._saveInstalledBuild()

  def _copyProfileCacheMac(self):
    """ Copy cache portion of profile on mac. """
    profile_folder = self._findProfileFolderIn(self.ffcache_path)

    if profile_folder:
      print 'copying profile cache...'
    else:
      print 'failed to find profile folder'
      return

    # Empty cache and replace only with gears folder
    gears_folder = os.path.join(profile_folder, 'Google Gears for Firefox')
    ffprofile_cache = 'ff2profile-mac/Google Gears for Firefox'
    shutil.rmtree(profile_folder, onerror=self._handleRmError)
    os.mkdir(profile_folder)
    self._copyAndChmod(ffprofile_cache, gears_folder)


class MacFirefox2Installer(BaseMacFirefoxInstaller):
  """ Firefox 2 installer for Mac OS X. """

  FIREFOX_PATH = '/Applications/Firefox.app/Contents/MacOS/firefox-bin'

  def __init__(self, profile_name):
    firefox_bin = MacFirefox2Installer.FIREFOX_PATH
    BaseMacFirefoxInstaller.__init__(self, profile_name, firefox_bin, 'ff2profile-mac')
  
  def type(self):
    return 'MacFirefox2Installer'


class MacFirefox3Installer(BaseMacFirefoxInstaller):
  """ Firefox 3 installer for Mac OS X. """

  FIREFOX_PATH = '/Applications/Firefox3.app/Contents/MacOS/firefox-bin'

  def __init__(self, profile_name):
    firefox_bin = MacFirefox3Installer.FIREFOX_PATH
    BaseMacFirefoxInstaller.__init__(self, profile_name, firefox_bin, 'ff3profile-mac')
  
  def type(self):
    return 'MacFirefox3Installer'


class MacSafariInstaller(BaseInstaller):
  """ Safari installer for Mac OS X. """

  def __init__(self):
    self._prepareProfiles()
    home = os.getenv('HOME')
    self.profile_path = os.path.join(home, 'Library/Application Support/Google')
    self.profile_name = 'Google Gears for Safari'
    self.src_profile = 'permissions'
  
  def install(self):
    """ Copy extension and profile for Safari. """
    # TODO(ace): install extension
    self._copyProfile(self.src_profile, self.profile_path, self.profile_name)
    self._saveInstalledBuild()


class LinuxInstaller(BaseInstaller):
  """ Installer for linux, extends Installer. """

  def __init__(self, profile_name):
    self.profile = profile_name
    self._prepareProfiles()
    home = os.getenv('HOME')
    self.current_build = os.path.join(home, 'current_gears_build')
    self.ffprofile_path = os.path.join(home, '.mozilla/firefox')
    self.firefox_bin = 'firefox'
    self.ffprofile = 'ff2profile-linux'
    self.profile_arg = '-CreateProfile %s' % self.profile

  def _buildPath(self, directory):
    return self._findBuildPath('xpi', directory)

  def type(self):
    return 'LinuxInstaller'

  def install(self):
    """ Do installation. """
    print 'Creating test profile and inserting extension'
    build_path = self._buildPath(BaseInstaller.BUILDS)
    os.system('%s %s' % (self.firefox_bin, self.profile_arg))
    self._installExtension(build_path)
    self._saveInstalledBuild()