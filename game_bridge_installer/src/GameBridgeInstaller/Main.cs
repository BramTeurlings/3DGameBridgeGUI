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

        private void installBtn_Click(object sender, EventArgs e)
        {
            // Check if ReShade files are present, if not: show warning dialog and disable install button.
            if (!handler.CheckIfReShadeInstallerPresent())
            {
                // ReShade installer not found.
                MessageBox.Show("Couldn't find a ReShade installer with addon support.\nPlease download one here:\n(https://reshade.me/)\nCopy it to the same folder as this program.", "ReShade installer not found", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            // Check if ReShade addon is present, if not: show warning dialog and disable install button.
            if (!handler.CheckIfReShadeAddonPresent())
            {
                // srReshade addon not found.
                MessageBox.Show("Couldn't find the srReshade addon.\nPlease download one here:\n(https://github.com/JoeyAnthony/3DGameBridgeProjects)\nCopy it to the same folder as this program.", "srReshade addon not found", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            // Check if SR install is present/valid, if not: show warning dialog and disable install button.
            if (!handler.CheckIfSRInstallIsValid(handler.GetSRInstallPathFromRegistry()))
            {
                // SR Install invalid
                MessageBox.Show("Couldn't find Simulated Reality install. Please (re)install the SR Platform:\n(https://www.srappstore.com/)", "SR install not found", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            // Check if the path is valid, if not valid: show warning dialog and return.
            if (pathToExeLbl.Text.Equals("") || !File.Exists(pathToExeLbl.Text))
            {
                // Path not valid, show warning.
                MessageBox.Show("Please supply a valid path to an executable (.exe).", "Invalid file path", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }

            // Install
            handler.InstallGameBridge(pathToExeLbl.Text, (GraphicsAPITypes)graphicsApiCmb.SelectedIndex);

            // Check if fix files are present. If no fix files were found: show warning dialog and return.
        }
    }
}