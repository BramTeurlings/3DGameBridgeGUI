using System.Windows.Forms;

namespace GameBridgeInstaller
{
    public partial class Main : Form
    {
        GUIHandler handler;

        public Main()
        {
            InitializeComponent();
            handler = new GUIHandler();

            // Fill graphics API combobox in order
            for (int i = 0; i < Enum.GetNames(typeof(GraphicsAPITypes)).Length; i++)
            {
                string enumText = ((GraphicsAPITypes)i).ToString();
                graphicsApiCmb.Items.Add(enumText);
            }

            graphicsApiCmb.SelectedIndex = 1;
        }
        private void browseBtn_Click(object sender, EventArgs e)
        {
            string explorerDialogResult = handler.OpenWindowsExplorerDialog();

            if (!explorerDialogResult.Equals(""))
            {
                // Valid path received.
                pathToExeLbl.Text = explorerDialogResult;
            }
        }

        private void enableButtons()
        {
            browseBtn.Enabled = true;
            installBtn.Enabled = true;
            uninstallBtn.Enabled = true;
        }

        private void disableButtons()
        {
            browseBtn.Enabled = false;
            installBtn.Enabled = false;
            uninstallBtn.Enabled = false;
        }

        private void installBtn_Click(object sender, EventArgs e)
        {
            disableButtons();

            if (!isProgramReadyForInstallOrUninstall())
            {
                return;
            }

            // Install
            string installResult = handler.InstallGameBridge(pathToExeLbl.Text, (GraphicsAPITypes)graphicsApiCmb.SelectedIndex);
            if (!installResult.Equals(""))
            {
                MessageBox.Show(installResult, "Error during installation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            else
            {
                // Install successful. Inform the user.
                MessageBox.Show("Installation of Game Bridge was successful.\n\nIf you require any extra files such as savegames, resolution fixes or mods, please make sure you install those now.", "Installation successful", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }

            enableButtons();
        }

        private void uninstallBtn_Click(object sender, EventArgs e)
        {
            disableButtons();

            if (!isProgramReadyForInstallOrUninstall())
            {
                return;
            }

            // Show warning that asks user if they are sure they want to uninstall and wait for response.
            DialogResult result = MessageBox.Show("Are you sure you want to uninstall all Game Bridge files and dependencies from this game?\n\nThis will also remove files you have edited after installing Game Bridge!", "Uninstall Game Bridge", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
            if (result == DialogResult.No)
            {
                return;
            }

            // Uninstall
            string uninstallResult = handler.UninstallGameBridge(pathToExeLbl.Text, (GraphicsAPITypes)graphicsApiCmb.SelectedIndex);
            if (!uninstallResult.Equals(""))
            {
                MessageBox.Show(uninstallResult, "Error during installation", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            else
            {
                // Uninstall successful. Inform the user.
                MessageBox.Show("Uninstallation of Game Bridge was successfull.", "Uninstall successful", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }

            enableButtons();
        }

        private bool isProgramReadyForInstallOrUninstall()
        {
            // Check if ReShade files are present, if not: show warning dialog and disable install button.
            if (!handler.CheckIfReShadeInstallerPresent())
            {
                // ReShade installer not found.
                MessageBox.Show("Couldn't find a ReShade installer with addon support.\nPlease download one here:\n(https://reshade.me/)\nCopy it to the same folder as this program.", "ReShade installer not found", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                enableButtons();
                return false;
            }

            // Check if ReShade addon is present, if not: show warning dialog and disable install button.
            if (!handler.CheckIfReShadeAddonPresent())
            {
                // srReshade addon not found.
                MessageBox.Show("Couldn't find the srReshade addon.\nPlease download one here:\n(https://github.com/JoeyAnthony/3DGameBridgeProjects)\nCopy it to the same folder as this program.", "srReshade addon not found", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                enableButtons();
                return false;
            }

            // Check if SR install is present/valid, if not: show warning dialog and disable install button.
            if (!handler.CheckIfSRInstallIsValid(handler.GetSRInstallPathFromRegistry()))
            {
                // SR Install invalid
                MessageBox.Show("Couldn't find Simulated Reality install. Please (re)install the SR Platform:\n(https://www.srappstore.com/)", "SR install not found", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                enableButtons();
                return false;
            }

            // Check if the path is valid, if not valid: show warning dialog and return.
            if (pathToExeLbl.Text.Equals("") || !File.Exists(pathToExeLbl.Text))
            {
                // Path not valid, show warning.
                MessageBox.Show("Please supply a valid path to an executable (.exe).", "Invalid file path", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                enableButtons();
                return false;
            }

            return true;
        }
    }
}