/*
 * LCD Lowlevel Tuning Abstraction
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/lcd_tuning.h>
#include <linux/lcd_ioctl.h>

#define PRIMARY_LCD "pri_lcd"

#define to_lcd_mdevice(obj) container_of(obj, struct lcd_mdevice, mdev)

static long lmdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int r = 0;
	struct miscdevice *mdev = (struct miscdevice *)filp->private_data;
	struct lcd_mdevice *lmdev = container_of(mdev, struct lcd_mdevice, mdev);
	struct lcd_tuning_dev *ltd = 0;
	void __user *ptr = (void __user *)arg;

	union lcd_param
	{
		u32 dgamma;
		u32 cabc;
 /*reason: modified for ACL fouction */ 
		u32 support_acl;
		u32 aclset;
	}par;

	ltd = &lmdev->ltd;

	if(!ltd->ops)
		return -1;

	mutex_lock(&lmdev->ops_lock);

	switch(cmd)
	{
		case LCD_TUNING_DGAMMA:
		{
			if(ltd->ops->set_gamma)
			{
				r = copy_from_user((void *)&par, ptr, sizeof(par.dgamma));
				if(r)
					goto out;
					
				if(par.dgamma)
				{
					if(ltd->props.default_gamma == GAMMA25)
						ltd->ops->set_gamma(ltd, GAMMA22);
					else if(ltd->props.default_gamma == GAMMA22)
						ltd->ops->set_gamma(ltd, GAMMA19);
					else
					{
						r = -EINVAL;
						pr_debug("%s: Unrecognized default gamma value %d\n", __func__, ltd->props.default_gamma);
						goto out;
					}
				}
				else
				{
					ltd->ops->set_gamma(ltd, ltd->props.default_gamma);
				}
			}
		}
		break;
 /*reason: modified for ACL fouction */ 
		case LCD_TUNING_ACL:
		{   
			if(ltd->ops->set_acl)
			{
				r = copy_from_user((void *)&par, ptr, sizeof(par.aclset));
				if(r)
					goto out;
				if(par.aclset)
				{
					ltd->ops->set_acl(ltd, ACL_ON);
				}
				else
				{
					ltd->ops->set_acl(ltd, ACL_OFF);
				}
			}
		}
		break;
        case WHETHER_SUPPROT_ACL: //modified to diff whether support acl
		{   
			if(ltd->props.default_aclsetting == SUPPORT_ACL)
			{
				par.support_acl = SUPPORT_ACL;
				r = copy_to_user(ptr, (void *)&(par.support_acl), sizeof(par.support_acl));
				if(r)
					goto out;
			}
			else if(ltd->props.default_aclsetting == UNSUPPORT_ACL)
			{
    			par.support_acl = UNSUPPORT_ACL;
    			r = copy_to_user(ptr, (void *)&(par.support_acl), sizeof(par.support_acl));
    			if(r)
    				goto out;
			}            
		}
		break;
		case LCD_TUNING_CABC:
		{
			if(ltd->ops->set_cabc)
			{
				r = copy_from_user((void *)&par, ptr, sizeof(par.cabc));
				if(r)
					goto out;
				if(par.cabc)
				{
					ltd->ops->set_cabc(ltd, CABC_VID);
			}
				else
			{
					ltd->ops->set_cabc(ltd, CABC_UI);
				}
			}
		}
		break;
		default:
		{
			r = -EINVAL;
		}
		break;
	}
out:
	mutex_unlock(&lmdev->ops_lock);
	return r;
}


static int lmdev_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations mdev_fops = 
{
	.owner		= THIS_MODULE,
	.open		= lmdev_open,
	.unlocked_ioctl = lmdev_ioctl,
};


static struct lcd_mdevice lmdev = 
{
	.ops_lock = __MUTEX_INITIALIZER(lmdev.ops_lock),
	.mdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = PRIMARY_LCD,
		.mode = 0666,
		.fops = &mdev_fops,
	},
};

struct lcd_tuning_dev *lcd_tuning_dev_register(struct lcd_properities *props, const struct lcd_tuning_ops *lcd_ops, void *devdata)
{
	if(props)
	{
		lmdev.ltd.props = *props;
	}
	else	/* Default to TFT with gamma2.5 */
	{
		lmdev.ltd.props.type = TFT;
		lmdev.ltd.props.default_gamma = GAMMA25;
/*reason: modified for ACL fouction */ 
        lmdev.ltd.props.default_aclsetting = UNSUPPORT_ACL;
	}

	lmdev.ltd.ops = lcd_ops;
	lmdev.ltd.data = devdata;

	return &lmdev.ltd;
}
EXPORT_SYMBOL(lcd_tuning_dev_register);

void lcd_tuning_dev_unregister(struct lcd_tuning_dev *ltd)
{
	if (!ltd)
		return;

	mutex_lock(&lmdev.ops_lock);
	lmdev.ltd.ops = NULL;
	mutex_unlock(&lmdev.ops_lock);
}
EXPORT_SYMBOL(lcd_tuning_dev_unregister);



static void __exit lcd_mdevice_exit(void)
{
	memset((void *)&lmdev, 0, sizeof(struct lcd_mdevice));
	
}

static int __init lcd_mdevice_init(void)
{
	int ret;

	ret = misc_register(&lmdev.mdev);

	if (ret) {
		return -1;
	}
	return 0;
}

/*
 * if this is compiled into the kernel, we need to ensure that the
 * class is registered before users of the class try to register lcd's
 */
module_init(lcd_mdevice_init);
module_exit(lcd_mdevice_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LCD Low-level Tuning Abstraction");
