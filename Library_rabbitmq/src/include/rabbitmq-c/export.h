
#ifndef RABBITMQ_C_EXPORT_H
#define RABBITMQ_C_EXPORT_H

#ifdef AMQP_STATIC
#  define AMQP_EXPORT
#  define AMQP_NO_EXPORT
#else
#  ifndef AMQP_EXPORT
#    ifdef rabbitmq_EXPORTS
        /* We are building this library */
#      define AMQP_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define AMQP_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef AMQP_NO_EXPORT
#    define AMQP_NO_EXPORT 
#  endif
#endif

#ifndef AMQP_DEPRECATED
#  define AMQP_DEPRECATED __declspec(deprecated)
#endif

#ifndef AMQP_DEPRECATED_EXPORT
#  define AMQP_DEPRECATED_EXPORT AMQP_EXPORT AMQP_DEPRECATED
#endif

#ifndef AMQP_DEPRECATED_NO_EXPORT
#  define AMQP_DEPRECATED_NO_EXPORT AMQP_NO_EXPORT AMQP_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef AMQP_NO_DEPRECATED
#    define AMQP_NO_DEPRECATED
#  endif
#endif

#endif /* RABBITMQ_C_EXPORT_H */
