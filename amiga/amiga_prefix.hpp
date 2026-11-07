/* Included before every C++ file (-include): the C++ streams first, since
 * the toolchain's pthread.h wraps read(), write() and close() in
 * cancellation macros once <unistd.h> is in, which breaks <istream> and
 * <fstream> read after it (the Team's Nikwi port). The AmiCradle Team.
 * GPL 2 or later, like Tux Football. */
#include <iostream>
#include <fstream>
#include <sstream>
