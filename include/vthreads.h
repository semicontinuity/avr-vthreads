#ifndef __VTHREADS_H__
#define __VTHREADS_H__

/**
 * \file
 * Virtual threads implementation for AVR microcontrollers.
 * Inspired by Protothreads library: http://dunkels.com/adam/pt/
 *
 * With some optimizations, provides faster thread switching.
 *
 * Differences to Protothreads:
 *  - Virtual thread functions do not take 'thread' argument - they can serve only one thread.
 *  - Virtual thread functions must have 'void' return type - thus it is possible to use inside interrupt handlers.
 *
 * Supported compiler: avr-gcc, or any other with support for address labels.
 *
 * Please refer to Protothreads documentation for usage details and concepts behind implementation.
 * \author
 * Igor Karpov
 */


/**
 * A virtual thread handle type.
 * Stores the continuation of the thread (the address, from which the virtual thread function will proceed).
 * For better performance, place it in the register.
 * For best performance, place it in the high register (r16-r24)
 */
typedef void * vthread_t;


#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

#define FC_CONCAT2(s1, s2) s1##s2
#define FC_CONCAT(s1, s2) LC_CONCAT2(s1, s2)

#define FC_LABEL(thread, mark)	#thread "__" mark
#define FC_ENTER_LABEL(thread)	FC_LABEL(thread, "ENTER")
#define FC_BEGIN_LABEL(thread)	FC_LABEL(thread, "BEGIN")
#define FC_STOP_LABEL(thread)	FC_LABEL(thread, "STOP")
#define FC_EXIT_LABEL(thread)	FC_LABEL(thread, "EXIT")
#define FC_YIELD_LABEL(thread)	FC_LABEL(thread, QUOTE(FC_CONCAT(YIELD_, __LINE__)))
#define FC_RESUME_LABEL(thread)	FC_LABEL(thread, QUOTE(FC_CONCAT(RESUME_, __LINE__)))


#define FC_SET(thread, label) do {	\
  __asm__ __volatile__ (		\
    "ldi %A0, lo8(" label ")\n\t"	\
    "ldi %B0, hi8(" label ")\n\t"	\
        : "=r"(thread)			\
  );					\
} while(0);


#define FC_MARK(label)		do { __asm__ __volatile__( label ":"); } while(0)

#define FC_RESUME(thread)	do { __asm__ __volatile__("ijmp" :: "z"(thread)); } while(0)

#define FC_RETURN(thread)	do { __asm__ __volatile__("rjmp " FC_EXIT_LABEL(thread)); } while(0)


/**
 * Initialize the virtual thread.
 *
 * This macro MUST be called before the first call to the virtual thread function.
 * \param thread A virtual thread variable
 */
#define VT_INIT(thread) do { FC_SET(thread, FC_BEGIN_LABEL(thread)); } while(0)


/**
 * Restart the virtual thread.
 * \param thread A virtual thread variable
 */
#define VT_RESTART(thread) VT_INIT(thread)


/**
 * Declare the start of a virtual thread inside the function implementing the virtual thread.
 * \param thread A virtual thread variable
 */
#define VT_BEGIN(thread) do {		\
  FC_MARK(FC_ENTER_LABEL(thread));	\
  FC_RESUME(thread);			\
  FC_MARK(FC_BEGIN_LABEL(thread));	\
} while(0)


/**
 * Yield control from the current virtual thread.
 * Once the virtual thread function is called again, it will resume from the following operator.
 * \param thread A virtual thread variable
 */
#define VT_YIELD(thread) do {			\
  FC_MARK(FC_YIELD_LABEL(thread));		\
  FC_SET(thread, FC_RESUME_LABEL(thread));	\
  FC_RETURN(thread);				\
  FC_MARK(FC_RESUME_LABEL(thread));		\
} while(0);


/**
 * Stop the virtual thread.
 *
 * The virtual thread function will return.
 * All following calls to the virtual thread function will return immediately.
 *
 * \param thread A virtual thread variable
 */
#define VT_STOP(thread) do {			\
  FC_MARK(FC_STOP_LABEL(thread));		\
  FC_SET(thread, FC_EXIT_LABEL(thread));	\
  FC_RETURN(thread);				\
} while(0);


/**
 * Declare the end of a virtual thread.
 * \param thread A virtual thread variable
 */
#define VT_END(thread) FC_MARK(FC_EXIT_LABEL(thread))


#endif