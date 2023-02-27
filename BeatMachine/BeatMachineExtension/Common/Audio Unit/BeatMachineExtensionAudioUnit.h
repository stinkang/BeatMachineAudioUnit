//
//  BeatMachineExtensionAudioUnit.h
//  BeatMachineExtension
//
//  Created by Austin Kang on 2/12/23.
//

#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>

@interface BeatMachineExtensionAudioUnit : AUAudioUnit
- (void)setupParameterTree:(AUParameterTree *)parameterTree;
@end
