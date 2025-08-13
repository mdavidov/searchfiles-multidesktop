/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the FOO module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

var maintenanceToolSize = 38 * 1024 * 1024;

function Component()
{
    installer.installationFinished.connect(this, Component.prototype.installationFinishedPageIsShown);
    installer.finishButtonClicked.connect(this, Component.prototype.installationFinished);
    // IMPORTANT: installer.requiredDiskSpace is a READ-ONLY PROPERTY!
    // installer.requiredDiskSpace = function() {
    //     return maintenanceToolSize;
    // }
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    // Does NOT work: Add a custom operation to calculate the total disk space
    // component.addOperation("CustomOperation", "calculateTotalDiskSpace");

    if (systemInfo.productType === "windows") {
        // Create shortcuts
        component.addOperation("CreateShortcut",
                               "@TargetDir@/foldersearch.exe",
                               "@StartMenuDir@/FolderSearch.lnk",
                               "workingDirectory=@TargetDir@");

        component.addOperation("CreateShortcut",
                               "@TargetDir@/foldersearch.exe",
                               "@DesktopDir@/FolderSearch.lnk",
                               "workingDirectory=@TargetDir@");
    }
    else if (systemInfo.productType === "mac") {
       // MacOS-specific operations
       component.addOperation("CreateDesktopEntry",
                             "@HomeDir@/Desktop/FolderSearch.desktop",
                             "Type=Application\nTerminal=false\nExec=@TargetDir@/foldersearch\nName=FolderSearch\nIcon=@TargetDir@/icons/foldersearch.icns");

       // Optional: Create application symlink in /Applications
       component.addOperation("Execute", "ln", "-sf",
                             "@TargetDir@",
                             "/Applications/FolderSearch");

       // Set executable permissions
       component.addOperation("Execute", "chmod", "+x",
                             "@TargetDir@/foldersearch");
    }
    else if (systemInfo.productType === "x11") {
       // Linux-specific operations
       component.addOperation("CreateDesktopEntry",
                             "@HomeDir@/Desktop/FolderSearch.desktop",
                             "Type=Application\nTerminal=false\nExec=@TargetDir@/foldersearch\nName=FolderSearch\nIcon=@TargetDir@/icons/foldersearch.png");

       // Create menu entry
       component.addOperation("CreateDesktopEntry",
                             "/usr/share/applications/FolderSearch.desktop",
                             "Type=Application\nTerminal=false\nExec=@TargetDir@/foldersearch\nName=FolderSearch\nIcon=@TargetDir@/icons/foldersearch.png\nCategories=Utility;");

       // Set executable permissions
       component.addOperation("Execute", "chmod", "+x",
                             "@TargetDir@/foldersearch");
   }
}

Component.prototype.installationFinishedPageIsShown = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {
            installer.addWizardPageItem( component, "ReadMeCheckBoxForm", QInstaller.InstallationFinished );
        }
    } catch(e) {
        console.log(e);
    }
}

Component.prototype.installationFinished = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {
            var checkboxForm = component.userInterface( "ReadMeCheckBoxForm" );
            if (checkboxForm && checkboxForm.readMeCheckBox.checked) {
                QDesktopServices.openUrl("file:///" + installer.value("TargetDir") + "/README.md");
            }
        }
    } catch(e) {
        console.log(e);
    }
}

