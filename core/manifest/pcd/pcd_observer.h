// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef PCD_OBSERVER_H_
#define PCD_OBSERVER_H_

#include "pcd.h"
#include "status/rot_status.h"


/**
 * Interface for notifying observers of PCD events. Unwanted event notifications will be set to null.
 */
struct pcd_observer {
	/**
	 * Notification that a new PCD has been verified and marked as pending.
	 *
	 * Arguments sent with this notification will not be null.
	 *
	 * @param observer The observer being notified.
	 * @param pending The PCD that passed verification and is pending.
	 */
	void (*on_pcd_verified) (struct pcd_observer *observer, struct pcd *pending);

	/**
	 * Notification that a new PCD has been made the active PCD.
	 *
	 * Arguments sent with this notification will not be null.
	 *
	 * @param observer The observer being notified.
	 * @param active The PCD that was activated.
	 */
	void (*on_pcd_activated) (struct pcd_observer *observer, struct pcd *active);

	/**
	 * Notification that the active PCD has been erased.
	 *
	 * @param observer The observer being notified.
	 */
	void (*on_clear_active) (struct pcd_observer *observer);
};


#define	PCD_OBSERVER_ERROR(code)		ROT_ERROR (ROT_MODULE_PCD_OBSERVER, code)

/**
 * Error codes that can be generated by a PCD observer.
 */
enum {
	PCD_OBSERVER_INVALID_ARGUMENT = PCD_OBSERVER_ERROR (0x00),				/**< Input parameter is null or not valid. */
	PCD_OBSERVER_NO_MEMORY = PCD_OBSERVER_ERROR (0x01),						/**< Memory allocation failed. */
	PCD_OBSERVER_MEASUREMENTS_NOT_UNIQUE = PCD_OBSERVER_ERROR (0x02),		/**< Measurement IDs are not unique */
};


#endif /* PCD_OBSERVER_H_ */
