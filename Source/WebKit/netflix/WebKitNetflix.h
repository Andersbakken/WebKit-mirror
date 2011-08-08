#ifndef __WEBKITNETFLIX_H__
#define __WEBKITNETFLIX_H__

#ifndef WTF_PLATFORM_NETFLIX
# define WTF_PLATFORM_NETFLIX 1
#endif
#ifndef WTF_USE_CAIRO
# define WTF_USE_CAIRO 1
#endif

#define WEBKIT_API __attribute__((visibility("default")))
#define WEBKIT_OBSOLETE_API WEBKIT_API __attribute__((deprecated))

#endif /* __WEBKITNETFLIX_H__ */
