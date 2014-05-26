/*
 * drivers/base/power/main.c - Where the driver meets power management.
 *
 * Copyright (c) 2003 Patrick Mochel
 * Copyright (c) 2003 Open Source Development Lab
 *
 * This file is released under the GPLv2
 *
 *
 * The driver model core calls device_pm_add() when a device is registered.
 * This will initialize the embedded device_pm_info object in the device
 * and add it to the list of power-controlled devices. sysfs entries for
 * controlling device power management will also be added.
 *
 * A separate list is used for keeping track of power info, because the power
 * domain dependencies may differ from the ancestral dependencies that the
 * subsystem list maintains.
 */
#define DEBUG
#include <linux/device.h>
#include <linux/kallsyms.h>
#include <linux/mutex.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/resume-trace.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/async.h>
#include <linux/suspend.h>
#include <linux/timer.h>

#include "../base.h"
#include "power.h"

typedef int (*pm_callback_t)(struct device *);

/*
 * The entries in the dpm_list list are in a depth first order, simply
 * because children are guaranteed to be discovered after parents, and
 * are inserted at the back of the list on discovery.
 *
 * Since device_pm_add() may be called with a device lock held,
 * we must never try to acquire a device lock while holding
 * dpm_list_mutex.
 */

LIST_HEAD(dpm_list);
static LIST_HEAD(dpm_prepared_list);
static LIST_HEAD(dpm_suspended_list);
static LIST_HEAD(dpm_late_early_list);
static LIST_HEAD(dpm_noirq_list);


static DEFINE_MUTEX(dpm_list_mtx);
static pm_message_t pm_transition;

struct dpm_drv_wd_data {
	struct device *dev;
	struct task_struct *tsk;
};

static int async_error;

/**
 * device_pm_init - Initialize the PM-related part of a device object.
 * @dev: Device object being initialized.
 */
void device_pm_init(struct device *dev)
{
	dev->power.is_prepared = false;
	dev->power.is_suspended = false;
	dev->power.is_noirq_suspended = false;
	dev->power.is_late_suspended = false;
	init_completion(&dev->power.completion);
	complete_all(&dev->power.completion);
	dev->power.wakeup = NULL;
	spin_lock_init(&dev->power.lock);
	pm_runtime_init(dev);
	INIT_LIST_HEAD(&dev->power.entry);
}

/**
 * device_pm_lock - Lock the list of active devices used by the PM core.
 */
void device_pm_lock(void)
{
	mutex_lock(&dpm_list_mtx);
}

/**
 * device_pm_unlock - Unlock the list of active devices used by the PM core.
 */
void device_pm_unlock(void)
{
	mutex_unlock(&dpm_list_mtx);
}

/**
 * device_pm_add - Add a device to the PM core's list of active devices.
 * @dev: Device to add to the list.
 */
void device_pm_add(struct device *dev)
{
	pr_debug("PM: Adding info for %s:%s\n",
		 dev->bus ? dev->bus->name : "No Bus", dev_name(dev));
	mutex_lock(&dpm_list_mtx);
	if (dev->parent && dev->parent->power.is_prepared)
		dev_warn(dev, "parent %s should not be sleeping\n",
			dev_name(dev->parent));
	list_add_tail(&dev->power.entry, &dpm_list);
	mutex_unlock(&dpm_list_mtx);
}

/**
 * device_pm_remove - Remove a device from the PM core's list of active devices.
 * @dev: Device to be removed from the list.
 */
void device_pm_remove(struct device *dev)
{
	pr_debug("PM: Removing info for %s:%s\n",
		 dev->bus ? dev->bus->name : "No Bus", dev_name(dev));
	complete_all(&dev->power.completion);
	mutex_lock(&dpm_list_mtx);
	list_del_init(&dev->power.entry);
	mutex_unlock(&dpm_list_mtx);
	device_wakeup_disable(dev);
	pm_runtime_remove(dev);
}

/**
 * device_pm_move_before - Move device in the PM core's list of active devices.
 * @deva: Device to move in dpm_list.
 * @devb: Device @deva should come before.
 */
void device_pm_move_before(struct device *deva, struct device *devb)
{
	pr_debug("PM: Moving %s:%s before %s:%s\n",
		 deva->bus ? deva->bus->name : "No Bus", dev_name(deva),
		 devb->bus ? devb->bus->name : "No Bus", dev_name(devb));
	/* Delete deva from dpm_list and reinsert before devb. */
	list_move_tail(&deva->power.entry, &devb->power.entry);
}

/**
 * device_pm_move_after - Move device in the PM core's list of active devices.
 * @deva: Device to move in dpm_list.
 * @devb: Device @deva should come after.
 */
void device_pm_move_after(struct device *deva, struct device *devb)
{
	pr_debug("PM: Moving %s:%s after %s:%s\n",
		 deva->bus ? deva->bus->name : "No Bus", dev_name(deva),
		 devb->bus ? devb->bus->name : "No Bus", dev_name(devb));
	/* Delete deva from dpm_list and reinsert after devb. */
	list_move(&deva->power.entry, &devb->power.entry);
}

/**
 * device_pm_move_last - Move device to end of the PM core's list of devices.
 * @dev: Device to move in dpm_list.
 */
void device_pm_move_last(struct device *dev)
{
	pr_debug("PM: Moving %s:%s to end of list\n",
		 dev->bus ? dev->bus->name : "No Bus", dev_name(dev));
	list_move_tail(&dev->power.entry, &dpm_list);
}

static ktime_t initcall_debug_start(struct device *dev)
{
	ktime_t calltime = ktime_set(0, 0);

	if (initcall_debug) {
		pr_info("calling  %s+ @ %i\n",
				dev_name(dev), task_pid_nr(current));
		calltime = ktime_get();
	}

	return calltime;
}

static void initcall_debug_report(struct device *dev, ktime_t calltime,
                                  int error)
{
        ktime_t rettime;
        s64 nsecs;
        bool pm_print_times_enabled = 0;

        rettime = ktime_get();
        nsecs = (s64) ktime_to_ns(ktime_sub(rettime, calltime));

        if (pm_print_times_enabled) {
                pr_info("call %s+ returned %d after %Ld usecs\n", dev_name(dev),
                        error, (unsigned long long)nsecs >> 10);
        }

//        trace_device_pm_report_time(dev, info, nsecs, pm_verb(state.event),
//                                    error);
}

/**
 * dpm_wait - Wait for a PM operation to complete.
 * @dev: Device to wait for.
 * @async: If unset, wait only if the device's power.async_suspend flag is set.
 */
static void dpm_wait(struct device *dev, bool async)
{
	if (!dev)
		return;

	if (async || (pm_async_enabled && dev->power.async_suspend))
		wait_for_completion(&dev->power.completion);
}

static int dpm_wait_fn(struct device *dev, void *async_ptr)
{
	dpm_wait(dev, *((bool *)async_ptr));
	return 0;
}

static void dpm_wait_for_children(struct device *dev, bool async)
{
       device_for_each_child(dev, &async, dpm_wait_fn);
}

/**
 * pm_op - Return the PM operation appropriate for given PM event.
 * @ops: PM operations to choose from.
 * @state: PM transition of the system being carried out.
 */
static pm_callback_t pm_op(const struct dev_pm_ops *ops, pm_message_t state)
{
        switch (state.event) {
#ifdef CONFIG_SUSPEND
        case PM_EVENT_SUSPEND:
                return ops->suspend;
        case PM_EVENT_RESUME:
                return ops->resume;
#endif /* CONFIG_SUSPEND */
#ifdef CONFIG_HIBERNATE_CALLBACKS
        case PM_EVENT_FREEZE:
        case PM_EVENT_QUIESCE:
                return ops->freeze;
        case PM_EVENT_HIBERNATE:
                return ops->poweroff;
        case PM_EVENT_THAW:
        case PM_EVENT_RECOVER:
                return ops->thaw;
                break;
        case PM_EVENT_RESTORE:
                return ops->restore;
#endif /* CONFIG_HIBERNATE_CALLBACKS */
        }

        return NULL;
}

///**
// * pm_op - Execute the PM operation appropriate for given PM event.
// * @dev: Device to handle.
// * @ops: PM operations to choose from.
// * @state: PM transition of the system being carried out.
// */
//static int pm_op(struct device *dev,
//		 const struct dev_pm_ops *ops,
//		 pm_message_t state)
//{
//	int error = 0;
//	ktime_t calltime;
//
//	calltime = initcall_debug_start(dev);
//
//	switch (state.event) {
//#ifdef CONFIG_SUSPEND
//	case PM_EVENT_SUSPEND:
//		if (ops->suspend) {
//			error = ops->suspend(dev);
//			suspend_report_result(ops->suspend, error);
//		}
//		break;
//	case PM_EVENT_RESUME:
//		if (ops->resume) {
//			error = ops->resume(dev);
//			suspend_report_result(ops->resume, error);
//		}
//		break;
//#endif /* CONFIG_SUSPEND */
//#ifdef CONFIG_HIBERNATE_CALLBACKS
//	case PM_EVENT_FREEZE:
//	case PM_EVENT_QUIESCE:
//		if (ops->freeze) {
//			error = ops->freeze(dev);
//			suspend_report_result(ops->freeze, error);
//		}
//		break;
//	case PM_EVENT_HIBERNATE:
//		if (ops->poweroff) {
//			error = ops->poweroff(dev);
//			suspend_report_result(ops->poweroff, error);
//		}
//		break;
//	case PM_EVENT_THAW:
//	case PM_EVENT_RECOVER:
//		if (ops->thaw) {
//			error = ops->thaw(dev);
//			suspend_report_result(ops->thaw, error);
//		}
//		break;
//	case PM_EVENT_RESTORE:
//		if (ops->restore) {
//			error = ops->restore(dev);
//			suspend_report_result(ops->restore, error);
//		}
//		break;
//#endif /* CONFIG_HIBERNATE_CALLBACKS */
//	default:
//		error = -EINVAL;
//	}
//
//	initcall_debug_report(dev, calltime, error);
//
//	return error;
//}

/**
 * pm_noirq_op - Execute the PM operation appropriate for given PM event.
 * @dev: Device to handle.
 * @ops: PM operations to choose from.
 * @state: PM transition of the system being carried out.
 *
 * The driver of @dev will not receive interrupts while this function is being
 * executed.
 */
/**
 * pm_noirq_op - Return the PM operation appropriate for given PM event.
 * @ops: PM operations to choose from.
 * @state: PM transition of the system being carried out.
 *
 * The driver of @dev will not receive interrupts while this function is being
 * executed.
 */
static pm_callback_t pm_noirq_op(const struct dev_pm_ops *ops, pm_message_t state)
{
        switch (state.event) {
#ifdef CONFIG_SUSPEND
        case PM_EVENT_SUSPEND:
                return ops->suspend_noirq;
        case PM_EVENT_RESUME:
                return ops->resume_noirq;
#endif /* CONFIG_SUSPEND */
#ifdef CONFIG_HIBERNATE_CALLBACKS
        case PM_EVENT_FREEZE:
        case PM_EVENT_QUIESCE:
                return ops->freeze_noirq;
        case PM_EVENT_HIBERNATE:
                return ops->poweroff_noirq;
        case PM_EVENT_THAW:
        case PM_EVENT_RECOVER:
                return ops->thaw_noirq;
        case PM_EVENT_RESTORE:
                return ops->restore_noirq;
#endif /* CONFIG_HIBERNATE_CALLBACKS */
        }

        return NULL;
}

static char *pm_verb(int event)
{
	switch (event) {
	case PM_EVENT_SUSPEND:
		return "suspend";
	case PM_EVENT_RESUME:
		return "resume";
	case PM_EVENT_FREEZE:
		return "freeze";
	case PM_EVENT_QUIESCE:
		return "quiesce";
	case PM_EVENT_HIBERNATE:
		return "hibernate";
	case PM_EVENT_THAW:
		return "thaw";
	case PM_EVENT_RESTORE:
		return "restore";
	case PM_EVENT_RECOVER:
		return "recover";
	default:
		return "(unknown PM event)";
	}
}

static void pm_dev_dbg(struct device *dev, pm_message_t state, char *info)
{
	dev_dbg(dev, "%s%s%s\n", info, pm_verb(state.event),
		((state.event & PM_EVENT_SLEEP) && device_may_wakeup(dev)) ?
		", may wakeup" : "");
}

static void pm_dev_err(struct device *dev, pm_message_t state, char *info,
			int error)
{
	printk(KERN_ERR "PM: Device %s failed to %s%s: error %d\n",
		dev_name(dev), pm_verb(state.event), info, error);
}

static void dpm_show_time(ktime_t starttime, pm_message_t state, char *info)
{
	ktime_t calltime;
	u64 usecs64;
	int usecs;

	calltime = ktime_get();
	usecs64 = ktime_to_ns(ktime_sub(calltime, starttime));
	do_div(usecs64, NSEC_PER_USEC);
	usecs = usecs64;
	if (usecs == 0)
		usecs = 1;
	pr_info("PM: %s%s%s of devices complete after %ld.%03ld msecs\n",
		info ?: "", info ? " " : "", pm_verb(state.event),
		usecs / USEC_PER_MSEC, usecs % USEC_PER_MSEC);
}

static int dpm_run_callback(pm_callback_t cb, struct device *dev,
                            pm_message_t state, char *info)
{
        ktime_t calltime;
        int error;

        if (!cb)
                return 0;

        calltime = initcall_debug_start(dev);

        pm_dev_dbg(dev, state, info);
        error = cb(dev);
        suspend_report_result(cb, error);

        initcall_debug_report(dev, calltime, error);

        return error;
}

/*------------------------- Resume routines -------------------------*/

/**
 * device_resume_noirq - Execute an "early resume" callback for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 *
 * The driver of @dev will not receive interrupts while this function is being
 * executed.
 */
static int device_resume_noirq(struct device *dev, pm_message_t state)
{
        pm_callback_t callback = NULL;
        char *info = NULL;
        int error = 0;

        TRACE_DEVICE(dev);
        TRACE_RESUME(0);

//        if (dev->power.syscore)
//                goto Out;

		if (!dev->power.is_noirq_suspended)
				goto Out;

        if (dev->pwr_domain) {
                info = "noirq power domain ";
                callback = pm_noirq_op(&dev->pwr_domain->ops, state);
        } else if (dev->type && dev->type->pm) {
                info = "noirq type ";
                callback = pm_noirq_op(dev->type->pm, state);
        } else if (dev->class && dev->class->pm) {
                info = "noirq class ";
                callback = pm_noirq_op(dev->class->pm, state);
        } else if (dev->bus && dev->bus->pm) {
                info = "noirq bus ";
                callback = pm_noirq_op(dev->bus->pm, state);
        }

        if (!callback && dev->driver && dev->driver->pm) {
                info = "noirq driver ";
                callback = pm_noirq_op(dev->driver->pm, state);
        }

        error = dpm_run_callback(callback, dev, state, info);
        dev->power.is_noirq_suspended = false;

 Out:
        TRACE_RESUME(error);
        return error;
}

/**
 * dpm_resume_noirq - Execute "early resume" callbacks for non-sysdev devices.
 * @state: PM transition of the system being carried out.
 *
 * Call the "noirq" resume handlers for all devices marked as DPM_OFF_IRQ and
 * enable device drivers to receive interrupts.
 */
void dpm_resume_noirq(pm_message_t state)
{
	ktime_t starttime = ktime_get();

	mutex_lock(&dpm_list_mtx);
	while (!list_empty(&dpm_noirq_list)) {
		struct device *dev = to_device(dpm_noirq_list.next);
		int error;

		get_device(dev);
		list_move_tail(&dev->power.entry, &dpm_suspended_list);
		mutex_unlock(&dpm_list_mtx);

		error = device_resume_noirq(dev, state);
		if (error)
			pm_dev_err(dev, state, " early", error);

		mutex_lock(&dpm_list_mtx);
		put_device(dev);
	}
	mutex_unlock(&dpm_list_mtx);
	dpm_show_time(starttime, state, "early");
	resume_device_irqs();
}
EXPORT_SYMBOL_GPL(dpm_resume_noirq);


/**
 * device_resume - Execute "resume" callbacks for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 * @async: If true, the device is being resumed asynchronously.
 */
static int device_resume(struct device *dev, pm_message_t state, bool async)
{
        pm_callback_t callback = NULL;
        char *info = NULL;
        int error = 0;

        TRACE_DEVICE(dev);
        TRACE_RESUME(0);

//        if (dev->power.syscore)
//                goto Complete;

        dpm_wait(dev->parent, async);
        device_lock(dev);

        /*
         * This is a fib.  But we'll allow new children to be added below
         * a resumed device, even if the device hasn't been completed yet.
         */
        dev->power.is_prepared = false;

        if (!dev->power.is_suspended)
                goto Unlock;

        if (dev->pwr_domain) {
                info = "power domain ";
                callback = pm_op(&dev->pwr_domain->ops, state);
                goto Driver;
        }

        if (dev->type && dev->type->pm) {
                info = "type ";
                callback = pm_op(dev->type->pm, state);
                goto Driver;
        }

        if (dev->class) {
                if (dev->class->pm) {
                        info = "class ";
                        callback = pm_op(dev->class->pm, state);
                        goto Driver;
                } else if (dev->class->resume) {
                        info = "legacy class ";
                        callback = dev->class->resume;
                        goto End;
                }
        }

        if (dev->bus) {
                if (dev->bus->pm) {
                        info = "bus ";
                        callback = pm_op(dev->bus->pm, state);
                } else if (dev->bus->resume) {
                        info = "legacy bus ";
                        callback = dev->bus->resume;
                        goto End;
                }
        }

 Driver:
        if (!callback && dev->driver && dev->driver->pm) {
                info = "driver ";
                callback = pm_op(dev->driver->pm, state);
        }

 End:
        error = dpm_run_callback(callback, dev, state, info);
        dev->power.is_suspended = false;

 Unlock:
        device_unlock(dev);

// Complete:
        complete_all(&dev->power.completion);

        TRACE_RESUME(error);

        return error;
}

static void async_resume(void *data, async_cookie_t cookie)
{
	struct device *dev = (struct device *)data;
	int error;

	error = device_resume(dev, pm_transition, true);
	if (error)
		pm_dev_err(dev, pm_transition, " async", error);
	put_device(dev);
}

static bool is_async(struct device *dev)
{
	return dev->power.async_suspend && pm_async_enabled
		&& !pm_trace_is_enabled();
}

/**
 * dpm_resume - Execute "resume" callbacks for non-sysdev devices.
 * @state: PM transition of the system being carried out.
 *
 * Execute the appropriate "resume" callback for all devices whose status
 * indicates that they are suspended.
 */
void dpm_resume(pm_message_t state)
{
	struct device *dev;
	ktime_t starttime = ktime_get();

	might_sleep();

	mutex_lock(&dpm_list_mtx);
	pm_transition = state;
	async_error = 0;

	list_for_each_entry(dev, &dpm_suspended_list, power.entry) {
		INIT_COMPLETION(dev->power.completion);
		if (is_async(dev)) {
			get_device(dev);
			async_schedule(async_resume, dev);
		}
	}

	while (!list_empty(&dpm_suspended_list)) {
		dev = to_device(dpm_suspended_list.next);
		get_device(dev);
		if (!is_async(dev)) {
			int error;

			mutex_unlock(&dpm_list_mtx);

			error = device_resume(dev, state, false);
			if (error)
				pm_dev_err(dev, state, "", error);

			mutex_lock(&dpm_list_mtx);
		}
		if (!list_empty(&dev->power.entry))
			list_move_tail(&dev->power.entry, &dpm_prepared_list);
		put_device(dev);
	}
	mutex_unlock(&dpm_list_mtx);
	async_synchronize_full();
	dpm_show_time(starttime, state, NULL);
}

/**
 * device_complete - Complete a PM transition for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 */
static void device_complete(struct device *dev, pm_message_t state)
{
	device_lock(dev);

	if (dev->pwr_domain) {
		pm_dev_dbg(dev, state, "completing power domain ");
		if (dev->pwr_domain->ops.complete)
			dev->pwr_domain->ops.complete(dev);
	} else if (dev->type && dev->type->pm) {
		pm_dev_dbg(dev, state, "completing type ");
		if (dev->type->pm->complete)
			dev->type->pm->complete(dev);
	} else if (dev->class && dev->class->pm) {
		pm_dev_dbg(dev, state, "completing class ");
		if (dev->class->pm->complete)
			dev->class->pm->complete(dev);
	} else if (dev->bus && dev->bus->pm) {
		pm_dev_dbg(dev, state, "completing ");
		if (dev->bus->pm->complete)
			dev->bus->pm->complete(dev);
	}

	device_unlock(dev);
}

/**
 * dpm_complete - Complete a PM transition for all non-sysdev devices.
 * @state: PM transition of the system being carried out.
 *
 * Execute the ->complete() callbacks for all devices whose PM status is not
 * DPM_ON (this allows new devices to be registered).
 */
void dpm_complete(pm_message_t state)
{
	struct list_head list;

	might_sleep();

	INIT_LIST_HEAD(&list);
	mutex_lock(&dpm_list_mtx);
	while (!list_empty(&dpm_prepared_list)) {
		struct device *dev = to_device(dpm_prepared_list.prev);

		get_device(dev);
		dev->power.is_prepared = false;
		list_move(&dev->power.entry, &list);
		mutex_unlock(&dpm_list_mtx);

		device_complete(dev, state);

		mutex_lock(&dpm_list_mtx);
		put_device(dev);
	}
	list_splice(&list, &dpm_list);
	mutex_unlock(&dpm_list_mtx);
}

/**
 * dpm_resume_end - Execute "resume" callbacks and complete system transition.
 * @state: PM transition of the system being carried out.
 *
 * Execute "resume" callbacks for all devices and complete the PM transition of
 * the system.
 */
void dpm_resume_end(pm_message_t state)
{
	dpm_resume(state);
	dpm_complete(state);
}
EXPORT_SYMBOL_GPL(dpm_resume_end);


/*------------------------- Suspend routines -------------------------*/

/**
 * resume_event - Return a "resume" message for given "suspend" sleep state.
 * @sleep_state: PM message representing a sleep state.
 *
 * Return a PM message representing the resume event corresponding to given
 * sleep state.
 */
static pm_message_t resume_event(pm_message_t sleep_state)
{
	switch (sleep_state.event) {
	case PM_EVENT_SUSPEND:
		return PMSG_RESUME;
	case PM_EVENT_FREEZE:
	case PM_EVENT_QUIESCE:
		return PMSG_RECOVER;
	case PM_EVENT_HIBERNATE:
		return PMSG_RESTORE;
	}
	return PMSG_ON;
}

/**
 * device_suspend_noirq - Execute a "late suspend" callback for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 *
 * The driver of @dev will not receive interrupts while this function is being
 * executed.
 */
static int device_suspend_noirq(struct device *dev, pm_message_t state)
{
        pm_callback_t callback = NULL;
        char *info = NULL;
        int error;

//        if (dev->power.syscore)
//                return 0;

        if (dev->pwr_domain) {
                info = "noirq power domain ";
                callback = pm_noirq_op(&dev->pwr_domain->ops, state);
        } else if (dev->type && dev->type->pm) {
                info = "noirq type ";
                callback = pm_noirq_op(dev->type->pm, state);
        } else if (dev->class && dev->class->pm) {
                info = "noirq class ";
                callback = pm_noirq_op(dev->class->pm, state);
        } else if (dev->bus && dev->bus->pm) {
                info = "noirq bus ";
                callback = pm_noirq_op(dev->bus->pm, state);
        }

        if (!callback && dev->driver && dev->driver->pm) {
                info = "noirq driver ";
                callback = pm_noirq_op(dev->driver->pm, state);
        }

        error = dpm_run_callback(callback, dev, state, info);
        	if (!error)
        		dev->power.is_noirq_suspended = true;

        return error;
}

/**
 * dpm_suspend_noirq - Execute "late suspend" callbacks for non-sysdev devices.
 * @state: PM transition of the system being carried out.
 *
 * Prevent device drivers from receiving interrupts and call the "noirq" suspend
 * handlers for all non-sysdev devices.
 */
int dpm_suspend_noirq(pm_message_t state)
{
	ktime_t starttime = ktime_get();
	int error = 0;

	suspend_device_irqs();
	mutex_lock(&dpm_list_mtx);
	while (!list_empty(&dpm_suspended_list)) {
		struct device *dev = to_device(dpm_suspended_list.prev);

		get_device(dev);
		mutex_unlock(&dpm_list_mtx);

		error = device_suspend_noirq(dev, state);

		mutex_lock(&dpm_list_mtx);
		if (error) {
			pm_dev_err(dev, state, " late", error);
			put_device(dev);
			break;
		}
		if (!list_empty(&dev->power.entry))
			list_move(&dev->power.entry, &dpm_noirq_list);
		put_device(dev);
	}
	mutex_unlock(&dpm_list_mtx);
	if (error)
		dpm_resume_noirq(resume_event(state));
	else
		dpm_show_time(starttime, state, "late");
	return error;
}
EXPORT_SYMBOL_GPL(dpm_suspend_noirq);

/**
 * legacy_suspend - Execute a legacy (bus or class) suspend callback for device.
 * @dev: Device to suspend.
 * @state: PM transition of the system being carried out.
 * @cb: Suspend callback to execute.
 */
static int legacy_suspend(struct device *dev, pm_message_t state,
                          int (*cb)(struct device *dev, pm_message_t state),
                          char *info)
{
        int error;
        ktime_t calltime;

        calltime = initcall_debug_start(dev);

        error = cb(dev, state);
        suspend_report_result(cb, error);

        initcall_debug_report(dev, calltime, error);

        return error;
}

/**
 * device_suspend - Execute "suspend" callbacks for given device.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 * @async: If true, the device is being suspended asynchronously.
 */
static int __device_suspend(struct device *dev, pm_message_t state, bool async)
{
        pm_callback_t callback = NULL;
        char *info = NULL;
        int error = 0;

        dpm_wait_for_children(dev, async);

        if (async_error)
                goto Complete;

        /*
         * If a device configured to wake up the system from sleep states
         * has been suspended at run time and there's a resume request pending
         * for it, this is equivalent to the device signaling wakeup, so the
         * system suspend operation should be aborted.
         */
        if (pm_runtime_barrier(dev) && device_may_wakeup(dev))
                pm_wakeup_event(dev, 0);

        if (pm_wakeup_pending()) {
                async_error = -EBUSY;
                goto Complete;
        }

//        if (dev->power.syscore)
//                goto Complete;
//
        device_lock(dev);

        if (dev->pwr_domain) {
                info = "power domain ";
                callback = pm_op(&dev->pwr_domain->ops, state);
                goto Run;
        }

        if (dev->type && dev->type->pm) {
                info = "type ";
                callback = pm_op(dev->type->pm, state);
                goto Run;
        }

        if (dev->class) {
                if (dev->class->pm) {
                        info = "class ";
                        callback = pm_op(dev->class->pm, state);
                        goto Run;
                } else if (dev->class->suspend) {
                        pm_dev_dbg(dev, state, "legacy class ");
                        error = legacy_suspend(dev, state, dev->class->suspend,
                                                "legacy class ");
                        goto End;
                }
        }

        if (dev->bus) {
                if (dev->bus->pm) {
                        info = "bus ";
                        callback = pm_op(dev->bus->pm, state);
                } else if (dev->bus->suspend) {
                        pm_dev_dbg(dev, state, "legacy bus ");
                        error = legacy_suspend(dev, state, dev->bus->suspend,
                                                "legacy bus ");
                        goto End;
                }
        }

 Run:
        if (!callback && dev->driver && dev->driver->pm) {
                info = "driver ";
                callback = pm_op(dev->driver->pm, state);
        }

        error = dpm_run_callback(callback, dev, state, info);

 End:
        if (!error) {
                dev->power.is_suspended = true;
//                if (dev->power.wakeup_path
//                    && dev->parent && !dev->parent->power.ignore_children)
//                        dev->parent->power.wakeup_path = true;
        }

        device_unlock(dev);

 Complete:
        complete_all(&dev->power.completion);
        if (error)
                async_error = error;

        return error;
}
static void async_suspend(void *data, async_cookie_t cookie)
{
	struct device *dev = (struct device *)data;
	int error;

	error = __device_suspend(dev, pm_transition, true);
	if (error)
		pm_dev_err(dev, pm_transition, " async", error);

	put_device(dev);
}

static int device_suspend(struct device *dev)
{
	INIT_COMPLETION(dev->power.completion);

	if (pm_async_enabled && dev->power.async_suspend) {
		get_device(dev);
		async_schedule(async_suspend, dev);
		return 0;
	}

	return __device_suspend(dev, pm_transition, false);
}

/**
 * dpm_suspend - Execute "suspend" callbacks for all non-sysdev devices.
 * @state: PM transition of the system being carried out.
 */
int dpm_suspend(pm_message_t state)
{
	ktime_t starttime = ktime_get();
	int error = 0;

	might_sleep();

	mutex_lock(&dpm_list_mtx);
	pm_transition = state;
	async_error = 0;
	while (!list_empty(&dpm_prepared_list)) {
		struct device *dev = to_device(dpm_prepared_list.prev);

		get_device(dev);
		mutex_unlock(&dpm_list_mtx);

		error = device_suspend(dev);

		mutex_lock(&dpm_list_mtx);
		if (error) {
			pm_dev_err(dev, state, "", error);
			put_device(dev);
			break;
		}
		if (!list_empty(&dev->power.entry))
			list_move(&dev->power.entry, &dpm_suspended_list);
		put_device(dev);
		if (async_error)
			break;
	}
	mutex_unlock(&dpm_list_mtx);
	async_synchronize_full();
	if (!error)
		error = async_error;
	if (!error)
		dpm_show_time(starttime, state, NULL);
	return error;
}

/**
 * device_prepare - Prepare a device for system power transition.
 * @dev: Device to handle.
 * @state: PM transition of the system being carried out.
 *
 * Execute the ->prepare() callback(s) for given device.  No new children of the
 * device may be registered after this function has returned.
 */
static int device_prepare(struct device *dev, pm_message_t state)
{
	int error = 0;

	device_lock(dev);

	if (dev->pwr_domain) {
		pm_dev_dbg(dev, state, "preparing power domain ");
		if (dev->pwr_domain->ops.prepare)
			error = dev->pwr_domain->ops.prepare(dev);
		suspend_report_result(dev->pwr_domain->ops.prepare, error);
		if (error)
			goto End;
	} else if (dev->type && dev->type->pm) {
		pm_dev_dbg(dev, state, "preparing type ");
		if (dev->type->pm->prepare)
			error = dev->type->pm->prepare(dev);
		suspend_report_result(dev->type->pm->prepare, error);
		if (error)
			goto End;
	} else if (dev->class && dev->class->pm) {
		pm_dev_dbg(dev, state, "preparing class ");
		if (dev->class->pm->prepare)
			error = dev->class->pm->prepare(dev);
		suspend_report_result(dev->class->pm->prepare, error);
		if (error)
			goto End;
	} else if (dev->bus && dev->bus->pm) {
		pm_dev_dbg(dev, state, "preparing ");
		if (dev->bus->pm->prepare)
			error = dev->bus->pm->prepare(dev);
		suspend_report_result(dev->bus->pm->prepare, error);
	}

 End:
	device_unlock(dev);

	return error;
}

/**
 * dpm_prepare - Prepare all non-sysdev devices for a system PM transition.
 * @state: PM transition of the system being carried out.
 *
 * Execute the ->prepare() callback(s) for all devices.
 */
int dpm_prepare(pm_message_t state)
{
	int error = 0;

	might_sleep();

	mutex_lock(&dpm_list_mtx);
	while (!list_empty(&dpm_list)) {
		struct device *dev = to_device(dpm_list.next);

		get_device(dev);
		mutex_unlock(&dpm_list_mtx);

		pm_runtime_get_noresume(dev);
		if (pm_runtime_barrier(dev) && device_may_wakeup(dev))
			pm_wakeup_event(dev, 0);

		pm_runtime_put_sync(dev);
		error = pm_wakeup_pending() ?
				-EBUSY : device_prepare(dev, state);

		mutex_lock(&dpm_list_mtx);
		if (error) {
			if (error == -EAGAIN) {
				put_device(dev);
				error = 0;
				continue;
			}
			printk(KERN_INFO "PM: Device %s not prepared "
				"for power transition: code %d\n",
				dev_name(dev), error);
			put_device(dev);
			break;
		}
		dev->power.is_prepared = true;
		if (!list_empty(&dev->power.entry))
			list_move_tail(&dev->power.entry, &dpm_prepared_list);
		put_device(dev);
	}
	mutex_unlock(&dpm_list_mtx);
	return error;
}

/**
 * dpm_suspend_start - Prepare devices for PM transition and suspend them.
 * @state: PM transition of the system being carried out.
 *
 * Prepare all non-sysdev devices for system PM transition and execute "suspend"
 * callbacks for them.
 */
int dpm_suspend_start(pm_message_t state)
{
	int error;

	error = dpm_prepare(state);
	if (!error)
		error = dpm_suspend(state);
	return error;
}
EXPORT_SYMBOL_GPL(dpm_suspend_start);

void __suspend_report_result(const char *function, void *fn, int ret)
{
	if (ret)
		printk(KERN_ERR "%s(): %pF returns %d\n", function, fn, ret);
}
EXPORT_SYMBOL_GPL(__suspend_report_result);

/**
 * device_pm_wait_for_dev - Wait for suspend/resume of a device to complete.
 * @dev: Device to wait for.
 * @subordinate: Device that needs to wait for @dev.
 */
int device_pm_wait_for_dev(struct device *subordinate, struct device *dev)
{
	dpm_wait(dev, subordinate->power.async_suspend);
	return async_error;
}
EXPORT_SYMBOL_GPL(device_pm_wait_for_dev);
