using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameBridgeInstaller
{
    internal interface IGUI
    {
        // Opens a Windows Explorer dialog
        // Returns: a path to the specified file
        public string OpenWindowsExplorerDialog();
        // Installs all necessary files for Game Bridge for the specified .exe file.
        public bool InstallGameBridge(string pathToGameExe, GraphicsAPITypes graphicsApi);
        // Gets the install path of Simulated Reality from the registry.
        public string GetSRInstallPathFromRegistry();
        // Checks the presence of all the required .dll files for Game Bridge to work.
        public bool CheckIfSRInstallIsValid(string SRInstallPath);
        // Checks the working directory for a ReShade with addon installer of any version, it will select the first one it finds for installation.
        public bool CheckIfReShadeInstallerPresent();
        // Checks if a srReshade addon is present, it will select the first one it finds for installation.
        public bool CheckIfReShadeAddonPresent();
    }
}