#ifndef	CEXT_VERSION_INCLUDED
#define CEXT_VERSION_INCLUDED

#define MAKE_STR_HELPER(a_str) #a_str
#define MAKE_STR(a_str) MAKE_STR_HELPER(a_str)

#define CEXT_VERSION_MAJOR	1
#define CEXT_VERSION_MINOR	1
#define CEXT_VERSION_PATCH	0
#define CEXT_VERSION_BETA	0
#define CEXT_VERSION_VERSTRING	MAKE_STR(CEXT_VERSION_MAJOR) "." MAKE_STR(CEXT_VERSION_MINOR) "." MAKE_STR(CEXT_VERSION_PATCH) "." MAKE_STR(CEXT_VERSION_BETA)

#endif
