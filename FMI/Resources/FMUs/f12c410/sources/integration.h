/* Internal header file for integration part of Dymosim FMI implementation. */

/* 
All functions handle the case when the number of continuos states are 0, for convenience.
*/

#ifndef integration_h
#define integration_h

#include "types.h"

/* ----------------- function delcarations ----------------- */

/**
 * Sets up integration.
 *
 * @param c the FMI component
 * @param relTol the relative tolerance, if non-positive, a default value is used
 * @return status
 */
DYMOLA_STATIC IntegrationStatus integration_setup(FMIComponent c, FMIReal relTol);

/**
 * Tears down integration. May be called also if comp or its iData pointer is NULL.
 *
 * @param c the FMI component
 */
DYMOLA_STATIC IntegrationStatus integration_teardown(FMIComponent c);


/**
 * Integrates a time step forward. Time events are supposed to be handled outside
 * this function, i.e. h should be set not to pass those. Also step events shall
 * be handled outside. State events however are handled inside this function,
 * effectively re-initializing integration when needed.
 *
 * @param comp the FMI component
 * @param tout the end time
 * @return status
 */
DYMOLA_STATIC IntegrationStatus integration_step(Component* comp, FMIReal* tout, FMIReal endStepTime);

/* re-initialize integration due to events
() */

/**
 * Re-initialize the integration. Necessary at events.
 * In particular multistep methods will otherwise use obsolete derivatives.
 *
 * @param comp the FMI component
 * @return status
 */
DYMOLA_STATIC IntegrationStatus integration_reinit(Component* comp, int updateStatistics);

/**
 * Synchronizes the value used as a base for input extrapolation. This is necessary when input is updated
 * externally and in particular handles the case when input is set after the input derivatives.
 *
 * @param comp the FMI component
 * @param ix the index
 */
DYMOLA_STATIC void integration_sync_extrapolation_input(Component* comp, size_t ix);

/**
 * Updates the inputs according to input derivatives, if these are set recent enough. 
 *
 * @param comp the FMI component
 * @param t the current time
 */
DYMOLA_STATIC void integration_extrapolate_inputs(Component* comp, double t);

/**
 * util function for updating integrationStatistics, only use at end of simulation or to store statistics before event
 *
 * @param comp the FMI component
 */

DYMOLA_STATIC void updateIntegrationStatistics(Component* comp, Statistics* stat);

#if (FMI_VERSION >= FMI_2)
/**
 * Gets the integration part of the FMU state.
 * 
 * @param source the source
 * @param target the target
 * @return status
 */
DYMOLA_STATIC FMIStatus integration_get_state(Component* source, Component* target);

/**
 * Sets the integration part of the previously saved FMU state.
 *
 * @param source the source
 * @param target the target
 * @return status
 */
DYMOLA_STATIC FMIStatus integration_set_state(Component* source, Component* target, FMIBoolean reinit_integration);

/**
 * Frees the integration part of the previously saved FMU state.
 *
 * @param comp the FMI component to log status on
 * @param target the component to free
 */
DYMOLA_STATIC IntegrationStatus integration_free_state(Component* comp, Component* target);

/**
*  Experimental function for HybridCosimulation
*
* @param tout is the propsed entime
* @param tRet is the returned endTime
* @param anyEvent indicates if there is an event
* Will be similar to normal if earlyReturn=FMIFalse and handleEvent=FMIFalse
*/
DYMOLA_STATIC IntegrationStatus integration_step_return_at_event(Component* comp, FMIReal* tout, FMIBoolean handleEvent, FMIBoolean earlyReturn, FMIReal* tRet, FMIBoolean*anyEvent, FMIReal endStepTime);

#endif /* FMI_2 */


DYMOLA_STATIC FMIStatus integration_allocate_ipData(Component* comp);
/**
Allocates InteporationData
**/
DYMOLA_STATIC void integration_free_ipData(Component* comp);
/**
Frees InteporationData
**/

/**
* Input smoother functions. See comments in function definitions.
*/
DYMOLA_STATIC FMIStatus InputSmoother_CheckSettings(Component* comp);
DYMOLA_STATIC FMIStatus InputSmoother_Allocate(Component* comp);
DYMOLA_STATIC FMIStatus InputSmoother_Initialize(Component* comp);
DYMOLA_STATIC FMIBoolean InputSmoother_SetInput(Component* comp, size_t i, FMIReal value);
DYMOLA_STATIC FMIStatus InputSmoother_Apply(Component* comp, FMIReal currentCommunicationPoint, FMIReal endStepTime);
DYMOLA_STATIC FMIStatus InputSmoother_UpdateOldInputs(Component* comp, const FMIReal endStepTime);

#endif /* integration_h */
