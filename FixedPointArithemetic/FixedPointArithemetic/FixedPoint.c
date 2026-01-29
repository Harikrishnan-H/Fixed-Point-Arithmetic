/** @file *************************************************************************************************************


Component   Fixed Point Arithmetic

Filename    FixedPoint.c

@brief      Fixed-point arithmetic module providing 8-bit and 16-bit operations.
 *
 * Detailed Description:
 * - Supports Add, Sub, Mult, and Div operations.
 * - Internal Logic: Performed in core static functions using integer types.
 * - 16-bit: The fractional bit positions (SHIFT_16 in FixedPoint_cfg.h) define the Q format used internally.
 * - 8-bit:  The fractional bit positions (SHIFT_8 in FixedPoint_cfg.h) define the Q format used internally.
 * - Public API: Accepts/returns float values
 * - Implements round-to-nearest (symmetric rounding), saturation at format boundaries and status reporting.

@author     Harikrishnan Haridas

@verbatim
***********************************************************************************************************************
* Changes                                                                                                             *
***********************************************************************************************************************

Version   Date        Sign  Description
--------  ----------  ----  -----------
01.00.00  2025-12-20  Hari   Initial check in
01.01.00  2025-12-21  Hari   Updated to fixed-point core and wrapper functions
01.02.00  2025-12-22  Hari   Updated rounding, saturation and error handling
01.03.00  2026-01-07  Hari   Updated and added detailed comments.

@endverbatim
**********************************************************************************************************************/

/**********************************************************************************************************************
INCLUDES
**********************************************************************************************************************/
#include <stddef.h>            /* for NULL */
#include "FixedPoint.h"
#include "FixedPoint_cfg.h"

/** @addtogroup g_FixedPoint
@{ */


 /**********************************************************************************************************************
 MACROS
 **********************************************************************************************************************/

 /**********************************************************************************************************************
 LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/

 /* Conversion helpers: external float to internal fixed-point representation
  * These helpers implement round-to-nearest and  saturation.
  */
static Std_ReturnType FixedPoint_FloatToFix16(float val, t_Fixed16* outVal);
static float          FixedPoint_Fix16ToFloat(t_Fixed16 val);

static Std_ReturnType FixedPoint_FloatToFix8(float val, t_Fixed8* outVal);
static float          FixedPoint_Fix8ToFloat(t_Fixed8 val);

/* Pure fixed-point core functions: integer-only arithmetic in Q-format.
 * All functions implement saturation on overflow and return E_NOT_OK if
 * saturation was applied.
 */
static Std_ReturnType FixedPoint_Add16_Core(t_Fixed16 a, t_Fixed16 b, t_Fixed16* r);
static Std_ReturnType FixedPoint_Sub16_Core(t_Fixed16 a, t_Fixed16 b, t_Fixed16* r);
static Std_ReturnType FixedPoint_Mult16_Core(t_Fixed16 a, t_Fixed16 b, t_Fixed16* r);
static Std_ReturnType FixedPoint_Div16_Core(t_Fixed16 a, t_Fixed16 b, t_Fixed16* r);

static Std_ReturnType FixedPoint_Add8_Core(t_Fixed8 a, t_Fixed8 b, t_Fixed8* r);
static Std_ReturnType FixedPoint_Sub8_Core(t_Fixed8 a, t_Fixed8 b, t_Fixed8* r);
static Std_ReturnType FixedPoint_Mult8_Core(t_Fixed8 a, t_Fixed8 b, t_Fixed8* r);
static Std_ReturnType FixedPoint_Div8_Core(t_Fixed8 a, t_Fixed8 b, t_Fixed8* r);

/**********************************************************************************************************************
LOCAL FUNCTIONS
**********************************************************************************************************************/

/*********************************************************************************************************************/
/*! @brief     Convert float value to 16-bit fixed-point with rounding and saturation.
 *
 *  Conversion uses round-to-nearest (ties away from zero). If the scaled value does not
 *  fit into the 16-bit fixed-point range [-32768, 32767], the result is saturated to the
 *  nearest boundary and E_NOT_OK is returned.
 *
 *  @param[in]  val       Input value in floating-point representation.
 *  @param[out] outVal    Pointer to store the fixed-point representation.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Conversion successful without saturation.
 *  @retval     E_NOT_OK    Saturation occurred or null pointer passed.
 */
static Std_ReturnType FixedPoint_FloatToFix16(float val, t_Fixed16* outVal)
{
    Std_ReturnType ret = E_NOT_OK;
    /* Initialize return status to NOT OK by default */

    if (outVal != NULL)
        /* Check that the output pointer is not NULL to prevent invalid memory access */
    {
        float scaled_f = val * (float)SCALE_16;
        /* Scale the floating-point input value by the fixed-point scaling factor */

        /* Round-to-nearest (ties away from zero) integer domain */

        sint32 scaled_i;
        /* Temporary 32-bit signed integer to store the rounded scaled value */

        if (scaled_f >= 0.0f)
            /* Check if the scaled floating-point value is non-negative */
        {
            scaled_i = (sint32)(scaled_f + 0.5f);
            /* Round positive values to the nearest integer (ties away from zero) */
        }
        else
            /* Handle negative scaled values */
        {
            scaled_i = (sint32)(scaled_f - 0.5f);
            /* Round negative values to the nearest integer (ties away from zero) */
        }

        /* Saturation check on the integer result */
        if (scaled_i > (sint32)FIX16_MAX)
            /* Check if the rounded value exceeds the maximum fixed-point limit */
        {
            scaled_i = (sint32)FIX16_MAX;
            /* Saturate the value to the maximum allowed fixed-point value */
            ret = E_NOT_OK;
            /* Indicate overflow condition */
        }
        else if (scaled_i < (sint32)FIX16_MIN)
            /* Check if the rounded value is below the minimum fixed-point limit */
        {
            scaled_i = (sint32)FIX16_MIN;
            /* Saturate the value to the minimum allowed fixed-point value */
            ret = E_NOT_OK;
            /* Indicate underflow condition */
        }
        else
            /* Value is within the valid fixed-point range */
        {
            ret = E_OK;
            /* Indicate successful conversion */
        }

        *outVal = (t_Fixed16)scaled_i;
        /* Store the final fixed-point value into the output variable */
    }

    return ret;
    /* Return the status of the conversion */
}

/*********************************************************************************************************************/
/*! @brief     Static function that converts a 16-bit fixed-point value to floating-point
 *
 *  @param[in]  val     Input value in fixed-point representation.
 *
 *  @return     float
 *  @retval     Floating-point representation of the input value.
 */
static float FixedPoint_Fix16ToFloat(t_Fixed16 val)
{
    float result = 0.0f;
    /* Initialize the floating-point result variable to zero */

    result = (float)val / (float)SCALE_16;
    /* Convert fixed-point value to float by dividing by the scaling factor */

    return result;
    /* Return the converted floating-point value */
}

/*********************************************************************************************************************/
/*! @brief     Convert float value to 8-bit fixed-point with rounding and saturation.
 *
 *  Conversion uses round-to-nearest (ties away from zero). If the scaled value does not
 *  fit into the 8-bit fixed-point range [-128, 127], the result is saturated to the
 *  nearest boundary and E_NOT_OK is returned.
 *
 *  @param[in]  val       Input value in floating-point representation.
 *  @param[out] outVal    Pointer to store the fixed-point representation.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Conversion successful without saturation.
 *  @retval     E_NOT_OK    Saturation occurred or null pointer passed.
 */
static Std_ReturnType FixedPoint_FloatToFix8(float val, t_Fixed8* outVal)
{
    Std_ReturnType ret = E_NOT_OK;
    /* Initialize return status to NOT OK by default */

    if (outVal != NULL)
        /* Check that the output pointer is valid (not NULL) */
    {
        float scaled_f = val * (float)SCALE_8;
        /* Scale the floating-point input using the fixed-point scaling factor */

        /* Round-to-nearest (ties away from zero) integer domain */
        sint16 scaled_i;
        /* Temporary 16-bit signed integer to hold the rounded scaled value */

        if (scaled_f >= 0.0f)
            /* Check if the scaled value is non-negative */
        {
            scaled_i = (sint16)(scaled_f + 0.5f);
            /* Round positive values to the nearest integer (ties away from zero) */
        }
        else
            /* Handle negative scaled values */
        {
            scaled_i = (sint16)(scaled_f - 0.5f);
            /* Round negative values to the nearest integer (ties away from zero) */
        }

        /* Saturation check on the integer result */

        if (scaled_i > (sint16)FIX8_MAX)
            /* Check if the result exceeds the maximum fixed-point range */
        {
            scaled_i = (sint16)FIX8_MAX;
            /* Saturate the value to the maximum allowed fixed-point value */
            ret = E_NOT_OK;
            /* Indicate overflow condition */
        }
        else if (scaled_i < (sint16)FIX8_MIN)
            /* Check if the result is below the minimum fixed-point range */
        {
            scaled_i = (sint16)FIX8_MIN;
            /* Saturate the value to the minimum allowed fixed-point value */
            ret = E_NOT_OK;
            /* Indicate underflow condition */
        }
        else
            /* Value is within the valid fixed-point range */
        {
            ret = E_OK;
            /* Indicate successful conversion */
        }

        *outVal = (t_Fixed8)scaled_i;
        /* Store the final fixed-point value in the output variable */
    }

    return ret;
    /* Return the conversion status */
}


/*********************************************************************************************************************/
/*! @brief     Convert 8-bit fixed-point to float.
 *
 *  @param[in]  val     Input value in fixed-point representation.
 *
 *  @return     float
 *  @retval     Floating-point representation of the input value.
 */
static float FixedPoint_Fix8ToFloat(t_Fixed8 val)
/* Static function that converts an 8-bit fixed-point value to floating-point */
{
    float result = 0.0f;
    /* Initialize the floating-point result variable to zero */

    result = (float)val / (float)SCALE_8;
    /* Convert the fixed-point value to float by dividing by the scaling factor */

    return result;
    /* Return the converted floating-point value */
}

/*********************************************************************************************************************/
/*! @brief     16-bit fixed-point addition core with saturation.
 *
 *  Performs integer addition of two values. If the result exceeds the representable
 *  range, it is saturated to FIX16_MAX or FIX16_MIN and E_NOT_OK is returned.
 *
 *  @param[in]  a       First operand in configured 16-bit Q-format.
 *  @param[in]  b       Second operand in configured 16-bit Q-format.
 *  @param[out] r       Pointer to store the fixed-point result in configured 16-bit Q-format.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Addition successful without saturation.
 *  @retval     E_NOT_OK    Saturation occurred or null pointer passed.
 */
static Std_ReturnType FixedPoint_Add16_Core(t_Fixed16 a, t_Fixed16 b, t_Fixed16* r)
{
    Std_ReturnType ret = E_NOT_OK;
    /* Initialize return status to NOT OK by default */

    if (r != NULL)
        /* Check that the result pointer is valid (not NULL) */
    {
        sint32 tmp = (sint32)a + (sint32)b;
        /* Perform addition in 32-bit signed integer to prevent overflow */

        if (tmp > (sint32)FIX16_MAX)
            /* Check if the addition result exceeds the maximum fixed-point value */
        {
            tmp = (sint32)FIX16_MAX;
            /* Saturate the result to the maximum allowed fixed-point value */
            ret = E_NOT_OK;
            /* Indicate overflow condition */
        }
        else if (tmp < (sint32)FIX16_MIN)
            /* Check if the addition result is below the minimum fixed-point value */
        {
            tmp = (sint32)FIX16_MIN;
            /* Saturate the result to the minimum allowed fixed-point value */
            ret = E_NOT_OK;
            /* Indicate underflow condition */
        }
        else
            /* Result is within the valid fixed-point range */
        {
            ret = E_OK;
            /* Indicate successful addition */
        }

        *r = (t_Fixed16)tmp;
        /* Store addition result into the output variable */
    }

    return ret;
    /* Return the status of the addition operation */
}


/*********************************************************************************************************************/
/*! @brief     16-bit fixed-point subtraction core with saturation.
 *
 *  Performs integer subtraction of two values. If the result exceeds the representable
 *  range, it is saturated to FIX16_MAX or FIX16_MIN and E_NOT_OK is returned.
 * 
 *  @param[in]  a       Minuend in configured 16-bit Q-format.
 *  @param[in]  b       Subtrahend in configured 16-bit Q-format.
 *  @param[out] r       Pointer to store the result in configured 16-bit Q-format.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Subtraction successful without saturation.
 *  @retval     E_NOT_OK    Saturation occurred or null pointer passed.
 */
static Std_ReturnType FixedPoint_Sub16_Core(t_Fixed16 a, t_Fixed16 b, t_Fixed16* r)
{
    Std_ReturnType ret = E_NOT_OK;
    /* Initialize return status to NOT OK by default */

    if (r != NULL)
        /* Check that the result pointer is valid (not NULL) */
    {
        sint32 tmp = (sint32)a - (sint32)b;
        /* Perform subtraction using 32-bit signed integer to avoid overflow */

        if (tmp > (sint32)FIX16_MAX)
            /* Check if the subtraction result exceeds the maximum fixed-point limit */
        {
            tmp = (sint32)FIX16_MAX;
            /* Saturate the result to the maximum allowed fixed-point value */
            ret = E_NOT_OK;
            /* Indicate overflow condition */
        }
        else if (tmp < (sint32)FIX16_MIN)
            /* Check if the subtraction result is below the minimum fixed-point limit */
        {
            tmp = (sint32)FIX16_MIN;
            /* Saturate the result to the minimum allowed fixed-point value */
            ret = E_NOT_OK;
            /* Indicate underflow condition */
        }
        else
            /* Result is within the valid fixed-point range */
        {
            ret = E_OK;
            /* Indicate successful subtraction */
        }

        *r = (t_Fixed16)tmp;
        /* Store the subtraction result in the output variable */
    }

    return ret;
    /* Return the status of the subtraction operation */
}


/*********************************************************************************************************************/
/*! @brief     16-bit fixed-point multiplication core with rounding and saturation.
 *
 *  The operands are in the configured 16-bit Q-format with SHIFT_16 fractional bits.
 *  Multiplication produces a widened intermediate product containing 2*SHIFT_16 fractional bits.
 *  Rounding to nearest is performed in the intermediate domain by adding half of the LSB that
 *  will be discarded during rescaling. To ensure symmetric rounding for negative values, rounding
 *  is applied to the magnitude and the original sign is restored afterwards. The rounded result is 
 *  then rescaled back by shifting right SHIFT_16 bits. If the final value
 *  exceeds the representable range, the result is saturated and E_NOT_OK is returned.
 *
 *  @param[in]  a       Multiplicand in configured 16-bit Q-format.
 *  @param[in]  b       Multiplier in configured 16-bit Q-format.
 *  @param[out] r       Pointer to store the result in configured 16-bit Q-format.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Multiplication successful without saturation.
 *  @retval     E_NOT_OK    Saturation occurred or null pointer passed.
 */

static Std_ReturnType FixedPoint_Mult16_Core(t_Fixed16 a, t_Fixed16 b, t_Fixed16* r)
{
    /* Initialize return status to NOT OK by default */
    Std_ReturnType ret = E_NOT_OK;

    if (r != NULL)
    {
        /* widening to 64 bit to avoid overflow during multiplication & subsequent rounding */
        sint64 tmp = (sint64)a * (sint64)b;


#if (SHIFT_16 > 0U) /* Compile time check to ensure that configuration allows for fractional bits (since SHIFT_16 is configurable) else rounding is not required */

        /* computing half of LSB to add for rounding */
        const sint64 half = ((sint64)1 << (SHIFT_16 - 1U));

        /* extracting the sign */
        const boolean neg = (tmp < 0) ? 1U : 0U;

        /* gets the magnitude of the number to ensure symmetric rounding for negative numbers */
        /* -tmp is safe here because tmp is a product of 16 bit operands widened to 64 bit */
        sint64 mag = neg ? -tmp : tmp;

        /* adds half to magnitude for rounding and shifts back by SHIFT_16 */
        mag = (mag + half) >> SHIFT_16;

        /* restore the sign */
        tmp = neg ? -mag : mag;

#else
        /* No fractional bits so no rounding or scaling required */
#endif

/* Saturation */

/* Checks if the result is within the min and max range */
        if (tmp > (sint64)FIX16_MAX)
        {
            /* saturates the result if out of range */
            tmp = (sint64)FIX16_MAX;
            /* ret is assigned failure code*/
            ret = E_NOT_OK;
        }
        else if (tmp < (sint64)FIX16_MIN)
        {
            /* saturates the result if out of range */
            tmp = (sint64)FIX16_MIN;
            /* ret is assigned failure code*/
            ret = E_NOT_OK;
        }
        else
        {
            /* Indicate successful multiplication */
            ret = E_OK;
        }

        /* Assigns the final result */
        *r = (t_Fixed16)tmp;
    }

    /* Return the status of the multiplication operation */
    return ret;
}


/*********************************************************************************************************************/
/*! @brief     16-bit fixed-point division core with rounding and saturation.
 *
 *  The operands are in the configured 16-bit Q-format with SHIFT_16 fractional bits.
 *  Division is performed fully using integer arithmetic by scaling the dividend prior to division:
 *  (|a| << SHIFT_16) / |b|. Rounding to nearest is implemented in the magnitude domain by adding
 *  half the divisor before integer division, then  the original sign is restored afterwards, ensures
 *  symmetric rounding for negative results (ties away from zero).
 *
 *  A 64-bit magnitude is used for the scaled numerator and divisor to prevent overflow and to
 *  preserve precision. The caller must ensure that the divisor is not zero (validated by the
 *  public API). If the final result exceeds the configured representable range (FIX16_MIN..FIX16_MAX),
 *  it is saturated and E_NOT_OK is returned and the saturated result is still written.
 *
 *  @param[in]  a       Dividend in configured 16-bit Q-format.
 *  @param[in]  b       Divisor in configured 16-bit Q-format (must not be zero).
 *  @param[out] r       Pointer to store the result in configured 16-bit Q-format.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Division successful without saturation.
 *  @retval     E_NOT_OK    Saturation occurred or null pointer passed.
 */

static Std_ReturnType FixedPoint_Div16_Core(t_Fixed16 a, t_Fixed16 b, t_Fixed16* r)
{
    /* Initialize return status to NOT OK by default */
    Std_ReturnType ret = E_NOT_OK;

    if (r != NULL)
    {
        /* b is guaranteed non-zero by the public API */

        /* Determine if result should be negative by checking if a and b have opposite signs */
        const boolean neg = (((sint32)a ^ (sint32)b) < 0) ? 1U : 0U;

        /* extract magnitudes */
        uint64 num_mag = (uint64)((a < 0) ? -(sint64)a : (sint64)a);
        uint64 den_mag = (uint64)((b < 0) ? -(sint64)b : (sint64)b);

        /* the numerator must be left-shifted before division to preserve fractional precision */
        num_mag <<= SHIFT_16;

        /* Add half the divisor to enable round-to-nearest during integer division */
        num_mag += (den_mag >> 1);

        /* divide in magnitude domain */
        sint64 tmp = (sint64)(num_mag / den_mag);

        /* restore sign */
        tmp = neg ? -tmp : tmp;

        /* Checks if the result is within the min and max range */
        if (tmp > (sint64)FIX16_MAX)
        {
            /* saturates the result if out of range */
            tmp = (sint64)FIX16_MAX;
            /* ret is assigned failure code*/
            ret = E_NOT_OK;
        }
        else if (tmp < (sint64)FIX16_MIN)
        {
            /* saturates the result if out of range */
            tmp = (sint64)FIX16_MIN;
            /* ret is assigned failure code*/
            ret = E_NOT_OK;
        }
        else
        {
            /* Indicate successful division */
            ret = E_OK;
        }

        /* Assigns the final result */
        *r = (t_Fixed16)tmp;
    }

    /* Return the status of the division operation */
    return ret;
}


/*********************************************************************************************************************/
/*! @brief     8-bit fixed-point addition core with saturation.
 *
 *  Performs integer addition of two values. If the result exceeds the representable
 *  range, it is saturated to FIX8_MAX or FIX8_MIN and E_NOT_OK is returned.
 *
 *  @param[in]  a       First operand in configured 8-bit Q-format.
 *  @param[in]  b       Second operand in configured 8-bit Q-format.
 *  @param[out] r       Pointer to store the result in configured 8-bit Q-format.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Addition successful without saturation.
 *  @retval     E_NOT_OK    Saturation occurred or null pointer passed.
 */
static Std_ReturnType FixedPoint_Add8_Core(t_Fixed8 a, t_Fixed8 b, t_Fixed8* r)
{
    Std_ReturnType ret = E_NOT_OK;

    if (r != NULL)
    {
        sint16 tmp = (sint16)a + (sint16)b;

        if (tmp > (sint16)FIX8_MAX)
        {
            tmp = (sint16)FIX8_MAX;
            ret = E_NOT_OK;
        }
        else if (tmp < (sint16)FIX8_MIN)
        {
            tmp = (sint16)FIX8_MIN;
            ret = E_NOT_OK;
        }
        else
        {
            ret = E_OK;
        }

        *r = (t_Fixed8)tmp;
    }

    return ret;
}

/*********************************************************************************************************************/
/*! @brief     8-bit fixed-point subtraction core with saturation.
 * 
 * Performs integer subtraction of two values. If the result exceeds the representable
 * range, it is saturated to FIX8_MAX or FIX8_MIN and E_NOT_OK is returned.
 *
 *  @param[in]  a       Minuend in configured 8-bit Q-format.
 *  @param[in]  b       Subtrahend in configured 8-bit Q-format.
 *  @param[out] r       Pointer to store the result in configured 8-bit Q-format.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Subtraction successful without saturation.
 *  @retval     E_NOT_OK    Saturation occurred or null pointer passed.
 */

static Std_ReturnType FixedPoint_Sub8_Core(t_Fixed8 a, t_Fixed8 b, t_Fixed8* r)
{
    Std_ReturnType ret = E_NOT_OK;

    if (r != NULL)
    {
        sint16 tmp = (sint16)a - (sint16)b;

        if (tmp > (sint16)FIX8_MAX)
        {
            tmp = (sint16)FIX8_MAX;
            ret = E_NOT_OK;
        }
        else if (tmp < (sint16)FIX8_MIN)
        {
            tmp = (sint16)FIX8_MIN;
            ret = E_NOT_OK;
        }
        else
        {
            ret = E_OK;
        }

        *r = (t_Fixed8)tmp;
    }

    return ret;
}

/*********************************************************************************************************************/
/*! @brief     8-bit fixed-point multiplication core with rounding and saturation.
 *
 *  The widened product contains 2*SHIFT_8 fractional bits. The result is rounded to nearest
 *  by adding half of the discarded LSB in magnitude domain (symmetric for negative values),
 *  then rescaled back by SHIFT_8 to the configured Q format. If the final result exceeds the
 *  representable range, it is saturated and E_NOT_OK is returned.
 *
 *  @param[in]  a       Multiplicand in configured 8-bit Q format.
 *  @param[in]  b       Multiplier in configured 8-bit Q format.
 *  @param[out] r       Pointer to store the result in configured 8-bit Q-format.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Multiplication successful without saturation.
 *  @retval     E_NOT_OK    Saturation occurred or null pointer passed.
 */
static Std_ReturnType FixedPoint_Mult8_Core(t_Fixed8 a, t_Fixed8 b, t_Fixed8* r)
{
    Std_ReturnType ret = E_NOT_OK;

    if (r != NULL)
    {
        /* widening to 32 bit to avoid overflow during multiplication & subsequent rounding */
        sint32 tmp = (sint32)a * (sint32)b; 

#if (SHIFT_8 > 0U) /* Compile time check to ensure that configuration allows for fractional bits (since SHIFT_8 is configurable) else rounding is not required */

        /* computing half of LSB to add for rounding */
        const sint32 half = ((sint32)1 << (SHIFT_8 - 1U));

        /* extracting the sign */
        const boolean neg = (tmp < 0) ? 1U : 0U;

        /* gets the magnitude of the number to ensure symmetric rounding for negative numbers */
        /* -tmp is safe here because tmp is a product of 8 bit operands widened to 32 bit */
        sint32 mag = neg ? -tmp : tmp; 

        /* adds half to magnitude for rounding and shifts back by SHIFT_8 */
        mag = (mag + half) >> SHIFT_8;

        /* restore the sign */
        tmp = neg ? -mag : mag;

#else
/* No fractional bits so no rounding or scaling required */
#endif


        /* Saturation */

        /* Checks if the result is within the min and max range */
        if (tmp > (sint32)FIX8_MAX)
        {
            /* saturates the result if out of range */
            tmp = (sint32)FIX8_MAX;
            /* returns failure code*/
            ret = E_NOT_OK;
        }
        else if (tmp < (sint32)FIX8_MIN)
        {
            tmp = (sint32)FIX8_MIN;
            ret = E_NOT_OK;
        }
        else
        {
            ret = E_OK;
        }

        /* Assigns the final result */
        *r = (t_Fixed8)tmp;
    }

    return ret;
}

/*********************************************************************************************************************/
/*! @brief     8-bit fixed-point division core with rounding and saturation.
 *
 *  The operands are in the configured 8-bit Q-format with SHIFT_8 fractional bits.
 *  Division is performed fully using integer arithmetic by scaling the dividend prior to division:
 *  (|a| << SHIFT_8) / |b|. Rounding to nearest is implemented in the magnitude domain by adding
 *  half the divisor before integer division and then the original sign is restored afterwards, ensures
 *  symmetric rounding for negative results (ties away from zero).
 *
 *  A widened magnitude is used for the scaled numerator and divisor to prevent overflow and to
 *  preserve precision. The caller must ensure that the divisor is not zero (validated by the
 *  public API). If the final result exceeds the configured representable range (FIX8_MIN..FIX8_MAX),
 *  it is saturated and E_NOT_OK is returned and the saturated result is still written.
 *
 *  @param[in]  a       Dividend in configured 8-bit Q-format.
 *  @param[in]  b       Divisor in configured 8-bit Q-format (must not be zero).
 *  @param[out] r       Pointer to store the result in configured 8-bit Q-format.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Division successful without saturation.
 *  @retval     E_NOT_OK    Saturation occurred or null pointer passed.
 */

static Std_ReturnType FixedPoint_Div8_Core(t_Fixed8 a, t_Fixed8 b, t_Fixed8* r)
{
    Std_ReturnType ret = E_NOT_OK;

    if (r != NULL)
    {
        /* b is guaranteed non-zero by the public API */

        const boolean neg = (((sint16)a ^ (sint16)b) < 0) ? 1U : 0U;

        uint32 num_mag = (uint32)((a < 0) ? -(sint32)a : (sint32)a);
        uint32 den_mag = (uint32)((b < 0) ? -(sint32)b : (sint32)b);

        num_mag <<= SHIFT_8;
        num_mag += (den_mag >> 1);

        sint32 tmp = (sint32)(num_mag / den_mag);
        tmp = neg ? -tmp : tmp;

        if (tmp > (sint32)FIX8_MAX)
        {
            tmp = (sint32)FIX8_MAX;
            ret = E_NOT_OK;
        }
        else if (tmp < (sint32)FIX8_MIN)
        {
            tmp = (sint32)FIX8_MIN;
            ret = E_NOT_OK;
        }
        else
        {
            ret = E_OK;
        }

        *r = (t_Fixed8)tmp;
    }

    return ret;
}

/**********************************************************************************************************************
GLOBAL FUNCTIONS
**********************************************************************************************************************/

/*********************************************************************************************************************/
/*! @brief     16-bit fixed-point addition with float interface.
 *
 *  Public API : accepts two floating-point values and
 *  returns the result as float. Internally, the values are converted to configured Q-format
 *  with rounding and saturation, added using integer arithmetic, and converted back to
 *  float. If any conversion or arithmetic operation saturates, the function returns
 *  E_NOT_OK but still provides a saturated result.
 *
 *  @param[in]  val1    First input operand in floating-point representation.
 *  @param[in]  val2    Second input operand in floating-point representation.
 *  @param[out] result  Pointer to location where the floating-point result is stored.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Calculation successful without saturation.
 *  @retval     E_NOT_OK    Null pointer, conversion saturation or arithmetic saturation.
 */
Std_ReturnType FixedPoint_Add16(float val1, float val2, float* result)
{
    /* Initialize return status */
    Std_ReturnType ret = E_NOT_OK;

    /* Validate output pointer before proceeding */
    if (result != NULL)
    {
        /* Fixed-point representation of first operand */
        t_Fixed16 a;

        /* Fixed-point representation of second operand */
        t_Fixed16 b;

        /* Fixed-point variable to store the addition result */
        t_Fixed16 rFixed = 0;

        /* Convert first floating-point input to 16-bit fixed-point with rounding/saturation */
        Std_ReturnType conv1 = FixedPoint_FloatToFix16(val1, &a);

        /* Convert second floating-point input to 16-bit fixed-point with rounding/saturation */
        Std_ReturnType conv2 = FixedPoint_FloatToFix16(val2, &b);

        /* Perform fixed-point addition using the core integer arithmetic function */
        Std_ReturnType core = FixedPoint_Add16_Core(a, b, &rFixed);

        /* Check whether both conversions and the core addition completed without saturation */
        if ((conv1 == E_OK) && (conv2 == E_OK) && (core == E_OK))
        {
            /* All operations succeeded without saturation */
            ret = E_OK;
        }
        else
        {
            /* At least one conversion or arithmetic operation saturated */
            ret = E_NOT_OK;
        }

        /* Convert the fixed-point result back to floating-point and store it */
        *result = FixedPoint_Fix16ToFloat(rFixed);
    }

    /* Return overall status of the operation */
    return ret;
}

/*********************************************************************************************************************/
/*! @brief     16-bit fixed-point subtraction with float interface.
 *
 *  Public API : accepts two floating-point values and
 *  returns the result as float. Internally, the values are converted to configured Q-format
 *  with rounding and saturation, subtracted using integer arithmetic, and converted back
 *  to float. If any conversion or arithmetic operation saturates, the function returns
 *  E_NOT_OK but still provides a saturated result.
 *
 *  @param[in]  val1    Minuend in floating-point representation.
 *  @param[in]  val2    Subtrahend in floating-point representation.
 *  @param[out] result  Pointer to location where the floating-point result is stored.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Calculation successful without saturation.
 *  @retval     E_NOT_OK    Null pointer, conversion saturation or arithmetic saturation.
 */

Std_ReturnType FixedPoint_Sub16(float val1, float val2, float* result)
{
    /* Initialize return status */
    Std_ReturnType ret = E_NOT_OK;

    /* Validate output pointer before proceeding */
    if (result != NULL)
    {
        /* Fixed-point representation of first operand */
        t_Fixed16 a;

        /* Fixed-point representation of second operand */
        t_Fixed16 b;

        /* Fixed-point variable to store the subtraction result */
        t_Fixed16 rFixed = 0;

        /* Convert first floating-point input to 16-bit fixed-point with rounding/saturation */
        Std_ReturnType conv1 = FixedPoint_FloatToFix16(val1, &a);

        /* Convert second floating-point input to 16-bit fixed-point with rounding/saturation */
        Std_ReturnType conv2 = FixedPoint_FloatToFix16(val2, &b);

        /* Perform fixed-point subtraction using the core integer arithmetic function */
        Std_ReturnType core = FixedPoint_Sub16_Core(a, b, &rFixed);

        /* Check whether both conversions and the core subtraction completed without saturation */
        if ((conv1 == E_OK) && (conv2 == E_OK) && (core == E_OK))
        {
            /* All operations succeeded without saturation */
            ret = E_OK;
        }
        else
        {
            /* At least one conversion or arithmetic operation saturated */
            ret = E_NOT_OK;
        }

        /* Convert the fixed-point result back to floating-point and store it */
        *result = FixedPoint_Fix16ToFloat(rFixed);
    }

    /* Return overall status of the operation */
    return ret;
}


/*********************************************************************************************************************/
/*! @brief     16-bit fixed-point multiplication with float interface.
 *
 *  Public API : accepts two floating-point values and
 *  returns the result as float. Internally, the values are converted to configured Q-format
 *  with rounding and saturation, multiplied using integer arithmetic, and scaled
 *  appropriately before being converted back to float. If any conversion, scaling,
 *  or arithmetic operation saturates, the function returns E_NOT_OK but still provides
 *  a saturated result.
 *
 *  @param[in]  val1    Multiplicand in floating-point representation.
 *  @param[in]  val2    Multiplier in floating-point representation.
 *  @param[out] result  Pointer to location where the floating-point result is stored.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Calculation successful without saturation.
 *  @retval     E_NOT_OK    Null pointer, conversion saturation or arithmetic saturation.
 */

Std_ReturnType FixedPoint_Mult16(float val1, float val2, float* result)
{
    /* Initialize return status */
    Std_ReturnType ret = E_NOT_OK;

    /* Validate output pointer before proceeding */
    if (result != NULL)
    {
        /* Fixed-point representation of first operand */
        t_Fixed16 a;

        /* Fixed-point representation of second operand */
        t_Fixed16 b;

        /* Fixed-point variable to store the multiplication result */
        t_Fixed16 rFixed = 0;

        /* Convert first floating-point input to 16-bit fixed-point with rounding/saturation */
        Std_ReturnType conv1 = FixedPoint_FloatToFix16(val1, &a);

        /* Convert second floating-point input to 16-bit fixed-point with rounding/saturation */
        Std_ReturnType conv2 = FixedPoint_FloatToFix16(val2, &b);

        /* Perform fixed-point multiplication using the core integer arithmetic function */
        Std_ReturnType core = FixedPoint_Mult16_Core(a, b, &rFixed);

        /* Check whether both conversions and the core multiplication completed without saturation */
        if ((conv1 == E_OK) && (conv2 == E_OK) && (core == E_OK))
        {
            /* All operations succeeded without saturation */
            ret = E_OK;
        }
        else
        {
            /* At least one conversion or arithmetic operation saturated */
            ret = E_NOT_OK;
        }

        /* Convert the fixed-point result back to floating-point and store it */
        *result = FixedPoint_Fix16ToFloat(rFixed);
    }

    /* Return overall status of the operation */
    return ret;
}


/*********************************************************************************************************************/
/*! @brief     16-bit fixed-point division with float interface.
 *
 *  Performs division only if the divisor is non-zero. In case of division by zero
 *  or null result pointer, the function returns E_NOT_OK and does not modify *result.
 *  For valid divisors, values are internally converted to configured Q-format, divided using integer arithmetic
 *  with rounding and saturation, and converted back to float.
 *
 *  @param[in]  val1    Dividend in floating-point representation.
 *  @param[in]  val2    Divisor in floating-point representation.
 *  @param[out] result  Pointer to location where the floating-point result is stored.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Calculation successful without saturation.
 *  @retval     E_NOT_OK    Null pointer, division by zero, conversion saturation or arithmetic saturation.
 */
Std_ReturnType FixedPoint_Div16(float val1, float val2, float* result)
{
    /* Initialize return status  */
    Std_ReturnType ret = E_NOT_OK;

    /* Validate output pointer before proceeding */
    if (result != NULL)
    {
        /* Fixed-point representation of the divisor */
        t_Fixed16 b;

        /* Convert second floating-point input to fixed-point to check for division by zero */
        Std_ReturnType conv2 = FixedPoint_FloatToFix16(val2, &b);

        /* Proceed only if the converted divisor is non-zero */
        if (b != 0)
        {
            /* Fixed-point representation of the dividend */
            t_Fixed16 a;

            /* Fixed-point variable to store the division result */
            t_Fixed16 rFixed = 0;

            /* Convert first floating-point input to 16-bit fixed-point with rounding/saturation */
            Std_ReturnType conv1 = FixedPoint_FloatToFix16(val1, &a);

            /* Perform fixed-point division using the core integer arithmetic function */
            Std_ReturnType core = FixedPoint_Div16_Core(a, b, &rFixed);

            /* Check whether both conversions and the core division completed without saturation */
            if ((conv1 == E_OK) && (conv2 == E_OK) && (core == E_OK))
            {
                /* All operations succeeded without saturation */
                ret = E_OK;
            }
            else
            {
                /* At least one conversion or arithmetic operation saturated */
                ret = E_NOT_OK;
            }

            /* Convert the fixed-point result back to floating-point and store it */
            *result = FixedPoint_Fix16ToFloat(rFixed);
        }
        else
        {
            /* Division by zero detected after conversion: report error and leave *result unchanged */
            ret = E_NOT_OK;
        }
    }

    /* Return overall status of the operation */
    return ret;
}

/*********************************************************************************************************************/
/*! @brief     8-bit fixed-point addition with float interface.
 *
 *  Public API : accepts two floating-point values and
 *  returns the result as float. Internally, the values are converted to the configured
 *  Q-format with rounding and saturation, added using integer arithmetic, and converted
 *  back to float. If any conversion or arithmetic operation saturates, the function
 *  returns E_NOT_OK but still provides a saturated result.
 *
 *  @param[in]  val1    First input operand in floating-point representation.
 *  @param[in]  val2    Second input operand in floating-point representation.
 *  @param[out] result  Pointer to location where the floating-point result is stored.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Calculation successful without saturation.
 *  @retval     E_NOT_OK    Null pointer, conversion saturation or arithmetic saturation.
 */

Std_ReturnType FixedPoint_Add8(float val1, float val2, float* result)
{
    Std_ReturnType ret = E_NOT_OK;

    if (result != NULL)
    {

        t_Fixed8 a;
        t_Fixed8 b;
        t_Fixed8 rFixed = 0;
        Std_ReturnType conv1 = FixedPoint_FloatToFix8(val1, &a);
        Std_ReturnType conv2 = FixedPoint_FloatToFix8(val2, &b);
        Std_ReturnType core = FixedPoint_Add8_Core(a, b, &rFixed);

        if ((conv1 == E_OK) && (conv2 == E_OK) && (core == E_OK))
        {
            ret = E_OK;
        }
        else
        {
            ret = E_NOT_OK;
        }

        *result = FixedPoint_Fix8ToFloat(rFixed);
    }

    return ret;
}

/*********************************************************************************************************************/
/*! @brief     8-bit fixed-point subtraction with float interface.
 *
 *  Public API : accepts two floating-point values and
 *  returns the result as float. Internally, the values are converted to the configured
 *  Q-format with rounding and saturation, subtracted using integer arithmetic, and
 *  converted back to float. If any conversion or arithmetic operation saturates, the
 *  function returns E_NOT_OK but still provides a saturated result.
 *
 *  @param[in]  val1    Minuend in floating-point representation.
 *  @param[in]  val2    Subtrahend in floating-point representation.
 *  @param[out] result  Pointer to location where the floating-point result is stored.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Calculation successful without saturation.
 *  @retval     E_NOT_OK    Null pointer, conversion saturation or arithmetic saturation.
 */

Std_ReturnType FixedPoint_Sub8(float val1, float val2, float* result)
{
    Std_ReturnType ret = E_NOT_OK;

    if (result != NULL)
    {
        t_Fixed8 a;
        t_Fixed8 b;
        t_Fixed8 rFixed = 0;
        Std_ReturnType conv1 = FixedPoint_FloatToFix8(val1, &a);
        Std_ReturnType conv2 = FixedPoint_FloatToFix8(val2, &b);
        Std_ReturnType core = FixedPoint_Sub8_Core(a, b, &rFixed);

        if ((conv1 == E_OK) && (conv2 == E_OK) && (core == E_OK))
        {
            ret = E_OK;
        }
        else
        {
            ret = E_NOT_OK;
        }

        *result = FixedPoint_Fix8ToFloat(rFixed);
    }

    return ret;
}

/*********************************************************************************************************************/
/*! @brief     8-bit fixed-point multiplication with float interface.
 *
 *  Public API : accepts two floating-point values and
 *  returns the result as float. Internally, the values are converted to the configured
 *  Q-format with rounding and saturation, multiplied using integer arithmetic, and
 *  scaled appropriately before being converted back to float. If any conversion,
 *  scaling, or arithmetic operation saturates, the function returns E_NOT_OK but still
 *  provides a saturated result.
 *
 *  @param[in]  val1    Multiplicand in floating-point representation.
 *  @param[in]  val2    Multiplier in floating-point representation.
 *  @param[out] result  Pointer to location where the floating-point result is stored.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Calculation successful without saturation.
 *  @retval     E_NOT_OK    Null pointer, conversion saturation or arithmetic saturation.
 */

Std_ReturnType FixedPoint_Mult8(float val1, float val2, float* result)
{
    Std_ReturnType ret = E_NOT_OK;


    if (result != NULL)
    {
        t_Fixed8 a;
        t_Fixed8 b;
        t_Fixed8 rFixed = 0;
        Std_ReturnType conv1 = FixedPoint_FloatToFix8(val1, &a);
        Std_ReturnType conv2 = FixedPoint_FloatToFix8(val2, &b);
        Std_ReturnType core = FixedPoint_Mult8_Core(a, b, &rFixed);

        if ((conv1 == E_OK) && (conv2 == E_OK) && (core == E_OK))
        {
            ret = E_OK;
        }
        else
        {
            ret = E_NOT_OK;
        }

        *result = FixedPoint_Fix8ToFloat(rFixed);
    }

    return ret;
}

/*********************************************************************************************************************/
/*! @brief     8-bit fixed-point division with float interface.
 *
 *  Performs division only if the divisor is non-zero. In case of division by zero
 *  or null result pointer, the function returns E_NOT_OK and does not modify *result.
 *  For valid divisors, values are converted to configured
 *  Q-format, divided using integer arithmetic
 *  with rounding and saturation, and converted back to float.
 *
 *  @param[in]  val1    Dividend in floating-point representation.
 *  @param[in]  val2    Divisor in floating-point representation.
 *  @param[out] result  Pointer to location where the floating-point result is stored.
 *
 *  @return     Std_ReturnType
 *  @retval     E_OK        Calculation successful without saturation.
 *  @retval     E_NOT_OK    Null pointer, division by zero, conversion saturation or arithmetic saturation.
 */
Std_ReturnType FixedPoint_Div8(float val1, float val2, float* result)
{
    Std_ReturnType ret = E_NOT_OK;

    if (result != NULL)
    {
        /* Converting val2 to fixed point and to check if its 0 */
        t_Fixed8 b; 
        Std_ReturnType conv2 = FixedPoint_FloatToFix8(val2, &b);
        if (b != 0)
        {
            t_Fixed8 a;
            t_Fixed8 rFixed = 0;
            Std_ReturnType conv1 = FixedPoint_FloatToFix8(val1, &a);
            Std_ReturnType core = FixedPoint_Div8_Core(a, b, &rFixed);

            if ((conv1 == E_OK) && (conv2 == E_OK) && (core == E_OK))
            {
                ret = E_OK;
            }
            else
            {
                ret = E_NOT_OK;
            }

            *result = FixedPoint_Fix8ToFloat(rFixed);
        }
        else
        {
            /* Division by zero: report error, here *result is not modified to avoid the undefined or misleading numerical result*/
            ret = E_NOT_OK;
        }
    }


    return ret;
}

/** @} end addtogroup */
/**********************************************************************************************************************
EOF
**********************************************************************************************************************/