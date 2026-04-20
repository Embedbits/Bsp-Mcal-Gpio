/**
 * \author Mr.Nobody
 * \file Gpio_Types.h
 * \ingroup Gpio
 * \brief General-Purpose Input/Output (GPIO) peripheral handle module global
 *        types definition
 *
 * This file contains the types definitions used across the module and are 
 * available for other modules through Port file.
 *
 */

#ifndef GPIO_GPIO_TYPES_H
#define GPIO_GPIO_TYPES_H
/* ============================== INCLUDES ================================== */
#include "stdint.h"                         /* Module types definition        */
#include "Stm32_gpio.h"                     /* GPIO utilities functionality   */
/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Null pointer definition */
#define GPIO_NULL_PTR                        ( ( void* ) 0u )

/* ========================== EXPORTED MACROS =============================== */

/* ============================== TYPEDEFS ================================== */

/** \brief Type signaling major version of SW module */
typedef uint8_t gpio_MajorVersion_t;


/** \brief Type signaling minor version of SW module */
typedef uint8_t gpio_MinorVersion_t;


/** \brief Type signaling patch version of SW module */
typedef uint8_t gpio_PatchVersion_t;


/** \brief Type signaling actual version of SW module */
typedef struct
{
    gpio_MajorVersion_t Major; /**< Major version */
    gpio_MinorVersion_t Minor; /**< Minor version */
    gpio_PatchVersion_t Patch; /**< Patch version */
}   gpio_ModuleVersion_t;


/** Function status enumeration */
typedef enum
{
    GPIO_FUNCTION_INACTIVE = 0u, /**< Function status is inactive */
    GPIO_FUNCTION_ACTIVE         /**< Function status is active   */
}   gpio_FunctionState_t;


/** Flag states enumeration */
typedef enum
{
    GPIO_FLAG_INACTIVE = 0u, /**< Inactive flag state */
    GPIO_FLAG_ACTIVE         /**< Active flag state   */
}   gpio_FlagState_t;


/** Enumeration used to signal request processing state */
typedef enum
{
    GPIO_REQUEST_ERROR = 0u, /**< Processing request failed  */
    GPIO_REQUEST_OK          /**< Processing request succeed */
}   gpio_RequestState_t;


/** Enumeration type used to signal pin logical level */
typedef enum
{
    GPIO_PIN_LEVEL_LOW = 0u,
    GPIO_PIN_LEVEL_HIGH
}   gpio_PinLevel_t;


/** Enumeration used to signal pull-up, pull-down or no pull for pin */
typedef enum
{
    GPIO_PIN_PULL_NONE = LL_GPIO_PULL_NO,   /**< No pull up/down is applied/ */
    GPIO_PIN_PULL_UP   = LL_GPIO_PULL_UP,   /**< Pull up resistor is used    */
    GPIO_PIN_PULL_DOWN = LL_GPIO_PULL_DOWN, /**< Pull down resistor is used  */
}   gpio_PinPullCfg_t;


typedef enum
{
    GPIO_PIN_MODE_INPUT     = LL_GPIO_MODE_INPUT,     /**< Select input mode              */
    GPIO_PIN_MODE_OUTPUT    = LL_GPIO_MODE_OUTPUT,    /**< Select output mode             */
    GPIO_PIN_MODE_ALTERNATE = LL_GPIO_MODE_ALTERNATE, /**< Select alternate function mode */
    GPIO_PIN_MODE_ANALOG    = LL_GPIO_MODE_ANALOG     /**< Select analog mode             */
}   gpio_PinMode_t;


typedef enum
{
    GPIO_PIN_SPEED_LOW       = LL_GPIO_SPEED_FREQ_LOW,      /**< Select I/O low output speed    */
    GPIO_PIN_SPEED_MEDIUM    = LL_GPIO_SPEED_FREQ_MEDIUM,   /**< Select I/O medium output speed */
    GPIO_PIN_SPEED_HIGH      = LL_GPIO_SPEED_FREQ_HIGH,     /**< Select I/O fast output speed   */
    GPIO_PIN_SPEED_VERY_HIGH = LL_GPIO_SPEED_FREQ_VERY_HIGH /**< Select I/O high output speed   */
}   gpio_PinSpeed_t;


typedef enum
{
    GPIO_PIN_OUTPUT_PUSHPULL  = LL_GPIO_OUTPUT_PUSHPULL, /**< Select push-pull as output type  */
    GPIO_PIN_OUTPUT_OPENDRAIN = LL_GPIO_OUTPUT_OPENDRAIN /**< Select open-drain as output type */
}   gpio_PinOutputType_t;


typedef enum
{
    GPIO_ALT_FUNC_0 = 0u, /**< Select alternate function 0  */
    GPIO_ALT_FUNC_1,      /**< Select alternate function 1  */
    GPIO_ALT_FUNC_2,      /**< Select alternate function 2  */
    GPIO_ALT_FUNC_3,      /**< Select alternate function 3  */
    GPIO_ALT_FUNC_4,      /**< Select alternate function 4  */
    GPIO_ALT_FUNC_5,      /**< Select alternate function 5  */
    GPIO_ALT_FUNC_6,      /**< Select alternate function 6  */
    GPIO_ALT_FUNC_7,      /**< Select alternate function 7  */
    GPIO_ALT_FUNC_8,      /**< Select alternate function 8  */
    GPIO_ALT_FUNC_9,      /**< Select alternate function 9  */
    GPIO_ALT_FUNC_10,     /**< Select alternate function 10 */
    GPIO_ALT_FUNC_11,     /**< Select alternate function 11 */
    GPIO_ALT_FUNC_12,     /**< Select alternate function 12 */
    GPIO_ALT_FUNC_13,     /**< Select alternate function 13 */
    GPIO_ALT_FUNC_14,     /**< Select alternate function 14 */
    GPIO_ALT_FUNC_15,     /**< Select alternate function 15 */
    GPIO_ALT_FUNC_CNT
}   gpio_AltFunction_t;


/** \brief GPIO peripheral identification enumeration.
 *
 * \note If the specific GPIO port is not available on the selected
 *       micro-controller, the corresponding enumeration constant is
 *       mapped to GPIO_PORT_CNT value to avoid errors in other MCAL modules
 *       that could use this enumeration.
 */
typedef enum
{
#if defined(GPIOA)
    GPIO_PORT_A = 0u, /**< GPIO Port A */
#endif
#if defined(GPIOB)
    GPIO_PORT_B,      /**< GPIO Port B */
#endif
#if defined(GPIOC)
    GPIO_PORT_C,      /**< GPIO Port C */
#endif
#if defined(GPIOD)
    GPIO_PORT_D,      /**< GPIO Port D */
#endif
#if defined(GPIOE)
    GPIO_PORT_E,      /**< GPIO Port E */
#endif
#if defined(GPIOF)
    GPIO_PORT_F,      /**< GPIO Port F */
#endif
#if defined(GPIOG)
    GPIO_PORT_G,      /**< GPIO Port G */
#endif
#if defined(GPIOH)
    GPIO_PORT_H,      /**< GPIO Port H */
#endif
#if defined(GPIOI)
    GPIO_PORT_I,      /**< GPIO Port I */
#endif
#if defined(GPIOJ)
    GPIO_PORT_J,      /**< GPIO Port J */
#endif
    GPIO_PORT_CNT
}   gpio_PortId_t;


/** GPIO Pins identification enumeration */
typedef enum
{
    GPIO_PIN_ID_0 = 0u, /**< GPIO Pin 0  */
    GPIO_PIN_ID_1,      /**< GPIO Pin 1  */
    GPIO_PIN_ID_2,      /**< GPIO Pin 2  */
    GPIO_PIN_ID_3,      /**< GPIO Pin 3  */
    GPIO_PIN_ID_4,      /**< GPIO Pin 4  */
    GPIO_PIN_ID_5,      /**< GPIO Pin 5  */
    GPIO_PIN_ID_6,      /**< GPIO Pin 6  */
    GPIO_PIN_ID_7,      /**< GPIO Pin 7  */
    GPIO_PIN_ID_8,      /**< GPIO Pin 8  */
    GPIO_PIN_ID_9,      /**< GPIO Pin 9  */
    GPIO_PIN_ID_10,     /**< GPIO Pin 10 */
    GPIO_PIN_ID_11,     /**< GPIO Pin 11 */
    GPIO_PIN_ID_12,     /**< GPIO Pin 12 */
    GPIO_PIN_ID_13,     /**< GPIO Pin 13 */
    GPIO_PIN_ID_14,     /**< GPIO Pin 14 */
    GPIO_PIN_ID_15,     /**< GPIO Pin 15 */
    GPIO_PIN_ID_CNT,
}   gpio_PinId_t;


typedef struct
{
    gpio_PortId_t        PortId;         /**< GPIO port identification       */
    gpio_PinId_t         PinId;          /**< GPIO pin identification        */
    gpio_PinMode_t       PinMode;        /**< GPIO pin type                  */
    gpio_PinPullCfg_t    PinPull;        /**< GPIO pin pull configuration    */
    gpio_PinSpeed_t      PinSpeed;       /**< GPIO output speed              */
    gpio_PinOutputType_t PinOutType;     /**< GPIO output style              */
    gpio_AltFunction_t   PinAltFunction; /**< Alternate function used by pin */
    gpio_PinLevel_t      PinActiveLevel; /**< Pin level in active state      */
}   gpio_Config_t;

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */


#endif /* GPIO_GPIO_TYPES_H */
