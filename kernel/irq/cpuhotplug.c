/*
 * Generic cpu hotunplug interrupt migration code copied from the
 * arch/arm implementation
 *
 * Copyright (C) Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/interrupt.h>
#include <linux/ratelimit.h>
#include <linux/irq.h>

#include "internals.h"

static bool migrate_one_irq(struct irq_desc *desc)
{
	struct irq_data *d = irq_desc_get_irq_data(desc);
	const struct cpumask *affinity = d->common->affinity;
	struct irq_chip *c;
	bool ret = false;

	/*
	 * If this is a per-CPU interrupt, or the affinity does not
	 * include this CPU, then we have nothing to do.
	 */
	if (irqd_is_per_cpu(d) ||
	    !cpumask_test_cpu(smp_processor_id(), affinity))
		return false;
	}

	if (irqd_has_set(d, IRQD_PERF_CRITICAL))
		return false;

	/*
	 * No move required, if:
	 * - Interrupt is per cpu
	 * - Interrupt is not started
	 * - Affinity mask does not include this CPU.
	 *
	 * Note: Do not check desc->action as this might be a chained
	 * interrupt.
	 */
	if (irqd_is_per_cpu(d) || !irqd_is_started(d) || !irq_needs_fixup(d)) {
		/*
		 * If an irq move is pending, abort it if the dying CPU is
		 * the sole target.
		 */
		irq_fixup_move_pending(desc, false);
		return false;
	}

	/*
	 * Complete an eventually pending irq move cleanup. If this
	 * interrupt was moved in hard irq context, then the vectors need
	 * to be cleaned up. It can't wait until this interrupt actually
	 * happens and this CPU was involved.
	 */
	irq_force_complete_move(desc);

	/*
	 * If there is a setaffinity pending, then try to reuse the pending
	 * mask, so the last change of the affinity does not get lost. If
	 * there is no move pending or the pending mask does not contain
	 * any online CPU, use the current affinity mask.
	 */
	if (irq_fixup_move_pending(desc, true))
		affinity = irq_desc_get_pending_mask(desc);
	else
		affinity = irq_data_get_affinity_mask(d);

	/* Mask the chip for interrupts which cannot move in process context */
	if (maskchip && chip->irq_mask)
		chip->irq_mask(d);

	if (cpumask_any_and(affinity, cpu_online_mask) >= nr_cpu_ids) {
		affinity = cpu_online_mask;
		ret = true;
	}

	c = irq_data_get_irq_chip(d);
	if (!c->irq_set_affinity) {
		pr_debug("IRQ%u: unable to set affinity\n", d->irq);
	} else {
		int r = irq_do_set_affinity(d, affinity, false);
		if (r)
			pr_warn_ratelimited("IRQ%u: set affinity failed(%d).\n",
					    d->irq, r);
	}

	return ret;
}

/**
 * irq_migrate_all_off_this_cpu - Migrate irqs away from offline cpu
 *
 * The current CPU has been marked offline.  Migrate IRQs off this CPU.
 * If the affinity settings do not allow other CPUs, force them onto any
 * available CPU.
 *
 * Note: we must iterate over all IRQs, whether they have an attached
 * action structure or not, as we need to get chained interrupts too.
 */
void irq_migrate_all_off_this_cpu(void)
{
	unsigned int irq;
	struct irq_desc *desc;
	unsigned long flags;

	local_irq_save(flags);

	for_each_active_irq(irq) {
		bool affinity_broken;

		desc = irq_to_desc(irq);
		raw_spin_lock(&desc->lock);
		affinity_broken = migrate_one_irq(desc);
		raw_spin_unlock(&desc->lock);

		if (affinity_broken)
			pr_warn_ratelimited("IRQ%u no longer affine to CPU%u\n",
					    irq, smp_processor_id());
	}

	local_irq_restore(flags);
		}
	}

	if (!cpumask_test_cpu(smp_processor_id(), cpu_lp_mask))
		reaffine_perf_irqs(true);
}

static void irq_restore_affinity_of_irq(struct irq_desc *desc, unsigned int cpu)
{
	struct irq_data *data = irq_desc_get_irq_data(desc);
	const struct cpumask *affinity = irq_data_get_affinity_mask(data);

	if (irqd_has_set(data, IRQD_PERF_CRITICAL))
		return;

	if (!irqd_affinity_is_managed(data) || !desc->action ||
	    !irq_data_get_irq_chip(data) || !cpumask_test_cpu(cpu, affinity))
		return;

	if (irqd_is_managed_and_shutdown(data)) {
		irq_startup(desc, IRQ_RESEND, IRQ_START_COND);
		return;
	}

	/*
	 * If the interrupt can only be directed to a single target
	 * CPU then it is already assigned to a CPU in the affinity
	 * mask. No point in trying to move it around.
	 */
	if (!irqd_is_single_target(data))
		irq_set_affinity_locked(data, affinity, false);
}

/**
 * irq_affinity_online_cpu - Restore affinity for managed interrupts
 * @cpu:	Upcoming CPU for which interrupts should be restored
 */
int irq_affinity_online_cpu(unsigned int cpu)
{
	struct irq_desc *desc;
	unsigned int irq;

	irq_lock_sparse();
	for_each_active_irq(irq) {
		desc = irq_to_desc(irq);
		raw_spin_lock_irq(&desc->lock);
		irq_restore_affinity_of_irq(desc, cpu);
		raw_spin_unlock_irq(&desc->lock);
	}
	irq_unlock_sparse();

	if (!cpumask_test_cpu(cpu, cpu_lp_mask))
		reaffine_perf_irqs(true);

	return 0;
}
