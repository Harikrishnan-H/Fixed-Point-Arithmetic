/** @file *************************************************************************************************************

Component   Fixed Point Arithmetic

Filename    FixedPoint.h

@brief      Interface for performance-efficient fixed-point arithmetic.

@author     Harikrishnan Haridas


@verbatim
***********************************************************************************************************************
* Changes                                                                                                             *
***********************************************************************************************************************

Version   Date        Sign  Description
--------  ----------  ----  -----------
01.00.00  2025-12-09  Hari   Initial check in
01.01.00  2025-12-29  Hari  Configuration header added

@endverbatim
**********************************************************************************************************************/
#ifndef FIXED_POINT_H
#define FIXED_POINT_H

/**********************************************************************************************************************
INCLUDES
**********************************************************************************************************************/

#include "Global_Types.h" /**< Fundamental data types header*/
#include "FixedPoint_cfg.h" /**< Configuration header*/

/** @defgroup g_FixedPoint Fixed Point Module
 *  @brief Fixed-point arithmetic module (8-bit and 16-bit).
 */


 /** @addtogroup g_FixedPoint
  *  @{
  */



/**********************************************************************************************************************
EXTERNAL FUNCTIONS
**********************************************************************************************************************/
extern Std_ReturnType FixedPoint_Add16(float val1, float val2, float* result);
extern Std_ReturnType FixedPoint_Sub16(float val1, float val2, float* result);
extern Std_ReturnType FixedPoint_Mult16(float val1, float val2, float* result);
extern Std_ReturnType FixedPoint_Div16(float val1, float val2, float* result);

extern Std_ReturnType FixedPoint_Add8(float val1, float val2, float* result);
extern Std_ReturnType FixedPoint_Sub8(float val1, float val2, float* result);
extern Std_ReturnType FixedPoint_Mult8(float val1, float val2, float* result);
extern Std_ReturnType FixedPoint_Div8(float val1, float val2, float* result);

/** @} end addtogroup */

#endif /* FIXED_POINT_H */
/**********************************************************************************************************************
EOF
**********************************************************************************************************************/