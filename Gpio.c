/**
 * \author Mr.Nobody
 * \file Gpio.c
 * \ingroup Gpio
 * \brief General-Purpose Input/Output (GPIO) peripheral handler module common
 *        functionality.
 *
 */
/* ============================== INCLUDES ================================== */
#include "Gpio.h"                           /* Self include                   */
#include "Gpio_Port.h"                      /* Own port file include          */
#include "Gpio_Types.h"                     /* Module types definitions       */
#include "Rcc_Port.h"                       /* RCC handler functionality      */
#include "Stm32_gpio.h"                     /* GPIO utilities functionality   */
/* ============================== TYPEDEFS ================================== */

/** \brief GPIO port configuration structure type */
typedef struct
{
    GPIO_TypeDef  *GpioReg;  /**< GPIO configuration register */
    rcc_PeriphId_t GpioRcc;  /**< GPIO RCC configuration ID   */
}   gpio_PortConfig_t;

/** \brief GPIO pin configuration structure type */
typedef struct
{
    uint32_t PinRegId; /**< GPIO identification of position in register */
}   gpio_PinConfig_t;

/** \brief GPIO pin alternate function configuration structure type */
typedef struct
{
    uint32_t AltFuncReg;
}   gpio_AltFuncConfig_t;

/* ======================== FORWARD DECLARATIONS ============================ */

static gpio_RequestState_t Gpio_Check_PinModeErrata( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinMode_t pinType );
static void                Gpio_Set_UnbondedPadErrata( gpio_PortId_t portId );

/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Value of major version of SW module */
#define GPIO_MAJOR_VERSION           ( 1u )

/** Value of minor version of SW module */
#define GPIO_MINOR_VERSION           ( 0u )

/** Value of patch version of SW module */
#define GPIO_PATCH_VERSION           ( 0u )


/** Maximum delay for execution/checks in raw value. */
#define GPIO_TIMEOUT_RAW            ( 0x84FCB )

#if defined(STM32F401xC) || defined(STM32F401xE) || defined(STM32F411xE)
/** Device errata "PH1 cannot be used as a GPIO in HSE bypass mode" (ES0222, ES0287, ES0299) */
#define GPIO_ERRATA_PH1_HSE_BYPASS

/** RCC CR bits of HSE used as external clock (bypass mode) */
#define GPIO_RCC_CR_HSE_BYPASS      ( RCC_CR_HSEON | RCC_CR_HSEBYP )
#endif /* STM32F401xC || STM32F401xE || STM32F411xE */

#if defined(STM32F401xC)
/** Device errata "Extra power consumption may be observed on the UQFN48, LQFP64 and LQPF100
 *  packages" (ES0222, STM32F401xB / xC): non-bonded PB11 pad is floating */
#define GPIO_ERRATA_PB11_NOT_BONDED
#endif /* STM32F401xC */

/* =============================== MACROS =================================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* =========================== LOCAL VARIABLES ============================== */

static const gpio_PortConfig_t      gpio_PeriphConf[ GPIO_PORT_CNT ] =
{
#if defined(GPIOA)
    { .GpioReg = GPIOA, .GpioRcc = RCC_PERIPH_GPIOA },
#endif
#if defined(GPIOB)
    { .GpioReg = GPIOB, .GpioRcc = RCC_PERIPH_GPIOB },
#endif
#if defined(GPIOC)
    { .GpioReg = GPIOC, .GpioRcc = RCC_PERIPH_GPIOC },
#endif
#if defined(GPIOD)
    { .GpioReg = GPIOD, .GpioRcc = RCC_PERIPH_GPIOD },
#endif
#if defined(GPIOE)
    { .GpioReg = GPIOE, .GpioRcc = RCC_PERIPH_GPIOE },
#endif
#if defined(GPIOF)
    { .GpioReg = GPIOF, .GpioRcc = RCC_PERIPH_GPIOF },
#endif
#if defined(GPIOG)
    { .GpioReg = GPIOG, .GpioRcc = RCC_PERIPH_GPIOG },
#endif
#if defined(GPIOH)
    { .GpioReg = GPIOH, .GpioRcc = RCC_PERIPH_GPIOH },
#endif
#if defined(GPIOI)
    { .GpioReg = GPIOI, .GpioRcc = RCC_PERIPH_GPIOI },
#endif
#if defined(GPIOJ)
    { .GpioReg = GPIOJ, .GpioRcc = RCC_PERIPH_GPIOJ },
#endif
#if defined(GPIOK)
    { .GpioReg = GPIOK, .GpioRcc = RCC_PERIPH_GPIOK },
#endif

};


static const gpio_PinConfig_t       gpio_PinConf[ GPIO_PIN_ID_CNT ] =
{
    { .PinRegId = LL_GPIO_PIN_0  },
    { .PinRegId = LL_GPIO_PIN_1  },
    { .PinRegId = LL_GPIO_PIN_2  },
    { .PinRegId = LL_GPIO_PIN_3  },
    { .PinRegId = LL_GPIO_PIN_4  },
    { .PinRegId = LL_GPIO_PIN_5  },
    { .PinRegId = LL_GPIO_PIN_6  },
    { .PinRegId = LL_GPIO_PIN_7  },
    { .PinRegId = LL_GPIO_PIN_8  },
    { .PinRegId = LL_GPIO_PIN_9  },
    { .PinRegId = LL_GPIO_PIN_10 },
    { .PinRegId = LL_GPIO_PIN_11 },
    { .PinRegId = LL_GPIO_PIN_12 },
    { .PinRegId = LL_GPIO_PIN_13 },
    { .PinRegId = LL_GPIO_PIN_14 },
    { .PinRegId = LL_GPIO_PIN_15 }
};


static const gpio_AltFuncConfig_t   gpio_AltFuncConfig[ GPIO_ALT_FUNC_CNT ] =
{
    { .AltFuncReg = LL_GPIO_AF_0  },
    { .AltFuncReg = LL_GPIO_AF_1  },
    { .AltFuncReg = LL_GPIO_AF_2  },
    { .AltFuncReg = LL_GPIO_AF_3  },
    { .AltFuncReg = LL_GPIO_AF_4  },
    { .AltFuncReg = LL_GPIO_AF_5  },
    { .AltFuncReg = LL_GPIO_AF_6  },
    { .AltFuncReg = LL_GPIO_AF_7  },
    { .AltFuncReg = LL_GPIO_AF_8  },
    { .AltFuncReg = LL_GPIO_AF_9  },
    { .AltFuncReg = LL_GPIO_AF_10 },
    { .AltFuncReg = LL_GPIO_AF_11 },
    { .AltFuncReg = LL_GPIO_AF_12 },
    { .AltFuncReg = LL_GPIO_AF_13 },
    { .AltFuncReg = LL_GPIO_AF_14 },
    { .AltFuncReg = LL_GPIO_AF_15 }
};

/* ========================= EXPORTED FUNCTIONS ============================= */

/**
 * \brief Returns module SW version
 *
 * \return Module SW version
 */
gpio_ModuleVersion_t Gpio_Get_ModuleVersion( void )
{
    gpio_ModuleVersion_t retVersion;

    retVersion.Major = GPIO_MAJOR_VERSION;
    retVersion.Minor = GPIO_MINOR_VERSION;
    retVersion.Patch = GPIO_PATCH_VERSION;

    return (retVersion);
}


/**
 * \brief Initializes module Gpio
 *
 * Activates the port clock and configures the pin. Output level (inactive state),
 * output type, speed, pull and alternate function are configured before the pin
 * mode, so an output pin starts directly with its inactive level (no glitch).
 * Configuration stops at the first failed step.
 *
 * \note STM32F401 / F411 device errata "PH1 cannot be used as a GPIO in HSE bypass
 *       mode": PH1 is refused in input, output and alternate function mode while HSE
 *       is used as external clock (HSEON and HSEBYP set), nothing is configured.
 *
 * \param gpioConfig [in]: GPIO pin configuration. Must not be NULL.
 *
 * \return State of request execution. Returns \ref GPIO_REQUEST_OK if request was
 *         success, otherwise returns \ref GPIO_REQUEST_ERROR.
 */
gpio_RequestState_t Gpio_Init( gpio_Config_t *gpioConfig )
{
    gpio_RequestState_t retState = GPIO_REQUEST_ERROR;

    if( GPIO_NULL_PTR != gpioConfig )
    {
        retState = Gpio_Check_PinModeErrata( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinMode );

        if( GPIO_REQUEST_OK == retState )
        {
            retState = Gpio_Set_PortActive( gpioConfig->PortId );
        }
        else
        {
            /* Pin can not be used in the required mode */
        }

        if( GPIO_REQUEST_OK == retState )
        {
            retState = Gpio_Set_PinStateInactive( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinActiveLevel );
        }
        else
        {
            /* Port activation failed */
        }

        if( GPIO_REQUEST_OK == retState )
        {
            retState = Gpio_Set_PinOutType( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinOutType );
        }
        else
        {
            /* Error during initialization process */
        }

        if( GPIO_REQUEST_OK == retState )
        {
            retState = Gpio_Set_PinSpeed( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinSpeed );
        }
        else
        {
            /* Error during initialization process */
        }

        if( GPIO_REQUEST_OK == retState )
        {
            retState = Gpio_Set_PinPull( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinPull );
        }
        else
        {
            /* Error during initialization process */
        }

        if( GPIO_REQUEST_OK == retState )
        {
            retState = Gpio_Set_PinAltFunction( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinAltFunction );
        }
        else
        {
            /* Error during initialization process */
        }

        /* Pin mode is configured as the last step */
        if( GPIO_REQUEST_OK == retState )
        {
            retState = Gpio_Set_PinMode( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinMode );
        }
        else
        {
            /* Error during initialization process */
        }
    }
    else
    {
        /* Null pointer assigned */
        retState = GPIO_REQUEST_ERROR;
    }

    return ( retState );
}


/**
 * \brief Deinitializes module Gpio
 *
 * This function shall call every necessary sub-module deinitialization function 
 * and free all the resources allocated by the module. In case of failure, the 
 * function shall handle it by itself and shall not be transferred to AppMain 
 * layer.
 */
void Gpio_Deinit( void )
{
    return;
}


/**
 * \brief Main task of module Gpio
 *
 * This function shall be called in the main loop of the application or the task
 * scheduler. It shall be called periodically, depending on the module's 
 * requirements.
 */
void Gpio_Task( void )
{
    return;
}


/**
 * \brief Activates the required port dependencies
 *
 * When port activation is requested, the needed dependencies must be activated
 * (eg. RCC). If needed dependencies are already active, new activation request
 * is not triggered to avoid restart or unspecified behavior.
 *
 * \note STM32F401xB / xC device errata "Extra power consumption may be observed on the
 *       UQFN48, LQFP64 and LQPF100 packages": the non-bonded PB11 pad is floating. When
 *       the port B clock is activated, PB11 still in reset (input) mode is configured as
 *       analog (harmless on packages with bonded PB11 - the application configures it
 *       afterwards).
 *
 * \param portId [in]: GPIO port identification
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpio_RequestState_t Gpio_Set_PortActive( gpio_PortId_t portId )
{
    gpio_RequestState_t retValue  = GPIO_REQUEST_ERROR;
    rcc_FunctionState_t funcState = RCC_FUNCTION_INACTIVE;

    if( GPIO_PORT_CNT > portId )
    {
        /* Activation of RCC */
        rcc_RequestState_t rccState = Rcc_Get_PeriphState( gpio_PeriphConf[ portId ].GpioRcc, &funcState );

        if( RCC_REQUEST_ERROR != rccState )
        {
            if( RCC_FUNCTION_INACTIVE == funcState )
            {
                /* Activation of RCC */
                rccState = Rcc_Set_PeriphActive( gpio_PeriphConf[ portId ].GpioRcc );

                if( RCC_REQUEST_ERROR != rccState )
                {
                    /* Clock has been successfully activated */
                    Gpio_Set_UnbondedPadErrata( portId );

                    retValue = GPIO_REQUEST_OK;
                }
                else
                {
                    /* Error occurred during clock activation */
                    retValue = GPIO_REQUEST_ERROR;
                }
            }
            else
            {
                /* Clock is already active. No need to request another activation */
                retValue = GPIO_REQUEST_OK;
            }
        }
        else
        {
            /* Clock activation state returned error state */
            retValue = GPIO_REQUEST_ERROR;
        }
    }
    else
    {
        /* Port ID is incorrect. Thus we had to avoid access to configuration array out of range */
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief De-activates the required port dependencies
 *
 * \param portId [in]: GPIO port identification
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpio_RequestState_t Gpio_Set_PortInactive( gpio_PortId_t portId )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( GPIO_PORT_CNT > portId )
    {
        /* Activation of RCC */
        rcc_RequestState_t rccState = Rcc_Set_PeriphInactive( gpio_PeriphConf[ portId ].GpioRcc );

        if( RCC_REQUEST_ERROR != rccState )
        {
            retValue = GPIO_REQUEST_OK;
        }
        else
        {
            retValue = GPIO_REQUEST_ERROR;
        }
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Returns the required port necessary dependencies activation state
 *
 * \param portId     [in]: GPIO port identification
 * \param portState [out]: GPIO port dependencies activation state
 * \return Processing request state. If request executed successfully returns "OK",
 *         otherwise returns error.
 */
gpio_RequestState_t Gpio_Get_PortState( gpio_PortId_t portId, gpio_FunctionState_t * const portState )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( GPIO_PORT_CNT > portId )
    {
        /* Read of RCC clock state */
        rcc_RequestState_t rccState = Rcc_Get_PeriphState( gpio_PeriphConf[ portId ].GpioRcc, (rcc_FunctionState_t*)portState );

        if( RCC_REQUEST_ERROR != rccState )
        {
            retValue = GPIO_REQUEST_OK;
        }
        else
        {
            retValue = GPIO_REQUEST_ERROR;
        }
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Configures pin type [analog/input/output/alternate]
 *
 * \param portId  [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId   [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinType [in]: Required pin type configuration [analog/input/output/alternate]
 *
 * \note STM32F401 / F411 device errata "PH1 cannot be used as a GPIO in HSE bypass
 *       mode": only analog mode is accepted for PH1 while HSE is used as external clock.
 *
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Set_PinMode(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinMode_t pinType )
{
    gpio_RequestState_t       retValue    = GPIO_REQUEST_ERROR;
    uint32_t                  regValue    = 0u;
    const gpio_RequestState_t errataState = Gpio_Check_PinModeErrata( portId, pinId, pinType );

    if( ( GPIO_PORT_CNT         > portId      ) &&
        ( GPIO_PIN_ID_CNT       > pinId       ) &&
        ( GPIO_PIN_MODE_ANALOG >= pinType     ) &&
        ( GPIO_REQUEST_OK      == errataState )    )
    {
        LL_GPIO_SetPinMode( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId, pinType );

        for( uint32_t iterationCnt = 0u; GPIO_TIMEOUT_RAW > iterationCnt; iterationCnt ++ )
        {
            regValue = LL_GPIO_GetPinMode( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );

            if( pinType == regValue )
            {
                retValue = GPIO_REQUEST_OK;
                break;
            }
            else
            {
                /* Register value has not been correctly configured yet */
                retValue = GPIO_REQUEST_ERROR;
            }
        }
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Configures pin type [analog/input/output/alternate]
 *
 * \param portId   [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId    [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinType [out]: Actual pin type configuration [analog/input/output/alternate]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Get_PinMode(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinMode_t * const pinType )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT   > portId  ) &&
        ( GPIO_PIN_ID_CNT > pinId   ) &&
        ( GPIO_NULL_PTR  != pinType )    )
    {
        *pinType = LL_GPIO_GetPinMode( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );

        retValue = GPIO_REQUEST_OK;
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Configures pin speed configuration [low/normal/high/very_high]
 *
 * \param portId   [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId    [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinSpeed [in]: Required pin speed configuration [low/normal/high/very_high]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Set_PinSpeed(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinSpeed_t pinSpeed )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;
    uint32_t            regValue = 0u;

    if( ( GPIO_PORT_CNT             > portId   ) &&
        ( GPIO_PIN_ID_CNT           > pinId    ) &&
        ( GPIO_PIN_SPEED_VERY_HIGH >= pinSpeed )    )
    {
        LL_GPIO_SetPinSpeed( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId, pinSpeed );

        for( uint32_t iterationCnt = 0u; GPIO_TIMEOUT_RAW > iterationCnt; iterationCnt ++ )
        {
            regValue = LL_GPIO_GetPinSpeed( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );

            if( pinSpeed == regValue )
            {
                retValue = GPIO_REQUEST_OK;
                break;
            }
            else
            {
                /* Register value has not been correctly configured yet */
                retValue = GPIO_REQUEST_ERROR;
            }
        }
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Returns pin speed configuration [low/normal/high/very_high]
 *
 * \param portId    [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId     [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinSpeed [out]: Actual pin speed configuration [low/normal/high/very_high]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Get_PinSpeed(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinSpeed_t * const pinSpeed )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT   > portId   ) &&
        ( GPIO_PIN_ID_CNT > pinId    ) &&
        ( GPIO_NULL_PTR  != pinSpeed )    )
    {
        *pinSpeed = LL_GPIO_GetPinSpeed( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );

        retValue = GPIO_REQUEST_OK;
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Configures pin output type [push_pull/open_drain]
 *
 * \param portId     [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId      [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinOutType [in]: Required pin output type configuration [push_pull/open_drain]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Set_PinOutType(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinOutputType_t pinOutType )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;
    uint32_t            regValue = 0u;

    if( ( GPIO_PORT_CNT              > portId     ) &&
        ( GPIO_PIN_ID_CNT            > pinId      ) &&
        ( GPIO_PIN_OUTPUT_OPENDRAIN >= pinOutType )    )
    {
        LL_GPIO_SetPinOutputType( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId, pinOutType );

        for( uint32_t iterationCnt = 0u; GPIO_TIMEOUT_RAW > iterationCnt; iterationCnt ++ )
        {
            regValue = LL_GPIO_GetPinOutputType( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );

            if( pinOutType == regValue )
            {
                retValue = GPIO_REQUEST_OK;
                break;
            }
            else
            {
                /* Register value has not been correctly configured yet */
                retValue = GPIO_REQUEST_ERROR;
            }
        }
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Returns pin output type [push_pull/open_drain]
 *
 * \param portId      [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId       [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinOutType [out]: Actual pin output type configuration [push_pull/open_drain]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Get_PinOutType(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinOutputType_t * const pinOutType )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT   > portId     ) &&
        ( GPIO_PIN_ID_CNT > pinId      ) &&
        ( GPIO_NULL_PTR  != pinOutType )    )
    {
        *pinOutType = LL_GPIO_GetPinOutputType( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );

        retValue = GPIO_REQUEST_OK;
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Configures pin alternate function [AF0-AF15]
 *
 * \param portId  [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId   [in]: GPIO pin identification [Pin0,Pin1...]
 * \param altFunc [in]: Required pin alternative function configuration [AF0-AF15]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Set_PinAltFunction(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_AltFunction_t altFunc )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;
    uint32_t            regValue = 0u;

    if( ( GPIO_PORT_CNT     > portId  ) &&
        ( GPIO_PIN_ID_CNT   > pinId   ) &&
        ( GPIO_ALT_FUNC_CNT > altFunc )    )
    {
        if( GPIO_PIN_ID_8 > pinId )
        {
            LL_GPIO_SetAFPin_0_7( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId, gpio_AltFuncConfig[ altFunc ].AltFuncReg );
        }
        else
        {
            LL_GPIO_SetAFPin_8_15( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId, gpio_AltFuncConfig[ altFunc ].AltFuncReg  );
        }

        for( uint32_t iterationCnt = 0u; GPIO_TIMEOUT_RAW > iterationCnt; iterationCnt ++ )
        {
            if( GPIO_PIN_ID_8 > pinId )
            {
                regValue = LL_GPIO_GetAFPin_0_7( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );
            }
            else
            {
                regValue = LL_GPIO_GetAFPin_8_15( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );
            }

            if( gpio_AltFuncConfig[ altFunc ].AltFuncReg == regValue )
            {
                retValue = GPIO_REQUEST_OK;
                break;
            }
            else
            {
                /* Register value has not been correctly configured yet */
                retValue = GPIO_REQUEST_ERROR;
            }
        }
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Returns pin alternate function [AF0-AF15]
 *
 * \param portId   [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId    [in]: GPIO pin identification [Pin0,Pin1...]
 * \param altFunc [out]: Actual pin alternative function configuration [AF0-AF15]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Get_PinAltFunction(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_AltFunction_t * const altFunc )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT   > portId  ) &&
        ( GPIO_PIN_ID_CNT > pinId   ) &&
        ( GPIO_NULL_PTR  != altFunc )    )
    {
        if( GPIO_PIN_ID_8 > pinId )
        {
            *altFunc = LL_GPIO_GetAFPin_0_7( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );
        }
        else
        {
            *altFunc = LL_GPIO_GetAFPin_8_15( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );
        }

        retValue = GPIO_REQUEST_OK;
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Configures pin pull up/down configuration [up/down/no]
 *
 * \param portId  [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId   [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pullCfg [in]: Required pin pull configuration [up/down/no]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Set_PinPull( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinPullCfg_t pullCfg )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;
    uint32_t            regValue = 0u;

    if( ( GPIO_PORT_CNT       > portId  ) &&
        ( GPIO_PIN_ID_CNT     > pinId   ) &&
        ( GPIO_PIN_PULL_DOWN >= pullCfg )    )
    {
        LL_GPIO_SetPinPull( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId, pullCfg );

        for( uint32_t iterationCnt = 0u; GPIO_TIMEOUT_RAW > iterationCnt; iterationCnt ++ )
        {
            regValue = LL_GPIO_GetPinPull( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );

            if( pullCfg == regValue )
            {
                retValue = GPIO_REQUEST_OK;
                break;
            }
            else
            {
                /* Register value has not been correctly configured yet */
                retValue = GPIO_REQUEST_ERROR;
            }
        }
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Returns pin pull up/down configuration [up/down/no]
 *
 * \param portId   [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId    [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pullCfg [out]: Actual pin pull configuration [up/down/no]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Get_PinPull( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinPullCfg_t * const pullCfg )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT   > portId  ) &&
        ( GPIO_PIN_ID_CNT > pinId   ) &&
        ( GPIO_NULL_PTR  != pullCfg )    )
    {
        *pullCfg = LL_GPIO_GetPinPull( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );

        retValue = GPIO_REQUEST_OK;
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Toggles pin output level [high/low]
 *
 * \param portId   [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId    [in]: GPIO pin identification [Pin0,Pin1...]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Toggle_PinLevel(gpio_PortId_t portId, gpio_PinId_t pinId )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT   > portId ) &&
        ( GPIO_PIN_ID_CNT > pinId  )    )
    {
        LL_GPIO_TogglePin( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );

        retValue = GPIO_REQUEST_OK;
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Configures pin output level [high/low]
 *
 * Output data register is written atomically (BSRR set / reset half, STM32F4
 * has no BRR register) and verified by read-back. Level is applied on the pin
 * only if the pin is configured as output.
 *
 * \param portId   [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId    [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinLevel [in]: Required pin output state, value from \ref gpio_PinLevel_t
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Set_PinLevel(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t pinLevel )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT        > portId   ) &&
        ( GPIO_PIN_ID_CNT      > pinId    ) &&
        ( GPIO_PIN_LEVEL_HIGH >= pinLevel )    )
    {
        if( GPIO_PIN_LEVEL_HIGH == pinLevel )
        {
            LL_GPIO_SetOutputPin( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );
        }
        else
        {
            LL_GPIO_ResetOutputPin( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );
        }

        for( uint32_t iterationCnt = 0u; GPIO_TIMEOUT_RAW > iterationCnt; iterationCnt ++ )
        {
            const uint32_t  regValue    = LL_GPIO_IsOutputPinSet( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );
            gpio_PinLevel_t actualLevel = GPIO_PIN_LEVEL_LOW;

            if( 0u != regValue )
            {
                actualLevel = GPIO_PIN_LEVEL_HIGH;
            }
            else
            {
                actualLevel = GPIO_PIN_LEVEL_LOW;
            }

            if( pinLevel == actualLevel )
            {
                retValue = GPIO_REQUEST_OK;
                break;
            }
            else
            {
                /* Register value has not been correctly configured yet */
                retValue = GPIO_REQUEST_ERROR;
            }
        }
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Returns pin output level [high/low]
 *
 * \note  Level is read from input data register (actual level on the pin).
 *
 * \param portId    [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId     [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinLevel [out]: Pointer to store actual pin level. Must not be NULL.
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Get_PinLevel(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t * const pinLevel )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT   > portId   ) &&
        ( GPIO_PIN_ID_CNT > pinId    ) &&
        ( GPIO_NULL_PTR  != pinLevel )    )
    {
        uint32_t regValue = LL_GPIO_IsInputPinSet( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );

        if( 0u != regValue )
        {
            *pinLevel = GPIO_PIN_LEVEL_HIGH;
        }
        else
        {
            *pinLevel = GPIO_PIN_LEVEL_LOW;
        }

        retValue = GPIO_REQUEST_OK;
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Sets pin active level
 *
 * \param portId         [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId          [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinActiveLevel [in]: Output level in active state [high/low]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Set_PinStateActive(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t pinActiveLevel )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT        > portId         ) &&
        ( GPIO_PIN_ID_CNT      > pinId          ) &&
        ( GPIO_PIN_LEVEL_HIGH >= pinActiveLevel )    )
    {
        retValue = Gpio_Set_PinLevel( portId, pinId, pinActiveLevel );
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/**
 * \brief Sets pin inactive level
 *
 * Pin is driven to the opposite of its active level.
 *
 * \param portId         [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId          [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinActiveLevel [in]: Output level in active state, value from \ref gpio_PinLevel_t
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Set_PinStateInactive(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t pinActiveLevel )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT        > portId         ) &&
        ( GPIO_PIN_ID_CNT      > pinId          ) &&
        ( GPIO_PIN_LEVEL_HIGH >= pinActiveLevel )    )
    {
        if( GPIO_PIN_LEVEL_HIGH == pinActiveLevel )
        {
            retValue = Gpio_Set_PinLevel( portId, pinId, GPIO_PIN_LEVEL_LOW );
        }
        else
        {
            retValue = Gpio_Set_PinLevel( portId, pinId, GPIO_PIN_LEVEL_HIGH );
        }
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/* =========================== LOCAL FUNCTIONS ============================== */

/**
 * \brief Checks the pin mode against device errata "PH1 cannot be used as a GPIO in HSE
 *        bypass mode" (STM32F401 / F411)
 *
 * PH1 can not be used in input, output or alternate function mode while HSE is used as
 * external clock (HSEON and HSEBYP set). Analog mode, other pins and other devices are
 * always accepted.
 *
 * \param portId  [in]: GPIO port identification
 * \param pinId   [in]: GPIO pin identification
 * \param pinType [in]: Required pin mode
 * \return Returns \ref GPIO_REQUEST_OK if the pin can be used in the mode, otherwise
 *         \ref GPIO_REQUEST_ERROR.
 */
static gpio_RequestState_t Gpio_Check_PinModeErrata( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinMode_t pinType )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_OK;

#if defined(GPIO_ERRATA_PH1_HSE_BYPASS)
    if( ( GPIO_PORT_H            == portId                                         ) &&
        ( GPIO_PIN_ID_1          == pinId                                          ) &&
        ( GPIO_PIN_MODE_ANALOG   != pinType                                        ) &&
        ( GPIO_RCC_CR_HSE_BYPASS == READ_BIT( RCC->CR, GPIO_RCC_CR_HSE_BYPASS )   )    )
    {
        /* PH1 does not work as GPIO while HSE is bypassed */
        retValue = GPIO_REQUEST_ERROR;
    }
    else
    {
        /* Pin can be used in the required mode */
        retValue = GPIO_REQUEST_OK;
    }
#else
    (void)portId;
    (void)pinId;
    (void)pinType;
#endif /* GPIO_ERRATA_PH1_HSE_BYPASS */

    return ( retValue );
}


/**
 * \brief Configures the non-bonded PB11 pad as analog after the port B clock was activated
 *        (STM32F401xB / xC)
 *
 * Device errata "Extra power consumption may be observed on the UQFN48, LQFP64 and LQPF100
 * packages": the non-bonded PB11 pad is in input floating state. Workaround: PB11 in analog
 * mode. PB11 is changed only if it is still in reset (input) mode - a configuration of the
 * application (packages with bonded PB11) is kept. Other ports and devices: no action.
 *
 * \param portId [in]: GPIO port whose clock was activated
 */
static void Gpio_Set_UnbondedPadErrata( gpio_PortId_t portId )
{
#if defined(GPIO_ERRATA_PB11_NOT_BONDED)
    if( ( GPIO_PORT_B        == portId                                  ) &&
        ( LL_GPIO_MODE_INPUT == LL_GPIO_GetPinMode( GPIOB, LL_GPIO_PIN_11 ) )    )
    {
        LL_GPIO_SetPinMode( GPIOB, LL_GPIO_PIN_11, LL_GPIO_MODE_ANALOG );
    }
    else
    {
        /* Other port or PB11 configured already */
    }
#else
    (void)portId;
#endif /* GPIO_ERRATA_PB11_NOT_BONDED */
}

/* =========================== INTERRUPT HANDLERS =========================== */

/* ================================ TASKS =================================== */
