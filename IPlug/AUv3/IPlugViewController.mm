#import <CoreAudioKit/AUViewController.h>
#import "IPlugAUAudioUnit.h"
#include "IPlugLogger.h"

//#define PLATFORM_VIEW UIView
#define PLATFORM_VIEW NSView

@interface IPlugViewController : AUViewController <AUAudioUnitFactory>
@property (nonatomic, retain) IPlugAUAudioUnit* audioUnit; // TODO: what is @property (nonatomic)
@end

@implementation IPlugViewController
//  AUParameterObserverToken parameterObserverToken;

- (AUAudioUnit*) createAudioUnitWithComponentDescription:(AudioComponentDescription) desc error:(NSError **)error
{
  TRACE;
  self.audioUnit = [[[IPlugAUAudioUnit alloc] initWithComponentDescription:desc error:error] retain];
  
  return self.audioUnit;
}
//
//- (void) viewDidLoad
//{
//  TRACE;
//  self.preferredContentSize = NSMakeSize(480, 122);
//
//  [super viewDidLoad];
//}

- (AUAudioUnit *)getAudioUnit
{
  TRACE;

  return _audioUnit;
}

- (void)setAudioUnit:(IPlugAUAudioUnit*) audioUnit
{
  TRACE;

  _audioUnit = audioUnit;
  
  dispatch_async(dispatch_get_main_queue(), ^
  {
    [_audioUnit openWindow:(void*)self.view];
    int viewWidth = 300;
    int viewHeight = 300;
    CGRect newSize = CGRectMake (0, 0, viewWidth, viewHeight);
    [self setFrame:newSize];
    self.preferredContentSize = CGSizeMake (viewWidth, viewHeight);
  });
}

//- (void)observeValueForKeyPath:(NSString *)keyPath
//                      ofObject:(id)object
//                        change:(NSDictionary<NSString *, id> *)change
//                       context:(void *)context
//{
//  NSLog(@"IPlugViewController allParameterValues key path changed: %s\n", keyPath.UTF8String);
//
//  dispatch_async(dispatch_get_main_queue(), ^
//  {
//    //TODO:
//  });
//}

//- (void)connectViewWithAU
//{
//  AUParameterTree *paramTree = _audioUnit.parameterTree;
//
//  if (paramTree)
//  {
//    [_audioUnit addObserver:self forKeyPath:@"allParameterValues"
//                    options:NSKeyValueObservingOptionNew
//                    context:parameterObserverToken];
//  }
//  else
//  {
//    NSLog(@"paramTree is NULL!\n");
//  }
//}
//
//- (void)disconnectViewWithAU
//{
//  if (parameterObserverToken)
//  {
//    [_audioUnit.parameterTree removeParameterObserver:parameterObserverToken];
//    [_audioUnit removeObserver:self forKeyPath:@"allParameterValues" context:parameterObserverToken];
//    parameterObserverToken = 0;
//  }
//}
//
- (void)setFrame:(CGRect)newSize
{
  TRACE;

  [super.view setFrame:newSize];

  //TODO: _audioUnit OnResize
}

- (void)viewDidLayoutSubviews
{
  TRACE;

}

- (NSIndexSet *)supportedViewConfigurations:(NSArray<AUAudioUnitViewConfiguration *> *)availableViewConfigurations
{
  TRACE;

  NSIndexSet* pSet = [[NSMutableIndexSet alloc] init];

  return pSet;
}

- (void)selectViewConfiguration:(AUAudioUnitViewConfiguration *)viewConfiguration
{
  TRACE;

}


@end


