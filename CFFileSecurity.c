/*	CFFileSecurity.c

	In-memory container for the ownership, permission and ACL properties
	of a file, as described by CFFileSecurity.h.
*/

#include <CoreFoundation/CFFileSecurity.h>
#include "CFInternal.h"
#include <sys/acl.h>

struct __CFFileSecurity {
    CFRuntimeBase _base;
    uid_t _owner;
    gid_t _group;
    mode_t _mode;
    CFUUIDRef _ownerUUID;
    CFUUIDRef _groupUUID;
    acl_t _acl;
    Boolean _hasOwner;
    Boolean _hasGroup;
    Boolean _hasMode;
};

static CFTypeID __kCFFileSecurityTypeID = _kCFRuntimeNotATypeID;

static void __CFFileSecurityReleaseProperties(CFFileSecurityRef fileSec, CFOptionFlags mask) {
    struct __CFFileSecurity *fs = (struct __CFFileSecurity *)fileSec;
    if (mask & kCFFileSecurityClearOwner) { fs->_hasOwner = false; fs->_owner = 0; }
    if (mask & kCFFileSecurityClearGroup) { fs->_hasGroup = false; fs->_group = 0; }
    if (mask & kCFFileSecurityClearMode) { fs->_hasMode = false; fs->_mode = 0; }
    if ((mask & kCFFileSecurityClearOwnerUUID) && fs->_ownerUUID) { CFRelease(fs->_ownerUUID); fs->_ownerUUID = NULL; }
    if ((mask & kCFFileSecurityClearGroupUUID) && fs->_groupUUID) { CFRelease(fs->_groupUUID); fs->_groupUUID = NULL; }
    if (mask & kCFFileSecurityClearAccessControlList) {
        if (fs->_acl && fs->_acl != kCFFileSecurityRemoveACL(acl_t)) acl_free(fs->_acl);
        fs->_acl = NULL;
    }
}

static void __CFFileSecurityDeallocate(CFTypeRef cf) {
    __CFFileSecurityReleaseProperties((CFFileSecurityRef)cf,
        kCFFileSecurityClearOwnerUUID | kCFFileSecurityClearGroupUUID | kCFFileSecurityClearAccessControlList);
}

static const CFRuntimeClass __CFFileSecurityClass = {
    0,
    "CFFileSecurity",
    NULL,	// init
    NULL,	// copy
    __CFFileSecurityDeallocate,
    NULL,	// equal
    NULL,	// hash
    NULL,	// formatting description
    NULL	// debug description
};

CFTypeID CFFileSecurityGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFFileSecurityTypeID = _CFRuntimeRegisterClass(&__CFFileSecurityClass); });
    return __kCFFileSecurityTypeID;
}

CFFileSecurityRef CFFileSecurityCreate(CFAllocatorRef allocator) {
    CFIndex size = sizeof(struct __CFFileSecurity) - sizeof(CFRuntimeBase);
    struct __CFFileSecurity *fs = (struct __CFFileSecurity *)_CFRuntimeCreateInstance(allocator, CFFileSecurityGetTypeID(), size, NULL);
    return (CFFileSecurityRef)fs;
}

CFFileSecurityRef CFFileSecurityCreateCopy(CFAllocatorRef allocator, CFFileSecurityRef fileSec) {
    const struct __CFFileSecurity *src = (const struct __CFFileSecurity *)fileSec;
    struct __CFFileSecurity *copy = (struct __CFFileSecurity *)CFFileSecurityCreate(allocator);
    if (!copy) return NULL;
    copy->_owner = src->_owner; copy->_hasOwner = src->_hasOwner;
    copy->_group = src->_group; copy->_hasGroup = src->_hasGroup;
    copy->_mode = src->_mode; copy->_hasMode = src->_hasMode;
    if (src->_ownerUUID) copy->_ownerUUID = CFRetain(src->_ownerUUID);
    if (src->_groupUUID) copy->_groupUUID = CFRetain(src->_groupUUID);
    if (src->_acl == kCFFileSecurityRemoveACL(acl_t)) copy->_acl = src->_acl;
    else if (src->_acl) copy->_acl = acl_dup(src->_acl);
    return (CFFileSecurityRef)copy;
}

Boolean CFFileSecurityCopyOwnerUUID(CFFileSecurityRef fileSec, CFUUIDRef *ownerUUID) {
    const struct __CFFileSecurity *fs = (const struct __CFFileSecurity *)fileSec;
    if (!fs->_ownerUUID) return false;
    if (ownerUUID) *ownerUUID = CFRetain(fs->_ownerUUID);
    return true;
}

Boolean CFFileSecuritySetOwnerUUID(CFFileSecurityRef fileSec, CFUUIDRef ownerUUID) {
    struct __CFFileSecurity *fs = (struct __CFFileSecurity *)fileSec;
    CFUUIDRef old = fs->_ownerUUID;
    fs->_ownerUUID = ownerUUID ? CFRetain(ownerUUID) : NULL;
    if (old) CFRelease(old);
    return true;
}

Boolean CFFileSecurityCopyGroupUUID(CFFileSecurityRef fileSec, CFUUIDRef *groupUUID) {
    const struct __CFFileSecurity *fs = (const struct __CFFileSecurity *)fileSec;
    if (!fs->_groupUUID) return false;
    if (groupUUID) *groupUUID = CFRetain(fs->_groupUUID);
    return true;
}

Boolean CFFileSecuritySetGroupUUID(CFFileSecurityRef fileSec, CFUUIDRef groupUUID) {
    struct __CFFileSecurity *fs = (struct __CFFileSecurity *)fileSec;
    CFUUIDRef old = fs->_groupUUID;
    fs->_groupUUID = groupUUID ? CFRetain(groupUUID) : NULL;
    if (old) CFRelease(old);
    return true;
}

Boolean CFFileSecurityCopyAccessControlList(CFFileSecurityRef fileSec, acl_t *accessControlList) {
    const struct __CFFileSecurity *fs = (const struct __CFFileSecurity *)fileSec;
    if (!fs->_acl || fs->_acl == kCFFileSecurityRemoveACL(acl_t)) return false;
    if (accessControlList) *accessControlList = acl_dup(fs->_acl);
    return true;
}

Boolean CFFileSecuritySetAccessControlList(CFFileSecurityRef fileSec, acl_t accessControlList) {
    struct __CFFileSecurity *fs = (struct __CFFileSecurity *)fileSec;
    acl_t newACL = accessControlList;
    if (accessControlList && accessControlList != kCFFileSecurityRemoveACL(acl_t)) {
        newACL = acl_dup(accessControlList);
        if (!newACL) return false;
    }
    __CFFileSecurityReleaseProperties(fileSec, kCFFileSecurityClearAccessControlList);
    fs->_acl = newACL;
    return true;
}

Boolean CFFileSecurityGetOwner(CFFileSecurityRef fileSec, uid_t *owner) {
    const struct __CFFileSecurity *fs = (const struct __CFFileSecurity *)fileSec;
    if (!fs->_hasOwner) return false;
    if (owner) *owner = fs->_owner;
    return true;
}

Boolean CFFileSecuritySetOwner(CFFileSecurityRef fileSec, uid_t owner) {
    struct __CFFileSecurity *fs = (struct __CFFileSecurity *)fileSec;
    fs->_owner = owner;
    fs->_hasOwner = true;
    return true;
}

Boolean CFFileSecurityGetGroup(CFFileSecurityRef fileSec, gid_t *group) {
    const struct __CFFileSecurity *fs = (const struct __CFFileSecurity *)fileSec;
    if (!fs->_hasGroup) return false;
    if (group) *group = fs->_group;
    return true;
}

Boolean CFFileSecuritySetGroup(CFFileSecurityRef fileSec, gid_t group) {
    struct __CFFileSecurity *fs = (struct __CFFileSecurity *)fileSec;
    fs->_group = group;
    fs->_hasGroup = true;
    return true;
}

Boolean CFFileSecurityGetMode(CFFileSecurityRef fileSec, mode_t *mode) {
    const struct __CFFileSecurity *fs = (const struct __CFFileSecurity *)fileSec;
    if (!fs->_hasMode) return false;
    if (mode) *mode = fs->_mode;
    return true;
}

Boolean CFFileSecuritySetMode(CFFileSecurityRef fileSec, mode_t mode) {
    struct __CFFileSecurity *fs = (struct __CFFileSecurity *)fileSec;
    fs->_mode = mode;
    fs->_hasMode = true;
    return true;
}

Boolean CFFileSecurityClearProperties(CFFileSecurityRef fileSec, CFOptionFlags clearPropertyMask) {
    __CFFileSecurityReleaseProperties(fileSec, clearPropertyMask);
    return true;
}
