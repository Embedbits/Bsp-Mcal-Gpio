/**
 * \author Mr.Nobody
 * \file Gpio.h
 * \ingroup Gpio
 * \brief Gpio module common functionality
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

/* ========================== SYMBOLIC CONSTANTS ============================ */

/** Value of major version of SW module */
#define GPIO_MAJOR_VERSION           ( 1u )

/** Value of minor version of SW module */
#define GPIO_MINOR_VERSION           ( 0u )

/** Value of patch version of SW module */
#define GPIO_PATCH_VERSION           ( 0u )


/** Maximum delay for execution/checks in raw value. */
#define GPIO_TIMEOUT_RAW            ( 0x84FCB )

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
 * This function shall call every necessary sub-module initialization function 
 * and set up all the necessary resources for the module to work. In case of
 * failure, the function shall handle it by itself and shall not be transferred
 * to AppMain layer.
 */
gpio_RequestState_t Gpio_Init( gpio_Config_t *gpioConfig )
{
    gpio_RequestState_t retState = GPIO_REQUEST_ERROR;

    if( GPIO_NULL_PTR != gpioConfig )
    {
        retState = Gpio_Set_PortActive( gpioConfig->PortId );
        if( GPIO_REQUEST_ERROR == retState )
        {
            return ( retState );
        }

        retState = Gpio_Set_PinMode( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinMode );
        if( GPIO_REQUEST_ERROR == retState )
        {
            return ( retState );
        }

        retState = Gpio_Set_PinSpeed( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinSpeed );
        if( GPIO_REQUEST_ERROR == retState )
        {
            return ( retState );
        }

        retState = Gpio_Set_PinOutType( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinOutType );
        if( GPIO_REQUEST_ERROR == retState )
        {
            return ( retState );
        }

        retState = Gpio_Set_PinAltFunction( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinAltFunction );
        if( GPIO_REQUEST_ERROR == retState )
        {
            return ( retState );
        }

        retState = Gpio_Set_PinPull( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinPull );
        if( GPIO_REQUEST_ERROR == retState )
        {
            return ( retState );
        }

        retState = Gpio_Set_PinStateInactive( gpioConfig->PortId, gpioConfig->PinId, gpioConfig->PinActiveLevel );
        if( GPIO_REQUEST_ERROR == retState )
        {
            return ( retState );
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

}


/**
 * \brief Activates the required port dependencies
 *
 * When port activation is requested, the needed dependencies must be activated
 * (eg. RCC). If needed dependencies are already active, new activation request
 * is not triggered to avoid restart or unspecified behavior.
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
        /* Activation of PWR and RCC */

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
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Set_PinMode(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinMode_t pinType )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;
    uint32_t            regValue = 0u;

    if( ( GPIO_PORT_CNT   > portId ) &&
        ( GPIO_PIN_ID_CNT > pinId  )    )
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

    if( ( GPIO_PORT_CNT   > portId ) &&
        ( GPIO_PIN_ID_CNT > pinId  )    )
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

    if( ( GPIO_PORT_CNT   > portId ) &&
        ( GPIO_PIN_ID_CNT > pinId  )    )
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

    if( ( GPIO_PORT_CNT   > portId ) &&
        ( GPIO_PIN_ID_CNT > pinId  )    )
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
 * \param portId   [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId    [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinLevel [in]: Required pin output state [high/low]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Set_PinLevel(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t pinLevel )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT   > portId ) &&
        ( GPIO_PIN_ID_CNT > pinId  )    )
    {
        if( GPIO_PIN_LEVEL_LOW != pinLevel )
        {
            LL_GPIO_ResetOutputPin( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );
        }
        else
        {
            LL_GPIO_SetOutputPin( gpio_PeriphConf[ portId ].GpioReg, gpio_PinConf[ pinId ].PinRegId );
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
 * \brief Returns pin output level [high/low]
 *
 * \param portId    [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId     [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinLevel [out]: Required pin output state [high/low]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Get_PinLevel(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t * const pinLevel )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT   > portId ) &&
        ( GPIO_PIN_ID_CNT > pinId  )    )
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

    if( ( GPIO_PORT_CNT   > portId ) &&
        ( GPIO_PIN_ID_CNT > pinId  )    )
    {
        Gpio_Set_PinLevel( portId, pinId, pinActiveLevel );

        retValue = GPIO_REQUEST_OK;
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
 * \param portId         [in]: GPIO port identification [GPIOA.GPIOB...]
 * \param pinId          [in]: GPIO pin identification [Pin0,Pin1...]
 * \param pinActiveLevel [in]: Output level in active state [high/low]
 * \return State of request execution. Returns "OK" if request was success,
 *         otherwise return error.
 */
gpio_RequestState_t Gpio_Set_PinStateInactive(gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t pinActiveLevel )
{
    gpio_RequestState_t retValue = GPIO_REQUEST_ERROR;

    if( ( GPIO_PORT_CNT   > portId ) &&
        ( GPIO_PIN_ID_CNT > pinId  )    )
    {
        Gpio_Set_PinLevel( portId, pinId, pinActiveLevel );

        if( GPIO_PIN_LEVEL_LOW != pinActiveLevel )
        {
            Gpio_Set_PinLevel( portId, pinId, GPIO_PIN_LEVEL_LOW );
        }
        else
        {
            Gpio_Set_PinLevel( portId, pinId, GPIO_PIN_LEVEL_HIGH );
        }

        retValue = GPIO_REQUEST_OK;
    }
    else
    {
        retValue = GPIO_REQUEST_ERROR;
    }

    return ( retValue );
}


/* =========================== LOCAL FUNCTIONS ============================== */

/* =========================== INTERRUPT HANDLERS =========================== */

/* ================================ TASKS =================================== */
