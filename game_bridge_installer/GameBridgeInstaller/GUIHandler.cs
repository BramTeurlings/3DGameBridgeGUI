using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;

namespace GameBridgeInstaller
{
    internal class GUIHandler : IGUI
    {
        bool geo11FixFound = false;
        bool superDepth3DFixFound = false;

        string geo11FixPathPrefix = "Geo-11";
        string superDepth3DFixPathPrefix = "SuperDepth3D";
        string superDepth3DDefaultFixPathPrefix = "SuperDepth3D\\Default";
        string superDepth3DShaderPathPrefix = "SuperDepth3D\\ReShade-Shaders";
        string reshadeShaderPathPrefix = "reshade-shaders";

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
            // Split exe name off from path
            string gameExeName = pathToGameExe.Substring(pathToGameExe.LastIndexOf("\\"), pathToGameExe.Length - pathToGameExe.LastIndexOf("\\"));

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
                        File.Delete(pathToGameExe + "\\bin\\" + dllName);
                    }
                    catch { }
                }
                return "An exception occured while copying the SR dlls:\n\n" + ex.Message;
            }

            // Copy the srReshade addon
            try
            {
                // Todo: Fix this tomorrow
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

            // Cut the '.exe' part off the gameExeName
            string gameExeNameWithoutExtension = gameExeName.Split('.')[0];
            // Cut the '.exe' part off the pathToGameExe. Pushed 1 index back to include the '\\' characters.
            string gameExeFolderPath = pathToGameExe.Substring(0, pathToGameExe.LastIndexOf("\\") + 1);

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
                    // Create all of the subdirectories of the geo-11 fix
                    foreach (string dirPath in Directory.GetDirectories(geo11FixPathPrefix + gameExeNameWithoutExtension, "*", SearchOption.AllDirectories))
                    {
                        Directory.CreateDirectory(dirPath.Replace(geo11FixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath));
                    }

                    // Copy all the files & Replaces any files with the same name
                    foreach (string newPath in Directory.GetFiles(geo11FixPathPrefix + gameExeNameWithoutExtension, "*.*", SearchOption.AllDirectories))
                    {
                        File.Copy(newPath, newPath.Replace(geo11FixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath), true);
                    }
                }
                catch (Exception ex)
                {
                    // Todo: Implement cleaning up the fix if copying fails
                    Console.WriteLine("WARNING: Cleaning up geo-11 fix files not yet implemented!");

                    return "An exception occured while copying the geo-11 fix:\n\n" + ex.Message;
                }
            } else // Copy the SuperDepth3D files (if no Geo-11 fix was found)
            {
                try
                {
                    // Create all of the subdirectories of the SuperDepth3D shader
                    foreach (string dirPath in Directory.GetDirectories(superDepth3DShaderPathPrefix, "*", SearchOption.AllDirectories))
                    {
                        Directory.CreateDirectory(dirPath.Replace(superDepth3DShaderPathPrefix, gameExeFolderPath + reshadeShaderPathPrefix));
                    }

                    // Copy all the files & Replaces any files with the same name
                    foreach (string newPath in Directory.GetFiles(superDepth3DShaderPathPrefix, "*.*", SearchOption.AllDirectories))
                    {
                        File.Copy(newPath, newPath.Replace(superDepth3DShaderPathPrefix, gameExeFolderPath + reshadeShaderPathPrefix), true);
                    }
                }
                catch (Exception ex)
                {
                    // Todo: Implement cleaning up the shader if copying fails
                    Console.WriteLine("WARNING: Cleaning up SuperDepth3D shader files not yet implemented!");

                    return "An exception occured while copying the SuperDepth3D shader:\n\n" + ex.Message;
                }
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
                    // Create all of the subdirectories of the SuperDepth3D fix
                    foreach (string dirPath in Directory.GetDirectories(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, "*", SearchOption.AllDirectories))
                    {
                        Directory.CreateDirectory(dirPath.Replace(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath));
                    }

                    // Copy all the files & Replaces any files with the same name
                    foreach (string newPath in Directory.GetFiles(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, "*.*", SearchOption.AllDirectories))
                    {
                        File.Copy(newPath, newPath.Replace(superDepth3DFixPathPrefix + gameExeNameWithoutExtension, gameExeFolderPath), true);
                    }
                }
                catch (Exception ex)
                {
                    // Todo: Implement cleaning up the shader if copying fails
                    Console.WriteLine("WARNING: Cleaning up SuperDepth3D fix files not yet implemented!");

                    return "An exception occured while copying the SuperDepth3D fix:\n\n" + ex.Message;
                }
            } else if (!superDepth3DFixFound && !geo11FixFound)
            {
                // Copy the SuperDepth3D/Reshade configs
                try
                {
                    // Create all of the subdirectories of the default SuperDepth3D fix
                    foreach (string dirPath in Directory.GetDirectories(superDepth3DDefaultFixPathPrefix, "*", SearchOption.AllDirectories))
                    {
                        Directory.CreateDirectory(dirPath.Replace(superDepth3DDefaultFixPathPrefix, gameExeFolderPath));
                    }

                    // Copy all the files & Replaces any files with the same name
                    foreach (string newPath in Directory.GetFiles(superDepth3DDefaultFixPathPrefix, "*.*", SearchOption.AllDirectories))
                    {
                        File.Copy(newPath, newPath.Replace(superDepth3DDefaultFixPathPrefix, gameExeFolderPath), true);
                    }
                }
                catch (Exception ex)
                {
                    // Todo: Implement cleaning up the shader if copying fails
                    Console.WriteLine("WARNING: Cleaning up SuperDepth3D default fix files not yet implemented!");

                    return "An exception occured while copying the default SuperDepth3D fix:\n\n" + ex.Message;
                }
            }

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
    }
}
