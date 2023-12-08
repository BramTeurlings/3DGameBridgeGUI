namespace GameBridgeInstaller
{
    partial class Main
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Main));
            pathToExeLbl = new TextBox();
            titleLbl = new Label();
            installBtn = new Button();
            browseBtn = new Button();
            graphicsApiCmb = new ComboBox();
            graphicsApiLbl = new Label();
            uninstallBtn = new Button();
            SuspendLayout();
            // 
            // pathToExeLbl
            // 
            pathToExeLbl.Location = new Point(12, 50);
            pathToExeLbl.Name = "pathToExeLbl";
            pathToExeLbl.Size = new Size(265, 23);
            pathToExeLbl.TabIndex = 0;
            // 
            // titleLbl
            // 
            titleLbl.AutoSize = true;
            titleLbl.Location = new Point(12, 27);
            titleLbl.Margin = new Padding(3, 0, 3, 6);
            titleLbl.Name = "titleLbl";
            titleLbl.Size = new Size(218, 15);
            titleLbl.TabIndex = 0;
            titleLbl.Text = "Please select a game's executable below";
            // 
            // installBtn
            // 
            installBtn.Location = new Point(12, 144);
            installBtn.Margin = new Padding(3, 3, 9, 6);
            installBtn.Name = "installBtn";
            installBtn.Size = new Size(163, 23);
            installBtn.TabIndex = 3;
            installBtn.Text = "Install";
            installBtn.UseVisualStyleBackColor = true;
            installBtn.Click += installBtn_Click;
            // 
            // browseBtn
            // 
            browseBtn.Location = new Point(283, 50);
            browseBtn.Margin = new Padding(3, 3, 9, 3);
            browseBtn.Name = "browseBtn";
            browseBtn.Size = new Size(73, 23);
            browseBtn.TabIndex = 1;
            browseBtn.Text = "Browse";
            browseBtn.UseVisualStyleBackColor = true;
            browseBtn.Click += browseBtn_Click;
            // 
            // graphicsApiCmb
            // 
            graphicsApiCmb.DropDownStyle = ComboBoxStyle.DropDownList;
            graphicsApiCmb.FormattingEnabled = true;
            graphicsApiCmb.Location = new Point(12, 112);
            graphicsApiCmb.Margin = new Padding(3, 3, 9, 6);
            graphicsApiCmb.Name = "graphicsApiCmb";
            graphicsApiCmb.Size = new Size(346, 23);
            graphicsApiCmb.TabIndex = 2;
            // 
            // graphicsApiLbl
            // 
            graphicsApiLbl.AutoSize = true;
            graphicsApiLbl.Location = new Point(12, 88);
            graphicsApiLbl.Margin = new Padding(3, 0, 3, 6);
            graphicsApiLbl.Name = "graphicsApiLbl";
            graphicsApiLbl.Size = new Size(227, 15);
            graphicsApiLbl.TabIndex = 4;
            graphicsApiLbl.Text = "Please select a game's graphics API below";
            // 
            // uninstallBtn
            // 
            uninstallBtn.Location = new Point(195, 144);
            uninstallBtn.Margin = new Padding(3, 3, 9, 6);
            uninstallBtn.Name = "uninstallBtn";
            uninstallBtn.Size = new Size(163, 23);
            uninstallBtn.TabIndex = 4;
            uninstallBtn.Text = "Uninstall";
            uninstallBtn.UseVisualStyleBackColor = true;
            uninstallBtn.Click += uninstallBtn_Click;
            // 
            // Main
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            AutoSize = true;
            AutoSizeMode = AutoSizeMode.GrowAndShrink;
            ClientSize = new Size(456, 326);
            Controls.Add(uninstallBtn);
            Controls.Add(graphicsApiLbl);
            Controls.Add(graphicsApiCmb);
            Controls.Add(browseBtn);
            Controls.Add(installBtn);
            Controls.Add(titleLbl);
            Controls.Add(pathToExeLbl);
            Icon = (Icon)resources.GetObject("$this.Icon");
            Name = "Main";
            Text = "Game Bridge Installer";
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion
        private TextBox pathToExeLbl;
        private Label titleLbl;
        private Button installBtn;
        private Button browseBtn;
        private ComboBox graphicsApiCmb;
        private Label graphicsApiLbl;
        private Button uninstallBtn;
    }
}