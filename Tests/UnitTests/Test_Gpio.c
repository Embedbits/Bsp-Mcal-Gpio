/**
 * \author Mr.Nobody
 * \file Test_Gpio.c
 * \ingroup Gpio
 * \brief Unit tests of General-Purpose Input/Output (GPIO) module.
 *
 * Gpio.c is compiled unchanged with real LL drivers. GPIO registers are emulated
 * by RegMem (host memory at MCU addresses), RCC module is mocked by CMock.
 *
 * \note Emulated registers are plain memory. Output data register (ODR) is not
 *       updated by writes to BSRR / BRR, tests preset ODR where the module
 *       verifies the written level by read-back.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "RegMem.h"                         /* Register memory emulation      */
#include "Gpio_Port.h"                      /* Module under test              */
#include "MockRcc_Port.h"                   /* RCC module mock                */
#include "Stm32_gpio.h"                     /* GPIO registers definition      */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

static void Ut_Gpio_Expect_PortActivation ( rcc_PeriphId_t periphId );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/** Width of one pin configuration field in MODER / OSPEEDR / PUPDR */
#define TEST_GPIO_FIELD2_WIDTH              ( 2u )

/** Width of one pin configuration field in AFRL / AFRH */
#define TEST_GPIO_FIELD4_WIDTH              ( 4u )

/** Count of pins configured by one alternate function register */
#define TEST_GPIO_AFR_PIN_CNT               ( 8u )

/** Mask of two bit wide configuration field */
#define TEST_GPIO_FIELD2_MASK               ( 0x3u )

/** Mask of four bit wide configuration field */
#define TEST_GPIO_FIELD4_MASK               ( 0xFu )

/** Index of AFRH register in AFR array */
#define TEST_GPIO_AFRH_IDX                  ( 1u )

/* ============================== MACROS ==================================== */

/** Mask of pin bit in one bit per pin registers (ODR, IDR, BSRR, BRR, OTYPER) */
#define TEST_GPIO_PIN_MASK( pin )           ( 1u << (uint32_t)( pin ) )

/** Value of two bit wide field of pin */
#define TEST_GPIO_FIELD2( reg, pin )        ( ( ( reg ) >> ( TEST_GPIO_FIELD2_WIDTH * (uint32_t)( pin ) ) ) & TEST_GPIO_FIELD2_MASK )

/* ========================== LOCAL VARIABLES =============================== */

/** Expected register block of every port, order of \ref gpio_PortId_t */
static GPIO_TypeDef * const utGpio_RegLut[ GPIO_PORT_CNT ] =
{
#if defined(GPIOA)
    GPIOA,
#endif
#if defined(GPIOB)
    GPIOB,
#endif
#if defined(GPIOC)
    GPIOC,
#endif
#if defined(GPIOD)
    GPIOD,
#endif
#if defined(GPIOE)
    GPIOE,
#endif
#if defined(GPIOF)
    GPIOF,
#endif
#if defined(GPIOG)
    GPIOG,
#endif
#if defined(GPIOH)
    GPIOH,
#endif
#if defined(GPIOI)
    GPIOI,
#endif
#if defined(GPIOJ)
    GPIOJ,
#endif
#if defined(GPIOK)
    GPIOK,
#endif
};

/* ============================ TEST FIXTURE ================================ */

void setUp( void )
{
    TEST_ASSERT_EQUAL( REGMEM_REQUEST_OK, RegMem_Reset() );
}


void tearDown( void )
{
    /* Mocks are verified by generated runner */
}

/* ========================== MODULE VERSION ================================ */

/**
 * \brief   Gpio_Get_ModuleVersion() returns version of the module.
 *
 * \details Reads the module version structure.
 *
 * \par Expected results
 * - Version is 1.0.0 (Major 1, Minor 0, Patch 0).
 */
void Ut_Gpio_Get_ModuleVersion_ReturnsVersion( void )
{
    gpio_ModuleVersion_t version = Gpio_Get_ModuleVersion();

    TEST_ASSERT_EQUAL_UINT8( 1u, version.Major );
    TEST_ASSERT_EQUAL_UINT8( 0u, version.Minor );
    TEST_ASSERT_EQUAL_UINT8( 0u, version.Patch );
}

/* ========================== PORT ACTIVATION =============================== */

/**
 * \brief   Gpio_Set_PortActive() activates inactive port clock.
 *
 * \details RCC mock reports inactive GPIOA clock, activation is expected.
 *
 * \par Expected results
 * - Rcc_Get_PeriphState() and Rcc_Set_PeriphActive() called for RCC_PERIPH_GPIOA.
 * - GPIO_REQUEST_OK is returned.
 */
void Ut_Gpio_Set_PortActive_ClockInactive_ActivatesClock( void )
{
    Ut_Gpio_Expect_PortActivation( RCC_PERIPH_GPIOA );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PortActive( GPIO_PORT_A ) );
}


/**
 * \brief   Gpio_Set_PortActive() does not activate already active clock.
 *
 * \details RCC mock reports active GPIOB clock.
 *
 * \par Expected results
 * - Only clock state is read, Rcc_Set_PeriphActive() is not called.
 * - GPIO_REQUEST_OK is returned.
 */
void Ut_Gpio_Set_PortActive_ClockActive_DoesNotReactivate( void )
{
    rcc_FunctionState_t clockState = RCC_FUNCTION_ACTIVE;

    Rcc_Get_PeriphState_ExpectAndReturn( RCC_PERIPH_GPIOB, NULL, RCC_REQUEST_OK );
    Rcc_Get_PeriphState_IgnoreArg_funcState();
    Rcc_Get_PeriphState_ReturnThruPtr_funcState( &clockState );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PortActive( GPIO_PORT_B ) );
}


/**
 * \brief   Gpio_Set_PortActive() reports error of clock state read.
 *
 * \details Rcc_Get_PeriphState() returns error.
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR is returned, clock is not activated.
 */
void Ut_Gpio_Set_PortActive_RccStateError_ReturnsError( void )
{
    Rcc_Get_PeriphState_ExpectAndReturn( RCC_PERIPH_GPIOA, NULL, RCC_REQUEST_ERROR );
    Rcc_Get_PeriphState_IgnoreArg_funcState();

    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PortActive( GPIO_PORT_A ) );
}


/**
 * \brief   Gpio_Set_PortActive() reports error of clock activation.
 *
 * \details Clock is inactive, Rcc_Set_PeriphActive() returns error.
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR is returned.
 */
void Ut_Gpio_Set_PortActive_RccActivationError_ReturnsError( void )
{
    rcc_FunctionState_t clockState = RCC_FUNCTION_INACTIVE;

    Rcc_Get_PeriphState_ExpectAndReturn( RCC_PERIPH_GPIOA, NULL, RCC_REQUEST_OK );
    Rcc_Get_PeriphState_IgnoreArg_funcState();
    Rcc_Get_PeriphState_ReturnThruPtr_funcState( &clockState );
    Rcc_Set_PeriphActive_ExpectAndReturn( RCC_PERIPH_GPIOA, RCC_REQUEST_ERROR );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PortActive( GPIO_PORT_A ) );
}


/**
 * \brief   Gpio_Set_PortActive() rejects invalid port.
 *
 * \details Calls Gpio_Set_PortActive( GPIO_PORT_CNT ).
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR is returned, RCC is not called (strict mock).
 */
void Ut_Gpio_Set_PortActive_InvalidPort_ReturnsErrorWithoutRccAccess( void )
{
    /* No RCC call expected - strict mock fails on any call */
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PortActive( GPIO_PORT_CNT ) );
}


/**
 * \brief   Gpio_Set_PortInactive() deactivates port clock.
 *
 * \details Deactivates port C, RCC deactivation is expected.
 *
 * \par Expected results
 * - Rcc_Set_PeriphInactive( RCC_PERIPH_GPIOC ) called, GPIO_REQUEST_OK returned.
 */
void Ut_Gpio_Set_PortInactive_DeactivatesClock( void )
{
    Rcc_Set_PeriphInactive_ExpectAndReturn( RCC_PERIPH_GPIOC, RCC_REQUEST_OK );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PortInactive( GPIO_PORT_C ) );
}

/* ============================== PIN MODE ================================== */

/**
 * \brief   Gpio_Set_PinMode() writes output mode of the pin into MODER.
 *
 * \details Sets PA5 to output mode.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, MODER = MODE5_0 only (field of pin 5 = output, other pins 0).
 */
void Ut_Gpio_Set_PinMode_Output_WritesModer( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( GPIO_PORT_A, GPIO_PIN_ID_5, GPIO_PIN_MODE_OUTPUT ) );

    TEST_ASSERT_EQUAL_HEX32( LL_GPIO_MODE_OUTPUT, TEST_GPIO_FIELD2( GPIOA->MODER, GPIO_PIN_ID_5 ) );
    TEST_ASSERT_EQUAL_HEX32( GPIO_MODER_MODE5_0, GPIOA->MODER );
}


/**
 * \brief   Gpio_Set_PinMode() addresses right register and field of every pin.
 *
 * \details Sets analog mode on every pin of every port available on the MCU.
 *
 * \par Expected results
 * - Every call returns GPIO_REQUEST_OK and writes the field of the pin in MODER of
 *   its own port.
 * - MODER of every port = 0xFFFFFFFF after all its pins are configured.
 */
void Ut_Gpio_Set_PinMode_AllPortsAllPins_WritesOwnField( void )
{
    for( uint32_t portId = 0u; GPIO_PORT_CNT > portId; portId++ )
    {
        for( uint32_t pinId = 0u; GPIO_PIN_ID_CNT > pinId; pinId++ )
        {
            gpio_RequestState_t reqState = Gpio_Set_PinMode( (gpio_PortId_t)portId, (gpio_PinId_t)pinId, GPIO_PIN_MODE_ANALOG );

            TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, reqState );
            TEST_ASSERT_EQUAL_HEX32( LL_GPIO_MODE_ANALOG, TEST_GPIO_FIELD2( utGpio_RegLut[ portId ]->MODER, pinId ) );
        }

        /* All pins of the port configured, no other port touched */
        TEST_ASSERT_EQUAL_HEX32( 0xFFFFFFFFu, utGpio_RegLut[ portId ]->MODER );
    }
}


/**
 * \brief   Gpio_Set_PinMode() rejects invalid arguments.
 *
 * \details Calls Gpio_Set_PinMode() with invalid port, pin and mode.
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR in all cases, GPIOA MODER is not written.
 */
void Ut_Gpio_Set_PinMode_InvalidArgs_ReturnsErrorWithoutWrite( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinMode( GPIO_PORT_CNT, GPIO_PIN_ID_0,   GPIO_PIN_MODE_OUTPUT ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinMode( GPIO_PORT_A,   GPIO_PIN_ID_CNT, GPIO_PIN_MODE_OUTPUT ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinMode( GPIO_PORT_A,   GPIO_PIN_ID_0,   (gpio_PinMode_t)( GPIO_PIN_MODE_ANALOG + 1u ) ) );

    TEST_ASSERT_EQUAL_HEX32( 0u, GPIOA->MODER );
}


/**
 * \brief   Gpio_Get_PinMode() decodes MODER field of the pin.
 *
 * \details Presets alternate function mode of PB3 in MODER and reads pin mode.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, mode is GPIO_PIN_MODE_ALTERNATE.
 */
void Ut_Gpio_Get_PinMode_ReadsModer( void )
{
    gpio_PinMode_t pinMode = GPIO_PIN_MODE_INPUT;

    GPIOB->MODER = GPIO_MODER_MODE3_1;  /* Alternate function mode on pin 3 */

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinMode( GPIO_PORT_B, GPIO_PIN_ID_3, &pinMode ) );
    TEST_ASSERT_EQUAL( GPIO_PIN_MODE_ALTERNATE, pinMode );
}


/**
 * \brief   Gpio_Get_PinMode() rejects NULL pointer.
 *
 * \details Calls Gpio_Get_PinMode() with NULL output pointer.
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR is returned.
 */
void Ut_Gpio_Get_PinMode_NullPtr_ReturnsError( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PinMode( GPIO_PORT_A, GPIO_PIN_ID_0, NULL ) );
}

/* ========================== ALTERNATE FUNCTION ============================ */

/**
 * \brief   Gpio_Set_PinAltFunction() of pin 0 - 7 writes AFRL.
 *
 * \details Sets alternate function 7 of PA2.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, AFRL field of pin 2 = AF7, AFRH not written.
 */
void Ut_Gpio_Set_PinAltFunction_LowPin_WritesAfrl( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinAltFunction( GPIO_PORT_A, GPIO_PIN_ID_2, GPIO_ALT_FUNC_7 ) );

    TEST_ASSERT_EQUAL_HEX32( LL_GPIO_AF_7 << ( TEST_GPIO_FIELD4_WIDTH * GPIO_PIN_ID_2 ), GPIOA->AFR[ 0 ] );
    TEST_ASSERT_EQUAL_HEX32( 0u, GPIOA->AFR[ TEST_GPIO_AFRH_IDX ] );
}


/**
 * \brief   Gpio_Set_PinAltFunction() of pin 8 - 15 writes AFRH.
 *
 * \details Sets alternate function 12 of PA9.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, AFRL not written, AFRH field of pin 9 = AF12.
 */
void Ut_Gpio_Set_PinAltFunction_HighPin_WritesAfrh( void )
{
    const uint32_t afrhPos = TEST_GPIO_FIELD4_WIDTH * ( GPIO_PIN_ID_9 - TEST_GPIO_AFR_PIN_CNT );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinAltFunction( GPIO_PORT_A, GPIO_PIN_ID_9, GPIO_ALT_FUNC_12 ) );

    TEST_ASSERT_EQUAL_HEX32( 0u, GPIOA->AFR[ 0 ] );
    TEST_ASSERT_EQUAL_HEX32( LL_GPIO_AF_12, ( GPIOA->AFR[ TEST_GPIO_AFRH_IDX ] >> afrhPos ) & TEST_GPIO_FIELD4_MASK );
}


/**
 * \brief   Gpio_Get_PinAltFunction() of pin 8 - 15 reads AFRH.
 *
 * \details Presets AF5 in AFRH field of PC15 and reads the alternate function.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, alternate function is GPIO_ALT_FUNC_5.
 */
void Ut_Gpio_Get_PinAltFunction_HighPin_ReadsAfrh( void )
{
    gpio_AltFunction_t altFunc = GPIO_ALT_FUNC_0;

    GPIOC->AFR[ TEST_GPIO_AFRH_IDX ] = LL_GPIO_AF_5 << ( TEST_GPIO_FIELD4_WIDTH * ( GPIO_PIN_ID_15 - TEST_GPIO_AFR_PIN_CNT ) );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinAltFunction( GPIO_PORT_C, GPIO_PIN_ID_15, &altFunc ) );
    TEST_ASSERT_EQUAL( GPIO_ALT_FUNC_5, altFunc );
}

/* ====================== OUTPUT TYPE, SPEED, PULL ========================== */

/**
 * \brief   Gpio_Set_PinOutType() writes open-drain output type.
 *
 * \details Sets PB7 to open-drain.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, OTYPER = bit 7 only.
 */
void Ut_Gpio_Set_PinOutType_OpenDrain_WritesOtyper( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinOutType( GPIO_PORT_B, GPIO_PIN_ID_7, GPIO_PIN_OUTPUT_OPENDRAIN ) );

    TEST_ASSERT_EQUAL_HEX32( TEST_GPIO_PIN_MASK( GPIO_PIN_ID_7 ), GPIOB->OTYPER );
}


/**
 * \brief   Gpio_Set_PinSpeed() writes very high speed into OSPEEDR.
 *
 * \details Sets very high speed of PA15.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, OSPEEDR field of pin 15 = very high speed.
 */
void Ut_Gpio_Set_PinSpeed_VeryHigh_WritesOspeedr( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinSpeed( GPIO_PORT_A, GPIO_PIN_ID_15, GPIO_PIN_SPEED_VERY_HIGH ) );

    TEST_ASSERT_EQUAL_HEX32( LL_GPIO_SPEED_FREQ_VERY_HIGH, TEST_GPIO_FIELD2( GPIOA->OSPEEDR, GPIO_PIN_ID_15 ) );
}


/**
 * \brief   Gpio_Set_PinPull() writes pull-down into PUPDR.
 *
 * \details Sets pull-down of PA1.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, PUPDR field of pin 1 = pull-down.
 */
void Ut_Gpio_Set_PinPull_Down_WritesPupdr( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinPull( GPIO_PORT_A, GPIO_PIN_ID_1, GPIO_PIN_PULL_DOWN ) );

    TEST_ASSERT_EQUAL_HEX32( LL_GPIO_PULL_DOWN, TEST_GPIO_FIELD2( GPIOA->PUPDR, GPIO_PIN_ID_1 ) );
}

/* ============================== PIN LEVEL ================================= */

/**
 * \brief   Gpio_Set_PinLevel() with high level writes BSRR set bit.
 *
 * \details Presets ODR bit 4 (emulates HW reaction for read-back check) and sets
 *          PA4 high.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, BSRR = bit 4, BRR not written.
 */
void Ut_Gpio_Set_PinLevel_High_WritesBsrr( void )
{
    GPIOA->ODR = TEST_GPIO_PIN_MASK( GPIO_PIN_ID_4 );   /* Emulates HW reaction on BSRR */

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinLevel( GPIO_PORT_A, GPIO_PIN_ID_4, GPIO_PIN_LEVEL_HIGH ) );

    TEST_ASSERT_EQUAL_HEX32( TEST_GPIO_PIN_MASK( GPIO_PIN_ID_4 ), GPIOA->BSRR );
    TEST_ASSERT_EQUAL_HEX32( 0u, GPIOA->BRR );
}


/**
 * \brief   Gpio_Set_PinLevel() with low level writes BRR.
 *
 * \details Sets PA4 low (ODR bit already 0).
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, BRR = bit 4, BSRR not written.
 */
void Ut_Gpio_Set_PinLevel_Low_WritesBrr( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinLevel( GPIO_PORT_A, GPIO_PIN_ID_4, GPIO_PIN_LEVEL_LOW ) );

    TEST_ASSERT_EQUAL_HEX32( TEST_GPIO_PIN_MASK( GPIO_PIN_ID_4 ), GPIOA->BRR );
    TEST_ASSERT_EQUAL_HEX32( 0u, GPIOA->BSRR );
}


/**
 * \brief   Gpio_Set_PinLevel() reports level not applied on the output.
 *
 * \details Sets PA4 high while ODR stays 0 (read-back check fails after timeout).
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR is returned.
 */
void Ut_Gpio_Set_PinLevel_LevelNotApplied_ReturnsError( void )
{
    /* ODR stays low - read-back verification has to fail after timeout */
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinLevel( GPIO_PORT_A, GPIO_PIN_ID_4, GPIO_PIN_LEVEL_HIGH ) );
}


/**
 * \brief   Gpio_Set_PinLevel() rejects invalid level.
 *
 * \details Sets PA4 to level out of range.
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR, BSRR and BRR are not written.
 */
void Ut_Gpio_Set_PinLevel_InvalidLevel_ReturnsErrorWithoutWrite( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinLevel( GPIO_PORT_A, GPIO_PIN_ID_4, (gpio_PinLevel_t)( GPIO_PIN_LEVEL_HIGH + 1u ) ) );

    TEST_ASSERT_EQUAL_HEX32( 0u, GPIOA->BSRR );
    TEST_ASSERT_EQUAL_HEX32( 0u, GPIOA->BRR );
}


/**
 * \brief   Gpio_Get_PinLevel() reads level from IDR.
 *
 * \details Presets IDR bit 12 of port B, reads level of PB12 and PB11.
 *
 * \par Expected results
 * - PB12 is HIGH, PB11 is LOW, both reads return GPIO_REQUEST_OK.
 */
void Ut_Gpio_Get_PinLevel_ReadsIdr( void )
{
    gpio_PinLevel_t pinLevel = GPIO_PIN_LEVEL_LOW;

    GPIOB->IDR = TEST_GPIO_PIN_MASK( GPIO_PIN_ID_12 );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinLevel( GPIO_PORT_B, GPIO_PIN_ID_12, &pinLevel ) );
    TEST_ASSERT_EQUAL( GPIO_PIN_LEVEL_HIGH, pinLevel );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinLevel( GPIO_PORT_B, GPIO_PIN_ID_11, &pinLevel ) );
    TEST_ASSERT_EQUAL( GPIO_PIN_LEVEL_LOW, pinLevel );
}


/**
 * \brief   Gpio_Get_PinLevel() rejects NULL pointer.
 *
 * \details Calls Gpio_Get_PinLevel() with NULL output pointer.
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR is returned.
 */
void Ut_Gpio_Get_PinLevel_NullPtr_ReturnsError( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PinLevel( GPIO_PORT_A, GPIO_PIN_ID_0, NULL ) );
}


/**
 * \brief   Gpio_Toggle_PinLevel() of high output resets the pin.
 *
 * \details Presets ODR bit 6 of port A (pin high) and toggles PA6.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, BSRR = reset bit of pin 6 (upper half).
 */
void Ut_Gpio_Toggle_PinLevel_High_WritesResetToBsrr( void )
{
    GPIOA->ODR = TEST_GPIO_PIN_MASK( GPIO_PIN_ID_6 );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Toggle_PinLevel( GPIO_PORT_A, GPIO_PIN_ID_6 ) );

    /* Upper half of BSRR resets the pin */
    TEST_ASSERT_EQUAL_HEX32( TEST_GPIO_PIN_MASK( GPIO_PIN_ID_6 ) << GPIO_BSRR_BR0_Pos, GPIOA->BSRR );
}


/**
 * \brief   Inactive state of active-high pin is low level.
 *
 * \details Sets PA8 inactive with active level HIGH.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, BRR = bit 8 (pin driven low).
 */
void Ut_Gpio_Set_PinStateInactive_ActiveHigh_DrivesLow( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinStateInactive( GPIO_PORT_A, GPIO_PIN_ID_8, GPIO_PIN_LEVEL_HIGH ) );

    TEST_ASSERT_EQUAL_HEX32( TEST_GPIO_PIN_MASK( GPIO_PIN_ID_8 ), GPIOA->BRR );
}


/**
 * \brief   Active state of active-low pin is low level.
 *
 * \details Sets PA8 active with active level LOW.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, BRR = bit 8 (pin driven low).
 */
void Ut_Gpio_Set_PinStateActive_ActiveLow_DrivesLow( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinStateActive( GPIO_PORT_A, GPIO_PIN_ID_8, GPIO_PIN_LEVEL_LOW ) );

    TEST_ASSERT_EQUAL_HEX32( TEST_GPIO_PIN_MASK( GPIO_PIN_ID_8 ), GPIOA->BRR );
}

/* ============================ INITIALIZATION ============================== */

/**
 * \brief   Gpio_Init() configures output pin completely.
 *
 * \details Initializes PB10 as open-drain output, pull-up, high speed, active high.
 *          Port clock is inactive - activation in RCC is expected.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK.
 * - Inactive level is set first (BRR = bit 10).
 * - OTYPER bit 10 set, OSPEEDR = high, PUPDR = pull-up, MODER = output for pin 10.
 */
void Ut_Gpio_Init_OutputPin_ConfiguresAllRegisters( void )
{
    gpio_Config_t config =
    {
        .PortId         = GPIO_PORT_B,
        .PinId          = GPIO_PIN_ID_10,
        .PinMode        = GPIO_PIN_MODE_OUTPUT,
        .PinPull        = GPIO_PIN_PULL_UP,
        .PinSpeed       = GPIO_PIN_SPEED_HIGH,
        .PinOutType     = GPIO_PIN_OUTPUT_OPENDRAIN,
        .PinAltFunction = GPIO_ALT_FUNC_0,
        .PinActiveLevel = GPIO_PIN_LEVEL_HIGH
    };

    Ut_Gpio_Expect_PortActivation( RCC_PERIPH_GPIOB );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Init( &config ) );

    TEST_ASSERT_EQUAL_HEX32( TEST_GPIO_PIN_MASK( GPIO_PIN_ID_10 ), GPIOB->BRR );    /* Inactive level */
    TEST_ASSERT_EQUAL_HEX32( TEST_GPIO_PIN_MASK( GPIO_PIN_ID_10 ), GPIOB->OTYPER );
    TEST_ASSERT_EQUAL_HEX32( LL_GPIO_SPEED_FREQ_HIGH, TEST_GPIO_FIELD2( GPIOB->OSPEEDR, GPIO_PIN_ID_10 ) );
    TEST_ASSERT_EQUAL_HEX32( LL_GPIO_PULL_UP,         TEST_GPIO_FIELD2( GPIOB->PUPDR,   GPIO_PIN_ID_10 ) );
    TEST_ASSERT_EQUAL_HEX32( LL_GPIO_MODE_OUTPUT,     TEST_GPIO_FIELD2( GPIOB->MODER,   GPIO_PIN_ID_10 ) );
}


/**
 * \brief   Gpio_Init() rejects NULL configuration.
 *
 * \details Calls Gpio_Init() with NULL pointer.
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR is returned, RCC is not called.
 */
void Ut_Gpio_Init_NullConfig_ReturnsError( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Init( NULL ) );
}


/**
 * \brief   Gpio_Init() stops on port clock activation error.
 *
 * \details Initializes PA0 as output, Rcc_Get_PeriphState() returns error.
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR, BRR and MODER of port A are not written.
 */
void Ut_Gpio_Init_PortActivationError_DoesNotTouchRegisters( void )
{
    gpio_Config_t config =
    {
        .PortId         = GPIO_PORT_A,
        .PinId          = GPIO_PIN_ID_0,
        .PinMode        = GPIO_PIN_MODE_OUTPUT,
        .PinPull        = GPIO_PIN_PULL_NONE,
        .PinSpeed       = GPIO_PIN_SPEED_LOW,
        .PinOutType     = GPIO_PIN_OUTPUT_PUSHPULL,
        .PinAltFunction = GPIO_ALT_FUNC_0,
        .PinActiveLevel = GPIO_PIN_LEVEL_HIGH
    };

    Rcc_Get_PeriphState_ExpectAndReturn( RCC_PERIPH_GPIOA, NULL, RCC_REQUEST_ERROR );
    Rcc_Get_PeriphState_IgnoreArg_funcState();

    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Init( &config ) );

    TEST_ASSERT_EQUAL_HEX32( 0u, GPIOA->BRR );
    TEST_ASSERT_EQUAL_HEX32( 0u, GPIOA->MODER );
}

/* ======================== DEINIT / TASK / STATE =========================== */

/**
 * \brief   Gpio_Deinit() and Gpio_Task() do not touch the hardware.
 *
 * \details Both functions are called, no RCC call is expected (strict mock).
 *
 * \par Expected results
 * - Registers of every port stay 0.
 */
void Ut_Gpio_Deinit_Task_NoRegisterAccess( void )
{
    Gpio_Deinit();
    Gpio_Task();

    for( uint32_t portIdx = 0u; GPIO_PORT_CNT > portIdx; portIdx++ )
    {
        TEST_ASSERT_EQUAL_HEX32( 0u, utGpio_RegLut[ portIdx ]->MODER | utGpio_RegLut[ portIdx ]->OTYPER |
                                     utGpio_RegLut[ portIdx ]->PUPDR | utGpio_RegLut[ portIdx ]->BSRR |
                                     utGpio_RegLut[ portIdx ]->BRR );
    }
}


/**
 * \brief   Gpio_Get_PortState() returns clock state of the port.
 *
 * \details RCC reports active clock of GPIOC, then RCC error, then invalid port.
 *
 * \par Expected results
 * - Active clock: GPIO_REQUEST_OK, GPIO_FUNCTION_ACTIVE.
 * - RCC error and invalid port: GPIO_REQUEST_ERROR (no RCC call for invalid port).
 */
void Ut_Gpio_Get_PortState_ReadsClockState( void )
{
    rcc_FunctionState_t  rccState  = RCC_FUNCTION_ACTIVE;
    gpio_FunctionState_t portState = GPIO_FUNCTION_INACTIVE;

    Rcc_Get_PeriphState_ExpectAndReturn( RCC_PERIPH_GPIOC, NULL, RCC_REQUEST_OK );
    Rcc_Get_PeriphState_IgnoreArg_funcState();
    Rcc_Get_PeriphState_ReturnThruPtr_funcState( &rccState );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PortState( GPIO_PORT_C, &portState ) );
    TEST_ASSERT_EQUAL( GPIO_FUNCTION_ACTIVE, portState );

    Rcc_Get_PeriphState_ExpectAndReturn( RCC_PERIPH_GPIOC, NULL, RCC_REQUEST_ERROR );
    Rcc_Get_PeriphState_IgnoreArg_funcState();

    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PortState( GPIO_PORT_C, &portState ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PortState( GPIO_PORT_CNT, &portState ) );
}


/**
 * \brief   Getters of pin configuration read the registers.
 *
 * \details OSPEEDR, OTYPER and PUPDR of PB9 preset (high speed, open-drain, pull-up).
 *
 * \par Expected results
 * - Gpio_Get_PinSpeed: high, Gpio_Get_PinOutType: open-drain, Gpio_Get_PinPull: pull-up.
 */
void Ut_Gpio_Get_PinConfig_ReadsRegisters( void )
{
    gpio_PinSpeed_t      speed   = GPIO_PIN_SPEED_LOW;
    gpio_PinOutputType_t outType = GPIO_PIN_OUTPUT_PUSHPULL;
    gpio_PinPullCfg_t    pull    = GPIO_PIN_PULL_NONE;

    GPIOB->OSPEEDR = LL_GPIO_SPEED_FREQ_HIGH << ( TEST_GPIO_FIELD2_WIDTH * GPIO_PIN_ID_9 );
    GPIOB->OTYPER  = TEST_GPIO_PIN_MASK( GPIO_PIN_ID_9 );
    GPIOB->PUPDR   = LL_GPIO_PULL_UP << ( TEST_GPIO_FIELD2_WIDTH * GPIO_PIN_ID_9 );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinSpeed( GPIO_PORT_B, GPIO_PIN_ID_9, &speed ) );
    TEST_ASSERT_EQUAL( GPIO_PIN_SPEED_HIGH, speed );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinOutType( GPIO_PORT_B, GPIO_PIN_ID_9, &outType ) );
    TEST_ASSERT_EQUAL( GPIO_PIN_OUTPUT_OPENDRAIN, outType );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinPull( GPIO_PORT_B, GPIO_PIN_ID_9, &pull ) );
    TEST_ASSERT_EQUAL( GPIO_PIN_PULL_UP, pull );
}


/**
 * \brief   Pin functions reject invalid arguments without register access.
 *
 * \details Invalid port, invalid pin, NULL pointers, invalid configuration values.
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR for every call, registers of GPIOA stay 0.
 */
void Ut_Gpio_PinFunctions_InvalidArgs_ReturnError( void )
{
    gpio_PinSpeed_t      speed   = GPIO_PIN_SPEED_LOW;
    gpio_PinOutputType_t outType = GPIO_PIN_OUTPUT_PUSHPULL;
    gpio_PinPullCfg_t    pull    = GPIO_PIN_PULL_NONE;
    gpio_AltFunction_t   altFunc = GPIO_ALT_FUNC_0;

    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PinSpeed( GPIO_PORT_CNT, GPIO_PIN_ID_0, &speed ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PinSpeed( GPIO_PORT_A, GPIO_PIN_ID_CNT, &speed ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PinSpeed( GPIO_PORT_A, GPIO_PIN_ID_0, NULL ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PinOutType( GPIO_PORT_CNT, GPIO_PIN_ID_0, &outType ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PinOutType( GPIO_PORT_A, GPIO_PIN_ID_0, NULL ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PinPull( GPIO_PORT_A, GPIO_PIN_ID_CNT, &pull ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PinPull( GPIO_PORT_A, GPIO_PIN_ID_0, NULL ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PinAltFunction( GPIO_PORT_A, GPIO_PIN_ID_0, NULL ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Get_PinAltFunction( GPIO_PORT_CNT, GPIO_PIN_ID_0, &altFunc ) );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinSpeed( GPIO_PORT_CNT, GPIO_PIN_ID_0, GPIO_PIN_SPEED_HIGH ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinSpeed( GPIO_PORT_A, GPIO_PIN_ID_CNT, GPIO_PIN_SPEED_HIGH ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinOutType( GPIO_PORT_CNT, GPIO_PIN_ID_0, GPIO_PIN_OUTPUT_OPENDRAIN ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinPull( GPIO_PORT_A, GPIO_PIN_ID_CNT, GPIO_PIN_PULL_UP ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinAltFunction( GPIO_PORT_A, GPIO_PIN_ID_0, GPIO_ALT_FUNC_CNT ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinAltFunction( GPIO_PORT_CNT, GPIO_PIN_ID_0, GPIO_ALT_FUNC_1 ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Toggle_PinLevel( GPIO_PORT_CNT, GPIO_PIN_ID_0 ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Toggle_PinLevel( GPIO_PORT_A, GPIO_PIN_ID_CNT ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinStateActive( GPIO_PORT_CNT, GPIO_PIN_ID_0, GPIO_PIN_LEVEL_HIGH ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinStateActive( GPIO_PORT_A, GPIO_PIN_ID_0, (gpio_PinLevel_t)( GPIO_PIN_LEVEL_HIGH + 1u ) ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PinStateInactive( GPIO_PORT_A, GPIO_PIN_ID_CNT, GPIO_PIN_LEVEL_HIGH ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PortInactive( GPIO_PORT_CNT ) );

    TEST_ASSERT_EQUAL_HEX32( 0u, GPIOA->MODER | GPIOA->OTYPER | GPIOA->OSPEEDR | GPIOA->PUPDR | GPIOA->BSRR | GPIOA->BRR | GPIOA->AFR[ 0u ] );
}


/**
 * \brief   Gpio_Set_PortInactive() reports RCC failure.
 *
 * \par Expected results
 * - GPIO_REQUEST_ERROR.
 */
void Ut_Gpio_Set_PortInactive_RccError_ReturnsError( void )
{
    Rcc_Set_PeriphInactive_ExpectAndReturn( RCC_PERIPH_GPIOA, RCC_REQUEST_ERROR );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_ERROR, Gpio_Set_PortInactive( GPIO_PORT_A ) );
}

/* ===================== ANALOG SWITCH (STM32L47x / L48x) =================== */

/**
 * \brief   Gpio_Set_PinMode() closes the analog switch of the pin in analog mode.
 *
 * \details PA1 is configured as analog, then as output (STM32L47x / L48x with analog
 *          switch control register ASCR). Other devices: test ignored.
 *
 * \par Expected results
 * - Analog mode: GPIO_REQUEST_OK, MODER analog, ASCR bit of PA1 set (pin connected to ADC).
 * - Output mode: GPIO_REQUEST_OK, MODER output, ASCR bit of PA1 cleared, other ASCR bits kept.
 */
void Ut_Gpio_Set_PinMode_Analog_ClosesAnalogSwitch( void )
{
#if defined(GPIO_ASCR_ASC0)
    GPIOA->ASCR = TEST_GPIO_PIN_MASK( GPIO_PIN_ID_5 );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( GPIO_PORT_A, GPIO_PIN_ID_1, GPIO_PIN_MODE_ANALOG ) );
    TEST_ASSERT_EQUAL_HEX32( LL_GPIO_MODE_ANALOG, TEST_GPIO_FIELD2( GPIOA->MODER, GPIO_PIN_ID_1 ) );
    TEST_ASSERT_EQUAL_HEX32( TEST_GPIO_PIN_MASK( GPIO_PIN_ID_1 ) | TEST_GPIO_PIN_MASK( GPIO_PIN_ID_5 ), GPIOA->ASCR );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( GPIO_PORT_A, GPIO_PIN_ID_1, GPIO_PIN_MODE_OUTPUT ) );
    TEST_ASSERT_EQUAL_HEX32( LL_GPIO_MODE_OUTPUT, TEST_GPIO_FIELD2( GPIOA->MODER, GPIO_PIN_ID_1 ) );
    TEST_ASSERT_EQUAL_HEX32( TEST_GPIO_PIN_MASK( GPIO_PIN_ID_5 ), GPIOA->ASCR );
#else
    TEST_IGNORE_MESSAGE( "Analog switch control register (ASCR) of STM32L47x / L48x only" );
#endif /* GPIO_ASCR_ASC0 */
}


/**
 * \brief   Gpio_Init() of an analog pin closes its analog switch.
 *
 * \details PC2 initialized as analog input (STM32L47x / L48x). Other devices: test ignored.
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, MODER of PC2 analog, ASCR of port C holds bit of PC2 only.
 */
void Ut_Gpio_Init_AnalogPin_ClosesAnalogSwitch( void )
{
#if defined(GPIO_ASCR_ASC0)
    gpio_Config_t config =
    {
        .PortId         = GPIO_PORT_C,
        .PinId          = GPIO_PIN_ID_2,
        .PinMode        = GPIO_PIN_MODE_ANALOG,
        .PinPull        = GPIO_PIN_PULL_NONE,
        .PinSpeed       = GPIO_PIN_SPEED_LOW,
        .PinOutType     = GPIO_PIN_OUTPUT_PUSHPULL,
        .PinAltFunction = GPIO_ALT_FUNC_0,
        .PinActiveLevel = GPIO_PIN_LEVEL_HIGH
    };


    Ut_Gpio_Expect_PortActivation( RCC_PERIPH_GPIOC );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Init( &config ) );
    TEST_ASSERT_EQUAL_HEX32( LL_GPIO_MODE_ANALOG, TEST_GPIO_FIELD2( GPIOC->MODER, GPIO_PIN_ID_2 ) );
    TEST_ASSERT_EQUAL_HEX32( TEST_GPIO_PIN_MASK( GPIO_PIN_ID_2 ), GPIOC->ASCR );
#else
    TEST_IGNORE_MESSAGE( "Analog switch control register (ASCR) of STM32L47x / L48x only" );
#endif /* GPIO_ASCR_ASC0 */
}

/* =========================== LOCAL FUNCTIONS ============================== */

/**
 * \brief Expects activation of port clock which is inactive.
 *
 * \param periphId [in]: RCC identification of the port
 */
static void Ut_Gpio_Expect_PortActivation( rcc_PeriphId_t periphId )
{
    static rcc_FunctionState_t clockState = RCC_FUNCTION_INACTIVE;

    Rcc_Get_PeriphState_ExpectAndReturn( periphId, NULL, RCC_REQUEST_OK );
    Rcc_Get_PeriphState_IgnoreArg_funcState();
    Rcc_Get_PeriphState_ReturnThruPtr_funcState( &clockState );
    Rcc_Set_PeriphActive_ExpectAndReturn( periphId, RCC_REQUEST_OK );
}
