/* APPLE LOCAL file radar 4698856 */
/* Test that _objc_empty_cache and _objc_empty_vtable may be user defined. */
/* { dg-options "-mmacosx-version-min=10.5 -fobjc-abi-version=2" } */

#include <objc/object.h>

static void * _objc_empty_cache;
static IMP _objc_empty_vtable;

@interface Foo
@end

@implementation Foo @end
