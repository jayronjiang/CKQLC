

/*--------------------------------------------------------------------------
__UT_DEBUG__
--------------------------------------------------------------------------*/

#ifndef __UT_DEBUG_H__
	#define __UT_DEBUG_H__

//////////////////////////////////////////

// 要通过串口输出来调试时，开放此宏定义
#define __UT_DEBUG__

//////////////////////////////////////////
extern void debug_init();
extern void debug_puts(const char *str);
extern void debug_printf_digit (const char *fmtstr, unsigned long num );

/////////////////////////////////////////////

/************** _INIT_ **************/
#ifdef __UT_DEBUG__
	#define DEBUG_INIT_() debug_init()
#else
	#define DEBUG_INIT_() 
#endif

/**************_PUTS_ **************/
#ifdef __UT_DEBUG__
	#define DEBUG_PUTS_(expr)  debug_puts(expr)
#else
	#define	DEBUG_PUTS_(expr)
#endif


/**************_PRINT_ **************/
#ifdef __UT_DEBUG__
	#define DEBUG_PRINTF_NUM_(fmt, x1)  debug_printf_digit(fmt, x1)
#else
	#define	DEBUG_PRINTF_NUM_(fmt, x1)
#endif


////////////////////////////////////////////
/************** _ASSERT_ **************/
#ifdef __UT_DEBUG__
	#define DEBUG_ASSERT_(expr) \
  			if (expr) { ; } \
  			else  	{ DEBUG_PUTS_("Assert failed: " #expr " \n"); while(1); }
#else
	#define	DEBUG_ASSERT_(expr)
#endif


#endif
