using Microsoft.Win32;
using System.Diagnostics;

namespace GameBridgeInstaller
{
    internal class GUIHandler : IGUI
    {
        bool geo11FixFound = false;
        bool superDepth3DFixFound = false;

        string gameExeName = "";
        string gameExeNameWithoutExtension = "";
        string gameExeFolderPath = "";

        string geo11FixPathPrefix = "Geo-11";
        string superDepth3DFixPathPrefix = "SuperDepth3D";
        string superDepth3DDefaultFixPathPrefix = "SuperDepth3D\\Default";
        string superDepth3DShaderPathPrefix = "SuperDepth3D\\ReShade-Shaders";
        string reshadeShaderPathPrefix = "reshade-shaders";
        string srBackupExtensionPostfix = "SRBACKUP";

        string SRInstallpath = "";
        string SRAddonPath = "";
        string ReshadeInstallerPath = "";
        List<string> SRDlls = new List<string>() { "DimencoWeaving.dll", "glog.dll", "opencv_world343.dll", "SimulatedRealityCore.dll", "SimulatedRealityDirectX.dll", "SimulatedRealityDisplays.dll", "SimulatedRealityFacetrackers.dll" };

        public bool CheckIfReShadeInstallerPresent()
        {
            try
            {
                string[] files = System.IO.Directory.GetFiles(Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName), "ReShade_Setup_*_Addon.exe", System.IO.SearchOption.TopDirectoryOnly);
                if (files.Length > 0)
                {
                    // Reshade is present
                    ReshadeInstallerPath = files.First();
                    return true;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("ERROR: Exception while trying to find ReShade installer:\n" + ex.Message);
            }
            return false;
        }

        public bool CheckIfReShadeAddonPresent()
        {
            try
            {
                string[] files = System.IO.Directory.GetFiles(Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName), "srReshade*.addon", System.IO.SearchOption.TopDirectoryOnly);
                if (files.Length > 0)
                {
                    // Reshade addon is present
                    SRAddonPath = files.First();
                    return true;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("ERROR: Exception while trying to find ReShade addon:\n" + ex.Message);
            }

            return false;
        }

        public bool CheckIfSRInstallIsValid(string SRInstallPath)
        {
            foreach (string file in SRDlls)
            {
                if (!File.Exists(SRInstallPath + "\\bin\\" + file))
                {
                    return false;
                }
            }

            return true;
        }

        public string GetSRInstallPathFromRegistry()
        {
            try
            {
                var subKey = "SOFTWARE\\Dimenco\\Simulated Reality";
                using (var key = Registry.LocalMachine.OpenSubKey(subKey, false)) // False means read only here.
                {
                    var s = key?.GetValue("") as string;
                    if (s != null)
                    {
                        SRInstallpath = s;
                        return s;
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("ERROR: Exception while trying to read SR install path from registry:\n" + ex.Message);
            }

            return "";
        }

        public string InstallGameBridge(string pathToGameExe, GraphicsAPITypes graphicsApi)
        {
            // Reset global variables
            ResetGlobals();

            // Split exe name off from path.
            gameExeName = pathToGameExe.Substring(pathToGameExe.LastIndexOf("\\"), pathToGameExe.Length - pathToGameExe.LastIndexOf("\\"));
            // Cut the '.exe' part off the gameExeName.
            gameExeNameWithoutExtension = gameExeName.Split('.')[0];
            // Cut the '.exe' part off the pathToGameExe. Pushed 1 index back to include the '\\' characters.
            gameExeFolderPath = pathToGameExe.Substring(0, pathToGameExe.LastIndexOf("\\") + 1);

            // First, copy all required SR files to the target path.
            try
            {
                foreach (string dllName in SRDlls)
                {
                    File.Copy(SRInstallpath + "\\bin\\" + dllName, pathToGameExe.Substring(0, pathToGameExe.Length - gameExeName.Length) + "\\" + dllName, true);
                }

            }
            catch (Exception ex)
            {
                // Remove any files that were potentially copied.
                foreach (string dllName in SRDlls)
                {
                    try
                    {
                        File.Delete(gameExeFolderPath + "\\bin\\" + dllName);
                    }
                    catch { }
                }
                return "An exception occured while copying the SR dlls:\n\n" + ex.Message;
            }

            // Copy the srReshade addon
            try
            {
                File.Copy(SRAddonPath, pathToGameExe.Substring(0, pathToGameExe.Length - gameExeName.Length) + "\\" + SRAddonPath.Substring(SRAddonPath.LastIndexOf("\\"), SRAddonPath.Length - SRAddonPath.LastIndexOf("\\")), true);
            }
            catch (Exception ex)
            {
                // Delete the addon if it failed.
                return "An exception occured while copying the srReshade addon:\n\n" + ex.Message;
            }

            // Install reshade with addon support
            try
            {
                string reshadeGraphicsApiArgument = "";
                switch (graphicsApi)
                {
                    case GraphicsAPITypes.DirectX9:
                        reshadeGraphicsApiArgument = "d3d9";
                        break;
                    case GraphicsAPITypes.DirectX10_11_12:
                        reshadeGraphicsApiArgument = "dxgi";
                        break;
                    case GraphicsAPITypes.OpenGL:
                        reshadeGraphicsApiArgument = "opengl";
                        break;
                    case GraphicsAPITypes.Vulkan:
                        reshadeGraphicsApiArgument = "vulkan";
                        break;
                    default:
                        reshadeGraphicsApiArgument = "dxgi";
                        break;
                }
                // Todo: Get the return code from the ReShade installer in case it fails.
                string cmdArguments = "--headless --elevated --api " + reshadeGraphicsApiArgument + " \"" + pathToGameExe + "\"";
                Process process = Process.Start(ReshadeInstallerPath, cmdArguments);
                if(!process.WaitForExit(10000))
                {
                    return "The Reshade installer has timed out.";
                }
            }
            catch (Exception ex)
            {
                return "An exception occured while installing ReShade:\n\n" + ex.Message;
            }

            // Check for any Geo-11 fix files
            if (Directory.Exists(geo11FixPathPrefix + gameExeNameWithoutExtension))
            {
                geo11FixFound = true;
            }

            // Copy the geo-11 fix files (if Geo-11 fix is found)
            if (geo11FixFound)
            {
                try
                {
                    CopyAndBackupFiles(geo11FixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath);
                }
                catch (Exception ex)
                {
                    // Todo: Implement cleaning up the fix if copying fails
                    Console.WriteLine("WARNING: Cleaning up geo-11 fix files not finished yet!");
                    Console.WriteLine("Restoring backed up Geo-11 files...");
                    RestoreBackedUpFiles(gameExeFolderPath);

                    return "An exception occured while copying the geo-11 fix:\n\n" + ex.Message;
                }
            }

            // Copy the SuperDepth3D files (to ensure we have at least one shader in ReShade)
            try
            {
                CopyAndBackupFiles(superDepth3DShaderPathPrefix, gameExeFolderPath + reshadeShaderPathPrefix);
            }
            catch (Exception ex)
            {
                // Todo: Implement cleaning up the shader if copying fails
                Console.WriteLine("WARNING: Cleaning up SuperDepth3D shader files not finished yet!");
                Console.WriteLine("Restoring backed up SuperDepth3D shader files...");
                RestoreBackedUpFiles(gameExeFolderPath);

                return "An exception occured while copying the SuperDepth3D shader:\n\n" + ex.Message;
            }

            // Check for any SuperDepth3D fix files, install default reshade(preset).ini if no fix is found
            if (Directory.Exists(superDepth3DFixPathPrefix + gameExeNameWithoutExtension))
            {
                superDepth3DFixFound = true;
            }

            if (superDepth3DFixFound && !geo11FixFound)
            {
                // Copy the SuperDepth3D/Reshade configs
                try
                {
                    CopyAndBackupFiles(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath);
                }
                catch (Exception ex)
                {
                    // Todo: Implement cleaning up the shader if copying fails
                    Console.WriteLine("WARNING: Cleaning up SuperDepth3D fix files not finished yet!");
                    Console.WriteLine("Restoring backed up SuperDepth3D fix files...");
                    RestoreBackedUpFiles(gameExeFolderPath);

                    return "An exception occured while copying the SuperDepth3D fix:\n\n" + ex.Message;
                }
            } else if (!superDepth3DFixFound && !geo11FixFound)
            {
                // Copy the SuperDepth3D/Reshade configs
                try
                {
                    CopyAndBackupFiles(superDepth3DDefaultFixPathPrefix, gameExeFolderPath);
                }
                catch (Exception ex)
                {
                    // Todo: Implement cleaning up the shader if copying fails
                    Console.WriteLine("WARNING: Cleaning up SuperDepth3D default fix files not finished yet!");
                    Console.WriteLine("Restoring backed up SuperDepth3D default fix files...");
                    RestoreBackedUpFiles(gameExeFolderPath);

                    return "An exception occured while copying the default SuperDepth3D fix:\n\n" + ex.Message;
                }
            }

            return "";
        }
        public string UninstallGameBridge(string pathToGameExe, GraphicsAPITypes graphicsApi)
        {
            // Todo: Test this code
            // Reset global variables
            ResetGlobals();

            // Split exe name off from path.
            gameExeName = pathToGameExe.Substring(pathToGameExe.LastIndexOf("\\"), pathToGameExe.Length - pathToGameExe.LastIndexOf("\\"));
            // Cut the '.exe' part off the gameExeName.
            gameExeNameWithoutExtension = gameExeName.Split('.')[0];
            // Cut the '.exe' part off the pathToGameExe. Pushed 1 index back to include the '\\' characters.
            gameExeFolderPath = pathToGameExe.Substring(0, pathToGameExe.LastIndexOf("\\") + 1);

            // Remove any files that were potentially copied.
            foreach (string dllName in SRDlls)
            {
                try
                {
                    File.Delete(gameExeFolderPath + dllName);
                }
                catch { }
            }

            // Remove the srReshade addon
            try
            {
                File.Delete(gameExeFolderPath + SRAddonPath.Substring(SRAddonPath.LastIndexOf("\\"), SRAddonPath.Length - SRAddonPath.LastIndexOf("\\")));
            }
            catch { }

            // Uninstall reshade with addon support
            try
            {
                string reshadeGraphicsApiArgument = "";
                switch (graphicsApi)
                {
                    case GraphicsAPITypes.DirectX9:
                        reshadeGraphicsApiArgument = "d3d9";
                        break;
                    case GraphicsAPITypes.DirectX10_11_12:
                        reshadeGraphicsApiArgument = "dxgi";
                        break;
                    case GraphicsAPITypes.OpenGL:
                        reshadeGraphicsApiArgument = "opengl";
                        break;
                    case GraphicsAPITypes.Vulkan:
                        reshadeGraphicsApiArgument = "vulkan";
                        break;
                    default:
                        reshadeGraphicsApiArgument = "dxgi";
                        break;
                }

                // Todo: Get the return code from the ReShade installer in case it fails.
                // Specifiying Vulkan as graphics api as a workaround to an uninstaller bug in ReShade.
                string cmdArguments = "--headless --elevated --api vulkan --state uninstall" + " \"" + pathToGameExe + "\"";
                Process process = Process.Start(ReshadeInstallerPath, cmdArguments);
                if (!process.WaitForExit(10000))
                {
                    return "The Reshade uninstaller has timed out.";
                }
            }
            catch (Exception ex)
            {
                return "An exception occured while uninstalling ReShade:\n\n" + ex.Message;
            }

            // Check for any Geo-11 fix files
            if (Directory.Exists(geo11FixPathPrefix + gameExeNameWithoutExtension))
            {
                geo11FixFound = true;
            }

            // Remove the geo-11 fix files (if Geo-11 fix is found)
            if (geo11FixFound)
            {
                foreach (string filePath in Directory.GetFiles(geo11FixPathPrefix + gameExeNameWithoutExtension, "*.*", SearchOption.TopDirectoryOnly))
                {
                    // Delete from Geo-11 base path.
                    DeleteFileByPathIfFoundInFix(filePath);
                }

                // Delete all of the files in all folders of the game exe path if they were previously installed as part of the Geo-11 fix.
                foreach (string dirPath in Directory.GetDirectories(geo11FixPathPrefix + gameExeNameWithoutExtension, "*", SearchOption.AllDirectories))
                {
                    foreach (string filePath in Directory.GetFiles(dirPath, "*.*", SearchOption.TopDirectoryOnly))
                    {
                        DeleteFileByPathIfFoundInFix(filePath);
                    }
                }


                // Todo: This is a lot of nested loops, performance can certainly be improved! Also you can make a method for this duplicated code!
                // Delete all of the folders that are empty after deleting the files.
                foreach (string dirPath in Directory.GetDirectories(geo11FixPathPrefix + gameExeNameWithoutExtension, "*", SearchOption.AllDirectories))
                {
                    try
                    {
                        if (Directory.GetFiles(dirPath.Replace(geo11FixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath), "*.*", SearchOption.AllDirectories).Count() == 0)
                        {
                            Directory.Delete(dirPath.Replace(geo11FixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath), true);
                        }
                    }
                    catch (DirectoryNotFoundException ex)
                    {
                        Console.WriteLine("Warning: a directory could not be found during a delete operation:\n\n" + ex.Message);
                    }
                }
            }

            // Check for any SuperDepth3D fix files
            if (Directory.Exists(superDepth3DFixPathPrefix + gameExeNameWithoutExtension))
            {
                superDepth3DFixFound = true;
            }

            // Remove the SupderDepth3D fix files (if SD3D fix is found)
            if (superDepth3DFixFound)
            {
                // Check for any SuperDepth3D fix files, uninstall default reshadepreset.ini
                foreach (string filePath in Directory.GetFiles(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, "*.*", SearchOption.TopDirectoryOnly))
                {
                    // Delete from SD3D base path.
                    DeleteFileByPathIfFoundInFix(filePath);
                }

                // Delete all of the files in all folders of the game exe path if they were previously installed as part of the SD3D fix.
                foreach (string dirPath in Directory.GetDirectories(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, "*", SearchOption.AllDirectories))
                {
                    foreach (string filePath in Directory.GetFiles(dirPath, "*.*", SearchOption.TopDirectoryOnly))
                    {
                        DeleteFileByPathIfFoundInFix(filePath);
                    }
                }

                // Todo: This is a lot of nested loops, performance can certainly be improved! Also you can make a method for this duplicated code > https://dimenco.atlassian.net/browse/GB-83
                // Delete the directory if it's empty at this point.
                foreach (string dirPath in Directory.GetDirectories(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, "*", SearchOption.AllDirectories))
                {
                    try
                    {
                        if (Directory.GetFiles(dirPath.Replace(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath), "*.*", SearchOption.AllDirectories).Count() == 0)
                        {
                            Directory.Delete(dirPath.Replace(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath), true);
                        }
                    }
                    catch (DirectoryNotFoundException ex)
                    {
                        Console.WriteLine("Warning: a directory could not be found during a delete operation:\n\n" + ex.Message);
                    }
                }
            }

            // Check for any SuperDepth3D fix files, uninstall default reshadepreset.ini
            if (File.Exists(gameExeFolderPath + "ReShadePreset.ini"))
            {
                // Remove ReShadePreset.ini
                try
                {
                    File.Delete(gameExeFolderPath + "ReShadePreset.ini");
                }
                catch { }
            }

            // Restore backed up files
            RestoreBackedUpFiles(gameExeFolderPath);

            return "";
        }

        public string OpenWindowsExplorerDialog()
        {
            // Open Windows browser dialog.
            OpenFileDialog fd = new OpenFileDialog();
            //fd.InitialDirectory = "::{20D04FE0-3AEA-1069-A2D8-08002B30309D}";
            fd.Filter = "Exe Files (.exe)|*.exe";
            fd.RestoreDirectory = true;

            if (fd.ShowDialog() == DialogResult.OK)
            {
                // Get the path of specified file
                return fd.FileName;
            } 
            else
            {
                return "";
            }
        }

        private void RestoreBackedUpFiles(string gameExeFolderPath)
        {

            DirectoryInfo hdDirectoryInWhichToSearch = new DirectoryInfo(gameExeFolderPath);
            FileInfo[] filesInDir = hdDirectoryInWhichToSearch.GetFiles("*.*" + srBackupExtensionPostfix, SearchOption.AllDirectories);

            foreach (FileInfo foundFile in filesInDir)
            {
                try
                {
                    string fullName = foundFile.FullName;
                    if (fullName.Contains(srBackupExtensionPostfix))
                    {
                        foundFile.MoveTo(fullName.Replace(srBackupExtensionPostfix, "")); // Todo: Not sure if this places the files in the right place.
                    }
                } catch (Exception ex)
                {
                    Console.WriteLine("An exception occured while restoring the following file from backup:\n\n" + foundFile.FullName + "\n\nError message:\n\n" +  ex.Message);
                }

            }
        }

        private void DeleteFileByPathIfFoundInFix(string filePath)
        {
            // Check if file in game directory also exists inside Geo-11 fix directory.
            // Make sure the file wasn't renamed with our backup postfix.
            string selectedFileName = filePath.Substring(filePath.LastIndexOf("\\"), filePath.Length - filePath.LastIndexOf("\\"));
            try
            {
                if (Directory.GetFiles(geo11FixPathPrefix + gameExeNameWithoutExtension, selectedFileName.Replace("\\", ""), SearchOption.AllDirectories).FirstOrDefault() != null && !selectedFileName.Contains(srBackupExtensionPostfix))
                {
                    File.Delete(filePath.Replace(geo11FixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath));
                    return;
                }
            }
            catch { }
            try
            {
                if (Directory.GetFiles(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, selectedFileName.Replace("\\", ""), SearchOption.AllDirectories).FirstOrDefault() != null  && !selectedFileName.Contains(srBackupExtensionPostfix))
                {
                    File.Delete(filePath.Replace(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath));
                    return;
                }
            }
            catch { }
        }

        private void ResetGlobals()
        {
            gameExeName = "";
            gameExeNameWithoutExtension = "";
            gameExeFolderPath = "";
            geo11FixFound = false;
            superDepth3DFixFound = false;
        }

        // Can throw an exception when it is unable to copy or move a file.
        // pathToNewFiles: The contents of this path copied to the pathToFilesToBackup folder and files that have to be overwritten are instead renamed with a special postFix in order to back them up.
        // pathToFilesToBackup: The target folder where the contents of pathToNewFiles are copied.
        private void CopyAndBackupFiles(string pathToNewFiles, string pathToFilesToBackup)
        {
            // Create all of the subdirectories of the fix
            foreach (string dirPath in Directory.GetDirectories(pathToNewFiles, "*", SearchOption.AllDirectories))
            {
                Directory.CreateDirectory(dirPath.Replace(pathToNewFiles, pathToFilesToBackup));
            }

            // Copy all the files & Replaces any files with the same name
            foreach (string newPath in Directory.GetFiles(pathToNewFiles, "*.*", SearchOption.AllDirectories))
            {
                // First see if the file already exists.
                // For now, we only want to back up .dll and .exe files to avoid confusion.
                if (File.Exists(newPath.Replace(pathToNewFiles, pathToFilesToBackup)) && newPath.Contains(".dll") || File.Exists(newPath.Replace(pathToNewFiles, pathToFilesToBackup)) && newPath.Contains(".exe"))
                {
                    // File exists, rename it so we can restore it during uninstallation.
                    File.Move(newPath.Replace(pathToNewFiles, pathToFilesToBackup), newPath.Replace(pathToNewFiles, pathToFilesToBackup) + srBackupExtensionPostfix, true);
                }

                File.Copy(newPath, newPath.Replace(pathToNewFiles, pathToFilesToBackup), true);
            }
        }
    }
}
