﻿namespace GameBridgeInstaller
{
    internal interface IGUI
    {
        // Opens a Windows Explorer dialog
        // Returns: a path to the specified file
        public string OpenWindowsExplorerDialog();
        // Installs all necessary files for Game Bridge for the specified .exe file. Returns error message if an exception occurs.
        public string InstallGameBridge(string pathToGameExe, GraphicsAPITypes graphicsApi);
        // Uninstalls all necessary files for Game Bridge for the specified .exe file. Attempts to delete all known files even if they were changed by the user. Returns error message if an exception occurs.
        public string UninstallGameBridge(string pathToGameExe, GraphicsAPITypes graphicsApi);
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