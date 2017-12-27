//
//  ViewController.m
//  CVM
//
//  Created by chudanqin on 13/12/2017.
//  Copyright © 2017 chudanqin. All rights reserved.
//

#import "ViewController.h"

#import "SettingsViewController.h"
#import "cvm.h"

@interface ViewController ()

@property (weak, nonatomic) IBOutlet UITextView *sourceTextView;

@property (weak, nonatomic) IBOutlet UITextView *assemblyTextView;

@property (weak, nonatomic) IBOutlet UITextView *outputTextView;

@property (nonatomic) NSMutableString *assemblyText;

@property (nonatomic) NSMutableString *outputText;

@end

int my_console_output_func(void *context, const char *restrict format, va_list va_l) {
    ViewController *vc = (__bridge ViewController *)context;
    NSString *ocFormat = [NSString stringWithUTF8String:format];
    NSString *str = [[NSString alloc] initWithFormat:ocFormat arguments:va_l];
    [vc.outputText appendString:str];
    return 0;
}

int my_console_assembly_func(void *context, const char *restrict format, va_list va_l) {
    ViewController *vc = (__bridge ViewController *)context;
    NSString *ocFormat = [NSString stringWithUTF8String:format];
    NSString *str = [[NSString alloc] initWithFormat:ocFormat arguments:va_l];
    [vc.assemblyText appendString:str];
    return 0;
}

void my_exit_signal_func(void *context, int code) {
    ViewController *vc = (__bridge ViewController *)context;
    [vc.outputText appendFormat:@"exit(%d)\n", code];
    vc.outputTextView.text = vc.outputText;
    vc.assemblyTextView.text = vc.assemblyText;
}

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    NSString *srcPath = [[NSBundle mainBundle] pathForResource:@"tt" ofType:@"txt"];
    NSString *srcText = [[NSString alloc] initWithContentsOfFile:srcPath encoding:NSUTF8StringEncoding error:NULL];
    
    _sourceTextView.text = srcText;
    
    console_output_set_handler((__bridge void *)self, my_console_output_func);
    console_assembly_set_handler((__bridge void *)self, my_console_assembly_func);
    exit_signal_set_handler((__bridge void *)self, my_exit_signal_func);
    
    self.navigationItem.rightBarButtonItems = @[
                                                [[UIBarButtonItem alloc] initWithTitle:@"Run" style:UIBarButtonItemStylePlain target:self action:@selector(run:)],
                                                
                                                [[UIBarButtonItem alloc] initWithTitle:@"Conf" style:UIBarButtonItemStyleDone target:self action:@selector(config:)]
                                                ];
}

- (void)run:(UIBarButtonItem *)bbi {
    [self.view.window endEditing:YES];
    
    _assemblyText = [[NSMutableString alloc] initWithCapacity:1000];
    _outputText = [[NSMutableString alloc] initWithCapacity:100];
    
    _assemblyTextView.text = nil;
    _outputTextView.text = nil;
    NSString *src = _sourceTextView.text;
    char *ascii_src = (char *)[src UTF8String];
    vm_init();
    vm_eval(ascii_src);
}

- (void)config:(UIBarButtonItem *)bbi {
    UIStoryboard *sb = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    UIViewController *vc = [sb instantiateViewControllerWithIdentifier:@"Settings"];
    [self.navigationController pushViewController:vc animated:NO];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
