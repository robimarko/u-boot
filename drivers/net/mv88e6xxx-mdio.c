// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Marvell Link Street (Amethyst) internal MDIO driver
 *
 * Copyright (c) 2022 Sartura Ltd.
 *
 * Based on Linux driver
 */

#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <miiphy.h>
#include <phy.h>

#include "mv88e6xxx.h"

enum mdio_type {
	MDIO_INTERNAL = 0,
	MDIO_EXTERNAL,
};

#define usleep_range(a, b) udelay((b))

static int mv88e6xxx_g2_scratch_read(struct mv88e6xxx_priv *priv, int reg, u8 *data)
{
	int ret;

	ret = mv88e6xxx_g2_write(priv, MV88E6XXX_G2_SCRATCH_MISC_MISC,
	                         FIELD_PREP(MV88E6XXX_G2_SCRATCH_MISC_REG_MASK, reg));
	if (ret)
		return ret;

	*data = FIELD_PREP(MV88E6XXX_G2_SCRATCH_MISC_DATA_MASK,
	                   mv88e6xxx_g2_read(priv, MV88E6XXX_G2_SCRATCH_MISC_MISC));

	return 0;
}

static int mv88e6xxx_g2_scratch_write(struct mv88e6xxx_priv *priv, int reg, u8 data)
{
	u16 value = 0;

	value |= MV88E6XXX_G2_SCRATCH_MISC_UPDATE;
	value |= FIELD_PREP(MV88E6XXX_G2_SCRATCH_MISC_REG_MASK, reg);
	value |= FIELD_PREP(MV88E6XXX_G2_SCRATCH_MISC_DATA_MASK, data);

	return mv88e6xxx_g2_write(priv, MV88E6XXX_G2_SCRATCH_MISC_MISC, value);
}

static int mv88e6393x_smi_gpio_setup(struct mv88e6xxx_priv *priv)
{
	u8 data;
	int ret;

	ret = mv88e6xxx_g2_scratch_read(priv, MV88E6XXX_G2_SCRATCH_MISC_CONFIG, &data);
	if (ret)
		return ret;

	data &= ~MV88E6XXX_G2_SCRATCH_MISC_SMI_PHY;

	return mv88e6xxx_g2_scratch_write(priv, MV88E6XXX_G2_SCRATCH_MISC_CONFIG, data);
}

static int mv88e6xxx_mdio_wait_busy(struct mv88e6xxx_priv *priv)
{
	int i, ret;

	for (i = 0; i < 16; i++) {
		ret = mv88e6xxx_g2_read(priv, MV88E6XXX_G2_SMI_PHY_CMD);

		if (FIELD_GET(MV88E6XXX_G2_SMI_PHY_CMD_BUSY, ret) == 0)
			return 0;

		usleep_range(1000, 2000);
	}

	return -ETIMEDOUT;
}

int mv88e6xxx_mdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct udevice *parent_dev = dev_get_parent(dev);
	struct mv88e6xxx_priv *priv = dev_get_priv(parent_dev);
	u16 cmd = 0;
	int ret;

	cmd |= MV88E6XXX_G2_SMI_PHY_CMD_BUSY;

	if (dev_get_driver_data(dev) == MDIO_INTERNAL)
		cmd |= FIELD_PREP(MV88E6390_G2_SMI_PHY_CMD_FUNC_MASK,
				  MV88E6390_G2_SMI_PHY_CMD_FUNC_INTERNAL);
	else
		cmd |= FIELD_PREP(MV88E6390_G2_SMI_PHY_CMD_FUNC_MASK,
				  MV88E6390_G2_SMI_PHY_CMD_FUNC_EXTERNAL);

	cmd |= FIELD_PREP(MV88E6XXX_G2_SMI_PHY_CMD_MODE_MASK,
			  MV88E6XXX_G2_SMI_PHY_CMD_MODE_22);
	cmd |= FIELD_PREP(MV88E6XXX_G2_SMI_PHY_CMD_OP_MASK,
			  MV88E6XXX_G2_SMI_PHY_CMD_OP_22_READ_DATA);
	cmd |= FIELD_PREP(MV88E6XXX_G2_SMI_PHY_CMD_DEV_ADDR_MASK, addr);
	cmd |= FIELD_PREP(MV88E6XXX_G2_SMI_PHY_CMD_REG_ADDR_MASK, reg);

	ret = mv88e6xxx_g2_write(priv, MV88E6XXX_G2_SMI_PHY_CMD, cmd);
	if (ret)
		return ret;

	ret = mv88e6xxx_mdio_wait_busy(priv);
	if (ret)
		return ret;

	return mv88e6xxx_g2_read(priv, MV88E6XXX_G2_SMI_PHY_DATA);
}

int mv88e6xxx_mdio_write(struct udevice *dev, int addr, int devad,
					  int reg, u16 val)
{
	struct udevice *parent_dev = dev_get_parent(dev);
	struct mv88e6xxx_priv *priv = dev_get_priv(parent_dev);
	u16 cmd = 0;
	int ret;

	ret = mv88e6xxx_g2_write(priv, MV88E6XXX_G2_SMI_PHY_DATA, val);
	if (ret)
		return ret;

	cmd |= MV88E6XXX_G2_SMI_PHY_CMD_BUSY;

	if (dev_get_driver_data(dev) == MDIO_INTERNAL)
		cmd |= FIELD_PREP(MV88E6390_G2_SMI_PHY_CMD_FUNC_MASK,
				  MV88E6390_G2_SMI_PHY_CMD_FUNC_INTERNAL);
	else
		cmd |= FIELD_PREP(MV88E6390_G2_SMI_PHY_CMD_FUNC_MASK,
				  MV88E6390_G2_SMI_PHY_CMD_FUNC_EXTERNAL);

	cmd |= FIELD_PREP(MV88E6XXX_G2_SMI_PHY_CMD_MODE_MASK,
			  MV88E6XXX_G2_SMI_PHY_CMD_MODE_22);
	cmd |= FIELD_PREP(MV88E6XXX_G2_SMI_PHY_CMD_OP_MASK,
			  MV88E6XXX_G2_SMI_PHY_CMD_OP_22_WRITE_DATA);
	cmd |= FIELD_PREP(MV88E6XXX_G2_SMI_PHY_CMD_DEV_ADDR_MASK, addr);
	cmd |= FIELD_PREP(MV88E6XXX_G2_SMI_PHY_CMD_REG_ADDR_MASK, reg);

	ret = mv88e6xxx_g2_write(priv, MV88E6XXX_G2_SMI_PHY_CMD, cmd);
	if (ret)
		return ret;

	return mv88e6xxx_mdio_wait_busy(priv);
}

static const struct mdio_ops mv88e6xxx_mdio_ops = {
	.read = mv88e6xxx_mdio_read,
	.write = mv88e6xxx_mdio_write,
};

static int mv88e6xxx_mdio_bind(struct udevice *dev)
{
	if (ofnode_valid(dev_ofnode(dev)))
		device_set_name(dev, ofnode_get_name(dev_ofnode(dev)));

	return 0;
}

static int mv88e6xxx_mdio_probe(struct udevice *dev)
{
	struct udevice *parent_dev = dev_get_parent(dev);
	struct mv88e6xxx_priv *priv = dev_get_priv(parent_dev);

	if (dev_get_driver_data(dev) == MDIO_EXTERNAL)
		return mv88e6393x_smi_gpio_setup(priv);

	return 0;
}

static const struct udevice_id mv88e6xxx_mdio_ids[] = {
	{ .compatible = "marvell,mv88e6xxx-mdio", .data = MDIO_INTERNAL },
	{ .compatible = "marvell,mv88e6xxx-mdio-external", .data = MDIO_EXTERNAL },
	{ }
};

U_BOOT_DRIVER(mv88e6xxx_mdio) = {
	.name		= "mv88e6xxx_mdio",
	.id		= UCLASS_MDIO,
	.of_match       = mv88e6xxx_mdio_ids,
	.bind		= mv88e6xxx_mdio_bind,
	.probe		= mv88e6xxx_mdio_probe,
	.ops		= &mv88e6xxx_mdio_ops,
};
