//
//  NSConstantCollections.m
//  CoreFoundation
//
//  Objects the compiler references for literal collections: the shared
//  empty array/dictionary singletons (@[] and @{}) and the classes used for
//  constant (statically emitted) array and dictionary literals.
//

#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>

#import <objc/runtime.h>

#import "CFInternal.h"

// Placeholders: only the class symbols are provided. The in-memory layout of
// compiler-emitted constant instances is not documented, so such instances
// are not yet supported.
CF_EXPORT
@interface NSConstantArray : NSArray
@end

@implementation NSConstantArray
@end

CF_EXPORT
@interface NSConstantDictionary : NSDictionary
@end

@implementation NSConstantDictionary
@end

// The empty singletons are statically allocated objects with no instance
// state beyond the isa pointer; the class is filled in when CoreFoundation
// loads. They are immortal, so reference counting is a no-op.
CF_PRIVATE
@interface __NSArray0 : NSArray
@end

@implementation __NSArray0

- (NSUInteger)count
{
    return 0;
}

- (id)objectAtIndex:(NSUInteger)index
{
    [NSException raise:NSRangeException format:@"index %lu beyond bounds for empty array", (unsigned long)index];
    return nil;
}

- (id)retain { return self; }
- (oneway void)release { }
- (id)autorelease { return self; }
- (NSUInteger)retainCount { return NSUIntegerMax; }
- (id)copyWithZone:(NSZone *)zone { return self; }
- (id)mutableCopyWithZone:(NSZone *)zone { return [[NSMutableArray allocWithZone:zone] init]; }

@end

CF_PRIVATE
@interface __NSDictionary0 : NSDictionary
@end

@implementation __NSDictionary0

- (NSUInteger)count
{
    return 0;
}

- (id)objectForKey:(id)key
{
    return nil;
}

- (NSEnumerator *)keyEnumerator
{
    return [[NSArray array] objectEnumerator];
}

- (id)retain { return self; }
- (oneway void)release { }
- (id)autorelease { return self; }
- (NSUInteger)retainCount { return NSUIntegerMax; }
- (id)copyWithZone:(NSZone *)zone { return self; }
- (id)mutableCopyWithZone:(NSZone *)zone { return [[NSMutableDictionary allocWithZone:zone] init]; }

@end

__attribute__((visibility("default"))) struct { Class isa; } __NSArray0__struct;
__attribute__((visibility("default"))) struct { Class isa; } __NSDictionary0__struct;

__attribute__((constructor))
static void __CFInitConstantCollections(void)
{
    __NSArray0__struct.isa = [__NSArray0 class];
    __NSDictionary0__struct.isa = [__NSDictionary0 class];
}
