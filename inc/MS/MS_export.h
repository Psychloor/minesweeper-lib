
#ifndef MS_EXPORT_H
#define MS_EXPORT_H

#ifdef MS_STATIC_DEFINE
#  define MS_EXPORT
#  define MS_NO_EXPORT
#else
#  ifndef MS_EXPORT
#    ifdef minesweeper_lib_EXPORTS
        /* We are building this library */
#      define MS_EXPORT 
#    else
        /* We are using this library */
#      define MS_EXPORT 
#    endif
#  endif

#  ifndef MS_NO_EXPORT
#    define MS_NO_EXPORT 
#  endif
#endif

#ifndef MS_DEPRECATED
#  define MS_DEPRECATED __declspec(deprecated)
#endif

#ifndef MS_DEPRECATED_EXPORT
#  define MS_DEPRECATED_EXPORT MS_EXPORT MS_DEPRECATED
#endif

#ifndef MS_DEPRECATED_NO_EXPORT
#  define MS_DEPRECATED_NO_EXPORT MS_NO_EXPORT MS_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MS_NO_DEPRECATED
#    define MS_NO_DEPRECATED
#  endif
#endif

#endif /* MS_EXPORT_H */
