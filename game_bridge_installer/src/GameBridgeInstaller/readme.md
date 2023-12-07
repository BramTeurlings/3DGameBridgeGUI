# Game Bridge Installer
A project for installing Game Bridge and all its dependencies using a simple graphical user interface (GUI) and mininal user input.

## Instructions
* Unzip the "Game Bridge Installer" file in an isolated folder anywhere on your computer or on an external drive.
* Run the "GameBridgeInstaller.exe" file
* Enter a path to the desired game's executable (.exe) file in the input field.
* Select the game's graphics API
    * DirectX 9*
    * DirectX 10/11/12 (Using DXGI)*
    * OpenGL (Not supported at the moment)
    * Vulkan (Not supported at the moment)
* Click "Install"


*DirectX 9 and 10 are only supported when using SR Platform 1.28.0 or higher and a corresponding compatible srReshade.addon.

## Game fixes and folder logic
The Game Bridge installer assumes a certain folder structure for game fixes. Using this method, it's easy to add your own fixes to these folders.

### Geo-11
The default search path for game fixes is as follows:<br/>
**[currentDirectory]/Geo-11/[targetGameExecutableName]**

The contents of the folder above are copied to the game's executable folder.

To add your own Geo-11 fix to the installer, create a new folder inside the "Geo-11" folder with the exact same name as the target game's executable **without** the ".exe" postfix.

### SuperDepth3D
When no Geo-11 fix is found for the selected game executable, SuperDepth3D is installed.

The default search path for game fixes is as follows:<br/>
**[currentDirectory]/SuperDepth3D/[targetGameExecutableName]**

The contents of the folder above are copied to the game's executable folder.

To add your own SuperDepth3D fix to the installer, create a new folder inside the "SuperDepth3D" folder with the exact same name as the target game's executable **without** the ".exe" postfix.

#### Default SuperDepth3D config path
When no pre-made SuperDepth3D fix can be found, a default configuration is installed. This configuration can be found in the following path:<br/>
**[currentDirectory]/SuperDepth3D/Default**

#### SuperDepth3D shader path
Since we're using a local version of SuperDepth3D, it may be nice to update the SuperDepth3D shader once in a while. The path to the SuperDepth3D shader can be found here:
**[currentDirectory]/SuperDepth3D/ReShade-Shaders**

## ReShade installer
A ReShade installer with addon support must be present alongside the "GameBridgeInstaller.exe" file.

The ReShade installer must be formatted as follows:<br/>
**ReShade_Setup_*_Addon.exe**

**Note:** Make sure that the version of ReShade you are installing is compatible with the srReshade.addon file you supply. If they are incompatible, ReShade will report an error loading the srReshade addon in the overlay.

## srReshade.addon
A srReshade.addon file must be present alongside the "GameBridgeInstaller.exe" file.

The ReShade installer must be formatted as follows:<br/>
**srReshade\*.addon**

**Note:** Make sure that the version of the srReshade.addon you are installing is compatible with the installed version of the SR Platform. A quick compatibility list is shown below:
* SR Platform version < = 1.27.4 **COMPATIBLE WITH** 
    * srReshade.addon with SR Platform 1.27.4 **OR LOWER**
    * ReShade 5.9.2 **OR LOWER**
* SR Platform version > = 1.28.0 **COMPATIBLE WITH**
    * srReshade.addon with SR Platform 1.28.0 **OR HIGHER**
    * ReShade 5.9.2 **OR HIGHER**

