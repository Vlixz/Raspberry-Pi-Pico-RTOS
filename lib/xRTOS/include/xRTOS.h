#ifndef INC_XRTOS_H_
#define INC_XRTOS_H_

/**
 * Starts the xRTOS schedular. This function should be called after all tasks have been created.
 *
 * Everything after this function call will not be executed.
 */
void xStartSchedular(void);

#endif // INC_XRTOS_H_