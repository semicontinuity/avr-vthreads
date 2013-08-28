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
 *  - Virtual thread functions do not have take 'thread' argument - ususally they can serve only one thread.
 *  - Virtual thread functions must have 'void' return type - thus it is possible to use inside interrupt handlers.
 *  - Virtual thread runs forever over and over - if the thread has finished or waiting, and it is not necessary to schedule
 *    the thread function, it should communicate this to scheduler via some shared variable.
 *  - Virtual thread functions do not reference thread's data via pointer - thus it is possible to place it to the registers.
 *  - It is possible to move the instruction pointer of the virtual thread by seeking to the specified mark in the thread function.
 *
 * Supported compiler: avr-gcc, or any other with support for address labels.
 *
 * Please refer to Protothreads documentation for usage details and concepts behind implementation.
 * \author semicontinuity
 */


/**
 * A virtual thread instruction pointer type.
 * Stores the continuation of the thread (the address, from which the virtual thread function will proceed).
 * For good performance, place it to the general purpose I/O register.
 * For better performance, place it in the register.
 * For best performance, place it in the high register (r16-r24).
 * For utmost performance, place it to r30.
 * Beware of compiler bug when placed to the register (except r30).
 */
typedef void * vthread_ip_t;


#define FC_ASM_LABEL_NAME(thread, mark) #thread "__" mark
#define FC_ASM_LABEL_BEGIN(thread)      FC_ASM_LABEL_NAME(thread, "BEGIN")
#define FC_ASM_LABEL_STOP(thread)	FC_ASM_LABEL_NAME(thread, "STOP")
#define FC_ASM_LABEL(label)             do { __asm__ __volatile__( label ":"); } while(0)

#define FC_ASM_LABEL_ADDRESS(label)	\
(__extension__({                        \
  uint16_t __result;                    \
  __asm__ __volatile__ (		\
    "ldi %A0, pm_lo8(" label ")\n\t"	\
    "ldi %B0, pm_hi8(" label ")\n\t"	\
        : "=d"(__result)		\
  );					\
  __result;                             \
}))

#define FC_POINTER(label) ((void*)FC_ASM_LABEL_ADDRESS(label))



#define FC_RESUME(s)			\
  do {					\
      goto *s;				\
  } while(0)

#define FC_CONCAT2(s1, s2) s1##s2
#define FC_CONCAT(s1, s2) FC_CONCAT2(s1, s2)



/**
 * Initialize the virtual thread.
 *
 * This macro MUST be called before the first call to the virtual thread function.
 * \param thread  A name of the virtual thread
 * \param ip      An instruction pointer of the virtual thread
 */
#define VT_INIT(thread, ip) do { ip = FC_POINTER(FC_ASM_LABEL_BEGIN(thread)); } while(0)


/**
 * Restart the virtual thread.
 * \param thread  A name of the virtual thread
 * \param ip      An instruction pointer of the virtual thread
 */
#define VT_RESTART(thread, ip) VT_INIT(thread, ip)


/**
 * Declare the start of a virtual thread inside the function implementing the virtual thread.
 * \param thread A virtual thread variable
 * \param ip      An instruction pointer of the virtual thread
 */
#define VT_BEGIN(thread, ip) do {               \
  char vt_flag = 1;                             \
  FC_RESUME(ip);                                \
  for (;;) {                                    \
      FC_ASM_LABEL(FC_ASM_LABEL_BEGIN(thread));


/**
 * Declare the end of a virtual thread.
 * \param thread A virtual thread name
 */
#define VT_END(thread)                          \
  }                                             \
  FC_ASM_LABEL(FC_ASM_LABEL_STOP(thread));      \
  (void)vt_flag;                                \
} while(0)


/**
 * Yield control from the current virtual thread.
 * Once the virtual thread function is called again, it will resume from the following operator.
 * \param thread  A virtual thread name
 * \param ip      An instruction pointer of the virtual thread
 */
#define VT_YIELD(thread, ip) \
  FC_CONCAT(FC_YIELD, __LINE__):   	        \
  (void)&&FC_CONCAT(FC_YIELD, __LINE__);        \
do {                                            \
  vt_flag = 0;				        \
  FC_CONCAT(FC_LABEL, __LINE__):                \
  if(vt_flag == 0) {                            \
    (ip) = &&FC_CONCAT(FC_LABEL, __LINE__);	\
    return;                                     \
  }                                             \
} while(0)


/**
 * Mark the current position in the virtual thread.
 * Later, seek operation can be used to restore this position.
 * \param thread  A virtual thread name
 * \param mark    The name of the mark
 */
#define VT_MARK(thread, mark) FC_ASM_LABEL(FC_ASM_LABEL_NAME(thread, mark))

/**
 * Set the current position in the given virtual thread to the specified mark.
 * Later, seek operation can be used to restore this position.
 * \param thread  A virtual thread name
 * \param ip      An instruction pointer of the virtual thread
 * \param mark    The name of the mark
 */
#define VT_SEEK(thread, ip, mark) do {          \
  ip = FC_POINTER(FC_ASM_LABEL_NAME(thread, mark)); \
} while(0)

#endif
