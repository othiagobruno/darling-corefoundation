/*	CFStringHyphenation.c

	No hyphenation dictionaries are available, so no locale is reported as
	supported and no hyphenation point is ever suggested.
*/

#include <CoreFoundation/CFString.h>
#include "CFInternal.h"

CFIndex CFStringGetHyphenationLocationBeforeIndex(CFStringRef string, CFIndex location, CFRange limitRange, CFOptionFlags options, CFLocaleRef locale, UTF32Char *character) {
    return kCFNotFound;
}

Boolean CFStringIsHyphenationAvailableForLocale(CFLocaleRef locale) {
    return false;
}
