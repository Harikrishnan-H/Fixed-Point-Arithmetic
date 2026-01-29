/** @file *************************************************************************************************************

Component   Fixed Point Arithmetic

Filename    Global_Types.h

@brief      Fundamental data types used.

@author     Harikrishnan Haridas

@verbatim
***********************************************************************************************************************
* Changes                                                                                                             *
***********************************************************************************************************************

Version   Date        Sign  Description
--------  ----------  ----  -----------
01.00.00  2025-12-10  Hari   Initial check in

@endverbatim
**********************************************************************************************************************/
#ifndef GLOBAL_TYPES_H
#define GLOBAL_TYPES_H

/** @defgroup g_GlobalTypes Global Types
 *  @brief Fundamental global data types and return codes.
 */


 /** @addtogroup g_GlobalTypes
  *  @{
  */


/**********************************************************************************************************************
TYPEDEFS
**********************************************************************************************************************/
typedef unsigned char Std_ReturnType; /* standard return type */
typedef unsigned char        boolean; /* boolean type */

typedef signed char        sint8;   /**< 8 bit signed integer -128 .. 127 */
typedef signed short       sint16;  /**< 16 bit signed integer -32768 .. 32767 */
typedef signed long        sint32;  /**< 32 bit signed integer */
typedef unsigned long        uint32;  /**< 32 bit unsigned integer */
typedef signed long long   sint64;  /**< 64 bit signed integer */
typedef unsigned long long   uint64;  /**< 64 bit unsigned integer */


typedef sint16 t_Fixed16; /**< for fixed point 16 bit */
typedef sint8  t_Fixed8; /**< for fixed point 8 bit  */

/**********************************************************************************************************************
(SYMBOLIC) CONSTANTS
**********************************************************************************************************************/

/** @brief Success return code */
#define E_OK        ((Std_ReturnType)0x00u)

/** @brief Failure return code */
#define E_NOT_OK    ((Std_ReturnType)0x01u)

/** @} end addtogroup */

#endif /* GLOBAL_TYPES_H */
/**********************************************************************************************************************
EOF
**********************************************************************************************************************/