
/*********************************************************
CAUTION : This file is Auto Generated by Perl based on *.xls
		so,don't modify this file manually!
*********************************************************/


#ifndef U9200EV3_POWER_TREE_H
#define U9200EV3_POWER_TREE_H

#include <linux/i2c/twl.h>
#include <linux/i2c.h>
#include <linux/regulator/machine.h>

#include <plat/config_twl6030_default.h>
#include <hsad/config_general_struct.h>
#include <hsad/plat_power_mux.h>

static struct regulator_consumer_supply u9200ev3_twl6030_v1v29_supply[] = {

};
static struct regulator_init_data u9200ev3_twl6030_v1v29 = {
	.constraints = { 
		.min_uV =1200000,
		.max_uV =1200000,
		.apply_uV =true,
		.always_on =true,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = true,
			.disabled = false,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_v1v29_supply),
	.consumer_supplies = u9200ev3_twl6030_v1v29_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_v1v8_supply[] = {

};
static struct regulator_init_data u9200ev3_twl6030_v1v8 = {
	.constraints = { 
		.min_uV =1500000,
		.max_uV =1800000,
		.apply_uV =true,
		.always_on =true,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = true,
			.disabled = false,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_v1v8_supply),
	.consumer_supplies = u9200ev3_twl6030_v1v8_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_v2v1_supply[] = {
	{
		.supply = "BT_VDD_2V1",
	},
};
static struct regulator_init_data u9200ev3_twl6030_v2v1 = {
	.constraints = { 
		.min_uV =1800000,
		.max_uV =2100000,
		.apply_uV =true,
		.always_on =true,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_v2v1_supply),
	.consumer_supplies = u9200ev3_twl6030_v2v1_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vana_supply[] = {

};
static struct regulator_init_data u9200ev3_twl6030_vana = {
	.constraints = { 
		.min_uV =2100000,
		.max_uV =2100000,
		.apply_uV =true,
		.always_on =true,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vana_supply),
	.consumer_supplies = u9200ev3_twl6030_vana_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vaux1_supply[] = {
	{
		.supply = "COMPASS_VDD_SENSOR",
	},
	{
		.supply = "ADI_ACCELEROMETER_VDD_SENSOR",
	},
	{
		.supply = "ST_ACCELEROMETER_VDD_SENSOR",
	},
	{
		.supply = "FS_ACCELEROMETER_VDD_SENSOR",
	},
	{
		.supply = "GYROSCOPE_VDD_SENSOR",
	},
	{
		.supply = "PROXIMITY_LIGHT_VDD_SENSOR",
	},
};
static struct regulator_init_data u9200ev3_twl6030_vaux1 = {
	.constraints = { 
		.min_uV =1000000,
		.max_uV =3000000,
		.apply_uV =true,
		.always_on =false,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vaux1_supply),
	.consumer_supplies = u9200ev3_twl6030_vaux1_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vaux2_supply[] = {
	{
		.supply = "vaux2",
	},
	{
		.supply = "av-switch",
		.dev_name = "soc-audio",
	},
	{
		.supply = "atmel_ts_power",
	},
	{
		.supply = "synaptics_ts_power",
	},
	{
		.supply = "LCD_LCD_VCI",
	},
	{
		.supply = "TOUCH_SCREEN_TP_AVDD",
	},
};
static struct regulator_init_data u9200ev3_twl6030_vaux2 = {
	.constraints = { 
		.min_uV =1200000,
		.max_uV =3000000,
		.apply_uV =true,
		.always_on =false,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = true,
			.disabled = false,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vaux2_supply),
	.consumer_supplies = u9200ev3_twl6030_vaux2_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vaux3_supply[] = {
	{
		.supply = "cam2pwr",
	},
	{
		.supply = "MOTOR_MOTOR",
	},
};
static struct regulator_init_data u9200ev3_twl6030_vaux3 = {
	.constraints = { 
		.min_uV =1000000,
		.max_uV =3000000,
		.apply_uV =true,
		.always_on =false,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vaux3_supply),
	.consumer_supplies = u9200ev3_twl6030_vaux3_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vcore1_supply[] = {

};
static struct regulator_init_data u9200ev3_twl6030_vcore1 = {
	.constraints = { 
		.min_uV =1200000,
		.max_uV =1200000,
		.apply_uV =true,
		.always_on =true,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vcore1_supply),
	.consumer_supplies = u9200ev3_twl6030_vcore1_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vcore2_supply[] = {

};
static struct regulator_init_data u9200ev3_twl6030_vcore2 = {
	.constraints = { 
		.min_uV =1200000,
		.max_uV =1200000,
		.apply_uV =true,
		.always_on =true,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vcore2_supply),
	.consumer_supplies = u9200ev3_twl6030_vcore2_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vcore3_supply[] = {

};
static struct regulator_init_data u9200ev3_twl6030_vcore3 = {
	.constraints = { 
		.min_uV =600000,
		.max_uV =2100000,
		.apply_uV =true,
		.always_on =false,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vcore3_supply),
	.consumer_supplies = u9200ev3_twl6030_vcore3_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vcxio_supply[] = {
	{
		.supply = "vdds_dsi",
		.dev_name = "omapdss_dss",
	},
	{
		.supply = "vdds_dsi",
		.dev_name = "omapdss_dsi1",
	},
};
static struct regulator_init_data u9200ev3_twl6030_vcxio = {
	.constraints = { 
		.min_uV =1800000,
		.max_uV =1800000,
		.apply_uV =true,
		.always_on =true,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vcxio_supply),
	.consumer_supplies = u9200ev3_twl6030_vcxio_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vdac_supply[] = {
	{
		.supply = "hdmi_vref",
	},
	{
		.supply = "BT_VDD_ANA",
	},
};
static struct regulator_init_data u9200ev3_twl6030_vdac = {
	.constraints = { 
		.min_uV =1800000,
		.max_uV =1800000,
		.apply_uV =true,
		.always_on =true,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vdac_supply),
	.consumer_supplies = u9200ev3_twl6030_vdac_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vmem_supply[] = {

};
static struct regulator_init_data u9200ev3_twl6030_vmem = {
	.constraints = { 
		.min_uV =1200000,
		.max_uV =1200000,
		.apply_uV =true,
		.always_on =true,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = true,
			.disabled = false,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vmem_supply),
	.consumer_supplies = u9200ev3_twl6030_vmem_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vmmc_supply[] = {
	{
		.supply = "vmmc",
		.dev_name = "omap_hsmmc.0",
	},
};
static struct regulator_init_data u9200ev3_twl6030_vmmc = {
	.constraints = { 
		.min_uV =1200000,
		.max_uV =3000000,
		.apply_uV =true,
		.always_on =false,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vmmc_supply),
	.consumer_supplies = u9200ev3_twl6030_vmmc_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vpp_supply[] = {

};
static struct regulator_init_data u9200ev3_twl6030_vpp = {
	.constraints = { 
		.min_uV =1800000,
		.max_uV =2500000,
		.apply_uV =true,
		.always_on =false,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vpp_supply),
	.consumer_supplies = u9200ev3_twl6030_vpp_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vusb_supply[] = {

};
static struct regulator_init_data u9200ev3_twl6030_vusb = {
	.constraints = { 
		.min_uV =3300000,
		.max_uV =3300000,
		.apply_uV =true,
		.always_on =false,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vusb_supply),
	.consumer_supplies = u9200ev3_twl6030_vusb_supply,
};

static struct regulator_consumer_supply u9200ev3_twl6030_vusim_supply[] = {
	{
		.supply = "PMU_VPP",
	},
};
static struct regulator_init_data u9200ev3_twl6030_vusim = {
	.constraints = { 
		.min_uV =1200000,
		.max_uV =2900000,
		.apply_uV =true,
		.always_on =false,
		.valid_modes_mask =REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
		.valid_ops_mask =REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE | REGULATOR_CHANGE_STATUS,
		.state_mem = { 
			.enabled = false,
			.disabled = true,
		},
	}, 
	.num_consumer_supplies = ARRAY_SIZE(u9200ev3_twl6030_vusim_supply),
	.consumer_supplies = u9200ev3_twl6030_vusim_supply,
};


static struct twl4030_platform_data u9200ev3_twl6030_twldata = {
	.irq_base	= TWL6030_IRQ_BASE,
	.irq_end	= TWL6030_IRQ_END,


	.usb		= &omap4_usbphy_data,
	.clk32kg        = &twl6030_clk32kg,
	.madc           = &twl6030_madc,
	.bci            = &twl6030_bci_data,
	.codec          = &twl6040_codec,


	.vaux1		= &u9200ev3_twl6030_vaux1,
	.vana		= &u9200ev3_twl6030_vana,
	.v2v1		= &u9200ev3_twl6030_v2v1,
	.vcxio		= &u9200ev3_twl6030_vcxio,
	.vmem		= &u9200ev3_twl6030_vmem,
	.vcore3		= &u9200ev3_twl6030_vcore3,
	.vusim		= &u9200ev3_twl6030_vusim,
	.vcore1		= &u9200ev3_twl6030_vcore1,
	.v1v8		= &u9200ev3_twl6030_v1v8,
	.vaux2		= &u9200ev3_twl6030_vaux2,
	.vcore2		= &u9200ev3_twl6030_vcore2,
	.v1v29		= &u9200ev3_twl6030_v1v29,
	.vmmc		= &u9200ev3_twl6030_vmmc,
	.vdac		= &u9200ev3_twl6030_vdac,
	.vaux3		= &u9200ev3_twl6030_vaux3,
	.vusb		= &u9200ev3_twl6030_vusb,
	.vpp		= &u9200ev3_twl6030_vpp,
};


static struct i2c_board_info u9200ev3_twl6030_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("twl6030", 0x48),
		.flags = I2C_CLIENT_WAKE,
		.irq = OMAP44XX_IRQ_SYS_1N,
		.platform_data = &u9200ev3_twl6030_twldata,
	},
};


static struct power_voltage_table u9200ev3_config_powervalue[] ={
	{"FS_ACCELEROMETER_VDD_SENSOR"	,2600000},
	{"ST_ACCELEROMETER_VDD_SENSOR"	,2600000},
	{"BT_VDD_ANA"	,1800000},
	{"BT_VDD_2V1"	,2100000},
	{"PROXIMITY_LIGHT_VDD_SENSOR"	,2600000},
	{"PMU_VPP"	,1800000},
	{"COMPASS_VDD_SENSOR"	,2600000},
	{"GYROSCOPE_VDD_SENSOR"	,2600000},
	{"LCD_LCD_VCI"	,3000000},
	{"ADI_ACCELEROMETER_VDD_SENSOR"	,2600000},
	{"TOUCH_SCREEN_TP_AVDD"	,3000000},
	{"MOTOR_MOTOR"	,3000000},
    {"" , 0},
};


static power_tree_config u9200ev3_config_powertree[] ={
	{u9200ev3_twl6030_i2c_boardinfo, u9200ev3_config_powervalue},
};


struct board_id_general_struct u9200ev3_power_tree ={
		.name=POWER_TREE_MODULE_NAME,
		.board_id=BOARD_ID_U9200EV3,
		.data_array={.config_powerconf_ptr=u9200ev3_config_powertree},
		.list={NULL,NULL},
};

#endif