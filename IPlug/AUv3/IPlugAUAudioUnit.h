#import <AudioToolbox/AudioToolbox.h>

@interface IPlugAUAudioUnit : AUAudioUnit
- (void*)openWindow:(void*) pParent;
- (NSInteger)width;
- (NSInteger)height;
@end


