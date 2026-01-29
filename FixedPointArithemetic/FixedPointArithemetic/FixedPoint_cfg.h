/** @file *************************************************************************************************************
 *
 * Component   Fixed Point Arithmetic
 *
 * Filename    FixedPoint_cfg.h
 *
 * @brief      Configuration header for FixedPoint module (Q-format selection, scaling and limits).
 *
 *             This file contains compile time configuration parameters for the FixedPoint module.
 *             The fractional bit positions (SHIFT_16 / SHIFT_8) define the Q format used internally.
 *             Scaling factors (SCALE_16 / SCALE_8) are computed from the shifts defined.
 *
 *             NOTE:
 *             - Default configuration corresponds to Q7.8 for 16-bit and Q3.4 for 8-bit.
 *             - Changing SHIFT values changes resolution and numeric range.
 *             - The real value corresponding to a fixed-point value is: real_value = fixed_value / (2^fractional_bits)
 *             - Saturation limits FIX16_* and FIX8_* represent the container boundaries.
 * 
 *			   - Default Q format definition and numeric ranges
 *	               - 16 bit Format: Q7.8
 *	               - Bit layout: 1 sign bit, 7 integer bits, 8 fractional bits
 *                 - Integer range: -32768 to +32767 (fixed-point)
 *                 - Approx. real range: -128.0 to +127.996
 *                 - Resolution: 1 / 2^8 = 0.0039
 *
 *                 - 8-bit Format: Q3.4
 *                 - Bit layout: 1 sign bit, 3 integer bits, 4 fractional bits
 *                 - Integer range: -128 to +127 (fixed-point)
 *                 - Approx. real range: -8.0 to +7.9375
 *                 - Resolution: 1 / 2^4 = 0.0625
 *
 * @author     Harikrishnan Haridas
 *
 * @verbatim
 ***********************************************************************************************************************
 * Changes                                                                                                             *
 ***********************************************************************************************************************
 *
 * Version   Date        Sign  Description
 * --------  ----------  ----  -----------
 * 01.00.00  2025-12-29  Hari  Initial check in
 * 01.01.00  2026-01-07  Hari   Updated and added comments.
 *
 * @endverbatim
 **********************************************************************************************************************/

#ifndef FIXED_POINT_CFG_H
#define FIXED_POINT_CFG_H

 /**********************************************************************************************************************
  INCLUDES
 **********************************************************************************************************************/
#include "Global_Types.h"

 /** @addtogroup g_FixedPoint
  *  @{
  */

  /**********************************************************************************************************************
   MACROS
  **********************************************************************************************************************/

  /* --- 16-bit Q-Format Configuration --- */
  /** @brief Number of fractional bits for 16-bit fixed-point arithmetic.
   *
   * Default value 8 corresponds to Q7.8 format: 1 sign bit, 7 integer bits, 8 fractional bits
   */
#define SHIFT_16    (8U)

   /** @brief Scaling factor for 16-bit fixed-point arithmetic (2^SHIFT_16). */
#define SCALE_16    (1U << SHIFT_16)


/* --- 8-bit Q-Format Configuration --- */
/** @brief Number of fractional bits for 8-bit fixed-point arithmetic.
 *
 * Default value 4 corresponds to Q3.4 format: 1 sign bit, 3 integer bits, 4 fractional bits
 */
#define SHIFT_8     (4U)

 /** @brief Scaling factor for 8-bit fixed-point arithmetic (2^SHIFT_8). */
#define SCALE_8     (1U << SHIFT_8)


/* --- Saturation Limits (container boundaries) --- */
/** @brief Maximum representable raw fixed-point value for 16-bit container (t_Fixed16). */
#define FIX16_MAX   ((t_Fixed16) 32767)

/** @brief Minimum representable raw fixed-point value for 16-bit container (t_Fixed16). */
#define FIX16_MIN   ((t_Fixed16)-32768)

/** @brief Maximum representable raw fixed-point value for 8-bit container (t_Fixed8). */
#define FIX8_MAX    ((t_Fixed8)  127)

/** @brief Minimum representable raw fixed-point value for 8-bit container (t_Fixed8). */
#define FIX8_MIN    ((t_Fixed8) -128)


/**********************************************************************************************************************
 CONFIG VALIDATION (COMPILE-TIME CHECKS)
**********************************************************************************************************************/
/* SHIFT must be within the bit-width of the container (signed). */
#if (SHIFT_16 > 15U)
#error "SHIFT_16 must be <= 15 for signed 16-bit fixed-point."
#endif

#if (SHIFT_8 > 7U)
#error "SHIFT_8 must be <= 7 for signed 8-bit fixed-point."
#endif

/** @} end addtogroup */

#endif /* FIXED_POINT_CFG_H */

/**********************************************************************************************************************
EOF
**********************************************************************************************************************/
