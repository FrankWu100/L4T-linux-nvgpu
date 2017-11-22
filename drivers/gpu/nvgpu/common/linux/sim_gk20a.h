/*
 * drivers/video/tegra/host/gk20a/sim_gk20a.h
 *
 * GK20A sim support
 *
 * Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SIM_GK20A_LINUX_H__
#define __SIM_GK20A_LINUX_H__

#include "gk20a/sim_gk20a.h"

struct sim_gk20a_linux {
	struct sim_gk20a sim;
	struct resource *reg_mem;
	void __iomem *regs;
	struct {
		struct page *page;
		void *kvaddr;
		u64 phys;
	} send_bfr, recv_bfr, msg_bfr;
};

int gk20a_init_sim_support(struct gk20a *g);

#endif
