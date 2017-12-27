//
//  SettingsViewController.m
//  CVM
//
//  Created by chudanqin on 25/12/2017.
//  Copyright © 2017 chudanqin. All rights reserved.
//

#import "SettingsViewController.h"

#include "cvm.h"

@interface SettingsViewController ()

@property (nonatomic, weak) IBOutlet UISwitch *debugSwitch;

@property (nonatomic, weak) IBOutlet UISwitch *compileOnlySwitch;

@end

@implementation SettingsViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    _debugSwitch.on = is_debug_enabled();
    _compileOnlySwitch.on = is_compile_only_enabled();
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)debugEnabled:(id)sender {
    debug_enabled(_debugSwitch.isOn);
}

- (IBAction)compileOnlyEnabled:(id)sender {
    compile_only_enabled(_compileOnlySwitch.isOn);
}

@end
