// DEFINE COMPILER

#ifdef __GNUC__
# define COMPILER_NAME "g++"
#elif defined(__clang__)
# define COMPILER_NAME "clang++"
#elif defined(_MSC_VER)
# define COMPILER_NAME "Visual Studio"
#elif defined(__MINGW32__) && defined(__MINGW64__)
# define COMPILER "MinGW-w64 32bit"
#elif defined(__MINGW32__)
# define COMPILER "MinGW 32bit"
#elif defined(__MINGW64__)
# define COMPILER "MinGW-w64 62bit"
#else
# define COMPILER "unknown"
#endif


#ifdef __VERSION__
# define COMPILER COMPILER_NAME "-" __VERSION__
#else
# define COMPILER COMPILER__NAME
#endif
