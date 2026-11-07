/* configure's config.h for Tux Football on AmigaOS. HAVE_GETTEXT without
 * ENABLE_NLS: src/gettext.h's stand-ins, which give back the English text
 * (gameengine.cpp needs its _() even then). The AmiCradle Team. GPL 2 or
 * later, like Tux Football. */
#define PACKAGE "tuxfootball"
#define PACKAGE_NAME "tuxfootball"
#define PACKAGE_TARNAME "tuxfootball"
#define PACKAGE_VERSION "0.3.1"
#define PACKAGE_STRING "Tux Football 0.3.1"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_URL ""
#define VERSION "0.3.1"
#define STDC_HEADERS 1
#define HAVE_GETTEXT 1
#define LOCALEDIR "PROGDIR:locale"
