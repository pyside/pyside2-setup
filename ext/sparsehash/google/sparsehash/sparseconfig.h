/*
 * NOTE: This file is for internal use only.
 *       Do not use these #defines in your own program!
 */

/* Namespace for Google classes */
#define GOOGLE_NAMESPACE ::google

#ifdef _MSC_VER
    /* the location of the header defining hash functions */
    #define HASH_FUN_H <hash_map>
    /* the namespace of the hash<> function */
    #define HASH_NAMESPACE stdext
    /* The system-provided hash function including the namespace. */
    #define SPARSEHASH_HASH  HASH_NAMESPACE::hash_compare
#else
    /* the location of the header defining hash functions */
    #define HASH_FUN_H <tr1/functional>
    /* the namespace of the hash<> function */
    #define HASH_NAMESPACE std::tr1
    /* The system-provided hash function including the namespace. */
    #define SPARSEHASH_HASH HASH_NAMESPACE::hash
#endif

/* Define to 1 if the system has the type `long long'. */
#define HAVE_LONG_LONG 1

/* the namespace where STL code like vector<> is defined */
#define STL_NAMESPACE std

/* Stops putting the code inside the Google namespace */
#define _END_GOOGLE_NAMESPACE_ }

/* Puts following code inside the Google namespace */
#define _START_GOOGLE_NAMESPACE_ namespace google {
