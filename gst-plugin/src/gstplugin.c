/* 
 * 
 * GStreamer
 * Copyright (C) 1999-2001 Erik Walthinsen <omega@cse.ogi.edu>
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <gst/control/control.h>
#include "gstplugin.h"



static GstElementDetails plugin_details = {
  "Plugin",
  "Generic/Plugin",
  "Generic Template Plugin",
  VERSION,
  "Thomas Vander Stichele <thomas@apestaart.org>",
  "(C) 2002"
};


/* Filter signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_SILENT,
};

GST_PAD_TEMPLATE_FACTORY (plugin_sink_factory,
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  NULL		/* no caps */
);

GST_PAD_TEMPLATE_FACTORY (plugin_src_factory,
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  NULL		/* no caps */
);

static void	plugin_class_init	(GstPluginClass *klass);
static void	plugin_init		(GstPlugin *filter);

static void	plugin_set_property	(GObject *object, guint prop_id, 
                                         const GValue *value, 
					 GParamSpec *pspec);
static void	plugin_get_property     (GObject *object, guint prop_id, 
                                         GValue *value, GParamSpec *pspec);

static void	plugin_update_plugin    (const GValue *value, gpointer data);
static void	plugin_update_mute      (const GValue *value, gpointer data);

static void	plugin_chain	(GstPad *pad, GstBuffer *buf);

static GstElementClass *parent_class = NULL;

/* this function handles the connection with other plug-ins */
static GstPadConnectReturn
plugin_connect (GstPad *pad, GstCaps *caps)
{
  GstPlugin *filter;
  GstPad *otherpad;
  
  filter = GST_PLUGIN (gst_pad_get_parent (pad));
  g_return_val_if_fail (filter != NULL, GST_PAD_CONNECT_REFUSED);
  g_return_val_if_fail (GST_IS_PLUGIN (filter), GST_PAD_CONNECT_REFUSED);
  otherpad = (pad == filter->srcpad ? filter->sinkpad : filter->srcpad);
  
  if (GST_CAPS_IS_FIXED (caps)) 
  {
    /* caps are not fixed, so try to connect on the other side and see if
     * that works */

    if (!gst_pad_try_set_caps (otherpad, caps))
      return GST_PAD_CONNECT_REFUSED;

    /* caps on other side were accepted, so we're ok */
    return GST_PAD_CONNECT_OK;
  }
  /* not enough information yet, delay negotation */ 
  return GST_PAD_CONNECT_DELAYED;
}

GType
gst_plugin_get_type (void) 
{
  static GType plugin_type = 0;

  if (!plugin_type) 
  {
    static const GTypeInfo plugin_info = 
    {
      sizeof (GstPluginClass),
      NULL,
      NULL,
      (GClassInitFunc) plugin_class_init,
      NULL,
      NULL,
      sizeof (GstPlugin),
      0,
      (GInstanceInitFunc) plugin_init,
    };
    plugin_type = g_type_register_static (GST_TYPE_ELEMENT, "GstPlugin", 
	                                  &plugin_info, 0);
  }
  return plugin_type;
}

static void
plugin_class_init (GstPluginClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*) klass;
  gstelement_class = (GstElementClass*) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_MUTE,
    g_param_spec_boolean ("silent", "silent", "silent",
                          FALSE, G_PARAM_READWRITE));
  
  gobject_class->set_property = plugin_set_property;
  gobject_class->get_property = plugin_get_property;
}

/* initialize the new plug-in
 * instantiate pads and add them to plug-in
 * set functions
 * initialize structure
 */
static void
plugin_init (GstPlugin *filter)
{
  filter->sinkpad = gst_pad_new_from_template (plugin_sink_factory (), "sink");
  gst_pad_set_connect_function (filter->sinkpad, plugin_connect);
  filter->srcpad = gst_pad_new_from_template (plugin_src_factory (), "src");
  gst_pad_set_connect_function (filter->srcpad, plugin_connect);
  
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
  gst_pad_set_chain_function (filter->sinkpad, plugin_chain);
  filter->silent = FALSE;

}

/* chain function
 * this function does the actual processing
 */

static void
plugin_chain (GstPad *pad, GstBuffer *buf)
{
  GstPlugin *filter;
  GstBuffer *out_buf;
  gfloat *data;
  gint i, num_samples;

  g_return_if_fail (GST_IS_PAD (pad));
  g_return_if_fail (buf != NULL);
  
  filter = GST_PLUGIN (GST_OBJECT_PARENT (pad));
  g_return_if_fail (GST_IS_PLUGIN (filter));

  if (filter->silent == FALSE)
    g_print ("I'm plugged, therefore I'm in.\n");

  /* just push out the incoming buffer without touching it */
  gst_pad_push (filter->srcpad, buf);
}

static void
plugin_set_property (GObject *object, guint prop_id, 
                     const GValue *value, GParamSpec *pspec)
{
  GstPlugin *filter;

  g_return_if_fail (GST_IS_PLUGIN (object));
  filter = GST_PLUGIN (object);

  switch (prop_id) 
  {
  case ARG_SILENT:
    filter->silent = g_value_get_boolean (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
plugin_get_property (GObject *object, guint prop_id, 
                     GValue *value, GParamSpec *pspec)
{
  GstPlugin *filter;

  g_return_if_fail (GST_IS_PLUGIN (object));
  filter = GST_PLUGIN (object);
  
  switch (prop_id) {
  case ARG_SILENT:
    g_value_set_boolean (value, filter->silent);
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

/* initialize the plugin
 * register the element factories and pad templates
 * register the features 
 */
static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  GstElementFactory *factory;

  factory = gst_element_factory_new ("plugin", GST_TYPE_PLUGIN,
                                     &plugin_details);
  g_return_val_if_fail (factory != NULL, FALSE);
  
  gst_element_factory_add_pad_template (factory, plugin_src_factory ());
  gst_element_factory_add_pad_template (factory, plugin_sink_factory ());

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  /* plugin initialisation succeeded */
  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "plugin",
  plugin_init
};