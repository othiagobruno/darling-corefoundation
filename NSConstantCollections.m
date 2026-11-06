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

// Constant (statically emitted) array and dictionary literals. The compiler
// lays these objects out as an isa pointer followed by the fields declared
// below, so the ivar order is significant. The objects live in the image's
// data segment and are never freed, hence the no-op reference counting.
//
// Layout observed from the objects the compiler emits (black-box, from
// constant literals present in a shipping Mach-O image): an array is
// { isa, count, objects }; a dictionary is { isa, <word that was always 1>,
// count, keys, objects }.
CF_EXPORT
@interface NSConstantArray : NSArray {
    NSUInteger _count;
    const id *_objects;
}
@end

@implementation NSConstantArray

- (NSUInteger)count
{
    return _count;
}

- (id)objectAtIndex:(NSUInteger)index
{
    if (index >= _count) {
        [NSException raise:NSRangeException format:@"index %lu beyond bounds [0 .. %lu]", (unsigned long)index, (unsigned long)_count - 1];
    }
    return _objects[index];
}

- (void)getObjects:(id __unsafe_unretained [])objects range:(NSRange)range
{
    if (range.location > _count || range.length > _count - range.location) {
        [NSException raise:NSRangeException format:@"range {%lu, %lu} beyond bounds [0 .. %lu]", (unsigned long)range.location, (unsigned long)range.length, (unsigned long)_count];
    }
    for (NSUInteger i = 0; i < range.length; i++) {
        objects[i] = _objects[range.location + i];
    }
}

- (NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state objects:(id __unsafe_unretained [])buffer count:(NSUInteger)len
{
    if (state->state != 0) {
        return 0;
    }
    static const unsigned long immutable = 1;
    state->mutationsPtr = (unsigned long *)&immutable;
    state->itemsPtr = (id __unsafe_unretained *)_objects;
    state->state = 1;
    return _count;
}

- (id)retain { return self; }
- (oneway void)release { }
- (id)autorelease { return self; }
- (NSUInteger)retainCount { return NSUIntegerMax; }
- (id)copyWithZone:(NSZone *)zone { return self; }
- (id)mutableCopyWithZone:(NSZone *)zone { return [[NSMutableArray allocWithZone:zone] initWithObjects:_objects count:_count]; }

@end

CF_EXPORT
@interface NSConstantDictionary : NSDictionary {
    NSUInteger _reserved;
    NSUInteger _count;
    const id *_keys;
    const id *_objects;
}
@end

@implementation NSConstantDictionary

- (NSUInteger)count
{
    return _count;
}

- (id)objectForKey:(id)key
{
    if (key == nil) {
        return nil;
    }
    for (NSUInteger i = 0; i < _count; i++) {
        if (_keys[i] == key || [_keys[i] isEqual:key]) {
            return _objects[i];
        }
    }
    return nil;
}

- (NSEnumerator *)keyEnumerator
{
    return [[NSArray arrayWithObjects:_keys count:_count] objectEnumerator];
}

- (void)getObjects:(id __unsafe_unretained [])objects andKeys:(id __unsafe_unretained [])keys
{
    for (NSUInteger i = 0; i < _count; i++) {
        if (objects) objects[i] = _objects[i];
        if (keys) keys[i] = _keys[i];
    }
}

- (id)retain { return self; }
- (oneway void)release { }
- (id)autorelease { return self; }
- (NSUInteger)retainCount { return NSUIntegerMax; }
- (id)copyWithZone:(NSZone *)zone { return self; }
- (id)mutableCopyWithZone:(NSZone *)zone { return [[NSMutableDictionary allocWithZone:zone] initWithObjects:_objects forKeys:_keys count:_count]; }

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
