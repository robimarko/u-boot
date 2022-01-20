// SPDX-License-Identifier: GPL-2.0-or-later

#include <asm/gpio.h>
#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <miiphy.h>
#include <net/dsa.h>
#include <phy.h>

#include "mv88e6xxx.h"

static int mv88e6xxx_read(struct mv88e6xxx_priv *priv, int addr, int devad, int reg)
{
	return mdio_get_ops(priv->mdio_device)->read(priv->mdio_device,
						     addr,
						     devad,
						     reg);
}

static int mv88e6xxx_write(struct mv88e6xxx_priv *priv, int addr, int devad, int reg, u16 val)
{
	return mdio_get_ops(priv->mdio_device)->write(priv->mdio_device,
						      addr,
						      devad,
						      reg,
						      val);
}

int mv88e6xxx_g2_read(struct mv88e6xxx_priv *priv, int reg)
{
	return mv88e6xxx_read(priv, priv->global2_addr, MDIO_DEVAD_NONE, reg);
}

int mv88e6xxx_g2_write(struct mv88e6xxx_priv *priv, int reg, u16 val)
{
	return mv88e6xxx_write(priv, priv->global2_addr, MDIO_DEVAD_NONE, reg, val);
}

static const struct dsa_ops mv88e6xxx_dsa_ops = {};

static int mv88e6xxx_probe(struct udevice *dev)
{
	struct udevice *parent_dev = dev_get_parent(dev);
	struct mv88e6xxx_priv *priv = dev_get_priv(dev);

	priv->mdio_device = parent_dev;

#if CONFIG_IS_ENABLED(DM_GPIO)
	gpio_request_by_name(dev, "reset-gpios", 0,
			     &priv->reset_gpio, GPIOD_IS_OUT);

	if (dm_gpio_is_valid(&priv->reset_gpio)) {
		dm_gpio_set_value(&priv->reset_gpio, 1);
		mdelay(20);
		dm_gpio_set_value(&priv->reset_gpio, 0);
		mdelay(20);
	}
#endif

	priv->sw_addr = dev_read_addr(dev);
	if (priv->sw_addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->port_base_addr = 0;
	priv->global1_addr = 0x1b;
	priv->global2_addr = 0x1c;

	return dm_scan_fdt_dev(dev);
}

static const struct udevice_id mv88e6xxx_ids[] = {
	{ .compatible = "marvell,mv88e6190" },
	{ }
};

U_BOOT_DRIVER(mv88e6xxx) = {
	.name			= "mv88e6xxx",
	.id			= UCLASS_DSA,
	.of_match		= mv88e6xxx_ids,
	.probe			= mv88e6xxx_probe,
	.ops			= &mv88e6xxx_dsa_ops,
	.priv_auto		= sizeof(struct mv88e6xxx_priv),
};
