#ifndef APIEXTRACTORMACROS_H
#define APIEXTRACTORMACROS_H


// APIEXTRACTOR_API is used for the public API symbols.
#if defined _WIN32 || defined __CYGWIN__
    #if APIEXTRACTOR_EXPORTS
        #define APIEXTRACTOR_API __declspec(dllexport)
    #else
        #define APIEXTRACTOR_API
    #endif
#else
    #if __GNUC__ >= 4
        #define APIEXTRACTOR_API __attribute__ ((visibility("default")))
    #else
        #define APIEXTRACTOR_API
    #endif
#endif

#endif
