/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"

#if USE(ACCELERATED_COMPOSITING)

#import "PlatformCAAnimation.h"

#import "FloatConversion.h"
#import "PlatformString.h"
#import "TimingFunction.h"
#import <QuartzCore/QuartzCore.h>
#import <wtf/UnusedParam.h>

#define HAVE_MODERN_QUARTZCORE (!defined(BUILDING_ON_TIGER) && !defined(BUILDING_ON_LEOPARD))

using namespace WebCore;

// This value must be the same as in PlatformCALayerMac.mm
static NSString * const WKNonZeroBeginTimeFlag = @"WKPlatformCAAnimationNonZeroBeginTimeFlag";

static bool hasNonZeroBeginTimeFlag(const PlatformCAAnimation* animation)
{
    return [[animation->platformAnimation() valueForKey:WKNonZeroBeginTimeFlag] boolValue];
}

static void setNonZeroBeginTimeFlag(PlatformCAAnimation* animation, bool value)
{
    [animation->platformAnimation() setValue:[NSNumber numberWithBool:value] forKey:WKNonZeroBeginTimeFlag];
}
    
static NSString* toCAFillModeType(PlatformCAAnimation::FillModeType type)
{
    switch (type) {
    case PlatformCAAnimation::NoFillMode:
    case PlatformCAAnimation::Forwards: return kCAFillModeForwards;
    case PlatformCAAnimation::Backwards: return kCAFillModeBackwards;
    case PlatformCAAnimation::Both: return kCAFillModeBoth;
    }
    return @"";
}

static PlatformCAAnimation::FillModeType fromCAFillModeType(NSString* string)
{
    if ([string isEqualToString:kCAFillModeBackwards])
        return PlatformCAAnimation::Backwards;

    if ([string isEqualToString:kCAFillModeBoth])
        return PlatformCAAnimation::Both;

    return PlatformCAAnimation::Forwards;
}

#if HAVE_MODERN_QUARTZCORE
static NSString* toCAValueFunctionType(PlatformCAAnimation::ValueFunctionType type)
{
    switch (type) {
    case PlatformCAAnimation::NoValueFunction: return @"";
    case PlatformCAAnimation::RotateX: return kCAValueFunctionRotateX;
    case PlatformCAAnimation::RotateY: return kCAValueFunctionRotateY;
    case PlatformCAAnimation::RotateZ: return kCAValueFunctionRotateZ;
    case PlatformCAAnimation::ScaleX: return kCAValueFunctionScaleX;
    case PlatformCAAnimation::ScaleY: return kCAValueFunctionScaleY;
    case PlatformCAAnimation::ScaleZ: return kCAValueFunctionScaleZ;
    case PlatformCAAnimation::Scale: return kCAValueFunctionScale;
    case PlatformCAAnimation::TranslateX: return kCAValueFunctionTranslateX;
    case PlatformCAAnimation::TranslateY: return kCAValueFunctionTranslateY;
    case PlatformCAAnimation::TranslateZ: return kCAValueFunctionTranslateZ;
    case PlatformCAAnimation::Translate: return kCAValueFunctionTranslate;
    }
    return @"";
}

static PlatformCAAnimation::ValueFunctionType fromCAValueFunctionType(NSString* string)
{
    if ([string isEqualToString:kCAValueFunctionRotateX])
        return PlatformCAAnimation::RotateX;

    if ([string isEqualToString:kCAValueFunctionRotateY])
        return PlatformCAAnimation::RotateY;

    if ([string isEqualToString:kCAValueFunctionRotateZ])
        return PlatformCAAnimation::RotateZ;

    if ([string isEqualToString:kCAValueFunctionScaleX])
        return PlatformCAAnimation::ScaleX;

    if ([string isEqualToString:kCAValueFunctionScaleY])
        return PlatformCAAnimation::ScaleY;

    if ([string isEqualToString:kCAValueFunctionScaleZ])
        return PlatformCAAnimation::ScaleZ;

    if ([string isEqualToString:kCAValueFunctionScale])
        return PlatformCAAnimation::Scale;

    if ([string isEqualToString:kCAValueFunctionTranslateX])
        return PlatformCAAnimation::TranslateX;

    if ([string isEqualToString:kCAValueFunctionTranslateY])
        return PlatformCAAnimation::TranslateY;

    if ([string isEqualToString:kCAValueFunctionTranslateZ])
        return PlatformCAAnimation::TranslateZ;

    if ([string isEqualToString:kCAValueFunctionTranslate])
        return PlatformCAAnimation::Translate;

    return PlatformCAAnimation::NoValueFunction;
}
#endif

static CAMediaTimingFunction* toCAMediaTimingFunction(const TimingFunction* timingFunction)
{
    if (!timingFunction)
        return [CAMediaTimingFunction functionWithControlPoints:0.25f :0.1f :0.25f :0.1f];
            
    if (timingFunction->isCubicBezierTimingFunction()) {
        const CubicBezierTimingFunction* ctf = static_cast<const CubicBezierTimingFunction*>(timingFunction);
        return [CAMediaTimingFunction functionWithControlPoints:static_cast<float>(ctf->x1()) :static_cast<float>(ctf->y1())
                                                               :static_cast<float>(ctf->x2()) :static_cast<float>(ctf->y2())];
    }
    
    return [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];

    return 0;
}

PassRefPtr<PlatformCAAnimation> PlatformCAAnimation::create(AnimationType type, const String& keyPath)
{
    return adoptRef(new PlatformCAAnimation(type, keyPath));
}

PassRefPtr<PlatformCAAnimation> PlatformCAAnimation::create(PlatformAnimationRef animation)
{
    return adoptRef(new PlatformCAAnimation(animation));
}

PassRefPtr<PlatformCAAnimation> PlatformCAAnimation::create(const PlatformCAAnimation* animation)
{
    return adoptRef(new PlatformCAAnimation(animation));
}

PlatformCAAnimation::PlatformCAAnimation(AnimationType type, const String& keyPath)
    : m_type(type)
{
    if (type == Basic)
        m_animation.adoptNS([[CABasicAnimation animationWithKeyPath:keyPath] retain]);
    else
        m_animation.adoptNS([[CAKeyframeAnimation animationWithKeyPath:keyPath] retain]);
}

PlatformCAAnimation::PlatformCAAnimation(PlatformAnimationRef animation)
{
    if ([static_cast<CAAnimation*>(animation) isKindOfClass:[CABasicAnimation class]])
        m_type = Basic;
    else if ([static_cast<CAAnimation*>(animation) isKindOfClass:[CAKeyframeAnimation class]])
        m_type = Keyframe;
    else {
        ASSERT(0);
        return;
    }
    
    m_animation = static_cast<CAPropertyAnimation*>(animation);
}

PlatformCAAnimation::PlatformCAAnimation(const PlatformCAAnimation* animation)
{
    PlatformCAAnimation* newAnimation = new PlatformCAAnimation(animation->animationType(), animation->keyPath());

    newAnimation->setBeginTime(animation->beginTime());
    newAnimation->setDuration(animation->duration());
    newAnimation->setSpeed(animation->speed());
    newAnimation->setTimeOffset(animation->timeOffset());
    newAnimation->setRepeatCount(animation->repeatCount());
    newAnimation->setAutoreverses(animation->autoreverses());
    newAnimation->setFillMode(animation->fillMode());
    newAnimation->setRemovedOnCompletion(animation->isRemovedOnCompletion());
    newAnimation->setAdditive(animation->isAdditive());
    newAnimation->copyTimingFunctionFrom(animation);

#if HAVE_MODERN_QUARTZCORE
    newAnimation->setValueFunction(animation->valueFunction());
#endif

    setNonZeroBeginTimeFlag(newAnimation, hasNonZeroBeginTimeFlag(animation));
    
    // Copy the specific Basic or Keyframe values
    if (animation->animationType() == Keyframe) {
        newAnimation->copyValuesFrom(animation);
        newAnimation->copyKeyTimesFrom(animation);
        newAnimation->copyTimingFunctionsFrom(animation);
    } else {
        newAnimation->copyFromValueFrom(animation);
        newAnimation->copyToValueFrom(animation);
    }
}

PlatformCAAnimation::~PlatformCAAnimation()
{
}

bool PlatformCAAnimation::supportsValueFunction()
{
    static bool sHaveValueFunction = [CAPropertyAnimation instancesRespondToSelector:@selector(setValueFunction:)];
    return sHaveValueFunction;
}

PlatformAnimationRef PlatformCAAnimation::platformAnimation() const
{
    return m_animation.get();
}

String PlatformCAAnimation::keyPath() const
{
    return [m_animation.get() keyPath];
}

CFTimeInterval PlatformCAAnimation::beginTime() const
{
    return [m_animation.get() beginTime];
}

void PlatformCAAnimation::setBeginTime(CFTimeInterval value)
{
    [m_animation.get() setBeginTime:value];
    
    // Also set a flag to tell us if we've passed in a 0 value. 
    // The flag is needed because later beginTime will get changed
    // to the time at which it fired and we need to know whether
    // or not it was 0 to begin with.
    if (value)
        setNonZeroBeginTimeFlag(this, true);
}

CFTimeInterval PlatformCAAnimation::duration() const
{
    return [m_animation.get() duration];
}

void PlatformCAAnimation::setDuration(CFTimeInterval value)
{
    [m_animation.get() setDuration:value];
}

float PlatformCAAnimation::speed() const
{
    return [m_animation.get() speed];
}

void PlatformCAAnimation::setSpeed(float value)
{
    [m_animation.get() setSpeed:value];
}

CFTimeInterval PlatformCAAnimation::timeOffset() const
{
    return [m_animation.get() timeOffset];
}

void PlatformCAAnimation::setTimeOffset(CFTimeInterval value)
{
    [m_animation.get() setTimeOffset:value];
}

float PlatformCAAnimation::repeatCount() const
{
    return [m_animation.get() repeatCount];
}

void PlatformCAAnimation::setRepeatCount(float value)
{
    [m_animation.get() setRepeatCount:value];
}

bool PlatformCAAnimation::autoreverses() const
{
    return [m_animation.get() autoreverses];
}

void PlatformCAAnimation::setAutoreverses(bool value)
{
    [m_animation.get() setAutoreverses:value];
}

PlatformCAAnimation::FillModeType PlatformCAAnimation::fillMode() const
{
    return fromCAFillModeType([m_animation.get() fillMode]);
}

void PlatformCAAnimation::setFillMode(FillModeType value)
{
    [m_animation.get() setFillMode:toCAFillModeType(value)];
}

void PlatformCAAnimation::setTimingFunction(const TimingFunction* value)
{
    [m_animation.get() setTimingFunction:toCAMediaTimingFunction(value)];
}

void PlatformCAAnimation::copyTimingFunctionFrom(const PlatformCAAnimation* value)
{
    [m_animation.get() setTimingFunction:[value->m_animation.get() timingFunction]];
}

bool PlatformCAAnimation::isRemovedOnCompletion() const
{
    return [m_animation.get() isRemovedOnCompletion];
}

void PlatformCAAnimation::setRemovedOnCompletion(bool value)
{
    [m_animation.get() setRemovedOnCompletion:value];
}

bool PlatformCAAnimation::isAdditive() const
{
    return [m_animation.get() isAdditive];
}

void PlatformCAAnimation::setAdditive(bool value)
{
    [m_animation.get() setAdditive:value];
}

PlatformCAAnimation::ValueFunctionType PlatformCAAnimation::valueFunction() const
{
#if HAVE_MODERN_QUARTZCORE
    CAValueFunction* vf = [m_animation.get() valueFunction];
    return fromCAValueFunctionType([vf name]);
#else
    return NoValueFunction;
#endif
}

void PlatformCAAnimation::setValueFunction(ValueFunctionType value)
{
#if HAVE_MODERN_QUARTZCORE
    [m_animation.get() setValueFunction:[CAValueFunction functionWithName:toCAValueFunctionType(value)]];
#else
    UNUSED_PARAM(value);
#endif
}

void PlatformCAAnimation::setFromValue(float value)
{
    if (animationType() != Basic)
        return;
    [static_cast<CABasicAnimation*>(m_animation.get()) setFromValue:[NSNumber numberWithDouble:value]];
}

void PlatformCAAnimation::setFromValue(const WebCore::TransformationMatrix& value)
{
    if (animationType() != Basic)
        return;
        
    [static_cast<CABasicAnimation*>(m_animation.get()) setFromValue:[NSValue valueWithCATransform3D:value]];
}

void PlatformCAAnimation::setFromValue(const FloatPoint3D& value)
{
    if (animationType() != Basic)
        return;

    NSArray* array = [NSArray arrayWithObjects:
                        [NSNumber numberWithDouble:value.x()],
                        [NSNumber numberWithDouble:value.y()],
                        [NSNumber numberWithDouble:value.z()],
                        nil];
    [static_cast<CABasicAnimation*>(m_animation.get()) setFromValue:array];
}

void PlatformCAAnimation::setFromValue(const WebCore::Color& value)
{
    if (animationType() != Basic)
        return;

    NSArray* array = [NSArray arrayWithObjects:
                        [NSNumber numberWithDouble:value.red()],
                        [NSNumber numberWithDouble:value.green()],
                        [NSNumber numberWithDouble:value.blue()],
                        [NSNumber numberWithDouble:value.alpha()],
                        nil];
    [static_cast<CABasicAnimation*>(m_animation.get()) setFromValue:array];
}

void PlatformCAAnimation::copyFromValueFrom(const PlatformCAAnimation* value)
{
    if (animationType() != Basic || value->animationType() != Basic)
        return;
        
    CABasicAnimation* otherAnimation = static_cast<CABasicAnimation*>(value->m_animation.get());
    [static_cast<CABasicAnimation*>(m_animation.get()) setFromValue:[otherAnimation fromValue]];
}

void PlatformCAAnimation::setToValue(float value)
{
    if (animationType() != Basic)
        return;
    [static_cast<CABasicAnimation*>(m_animation.get()) setToValue:[NSNumber numberWithDouble:value]];
}

void PlatformCAAnimation::setToValue(const WebCore::TransformationMatrix& value)
{
    if (animationType() != Basic)
        return;

    [static_cast<CABasicAnimation*>(m_animation.get()) setToValue:[NSValue valueWithCATransform3D:value]];
}

void PlatformCAAnimation::setToValue(const FloatPoint3D& value)
{
    if (animationType() != Basic)
        return;

    NSArray* array = [NSArray arrayWithObjects:
                        [NSNumber numberWithDouble:value.x()],
                        [NSNumber numberWithDouble:value.y()],
                        [NSNumber numberWithDouble:value.z()],
                        nil];
    [static_cast<CABasicAnimation*>(m_animation.get()) setToValue:array];
}

void PlatformCAAnimation::setToValue(const WebCore::Color& value)
{
    if (animationType() != Basic)
        return;

    NSArray* array = [NSArray arrayWithObjects:
                        [NSNumber numberWithDouble:value.red()],
                        [NSNumber numberWithDouble:value.green()],
                        [NSNumber numberWithDouble:value.blue()],
                        [NSNumber numberWithDouble:value.alpha()],
                        nil];
    [static_cast<CABasicAnimation*>(m_animation.get()) setToValue:array];
}

void PlatformCAAnimation::copyToValueFrom(const PlatformCAAnimation* value)
{
    if (animationType() != Basic || value->animationType() != Basic)
        return;
        
    CABasicAnimation* otherAnimation = static_cast<CABasicAnimation*>(value->m_animation.get());
    [static_cast<CABasicAnimation*>(m_animation.get()) setToValue:[otherAnimation toValue]];
}


// Keyframe-animation properties.
void PlatformCAAnimation::setValues(const Vector<float>& value)
{
    if (animationType() != Keyframe)
        return;
        
    NSMutableArray* array = [NSMutableArray array];
    for (size_t i = 0; i < value.size(); ++i)
        [array addObject:[NSNumber numberWithDouble:value[i]]];
    [static_cast<CAKeyframeAnimation*>(m_animation.get()) setValues:array];
}

void PlatformCAAnimation::setValues(const Vector<WebCore::TransformationMatrix>& value)
{
    if (animationType() != Keyframe)
        return;
        
    NSMutableArray* array = [NSMutableArray array];

    for (size_t i = 0; i < value.size(); ++i)
        [array addObject:[NSValue valueWithCATransform3D:value[i]]];
        
    [static_cast<CAKeyframeAnimation*>(m_animation.get()) setValues:array];
}

void PlatformCAAnimation::setValues(const Vector<FloatPoint3D>& value)
{
    if (animationType() != Keyframe)
        return;
        
    NSMutableArray* array = [NSMutableArray array];

    for (size_t i = 0; i < value.size(); ++i) {
        NSArray* object = [NSArray arrayWithObjects:
                        [NSNumber numberWithDouble:value[i].x()],
                        [NSNumber numberWithDouble:value[i].y()],
                        [NSNumber numberWithDouble:value[i].z()],
                        nil];
        [array addObject:object];
    }
    [static_cast<CAKeyframeAnimation*>(m_animation.get()) setValues:array];
}

void PlatformCAAnimation::setValues(const Vector<WebCore::Color>& value)
{
    if (animationType() != Keyframe)
        return;
        
    NSMutableArray* array = [NSMutableArray array];

    for (size_t i = 0; i < value.size(); ++i) {
        NSArray* object = [NSArray arrayWithObjects:
                        [NSNumber numberWithDouble:value[i].red()],
                        [NSNumber numberWithDouble:value[i].green()],
                        [NSNumber numberWithDouble:value[i].blue()],
                        [NSNumber numberWithDouble:value[i].alpha()],
                        nil];
        [array addObject:object];
    }
    [static_cast<CAKeyframeAnimation*>(m_animation.get()) setValues:array];
}

void PlatformCAAnimation::copyValuesFrom(const PlatformCAAnimation* value)
{
    if (animationType() != Keyframe || value->animationType() != Keyframe)
        return;
        
    CAKeyframeAnimation* otherAnimation = static_cast<CAKeyframeAnimation*>(value->m_animation.get());
    [static_cast<CAKeyframeAnimation*>(m_animation.get()) setValues:[otherAnimation values]];
}

void PlatformCAAnimation::setKeyTimes(const Vector<float>& value)
{
    NSMutableArray* array = [NSMutableArray array];

    for (size_t i = 0; i < value.size(); ++i)
        [array addObject:[NSNumber numberWithFloat:value[i]]];
    
    [static_cast<CAKeyframeAnimation*>(m_animation.get()) setKeyTimes:array];
}

void PlatformCAAnimation::copyKeyTimesFrom(const PlatformCAAnimation* value)
{
    CAKeyframeAnimation* other = static_cast<CAKeyframeAnimation*>(value->m_animation.get());
    [static_cast<CAKeyframeAnimation*>(m_animation.get()) setKeyTimes:[other keyTimes]];
}

void PlatformCAAnimation::setTimingFunctions(const Vector<const TimingFunction*>& value)
{
    NSMutableArray* array = [NSMutableArray array];

    for (size_t i = 0; i < value.size(); ++i)
        [array addObject:toCAMediaTimingFunction(value[i])];
    
    [static_cast<CAKeyframeAnimation*>(m_animation.get()) setTimingFunctions:array];
}

void PlatformCAAnimation::copyTimingFunctionsFrom(const PlatformCAAnimation* value)
{
    CAKeyframeAnimation* other = static_cast<CAKeyframeAnimation*>(value->m_animation.get());
    [static_cast<CAKeyframeAnimation*>(m_animation.get()) setTimingFunctions:[other timingFunctions]];
}

#endif // USE(ACCELERATED_COMPOSITING)
