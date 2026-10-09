/**
 * \author Mr.Nobody
 * \file ItTest_Gpio.c
 * \ingroup Gpio
 * \brief Integration tests of General-Purpose Input/Output (GPIO) module on target.
 *
 * Gpio module runs on the MCU together with real RCC module and hardware. Tests
 * verify behavior which cannot be verified by unit tests (emulated registers):
 * clock activation, output data -> pin -> input data path, pull resistors,
 * open-drain output, analog mode input and analog switch (ASCR) of
 * STM32L47x / L48x.
 *
 * Used pins (see board configuration below):
 * - IT_GPIO_LED_*  - push-pull output with load - user LED of the board (LED
 *                    blinks) or a pin with a fixed input load on the boards
 *                    without a common LED pin
 * - IT_GPIO_FREE_* - pin not connected on the board (no LED, pull resistor or
 *                    solder bridge) and on connectors. Pull resistor tests
 *                    require floating pin.
 *
 * \note Boards are named by the MCU detected by integration testing (DEV_ID and
 *       flash size - the first MCU of the same identification in the Ral presets,
 *       other MCUs of the identification are listed too). Boards sharing the
 *       name use pins valid on every board of the group (STM32CubeMX board
 *       files): Nucleo-64-P boards control the SMPS by PA4 - PA7, Nucleo-32
 *       boards have no port C pins except the LSE pins.
 */

/* ============================= INCLUDES =================================== */
#include "unity.h"                          /* Unity testing framework        */
#include "IntegrationTesting.h"             /* Integration testing on target  */
#include "Gpio_Port.h"                      /* Module under test              */
/* ============================= TYPEDEFS =================================== */

/* ======================= FORWARD DECLARATIONS ============================= */

static void It_Gpio_WaitPinSettled  ( void );
static void It_Gpio_Set_PinDefault  ( gpio_PortId_t portId, gpio_PinId_t pinId );
static void It_Gpio_Check_PinLevel  ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t expectedLevel );

/* ========================= SYMBOLIC CONSTANTS ============================= */

/*----------------------------- Board configuration --------------------------*/
#if defined(IT_BOARD_STM32L412xB) || \
    defined(IT_BOARD_STM32L422xB) || \
    defined(IT_BOARD_STM32L431xC) || \
    defined(IT_BOARD_STM32L432xC) || \
    defined(IT_BOARD_STM32L433xC) || \
    defined(IT_BOARD_STM32L442xC) || \
    defined(IT_BOARD_STM32L443xC)

    /** NUCLEO-L412KB / NUCLEO-L432KC (Nucleo-32 MB1180): user LED LD3 (PB3, Arduino D13),
     *  NUCLEO-L412RB-P / NUCLEO-L433RC-P (Nucleo-64-P MB1319): PB3 = SWO input of ST-LINK */
    #define IT_GPIO_LED_PORT                ( GPIO_PORT_B )
    #define IT_GPIO_LED_PIN                 ( GPIO_PIN_ID_3 )

    /** PB5 (Nucleo-32 Arduino D11, not used on Nucleo-64-P) - not connected on the boards */
    #define IT_GPIO_FREE_PORT               ( GPIO_PORT_B )
    #define IT_GPIO_FREE_PIN                ( GPIO_PIN_ID_5 )
    #define IT_GPIO_FREE_REG                ( GPIOB )

#elif defined(IT_BOARD_STM32L451xE) || \
      defined(IT_BOARD_STM32L452xE) || \
      defined(IT_BOARD_STM32L462xE)

    /** NUCLEO-L452RE-P (Nucleo-64-P MB1319): user LED LD4 (PB13),
     *  NUCLEO-L452RE (Nucleo-64 MB1136): PB13 morpho CN10 pin 30 - not connected
     *  (LED LD2 PA5 of Nucleo-64 is SMPS_V1 on Nucleo-64-P) */
    #define IT_GPIO_LED_PORT                ( GPIO_PORT_B )
    #define IT_GPIO_LED_PIN                 ( GPIO_PIN_ID_13 )

    /** Morpho CN10 pin 2 (PC8) - not connected on the boards */
    #define IT_GPIO_FREE_PORT               ( GPIO_PORT_C )
    #define IT_GPIO_FREE_PIN                ( GPIO_PIN_ID_8 )
    #define IT_GPIO_FREE_REG                ( GPIOC )

#elif defined(IT_BOARD_STM32L471xG) || \
      defined(IT_BOARD_STM32L475xG) || \
      defined(IT_BOARD_STM32L476xG) || \
      defined(IT_BOARD_STM32L486xG)

    /** NUCLEO-L476RG (Nucleo-64 MB1136): user LED LD2 (PA5, Arduino D13) */
    #define IT_GPIO_LED_PORT                ( GPIO_PORT_A )
    #define IT_GPIO_LED_PIN                 ( GPIO_PIN_ID_5 )

    /** Morpho CN10 pin 2 (PC8) - not connected on the board */
    #define IT_GPIO_FREE_PORT               ( GPIO_PORT_C )
    #define IT_GPIO_FREE_PIN                ( GPIO_PIN_ID_8 )
    #define IT_GPIO_FREE_REG                ( GPIOC )

#elif defined(IT_BOARD_STM32L496xG) || \
      defined(IT_BOARD_STM32L4A6xG) || \
      defined(IT_BOARD_STM32L4P5xG) || \
      defined(IT_BOARD_STM32L4Q5xG) || \
      defined(IT_BOARD_STM32L4R5xI) || \
      defined(IT_BOARD_STM32L4R7xI) || \
      defined(IT_BOARD_STM32L4R9xI) || \
      defined(IT_BOARD_STM32L4S5xI) || \
      defined(IT_BOARD_STM32L4S7xI) || \
      defined(IT_BOARD_STM32L4S9xI)

    /** NUCLEO-L496ZG / L496ZG-P / L4A6ZG / L4P5ZG / L4R5ZI / L4R5ZI-P (Nucleo-144 MB1312):
     *  user LED LD2 (PB7, blue) */
    #define IT_GPIO_LED_PORT                ( GPIO_PORT_B )
    #define IT_GPIO_LED_PIN                 ( GPIO_PIN_ID_7 )

    /** CN8 pin 2 (PC8) - not connected on the boards */
    #define IT_GPIO_FREE_PORT               ( GPIO_PORT_C )
    #define IT_GPIO_FREE_PIN                ( GPIO_PIN_ID_8 )
    #define IT_GPIO_FREE_REG                ( GPIOC )

#else
    #error "Board of Gpio integration tests is not defined (INTEGRATION_TEST_BOARD)."
#endif

/** Count of wait loop iterations until pin level is settled (pull resistor charges pin capacity) */
#define IT_GPIO_SETTLE_LOOPS                ( 2000u )

/* ============================== MACROS ==================================== */

/* ========================== LOCAL VARIABLES =============================== */

/* ============================= TEST SETUP ================================= */

void setUp( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PortActive( IT_GPIO_LED_PORT ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PortActive( IT_GPIO_FREE_PORT ) );
}


void tearDown( void )
{
    It_Gpio_Set_PinDefault( IT_GPIO_LED_PORT,  IT_GPIO_LED_PIN  );
    It_Gpio_Set_PinDefault( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN );
}

/* =============================== TESTS ==================================== */

/*------------------------------ Port activation -----------------------------*/

/**
 * \brief   Port activated in setUp() is reported as active.
 *
 * \details Reads state of the free pin port after Gpio_Set_PortActive() in setUp().
 *
 * \par Expected results
 * - GPIO_REQUEST_OK, port state is GPIO_FUNCTION_ACTIVE.
 */
void It_Gpio_Set_PortActive_ClockEnabled_PortStateActive( void )
{
    gpio_FunctionState_t portState = GPIO_FUNCTION_INACTIVE;

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PortState( IT_GPIO_FREE_PORT, &portState ) );
    TEST_ASSERT_EQUAL( GPIO_FUNCTION_ACTIVE, portState );
}


/**
 * \brief   Activation of already active port is accepted.
 *
 * \details Calls Gpio_Set_PortActive() again for the port activated in setUp().
 *
 * \par Expected results
 * - GPIO_REQUEST_OK is returned.
 */
void It_Gpio_Set_PortActive_AlreadyActive_ReturnsOk( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PortActive( IT_GPIO_FREE_PORT ) );
}

/*----------------------------- Initialization -------------------------------*/

/**
 * \brief   Gpio_Init() on target configures all pin parameters.
 *
 * \details Initializes the free pin as open-drain output, pull-down, high speed,
 *          alternate function 7, active high, then reads all parameters back and
 *          checks the pin level.
 *
 * \par Expected results
 * - Mode output, pull-down, high speed, open-drain, AF7 are read back.
 * - Pin level is LOW (inactive state - open drain released, pulled down).
 */
void It_Gpio_Init_OutputConfiguration_AllParametersReadBack( void )
{
    gpio_Config_t        gpioConfig;
    gpio_PinMode_t       pinMode    = GPIO_PIN_MODE_ANALOG;
    gpio_PinPullCfg_t    pinPull    = GPIO_PIN_PULL_NONE;
    gpio_PinSpeed_t      pinSpeed   = GPIO_PIN_SPEED_LOW;
    gpio_PinOutputType_t pinOutType = GPIO_PIN_OUTPUT_PUSHPULL;
    gpio_AltFunction_t   altFunc    = GPIO_ALT_FUNC_0;

    gpioConfig.PortId         = IT_GPIO_FREE_PORT;
    gpioConfig.PinId          = IT_GPIO_FREE_PIN;
    gpioConfig.PinMode        = GPIO_PIN_MODE_OUTPUT;
    gpioConfig.PinPull        = GPIO_PIN_PULL_DOWN;
    gpioConfig.PinSpeed       = GPIO_PIN_SPEED_HIGH;
    gpioConfig.PinOutType     = GPIO_PIN_OUTPUT_OPENDRAIN;
    gpioConfig.PinAltFunction = GPIO_ALT_FUNC_7;
    gpioConfig.PinActiveLevel = GPIO_PIN_LEVEL_HIGH;

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Init( &gpioConfig ) );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, &pinMode ) );
    TEST_ASSERT_EQUAL( GPIO_PIN_MODE_OUTPUT, pinMode );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinPull( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, &pinPull ) );
    TEST_ASSERT_EQUAL( GPIO_PIN_PULL_DOWN, pinPull );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinSpeed( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, &pinSpeed ) );
    TEST_ASSERT_EQUAL( GPIO_PIN_SPEED_HIGH, pinSpeed );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinOutType( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, &pinOutType ) );
    TEST_ASSERT_EQUAL( GPIO_PIN_OUTPUT_OPENDRAIN, pinOutType );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinAltFunction( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, &altFunc ) );
    TEST_ASSERT_EQUAL( GPIO_ALT_FUNC_7, altFunc );

    /* Pin is initialized in inactive state - open drain released, pulled down */
    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW );
}

/*--------------------------- Push-pull output -------------------------------*/

/**
 * \brief   Push-pull output drives high level on the pin.
 *
 * \details Sets output level high, switches the free pin to output mode and reads
 *          the input level after settling time.
 *
 * \par Expected results
 * - Input data register reads HIGH.
 */
void It_Gpio_Set_PinLevel_PushPullHigh_PinReadsHigh( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_HIGH ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_OUTPUT ) );

    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_HIGH );
}


/**
 * \brief   Push-pull output drives low level on the pin.
 *
 * \details Sets output level low, switches the free pin to output mode and reads
 *          the input level after settling time.
 *
 * \par Expected results
 * - Input data register reads LOW.
 */
void It_Gpio_Set_PinLevel_PushPullLow_PinReadsLow( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_OUTPUT ) );

    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW );
}


/**
 * \brief   Output with load (board LED) follows the set level.
 *
 * \details Switches the LED pin to output, sets level high, then low (LED blinks).
 *
 * \par Expected results
 * - Pin reads HIGH after high level and LOW after low level.
 */
void It_Gpio_Set_PinLevel_LedHighAndLow_PinFollowsLevel( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_LED_PORT, IT_GPIO_LED_PIN, GPIO_PIN_MODE_OUTPUT ) );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinLevel( IT_GPIO_LED_PORT, IT_GPIO_LED_PIN, GPIO_PIN_LEVEL_HIGH ) );
    It_Gpio_Check_PinLevel( IT_GPIO_LED_PORT, IT_GPIO_LED_PIN, GPIO_PIN_LEVEL_HIGH );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinLevel( IT_GPIO_LED_PORT, IT_GPIO_LED_PIN, GPIO_PIN_LEVEL_LOW ) );
    It_Gpio_Check_PinLevel( IT_GPIO_LED_PORT, IT_GPIO_LED_PIN, GPIO_PIN_LEVEL_LOW );
}


/**
 * \brief   Gpio_Toggle_PinLevel() inverts the pin level.
 *
 * \details Free pin is push-pull output with low level, it is toggled twice.
 *
 * \par Expected results
 * - Pin reads HIGH after 1st toggle and LOW after 2nd toggle.
 */
void It_Gpio_Toggle_PinLevel_PushPull_PinLevelInverted( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_OUTPUT ) );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Toggle_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN ) );
    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_HIGH );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Toggle_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN ) );
    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW );
}


/**
 * \brief   Active / inactive state of active-low pin on target.
 *
 * \details Free pin is output, it is set inactive, then active with active level LOW.
 *
 * \par Expected results
 * - Inactive state: pin reads HIGH.
 * - Active state: pin reads LOW.
 */
void It_Gpio_Set_PinStateActive_ActiveLow_PinReadsLow( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_OUTPUT ) );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinStateInactive( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW ) );
    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_HIGH );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinStateActive( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW ) );
    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW );
}

/*--------------------------- Pull resistors ---------------------------------*/

/**
 * \brief   Internal pull-up pulls floating input high.
 *
 * \details Free (floating) pin is input with pull-up.
 *
 * \par Expected results
 * - Pin reads HIGH after settling time.
 */
void It_Gpio_Set_PinPull_InputPullUp_PinReadsHigh( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_INPUT ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinPull( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_PULL_UP ) );

    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_HIGH );
}


/**
 * \brief   Internal pull-down pulls floating input low.
 *
 * \details Free (floating) pin is input with pull-down.
 *
 * \par Expected results
 * - Pin reads LOW after settling time.
 */
void It_Gpio_Set_PinPull_InputPullDown_PinReadsLow( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_INPUT ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinPull( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_PULL_DOWN ) );

    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW );
}


/**
 * \brief   Change of pull resistor changes level of floating input.
 *
 * \details Free pin is input with pull-up, then pull-down.
 *
 * \par Expected results
 * - Pin reads HIGH with pull-up and LOW with pull-down.
 */
void It_Gpio_Set_PinPull_PullUpToPullDown_PinLevelFollows( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_INPUT ) );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinPull( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_PULL_UP ) );
    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_HIGH );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinPull( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_PULL_DOWN ) );
    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW );
}

/*--------------------------- Open-drain output ------------------------------*/

/**
 * \brief   Open-drain output with high level and pull-up reads high.
 *
 * \details Free pin is open-drain output with pull-up and high output level.
 *
 * \par Expected results
 * - Pin reads HIGH (released, pulled up).
 */
void It_Gpio_Set_PinOutType_OpenDrainHighPullUp_PinReadsHigh( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinOutType( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_OUTPUT_OPENDRAIN ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinPull( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_PULL_UP ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_HIGH ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_OUTPUT ) );

    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_HIGH );
}


/**
 * \brief   Open-drain output does not drive high level.
 *
 * \details Free pin is open-drain output with pull-down and high output level.
 *
 * \par Expected results
 * - Pin reads LOW (open drain released, pin pulled down).
 */
void It_Gpio_Set_PinOutType_OpenDrainHighPullDown_PinReleasedReadsLow( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinOutType( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_OUTPUT_OPENDRAIN ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinPull( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_PULL_DOWN ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_HIGH ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_OUTPUT ) );

    /* Open drain does not drive high level - pin is pulled down */
    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW );
}


/**
 * \brief   Open-drain output drives low level against pull-up.
 *
 * \details Free pin is open-drain output with pull-up and low output level.
 *
 * \par Expected results
 * - Pin reads LOW.
 */
void It_Gpio_Set_PinOutType_OpenDrainLowPullUp_PinDrivenLow( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinOutType( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_OUTPUT_OPENDRAIN ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinPull( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_PULL_UP ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_OUTPUT ) );

    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW );
}

/*----------------------- Analog and alternate mode --------------------------*/

/**
 * \brief   Analog mode disables digital input.
 *
 * \details Free pin is input with pull-up (reads high), then switched to analog mode.
 *
 * \par Expected results
 * - Input mode: pin reads HIGH.
 * - Analog mode: input data register reads LOW (Schmitt trigger disabled).
 */
void It_Gpio_Set_PinMode_AnalogWithPullUp_InputReadsLow( void )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_INPUT ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinPull( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_PULL_UP ) );
    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_HIGH );

    /* Schmitt trigger input is disabled in analog mode, input data register reads 0 */
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_ANALOG ) );
    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_LOW );
}


/**
 * \brief   Analog switch of the pin follows the analog mode (STM32L47x / L48x).
 *
 * \details Free pin is switched to analog mode, then to input mode and back to
 *          analog mode. Analog switch control register (ASCR) of the port is
 *          read directly. MCUs without ASCR: test ignored.
 *
 * \par Expected results
 * - Analog mode: ASCR bit of the pin set (pin connected to ADC input).
 * - Input mode: ASCR bit of the pin cleared, input reads pull-up level HIGH.
 * - Analog mode again: ASCR bit set.
 */
void It_Gpio_Set_PinMode_AnalogSwitch_FollowsAnalogMode( void )
{
#if defined(GPIO_ASCR_ASC0)
    const uint32_t pinMask = 1u << (uint32_t)IT_GPIO_FREE_PIN;

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_ANALOG ) );
    TEST_ASSERT_EQUAL_HEX32( pinMask, IT_GPIO_FREE_REG->ASCR & pinMask );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_INPUT ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinPull( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_PULL_UP ) );
    TEST_ASSERT_EQUAL_HEX32( 0u, IT_GPIO_FREE_REG->ASCR & pinMask );
    It_Gpio_Check_PinLevel( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_LEVEL_HIGH );

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_ANALOG ) );
    TEST_ASSERT_EQUAL_HEX32( pinMask, IT_GPIO_FREE_REG->ASCR & pinMask );
#else
    TEST_IGNORE_MESSAGE( "MCU without analog switch control (ASCR)" );
#endif /* GPIO_ASCR_ASC0 */
}


/**
 * \brief   All alternate functions are written and read back on target.
 *
 * \details Free pin stays in input mode (function not connected), alternate
 *          functions 0 - 15 are set one by one.
 *
 * \par Expected results
 * - Every Set / Get returns GPIO_REQUEST_OK and read function equals the set one.
 */
void It_Gpio_Set_PinAltFunction_AllFunctions_ReadBack( void )
{
    gpio_AltFunction_t altFunc = GPIO_ALT_FUNC_0;

    /* Pin stays in input mode - alternate function is only selected, not connected */
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, GPIO_PIN_MODE_INPUT ) );

    for( uint32_t funcIdx = (uint32_t)GPIO_ALT_FUNC_0; (uint32_t)GPIO_ALT_FUNC_15 >= funcIdx; funcIdx++ )
    {
        TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinAltFunction( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, (gpio_AltFunction_t)funcIdx ) );
        TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinAltFunction( IT_GPIO_FREE_PORT, IT_GPIO_FREE_PIN, &altFunc ) );
        TEST_ASSERT_EQUAL( funcIdx, altFunc );
    }
}

/* ========================== LOCAL FUNCTIONS =============================== */

/**
 * \brief Waits until pin level is settled after configuration change.
 *
 * \note Input data register is sampled by AHB clock (2 cycles delay), pull
 *       resistor charges pin capacity - both are far below the wait time.
 */
static void It_Gpio_WaitPinSettled( void )
{
    for( volatile uint32_t loopIdx = 0u; IT_GPIO_SETTLE_LOOPS > loopIdx; loopIdx++ )
    {
        /* Busy wait */
    }
}


/**
 * \brief Returns pin to reset configuration (analog, push-pull, no pull, low).
 *
 * \param portId [in]: GPIO port identification
 * \param pinId  [in]: GPIO pin identification
 */
static void It_Gpio_Set_PinDefault( gpio_PortId_t portId, gpio_PinId_t pinId )
{
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinMode    ( portId, pinId, GPIO_PIN_MODE_ANALOG     ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinPull    ( portId, pinId, GPIO_PIN_PULL_NONE       ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinOutType ( portId, pinId, GPIO_PIN_OUTPUT_PUSHPULL ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinSpeed   ( portId, pinId, GPIO_PIN_SPEED_LOW       ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinAltFunction( portId, pinId, GPIO_ALT_FUNC_0       ) );
    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Set_PinLevel   ( portId, pinId, GPIO_PIN_LEVEL_LOW       ) );
}


/**
 * \brief Checks actual pin level (input data register) after settling time.
 *
 * \param portId        [in]: GPIO port identification
 * \param pinId         [in]: GPIO pin identification
 * \param expectedLevel [in]: Expected pin level
 */
static void It_Gpio_Check_PinLevel( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t expectedLevel )
{
    gpio_PinLevel_t pinLevel = GPIO_PIN_LEVEL_LOW;

    It_Gpio_WaitPinSettled();

    TEST_ASSERT_EQUAL( GPIO_REQUEST_OK, Gpio_Get_PinLevel( portId, pinId, &pinLevel ) );
    TEST_ASSERT_EQUAL( expectedLevel, pinLevel );
}
