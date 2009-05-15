/*
 * viceappcontroller.h - VICEAppController - base app controller class
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#import <Cocoa/Cocoa.h>
#import "drivesettingswindowcontroller.h"
#import "iecdrivesettingswindowcontroller.h"
#import "printersettingswindowcontroller.h"
#import "keyboardsettingswindowcontroller.h"
#import "joysticksettingswindowcontroller.h"
#import "soundsettingswindowcontroller.h"
#import "videosettingswindowcontroller.h"

#import "viceinformationwindowcontroller.h"
#import "resourceeditorcontroller.h"
#import "recordhistorywindowcontroller.h"
#import "recordmediawindowcontroller.h"
#import "netplaycontrolwindowcontroller.h"

@interface VICEAppController : NSObject
{
    // Dialog Controllers
    DriveSettingsWindowController *driveSettingsController;
    IECDriveSettingsWindowController *iecDriveSettingsController;
    PrinterSettingsWindowController *printerSettingsController;
    KeyboardSettingsWindowController *keyboardSettingsController;
    JoystickSettingsWindowController *joystickSettingsController;
    SoundSettingsWindowController *soundSettingsController;
    VideoSettingsWindowController *videoSettingsController;

    VICEInformationWindowController *infoController;
    ResourceEditorController *resourceEditorController;
    RecordHistoryWindowController *recordHistoryController;
    RecordMediaWindowController *recordMediaController;
    NetplayControlWindowController *netplayController;
    
    // Options Outlets
    IBOutlet NSMenu *refreshRateMenu;
    IBOutlet NSMenu *maximumSpeedMenu;
    IBOutlet NSMenuItem *warpModeMenuItem;
    
    IBOutlet NSMenu *machineVideoStandardMenu;
    
    IBOutlet NSMenuItem *soundPlaybackMenuItem;
    IBOutlet NSMenuItem *trueDriveEmulationMenuItem;
    IBOutlet NSMenuItem *virtualDevicesMenuItem;
    IBOutlet NSMenuItem *emuIDMenuItem;
    IBOutlet NSMenuItem *keySetEnableMenuItem;
    
    // Settings Outlets
    IBOutlet NSMenuItem *saveResourcesOnExitMenuItem;
    IBOutlet NSMenuItem *confirmOnExitMenuItem;
}

// File
- (IBAction)togglePause:(id)sender;
- (IBAction)resetMachine:(id)sender;
- (IBAction)resetDrive:(id)sender;

- (IBAction)activateMonitor:(id)sender;

- (IBAction)smartAttachImage:(id)sender;

- (IBAction)attachDiskImage:(id)sender;
- (IBAction)createAndAttachDiskImage:(id)sender;
- (IBAction)detachDiskImage:(id)sender;

- (IBAction)fliplistAddCurrentImage:(id)sender;
- (IBAction)fliplistAttachNextImage:(id)sender;
- (IBAction)fliplistAttachPrevImage:(id)sender;
- (IBAction)fliplistLoad:(id)sender;
- (IBAction)fliplistRemoveCurrentImage:(id)sender;
- (IBAction)fliplistSave:(id)sender;

- (IBAction)attachTapeImage:(id)sender;
- (IBAction)detachTapeImage:(id)sender;

// Snapshot
- (IBAction)loadSnapshot:(id)sender;
- (IBAction)saveSnapshot:(id)sender;
- (IBAction)loadQuickSnapshot:(id)sender;
- (IBAction)saveQuickSnapshot:(id)sender;

- (IBAction)showRecordHistory:(id)sender;
- (IBAction)showRecordMedia:(id)sender;

- (IBAction)showNetplay:(id)sender; 

// Options
- (IBAction)pickRefreshRate:(id)sender;
- (IBAction)pickMaximumSpeed:(id)sender;
- (IBAction)pickCustomMaximumSpeed:(id)sender;
- (IBAction)toggleWarpMode:(id)sender;

- (IBAction)toggleMachineVideoStandard:(id)sender;

- (IBAction)toggleSoundPlayback:(id)sender;
- (IBAction)toggleTrueDriveEmulation:(id)sender;
- (IBAction)toggleVirtualDevices:(id)sender;

- (IBAction)swapJoysticks:(id)sender;
- (IBAction)toggleKeySetEnable:(id)sender;
- (IBAction)toggleEmuID:(id)sender;

// Settings
- (IBAction)showDriveSettings:(id)sender;
- (IBAction)showIECDriveSettings:(id)sender;
- (IBAction)showPrinterSettings:(id)sender;
- (IBAction)showKeyboardSettings:(id)sender;
- (IBAction)showJoystickSettings:(id)sender;
- (IBAction)showSoundSettings:(id)sender;
- (IBAction)showVideoSettings:(id)sender;

// Resources
- (IBAction)showResourceEditor:(id)sender;
- (IBAction)saveResources:(id)sender;
- (IBAction)loadResources:(id)sender;
- (IBAction)resetResources:(id)sender;
- (IBAction)toggleSaveResourcesOnExit:(id)sender;
- (IBAction)toggleConfirmOnExit:(id)sender;

// Help
- (IBAction)showInformation:(id)sender;

// Tools
- (NSString *)pickOpenFileWithTitle:(NSString *)title types:(NSArray *)types;
- (NSString *)pickSaveFileWithTitle:(NSString *)title types:(NSArray *)types;
- (NSString *)pickDirectoryWithTitle:(NSString *)title;

- (void)attachDiskImageForUnit:(int)unit;

- (void)updateResources:(id)sender;
- (void)updateSettingsResources;
- (void)updateOptionsResources;
- (void)updateMachineResources;

- (BOOL)updateSubMenuCheckState:(NSMenu *)menu withTag:(int)tagValue;

- (BOOL)setIntResource:(NSString *)name toValue:(int)value;
- (int)getIntResource:(NSString *)name;

@end
