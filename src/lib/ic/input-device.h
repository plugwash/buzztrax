/* Buzztrax
 * Copyright (C) 2007 Buzztrax team <buzztrax-devel@buzztrax.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BTIC_INPUT_DEVICE_H
#define BTIC_INPUT_DEVICE_H

#include <glib.h>
#include <glib-object.h>

/* type macros */

#define BTIC_TYPE_INPUT_DEVICE            (btic_input_device_get_type ())
#define BTIC_INPUT_DEVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BTIC_TYPE_INPUT_DEVICE, BtIcInputDevice))
#define BTIC_INPUT_DEVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BTIC_TYPE_INPUT_DEVICE, BtIcInputDeviceClass))
#define BTIC_IS_INPUT_DEVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BTIC_TYPE_INPUT_DEVICE))
#define BTIC_IS_INPUT_DEVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BTIC_TYPE_INPUT_DEVICE))
#define BTIC_INPUT_DEVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BTIC_TYPE_INPUT_DEVICE, BtIcInputDeviceClass))

typedef struct _BtIcInputDevice BtIcInputDevice;
typedef struct _BtIcInputDeviceClass BtIcInputDeviceClass;
typedef struct _BtIcInputDevicePrivate BtIcInputDevicePrivate;

/**
 * BtIcInputDevice:
 *
 * buzztraxs interaction controller registry
 */
struct _BtIcInputDevice {
  const BtIcDevice parent;
  
  /*< private >*/
  BtIcInputDevicePrivate *priv;
};

struct _BtIcInputDeviceClass {
  const BtIcDeviceClass parent;
  
};

GType btic_input_device_get_type(void) G_GNUC_CONST;

BtIcInputDevice *btic_input_device_new(const gchar *udi,const gchar *name,const gchar *devnode);

#endif // BTIC_INPUT_DEVICE_H
