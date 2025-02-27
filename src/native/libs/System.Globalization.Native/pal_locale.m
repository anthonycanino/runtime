// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#include <stdlib.h>
#include "pal_locale_internal.h"

#import <Foundation/Foundation.h>

char* DetectDefaultAppleLocaleName()
{
    NSLocale *currentLocale = [NSLocale currentLocale];
    NSString *localeName = @"";
    
    if (!currentLocale)
    {
        return strdup([localeName UTF8String]);
    }

    if ([currentLocale.languageCode length] > 0 && [currentLocale.countryCode length] > 0)
    {
        localeName = [NSString stringWithFormat:@"%@-%@", currentLocale.languageCode, currentLocale.countryCode];
    }
    else
    {
        localeName = currentLocale.localeIdentifier;
    }
    
    return strdup([localeName UTF8String]);
}

#if defined(TARGET_MACCATALYST) || defined(TARGET_IOS) || defined(TARGET_TVOS)
const char* GlobalizationNative_GetICUDataPathFallback(void)
{
    NSString *bundlePath = [[NSBundle mainBundle] pathForResource:@"icudt" ofType:@"dat"];
    return strdup([bundlePath UTF8String]);
}
#endif
