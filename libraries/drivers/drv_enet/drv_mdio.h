/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-06-13   Evlers      first implementation
 */

#ifndef DRV_MDIO_H__
#define DRV_MDIO_H__

#include <rtdevice.h>


rt_mdio_t *rt_hw_mdio_register (void *hw_obj, char *name);

#endif /*DRV_MDIO_H__*/
