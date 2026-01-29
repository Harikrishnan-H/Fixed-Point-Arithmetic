/** @mainpage Fixed-Point Arithmetic Project
 *
 * @section intro_sec Purpose of the project
 *
 * This project implements performance efficient fixed point arithmetic (Add, Sub, Mult, Div)
 * in both 8-bit and 16-bit with configurable Q-format. The main module provides a small test
 * harness that executes a set of predefined tests for both of the
 * 16-bit and 8-bit formats and reports PASS/FAIL for each of the tests.
 * This includes cases covering different behaviours like normal operation, 
 * boundary handling and saturation, rounding and precision effects and failure conditions.
 *  
 */

/** @file *************************************************************************************************************
  *
  *
  * Component   Fixed Point Arithmetic
  *
  * Filename    Main.c
  *
  * @brief      Automated test suite for FixedPoint arithmetic (8-bit and 16-bit).
  *
  *            This module defines a set of test vectors to verify:
  *            - Typical arithmetic behaviour
  *            - Boundary conditions and saturation
  *            - Rounding and precision loss at very small magnitudes
  *            - Division by zero handling
  * 
  *            Each test case defines: 
  *            - Inputs A and B
  *            - Operation type
  *            - Expected floating point result
  *            - Allowed tolerance epsilon
  *            - Expected return code
  *            - Description
  *            
  *            The tests execute on the application start and print a
  *            PASS/FAIL report based on results and tolerance to the console.
  * 
  * @note     TEST HARNESS CONSTRAINTS & BEHAVIOR:
  * 
  *           1. Configuration Dependency:
  *           The saturation, boundary, rounding and resolution test cases are configuration aware and adjusts 
  *           according to the SHIFT_x parameters.The test vectors and expected values for  typical range tests in 
  *           this test harness are calibrated for the default Q7.8 (16-bit) and Q3.4 (8-bit) configurations.
  *           Since the fixed point implementation is flexible and the the SHIFT_x parameters in FixedPoint_cfg.h 
  *           can be modified as per user requirement to set a different Q m.n configuration other than default, 
  *           this modification may affect the numerical range and resolution, which can lead to failures in the 
  *           typical range tests in this harness and this does not indicate an implementation error. 
  *
  *           2. Error Case Validation (E_NOT_OK):
  *           For operations resulting in saturation or division by zero, this test harness
  *           results are printed for informational purposes and are not automatically
  *           compared against expected values to avoid false negatives.
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
  * 01.00.00  2025-12-14  Hari   Initial check in
  * 01.01.00  2025-12-22  Hari   Updated into automated test setup with structured test vectors and PASS/FAIL report.
  * 01.02.00  2025-12-27  Hari   Added more cases.
  * 01.03.00  2026-01-09  Hari   Updated and added detailed comments.
  *
  * @endverbatim
  **********************************************************************************************************************/

/***********************************************************************************************************************
 INCLUDES
**********************************************************************************************************************/
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include "Global_Types.h"
#include "FixedPoint.h"

/** @defgroup g_TestHarness Test Harness (Main.c)
 *  @brief Command-line test application for the FixedPoint module.
 */


 /** @addtogroup g_TestHarness
  *  @{
  */

/***********************************************************************************************************************
 MACROS
**********************************************************************************************************************/

/**
 * @brief Tolerance for  16-bit tests.
 *
 * Epsilon is calculated from the SCALE_16 and is set slightly above one least significant bit (LSB) to allow
 * quantization effects because of the fixed point rounding of non-representable values.
 *
 * Any deviation larger than one LSB may indicate an actual error or quantization effects because of the 
 * fixed point rounding of non-representable values.
 */

#define TEST_EPSILON_16      (1.1f / (float)SCALE_16)



/**
 * @brief Tolerance for 8-bit fixed-point tests.
 *
 * Epsilon is calculated from the SCALE_8 configuration and set slightly above one least significant bit (LSB) to allow
 * quantization effects because of the fixed point rounding of non-representable values.
 *
 * Any deviation larger than one LSB may indicate an actual error or quantization effects because of the 
 * fixed point rounding of non-representable values.
 */

#define TEST_EPSILON_8       (1.1f / (float)SCALE_8)



/** @brief Maximum representable real value for 16-bit fixed-point format derived from configuration SCALE_X value. */
#define FIX16_REAL_MAX   ((float)FIX16_MAX / (float)SCALE_16)

/** @brief Minimum representable real value for 16-bit fixed-point format derived from configuration SCALE_X value. */
#define FIX16_REAL_MIN   ((float)FIX16_MIN / (float)SCALE_16)

/** @brief Maximum representable real value for 8-bit fixed-point format derived from configuration SCALE_X value. */
#define FIX8_REAL_MAX    ((float)FIX8_MAX  / (float)SCALE_8)

/** @brief Minimum representable real value for 8-bit fixed-point format derived from configuration SCALE_X value. */
#define FIX8_REAL_MIN    ((float)FIX8_MIN  / (float)SCALE_8)



/** @brief One LSB resolution derived from SCALE_X for 16-bit fixed-point format (real domain). */
#define FIX16_RESOLUTION (1.0f / (float)SCALE_16)

/** @brief Value below one LSB to trigger precision underflow for test case in 16-bit format. */
#define FIX16_BELOW_RES  (0.49f * FIX16_RESOLUTION)

  /** @brief One LSB resolution derived from SCALE_X for 8-bit fixed-point format (real domain). */
#define FIX8_RESOLUTION  (1.0f / (float)SCALE_8)

/** @brief Value below one LSB to trigger precision underflow for test case in 8-bit format. */
#define FIX8_BELOW_RES   (0.49f * FIX8_RESOLUTION)



/***********************************************************************************************************************
 TYPEDEFS
**********************************************************************************************************************/

 /** @brief   Enumeration of all supported arithmetic operations in the test harness. */
typedef enum
{
    TEST_OP_ADD = 0,
    TEST_OP_SUB,
    TEST_OP_MUL,
    TEST_OP_DIV
} Test_Operation_t;

/** @brief   Enumeration of tested fixed point widths. 8 and 16 bits */
typedef enum
{
    TEST_WIDTH_8 = 0,
    TEST_WIDTH_16
} Test_Width_t;

/** @brief   Single test case definition.
 *
 * Each test case defines an operation with inputs  in float,
 * an expected result in float, a tolerance epsilon, description and an expected status
 * (E_OK or E_NOT_OK). The width field selects whether the 8-bit or
 * 16-bit fixed-point function is used.
 * 
 */
typedef struct
{
    float            a;                /**< First operand (float input to the API) */
    float            b;                /**< Second operand (float input to API) */
    float            expected;         /**< Expected result (float) */
    float            epsilon;          /**< Allowed absolute error or tolerance (float) */
    const char*  description;      /**< Short description for displaying in console output */
    Test_Operation_t op;               /**< Operation under test (Add/Sub/Mult/Div) */
    Test_Width_t     width;            /**< Fixed-point width (8-bit or 16-bit) */
    Std_ReturnType   expectedStatus;   /**< Expected return status (E_OK / E_NOT_OK) */
} TestVector_t;

/***********************************************************************************************************************
 LOCAL FUNCTION PROTOTYPES
 **********************************************************************************************************************/
static void RunAllTests(void);
static void RunSingleTest(const TestVector_t* test, unsigned int id, unsigned int* passCount, unsigned int* failCount);

/***********************************************************************************************************************
 LOCAL FUNCTIONS
 **********************************************************************************************************************/

 /*********************************************************************************************************************/
 /*! @brief     Execute a single test vector on the appropriate Fixed Point API.
  *
  *  This function:
  *  - selects the correct API based on test->width and test->op,
  *  - calls the FixedPoint function with float inputs,
  *  - compares the returned status with test->expectedStatus,
  *  - if the status is E_OK, compares the numeric result with expected value
  *    using the provided epsilon tolerance,
  *  - prints a PASS/FAIL line to the console including input, expected and
  *    actual values as well as the status. 
  *  
  *  @param[in]      test        Pointer to test case definition.
  *  @param[in]      id          Test case identifier (index).
  *  @param[in,out]  passCount   Pointer to counter for passed tests.
  *  @param[in,out]  failCount   Pointer to counter for failed tests.
  */
static void RunSingleTest(const TestVector_t* test, unsigned int id, unsigned int* passCount, unsigned int* failCount)
{
    /* Checks and ensures the test vector is not null */
    if (test == NULL)
    {
        /* prints an error and returns if test is NULL */
        printf("Error: Test Vector NULL (id = %u)\n", id);
        return;
    }

    /* Variable to store result */
    float result = 0.0f;

    /* Initialise status */
    Std_ReturnType status = E_NOT_OK;
    
    /* storing the width according to passed the test vector */
    const char* widthStr = (test->width == TEST_WIDTH_16) ? "16-bit" : "8-bit";

    /* for storing operation name */
    const char* opStr = "";

    
    /* Select operation name for logging */
    switch (test->op)
    {
    case TEST_OP_ADD:
    {
        opStr = "ADD";
        break;
    }
    case TEST_OP_SUB:
    {
        opStr = "SUB";
        break;
    }
    case TEST_OP_MUL:
    {
        opStr = "MUL";
        break;
    }
    case TEST_OP_DIV:
    {
        opStr = "DIV";
        break;
    }
    default:
    {
        opStr = "UNK";
        break;
    }
    }

    /* Dispatch to appropriate FixedPoint API as per width and operation type */
    if (test->width == TEST_WIDTH_16)
    {
        switch (test->op)
        {
        case TEST_OP_ADD:
        {
            status = FixedPoint_Add16(test->a, test->b, &result);
            break;
        }
        case TEST_OP_SUB:
        {
            status = FixedPoint_Sub16(test->a, test->b, &result);
            break;
        }
        case TEST_OP_MUL:
        {
            status = FixedPoint_Mult16(test->a, test->b, &result);
            break;
        }
        case TEST_OP_DIV:
        {
            status = FixedPoint_Div16(test->a, test->b, &result);
            break;
        }
        default:
        {
            /* Should not occur */
            break;
        }
        }
    }
    else /* TEST_WIDTH_8 */
    {
        switch (test->op)
        {
        case TEST_OP_ADD:
        {
            status = FixedPoint_Add8(test->a, test->b, &result);
            break;
        }
        case TEST_OP_SUB:
        {
            status = FixedPoint_Sub8(test->a, test->b, &result);
            break;
        }
        case TEST_OP_MUL:
        {
            status = FixedPoint_Mult8(test->a, test->b, &result);
            break;
        }
        case TEST_OP_DIV:
        {
            status = FixedPoint_Div8(test->a, test->b, &result);
            break;
        }
        default:
        {
            /* Should not occur */
            break;
        }
        }
    }

    /* Evaluate status and numeric result */
    /* First: status must match expectation */
    int statusOk = (status == test->expectedStatus) ? 1 : 0;
    int valueOk = 0;

    if (status == E_OK)
    {
        /* Only check numeric result if the operation is expected to be valid */
        float diff = (result - test->expected);
        if (diff < 0.0f)
        {
            diff = -diff;
        }

        valueOk = (diff <= test->epsilon) ? 1 : 0;
    }
    else
    {
        /* For error cases like division by zero do not check the result value */
        valueOk = 1;
    }

    if ((statusOk != 0) && (valueOk != 0))
    {
        printf("[PASS] TC_%02u (%s, %s): A=%8.4f  B=%8.4f  Exp=%8.4f  Got=%8.4f  Stat(exp/act)=%u/%u  %s\n",
            id, widthStr, opStr,
            test->a, test->b,
            test->expected, result,
            test->expectedStatus, status,
            test->description);
        if (passCount != NULL)
        {
            /* increment pass count */
            (*passCount)++;
        }
    }
    else
    {
        printf("[FAIL] TC_%02u (%s, %s): A=%8.4f  B=%8.4f  Exp=%8.4f  Got=%8.4f  Stat(exp/act)=%u/%u  %s\n",
            id, widthStr, opStr,
            test->a, test->b,
            test->expected, result,
            test->expectedStatus, status,
            test->description);
        if (failCount != NULL)
        {
            /* increment fail count */
            (*failCount)++;
        }
    }
}

/*********************************************************************************************************************/
/*! @brief     Execute all predefined test vectors and print a summary.
 *
 *  The test set covers:
 *  - Typical operations in 16-bit and 8-bit
 *  - Positive and negative boundary and saturation behaviour
 *  - Rounding and precision loss near resolution limits
 *  - Division by zero handling
 */
static void RunAllTests(void)
{
    unsigned int passCount = 0u;
    unsigned int failCount = 0u;
    unsigned int i;

    /* Test vector definition.
     * The expected values are approximate and tolerances are defined according to the fixed-point resolution.
     */
    static const TestVector_t tests[] =
    {
        /* 16-bit : Typical arithmetic (calibrated for default Q7.8) */
        {  100.5f,  20.22f, 120.72f,  TEST_EPSILON_16, "16-bit add: typical range",          TEST_OP_ADD, TEST_WIDTH_16, E_OK },
        {  10.0f,   3.0f,   7.0f,    TEST_EPSILON_16, "16-bit sub: typical range",          TEST_OP_SUB, TEST_WIDTH_16, E_OK },
        {   2.0f,   (-1.55f),   (-3.1f),    TEST_EPSILON_16, "16-bit mul: typical range",          TEST_OP_MUL, TEST_WIDTH_16, E_OK},
        {   2.0f,   (1.55f),   (3.1f),    TEST_EPSILON_16, "16-bit mul: typical range",          TEST_OP_MUL, TEST_WIDTH_16, E_OK},
        {   13.50f,   (8.50f),   (114.75f),    TEST_EPSILON_16, "16-bit mul: typical range",          TEST_OP_MUL, TEST_WIDTH_16, E_OK},
        {  11.2f,   (-7.0f),   (-1.6f),    TEST_EPSILON_16, "16-bit div: typical range",          TEST_OP_DIV, TEST_WIDTH_16, E_OK },
        {  8.0f,   (3.0f),   (2.666f),    TEST_EPSILON_16, "16-bit div: typical range",          TEST_OP_DIV, TEST_WIDTH_16, E_OK },
        {  1.99f,   (5.373f), (0.3704f)  ,    TEST_EPSILON_16, "16-bit div: typical range",          TEST_OP_DIV, TEST_WIDTH_16, E_OK },
        

        /* 16-bit Saturation Cases (Configuration aware)*/
        {  20000.0f, 20000.0f,   FIX16_REAL_MAX, TEST_EPSILON_16, "16-bit add: positive saturation", TEST_OP_ADD, TEST_WIDTH_16, E_NOT_OK },
        { (-20000.0f),(-20000.0f), FIX16_REAL_MIN ,  TEST_EPSILON_16, "16-bit add: negative saturation", TEST_OP_ADD, TEST_WIDTH_16, E_NOT_OK },
        { 20000.0f, 2.0f, FIX16_REAL_MAX, TEST_EPSILON_16, "16-bit mul: positive saturation", TEST_OP_MUL, TEST_WIDTH_16, E_NOT_OK },
        { (-20000.0f), 2.0f, FIX16_REAL_MIN, TEST_EPSILON_16, "16-bit mul: negative saturation", TEST_OP_MUL, TEST_WIDTH_16, E_NOT_OK },

        /* 16-bit Boundary Cases (Configuration aware)*/        
        /* Exact boundary representability */
        { FIX16_REAL_MAX, 0.0f, FIX16_REAL_MAX, TEST_EPSILON_16, "16-bit add: exact MAX boundary", TEST_OP_ADD, TEST_WIDTH_16, E_OK },
        { FIX16_REAL_MIN, 0.0f, FIX16_REAL_MIN, TEST_EPSILON_16, "16-bit add: exact MIN boundary", TEST_OP_ADD, TEST_WIDTH_16, E_OK },
        /* Just over boundary saturation (1 LSB above MAX and below MIN)*/
        { FIX16_REAL_MAX, FIX16_RESOLUTION, FIX16_REAL_MAX, TEST_EPSILON_16, "16-bit add: MAX + 1 LSB -> saturation", TEST_OP_ADD, TEST_WIDTH_16, E_NOT_OK },
        { FIX16_REAL_MIN, (-FIX16_RESOLUTION), FIX16_REAL_MIN, TEST_EPSILON_16, "16-bit add: MIN - 1 LSB ->  saturation", TEST_OP_ADD, TEST_WIDTH_16, E_NOT_OK},

        /*16-bit: Rounding midpoint (tie) behaviour (ties away from zero)  (Configuration aware)*/
       { (0.5f * FIX16_RESOLUTION), 0.0f, FIX16_RESOLUTION, TEST_EPSILON_16, "16-bit add: +0.5 LSB rounds to +1 LSB", TEST_OP_ADD, TEST_WIDTH_16, E_OK },
       { (-0.5f * FIX16_RESOLUTION), 0.0f, (-FIX16_RESOLUTION), TEST_EPSILON_16, "16-bit add: -0.5 LSB rounds to -1 LSB", TEST_OP_ADD, TEST_WIDTH_16, E_OK },
 
        /* 16-bit :  Precision underflow (values below half of resolution, Configuration aware) */
        {   FIX16_BELOW_RES,  FIX16_BELOW_RES,  0.0f ,  TEST_EPSILON_16,  "16-bit add: below resolution",         TEST_OP_ADD, TEST_WIDTH_16,  E_OK },

        /* 16-bit : Division by zero */
        {  10.0f,   0.0f,   0.0f,  TEST_EPSILON_16, "16-bit div: division by zero",        TEST_OP_DIV, TEST_WIDTH_16, E_NOT_OK },

        /* 8-bit : Typical arithmetic within range (calibrated for default Q3.4) */
        {   2.0f,   3.5f,   5.5f,  TEST_EPSILON_8,  "8-bit add: typical range",            TEST_OP_ADD, TEST_WIDTH_8,  E_OK },
        {   4.0f,   6.0f,  (-2.0f),  TEST_EPSILON_8,  "8-bit sub: typical range",            TEST_OP_SUB, TEST_WIDTH_8,  E_OK },
        {   2.0f,   -3.12f,   -6.24f,  TEST_EPSILON_8,  "8-bit mul: typical range",            TEST_OP_MUL, TEST_WIDTH_8,  E_OK },
        {   2.0f,   3.12f,   6.24f,  TEST_EPSILON_8,  "8-bit mul: typical range",            TEST_OP_MUL, TEST_WIDTH_8,  E_OK },
        {   5.0f,   2.0f,   2.5f,  TEST_EPSILON_8,  "8-bit div: typical range",            TEST_OP_DIV, TEST_WIDTH_8,  E_OK },
        {   5.0f,   (-2.0f),   (-2.5f),  TEST_EPSILON_8,  "8-bit div: typical range",            TEST_OP_DIV, TEST_WIDTH_8,  E_OK},
         {  7.9f,   (2.0f),   (3.95f),    TEST_EPSILON_8, "8-bit div: typical range",          TEST_OP_DIV, TEST_WIDTH_8, E_OK },
        {  1.99f,   (5.373f), (0.3704f)  ,    TEST_EPSILON_8, "8-bit div: typical range",          TEST_OP_DIV, TEST_WIDTH_8, E_OK },
        
        


        /* 8-bit: Saturation cases (Configuration aware) */
        {   250.0f,   310.0f,   FIX8_REAL_MAX, TEST_EPSILON_8, "8-bit add: positive saturation",      TEST_OP_ADD, TEST_WIDTH_8,  E_NOT_OK },
        {  (-250.0f),  (-310.0f),  FIX8_REAL_MIN,  TEST_EPSILON_8,  "8-bit add: negative saturation",      TEST_OP_ADD, TEST_WIDTH_8,  E_NOT_OK },
        { 300.0f, 2.0f, FIX8_REAL_MAX, TEST_EPSILON_8, "8-bit mul: positive saturation ", TEST_OP_MUL, TEST_WIDTH_8, E_NOT_OK },
        { (-300.0f), 2.0f, FIX8_REAL_MIN, TEST_EPSILON_8, "8-bit mul: negative saturation", TEST_OP_MUL, TEST_WIDTH_8, E_NOT_OK },


        /* 8-bit Boundary Cases (Configuration aware)*/
        /* Exact boundary representability */
        { FIX8_REAL_MAX, 0.0f, FIX8_REAL_MAX, TEST_EPSILON_8, "8-bit add: exact MAX boundary", TEST_OP_ADD, TEST_WIDTH_8, E_OK },
        { FIX8_REAL_MIN, 0.0f, FIX8_REAL_MIN, TEST_EPSILON_8, "8-bit add: exact MIN boundary", TEST_OP_ADD, TEST_WIDTH_8, E_OK },
        /* Just over boundary saturation (1 LSB above MAX and below MIN)*/
        { FIX8_REAL_MAX, FIX8_RESOLUTION, FIX8_REAL_MAX, TEST_EPSILON_8, "8-bit add: MAX + 1 LSB ->  saturation", TEST_OP_ADD, TEST_WIDTH_8, E_NOT_OK },
        { FIX8_REAL_MIN, (-FIX8_RESOLUTION), FIX8_REAL_MIN, TEST_EPSILON_8, "8-bit add: MIN - 1 LSB ->  saturation", TEST_OP_ADD, TEST_WIDTH_8, E_NOT_OK},

        /*8-bit: Rounding midpoint (tie) behaviour (ties away from zero)  (Configuration aware)*/
       { (0.5f * FIX8_RESOLUTION), 0.0f, FIX8_RESOLUTION, TEST_EPSILON_8, "8-bit add: +0.5 LSB rounds to +1 LSB", TEST_OP_ADD, TEST_WIDTH_8, E_OK },
       { (-0.5f * FIX8_RESOLUTION), 0.0f, (-FIX8_RESOLUTION), TEST_EPSILON_8, "8-bit add: -0.5 LSB rounds to -1 LSB", TEST_OP_ADD, TEST_WIDTH_8, E_OK },


        /* 8-bit Precision underflow (values below half of resolution, Configuration aware) */
        {   FIX8_BELOW_RES,  FIX8_BELOW_RES,  0.0f ,  TEST_EPSILON_8,  "8-bit add: below resolution",         TEST_OP_ADD, TEST_WIDTH_8,  E_OK },
       

        /* 8-bit : Division by zero */
        {   5.0f,   0.0f,   0.0f,  TEST_EPSILON_8,  "8-bit div: division by zero",         TEST_OP_DIV, TEST_WIDTH_8,  E_NOT_OK }
    };

    const unsigned int numTests = (unsigned int)(sizeof(tests) / sizeof(tests[0]));

    printf("--- FIXED POINT ARITHMETIC TEST REPORT ---\n\n");

    for (i = 0u; i < numTests; i++)
    {
        /* Run each test case */
        RunSingleTest(&tests[i], i + 1u, &passCount, &failCount);
    }

    printf("\n--- SUMMARY ---\n");
    printf("Total tests : %u\n", numTests);
    printf("Passed      : %u\n", passCount);
    printf("Failed      : %u\n", failCount);
}

/***********************************************************************************************************************
 GLOBAL FUNCTIONS
 **********************************************************************************************************************/

 /*********************************************************************************************************************/
 /*! @brief        Main entry point for the FixedPoint test suite.
  *
  *  This function executes all predefined test vectors for both 16-bit and
  *  8 bit fixed-point arithmetic. The test results are printed to the console
  *  as PASS/FAIL. Afterwards, the application waits for a key press
  *  before terminating, so that output remains visible.
  *
  *  @return       int
  *
  *  @retval       0            Execution finished successfully.
  */
int main(void)
{
    RunAllTests();

    printf("\nPress any key to close.....\n");
    while (!_kbhit())
    {
        Sleep(1);
    }

    return 0;
}

/** @} end addtogroup */

/***********************************************************************************************************************
EOF
**********************************************************************************************************************/
