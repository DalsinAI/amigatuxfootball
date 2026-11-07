/* A Shell gives 4 KB of stack; libnix's startup sets this much up instead.
 * The AmiCradle Team. GPL 2 or later, like Tux Football. */
unsigned long __stack = 256 * 1024;
static const char version[] __attribute__((used)) = "$VER: Tux Football 0.3.1 Amiga (07.11.2026)";
