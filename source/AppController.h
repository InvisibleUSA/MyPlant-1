/*
 *  @file
 *
 *  @brief Configuration header for the AppController.c file.
 *
 */

#ifndef APPCONTROLLER_H_
#define APPCONTROLLER_H_

#include "XDK_Utils.h"

/**
 * @brief Gives control to the Application controller.
 *
 * @param[in] cmdProcessorHandle
 * Handle of the main command processor which shall be used based on the application needs
 *
 * @param[in] param2
 * Unused
 */
void AppController_Init(void * cmdProcessorHandle, uint32_t param2);

#endif /* APPCONTROLLER_H_ */

/** ************************************************************************* */
