using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameBridgeInstaller
{
    internal class GUIHandler : IGUI
    {
        bool geo11FixFound = false;
        bool superDepth3DFixFound = false;

        string geo11FixPathPrefix = "Geo-11\\";
        string superDepth3DFixPathPrefix = "SuperDepth3D\\";
        string superDepth3DDefaultFixPathPrefix = "SuperDepth3D\\Default";
        string superDepth3DShaderPathPrefix = "SuperDepth3D\\Shaders";
        string reshadeShaderPathPrefix = "reshade-shaders\\Shaders";

        string SRInstallpath = "";
        string SRAddonPath = "";
        string ReshadeInstallerPath = "";
        List<string> SRDlls = new List<string>() { "DimencoWeaving.dll", "glog.dll", "opencv_world343.dll", "SimulatedRealityCore.dll", "SimulatedRealityDirectX.dll", "SimulatedRealityDisplays.dll", "SimulatedRealityFacetrackers.dll" };

        public bool CheckIfReShadeAddonPresent()
        {
            try
            {
                string[] files = System.IO.Directory.GetFiles("", "srReshade*.addon", System.IO.SearchOption.TopDirectoryOnly);
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

        public bool CheckIfReShadeInstallerPresent()
        {
            try
            {
                string[] files = System.IO.Directory.GetFiles("", "ReShade_Setup_*_Addon.exe", System.IO.SearchOption.TopDirectoryOnly);
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

        public bool CheckIfSRInstallIsValid(string SRInstallPath)
        {
            foreach (string file in SRDlls)
            {
                if (!File.Exists(SRInstallPath + "\\" + file))
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
                var subKey = "Computer\\HKEY_LOCAL_MACHINE\\SOFTWARE\\Dimenco\\Simulated Reality";
                using (var key = Registry.LocalMachine.OpenSubKey(subKey, false)) // False means read only here.
                {
                    var s = key?.GetValue("") as string;
                    SRInstallpath = s;
                    return s;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("ERROR: Exception while trying to read SR install path from registry:\n" + ex.Message);
            }

            return "";
        }

        public bool InstallGameBridge(string pathToGameExe, GraphicsAPITypes graphicsApi)
        {
            // First, copy all required SR files to the target path.
            try
            {
                foreach (string dllName in SRDlls)
                {
                    File.Copy(SRInstallpath + "\\" + dllName, pathToGameExe);
                }

            }
            catch (Exception ex)
            {
                // Remove any files that were potentially copied.
                foreach (string dllName in SRDlls)
                {
                    try
                    {
                        File.Delete(pathToGameExe + "\\" + dllName);
                    }
                    catch { }
                }
                Console.WriteLine("ERROR: An exception occured while copying the SR dlls: " + ex.Message);

                return false;
            }

            // Copy the srReshade addon
            try
            {
                File.Copy("\\" + SRAddonPath, pathToGameExe);
            }
            catch (Exception ex)
            {
                // Delete the addon if it failed.
                Console.WriteLine("ERROR: An exception occured while copying the srReshade addon: " + ex.Message);

                return false;
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
                Process process = Process.Start(ReshadeInstallerPath, "--headless --elevated --api " + reshadeGraphicsApiArgument);
                if(!process.WaitForExit(30000))
                {
                    Console.WriteLine("ERROR: The Reshade installer has timed out.");
                    return false;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("ERROR: An exception occured while installing ReShade: " + ex.Message);
                return false;
            }

            // Check for any Geo-11 fix files
            string gameExeName = pathToGameExe.Substring(pathToGameExe.LastIndexOf("\\"), pathToGameExe.Length);
            if (Directory.Exists(geo11FixPathPrefix + gameExeName))
            {
                geo11FixFound = true;
            }

            // Copy the geo-11 fix files
            try
            {
                // Create all of the subdirectories of the geo-11 fix
                foreach (string dirPath in Directory.GetDirectories(geo11FixPathPrefix + gameExeName, "*", SearchOption.AllDirectories))
                {
                    Directory.CreateDirectory(dirPath.Replace(geo11FixPathPrefix + gameExeName, pathToGameExe));
                }

                // Copy all the files & Replaces any files with the same name
                foreach (string newPath in Directory.GetFiles(geo11FixPathPrefix + gameExeName, "*.*", SearchOption.AllDirectories))
                {
                    File.Copy(newPath, newPath.Replace(geo11FixPathPrefix + gameExeName, pathToGameExe), true);
                }
            }
            catch (Exception ex)
            {
                // Todo: Implement cleaning up the fix if copying fails
                Console.WriteLine("ERROR: An exception occured while copying the geo-11 fix: " + ex.Message);
                Console.WriteLine("WARNING: Cleaning up geo-11 fix files not yet implemented!");

                return false;
            }

            // Install SuperDepth3D (if no Geo-11 fix was found)
            if(!geo11FixFound)
            {
                // Copy the SuperDepth3D files
                try
                {
                    // Create all of the subdirectories of the SuperDepth3D shader
                    foreach (string dirPath in Directory.GetDirectories(superDepth3DShaderPathPrefix, "*", SearchOption.AllDirectories))
                    {
                        Directory.CreateDirectory(dirPath.Replace(superDepth3DShaderPathPrefix, pathToGameExe + reshadeShaderPathPrefix));
                    }

                    // Copy all the files & Replaces any files with the same name
                    foreach (string newPath in Directory.GetFiles(superDepth3DShaderPathPrefix, "*.*", SearchOption.AllDirectories))
                    {
                        File.Copy(newPath, newPath.Replace(superDepth3DShaderPathPrefix, pathToGameExe + reshadeShaderPathPrefix), true);
                    }
                }
                catch (Exception ex)
                {
                    // Todo: Implement cleaning up the shader if copying fails
                    Console.WriteLine("ERROR: An exception occured while copying the SuperDepth3D shader: " + ex.Message);
                    Console.WriteLine("WARNING: Cleaning up SuperDepth3D shader files not yet implemented!");

                    return false;
                }
            }

            // Check for any SuperDepth3D fix files, install default reshade(preset).ini if no fix is found
            if (Directory.Exists(superDepth3DFixPathPrefix + gameExeName))
            {
                superDepth3DFixFound = true;
            }

            if (superDepth3DFixFound)
            {
                // Copy the SuperDepth3D/Reshade configs
                try
                {
                    // Create all of the subdirectories of the SuperDepth3D fix
                    foreach (string dirPath in Directory.GetDirectories(superDepth3DFixPathPrefix + gameExeName, "*", SearchOption.AllDirectories))
                    {
                        Directory.CreateDirectory(dirPath.Replace(superDepth3DFixPathPrefix + gameExeName, pathToGameExe));
                    }

                    // Copy all the files & Replaces any files with the same name
                    foreach (string newPath in Directory.GetFiles(superDepth3DFixPathPrefix + gameExeName, "*.*", SearchOption.AllDirectories))
                    {
                        File.Copy(newPath, newPath.Replace(superDepth3DFixPathPrefix + gameExeName, pathToGameExe), true);
                    }
                }
                catch (Exception ex)
                {
                    // Todo: Implement cleaning up the shader if copying fails
                    Console.WriteLine("ERROR: An exception occured while copying the SuperDepth3D fix: " + ex.Message);
                    Console.WriteLine("WARNING: Cleaning up SuperDepth3D fix files not yet implemented!");

                    return false;
                }
            } else
            {
                // Copy the SuperDepth3D/Reshade configs
                try
                {
                    // Create all of the subdirectories of the default SuperDepth3D fix
                    foreach (string dirPath in Directory.GetDirectories(superDepth3DDefaultFixPathPrefix, "*", SearchOption.AllDirectories))
                    {
                        Directory.CreateDirectory(dirPath.Replace(superDepth3DDefaultFixPathPrefix, pathToGameExe));
                    }

                    // Copy all the files & Replaces any files with the same name
                    foreach (string newPath in Directory.GetFiles(superDepth3DDefaultFixPathPrefix, "*.*", SearchOption.AllDirectories))
                    {
                        File.Copy(newPath, newPath.Replace(superDepth3DDefaultFixPathPrefix, pathToGameExe), true);
                    }
                }
                catch (Exception ex)
                {
                    // Todo: Implement cleaning up the shader if copying fails
                    Console.WriteLine("ERROR: An exception occured while copying the default SuperDepth3D fix: " + ex.Message);
                    Console.WriteLine("WARNING: Cleaning up SuperDepth3D default fix files not yet implemented!");

                    return false;
                }
            }

            return true;
        }

        public string OpenWindowsExplorerDialog()
        {
            // Open Windows browser dialog.
            OpenFileDialog fd = new OpenFileDialog();
            fd.InitialDirectory = "::{20D04FE0-3AEA-1069-A2D8-08002B30309D}";
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
