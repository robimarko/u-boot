// SPDX-License-Identifier: GPL-2.0-or-later

#include <asm/gpio.h>

#ifndef _MV88E6XXX_H
#define _MV88E6XXX_H

/* Port registers */
/* Offset 0x03: Switch Identifier Register */
#define MV88E6XXX_PORT_SWITCH_ID		0x3
#define MV88E6XXX_PORT_SWITCH_ID_PROD_MASK	GENMASK(15, 4)
#define MV88E6XXX_PORT_SWITCH_ID_REV_MASK	GENMASK(3, 0)

/* Global 2 registers */
/* Offset 0x18: SMI PHY Command Register */
#define MV88E6XXX_G2_SMI_PHY_CMD			0x18
#define MV88E6XXX_G2_SMI_PHY_CMD_BUSY			BIT(15)
#define MV88E6390_G2_SMI_PHY_CMD_FUNC_MASK		GENMASK(14, 13)
#define MV88E6390_G2_SMI_PHY_CMD_FUNC_INTERNAL		0
#define MV88E6390_G2_SMI_PHY_CMD_FUNC_EXTERNAL		1
#define MV88E6390_G2_SMI_PHY_CMD_FUNC_SETUP		2
#define MV88E6XXX_G2_SMI_PHY_CMD_MODE_MASK		BIT(12)
#define MV88E6XXX_G2_SMI_PHY_CMD_MODE_45		0
#define MV88E6XXX_G2_SMI_PHY_CMD_MODE_22		1
#define MV88E6XXX_G2_SMI_PHY_CMD_OP_MASK		GENMASK(11, 10)
#define MV88E6XXX_G2_SMI_PHY_CMD_OP_22_WRITE_DATA	1
#define MV88E6XXX_G2_SMI_PHY_CMD_OP_22_READ_DATA	2
#define MV88E6XXX_G2_SMI_PHY_CMD_OP_45_WRITE_ADDR	0
#define MV88E6XXX_G2_SMI_PHY_CMD_OP_45_WRITE_DATA	1
#define MV88E6XXX_G2_SMI_PHY_CMD_OP_45_READ_DATA_INC	2
#define MV88E6XXX_G2_SMI_PHY_CMD_OP_45_READ_DATA	3
#define MV88E6XXX_G2_SMI_PHY_CMD_DEV_ADDR_MASK		GENMASK(9, 5)
#define MV88E6XXX_G2_SMI_PHY_CMD_REG_ADDR_MASK		GENMASK(4, 0)

/* Offset 0x19: SMI PHY Data Register */
#define MV88E6XXX_G2_SMI_PHY_DATA	0x19

/* Offset 0x1A: Scratch and Misc. Register */
#define MV88E6XXX_G2_SCRATCH_MISC_MISC		0x1a
#define MV88E6XXX_G2_SCRATCH_MISC_UPDATE	BIT(15)
#define MV88E6XXX_G2_SCRATCH_MISC_REG_MASK	GENMASK(14, 8)
#define MV88E6XXX_G2_SCRATCH_MISC_DATA_MASK	GENMASK(7, 0)
#define MV88E6XXX_G2_SCRATCH_MISC_CONFIG	0x2
#define MV88E6XXX_G2_SCRATCH_MISC_SMI_PHY	BIT(7)

struct mv88e6xxx_priv {
#if CONFIG_IS_ENABLED(DM_GPIO)
	struct gpio_desc reset_gpio;
#endif
	struct udevice *mdio_device;
	int sw_addr;
	int port_base_addr;
	unsigned int global1_addr;
	unsigned int global2_addr;
};

int mv88e6xxx_g2_read(struct mv88e6xxx_priv *priv, int reg);
int mv88e6xxx_g2_write(struct mv88e6xxx_priv *priv, int reg, u16 val);

#endif /* _MV88E6XXX_H */
