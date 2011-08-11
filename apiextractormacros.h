#ifndef APIEXTRACTORMACROS_H
#define APIEXTRACTORMACROS_H


// APIEXTRACTOR_API is used for the public API symbols.
#if defined _WIN32 || defined __CYGWIN__
    #if APIEXTRACTOR_EXPORTS
        #define APIEXTRACTOR_API __declspec(dllexport)
    #else
        #define APIEXTRACTOR_API
    #endif
    #define APIEXTRACTOR_DEPRECATED(func) __declspec(deprecated) func
#elif __GNUC__ >= 4
    #define APIEXTRACTOR_API __attribute__ ((visibility("default")))
    #define APIEXTRACTOR_DEPRECATED(func) func __attribute__ ((deprecated))
#endif

#ifndef APIEXTRACTOR_API
    #define APIEXTRACTOR_API
    #define APIEXTRACTOR_API(func) func
#endif

#endif
