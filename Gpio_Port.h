/*
 *    Mr.Nobody, COPYRIGHT (c) 2021
 *    ALL RIGHTS RESERVED
 *
 */

/**
 * \file Gpio_Port.h
 * \ingroup Gpio
 * \brief Gpio module public functionality
 *
 * This file contains all available public functionality, any other files shall 
 * not used outside of the module.
 *
 */

#ifndef GPIO_GPIO_PORT_H
#define GPIO_GPIO_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================== INCLUDES ================================== */
#include "Gpio_Types.h"                     /* Module types definition        */
/* ============================== TYPEDEFS ================================== */

/* ========================== SYMBOLIC CONSTANTS ============================ */

/* ========================== EXPORTED MACROS =============================== */

/* ========================== EXPORTED VARIABLES ============================ */

/* ========================= EXPORTED FUNCTIONS ============================= */

gpio_ModuleVersion_t        Gpio_Get_ModuleVersion      ( void );

gpio_RequestState_t         Gpio_Init                   ( gpio_Config_t *gpioConfig );
void                        Gpio_Deinit                 ( void );
void                        Gpio_Task                   ( void );

gpio_RequestState_t         Gpio_Set_PortActive         ( gpio_PortId_t portId );
gpio_RequestState_t         Gpio_Set_PortInactive       ( gpio_PortId_t portId );
gpio_RequestState_t         Gpio_Get_PortState          ( gpio_PortId_t portId, gpio_FunctionState_t * const portState );

gpio_RequestState_t         Gpio_Set_PinMode            ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinMode_t pinType );
gpio_RequestState_t         Gpio_Get_PinMode            ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinMode_t * const pinType );

gpio_RequestState_t         Gpio_Set_PinSpeed           ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinSpeed_t pinSpeed );
gpio_RequestState_t         Gpio_Get_PinSpeed           ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinSpeed_t * const pinSpeed );

gpio_RequestState_t         Gpio_Set_PinOutType         ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinOutputType_t pinOutType );
gpio_RequestState_t         Gpio_Get_PinOutType         ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinOutputType_t * const pinOutType );

gpio_RequestState_t         Gpio_Set_PinAltFunction     ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_AltFunction_t altFunc );
gpio_RequestState_t         Gpio_Get_PinAltFunction     ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_AltFunction_t * const altFunc );

gpio_RequestState_t         Gpio_Set_PinPull            ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinPullCfg_t pullCfg );
gpio_RequestState_t         Gpio_Get_PinPull            ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinPullCfg_t * const pullCfg );


gpio_RequestState_t         Gpio_Toggle_PinLevel        ( gpio_PortId_t portId, gpio_PinId_t pinId );
gpio_RequestState_t         Gpio_Set_PinLevel           ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t pinLevel );
gpio_RequestState_t         Gpio_Get_PinLevel           ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t * const pinLevel );

gpio_RequestState_t         Gpio_Set_PinStateActive     ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t pinActiveLevel );
gpio_RequestState_t         Gpio_Set_PinStateInactive   ( gpio_PortId_t portId, gpio_PinId_t pinId, gpio_PinLevel_t pinActiveLevel );

#ifdef __cplusplus
}
#endif

#endif /* GPIO_GPIO_PORT_H */

