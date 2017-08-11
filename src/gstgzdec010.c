/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2017 Shawn <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
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

/**
 * SECTION:element-gzdec010
 *
 * gzip decoder that receives a stream compressed with gzip and emits an
 * uncompressed stream.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m filesrc location=file.txt.gz ! gzdec ! filesink location="file.txt"
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstgzdec010.h"
#include "zlibdec.h"

GST_DEBUG_CATEGORY_STATIC (gst_gzdec010_debug);
#define GST_CAT_DEFAULT gst_gzdec010_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

GST_BOILERPLATE (Gstgzdec010, gst_gzdec010, GstElement,
    GST_TYPE_ELEMENT);

static void gst_gzdec010_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_gzdec010_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_gzdec010_set_caps (GstPad * pad, GstCaps * caps);
static GstFlowReturn gst_gzdec010_chain (GstPad * pad, GstBuffer * buf);

/* GObject vmethod implementations */

static void
gst_gzdec010_base_init (gpointer gclass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "gzip decoder",
    "Decoder/File",
    "Receives a stream compressed with gzip and emits an uncompressed stream",
    "Siwon Kang <<kkangshawn@gmail.com>>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the gzdec010's class */
static void
gst_gzdec010_class_init (Gstgzdec010Class * klass)
{
  GObjectClass *gobject_class;

  gobject_class = (GObjectClass *) klass;

  gobject_class->set_property = gst_gzdec010_set_property;
  gobject_class->get_property = gst_gzdec010_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_gzdec010_init (Gstgzdec010 * filter,
    Gstgzdec010Class * gclass)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_setcaps_function (filter->sinkpad,
                                GST_DEBUG_FUNCPTR(gst_gzdec010_set_caps));
  gst_pad_set_getcaps_function (filter->sinkpad,
                                GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));
  gst_pad_set_chain_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_gzdec010_chain));

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  gst_pad_set_getcaps_function (filter->srcpad,
                                GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));

  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
  filter->silent = FALSE;

  init_decoder ();
}

static void
gst_gzdec010_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstgzdec010 *filter = GST_GZDEC010 (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gzdec010_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstgzdec010 *filter = GST_GZDEC010 (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles the link with other elements */
static gboolean
gst_gzdec010_set_caps (GstPad * pad, GstCaps * caps)
{
  Gstgzdec010 *filter;
  GstPad *otherpad;

  filter = GST_GZDEC010 (gst_pad_get_parent (pad));
  otherpad = (pad == filter->srcpad) ? filter->sinkpad : filter->srcpad;
  gst_object_unref (filter);

  return gst_pad_set_caps (otherpad, caps);
}

static GstBuffer *
gst_gzdec_process_data (GstBuffer * buf)
{
  GstBuffer *outbuf = NULL;
  guchar *srcmsg, *decodedmsg = NULL;
  gulong decodedmsglen = 0;

  srcmsg = (guchar *)GST_BUFFER_DATA (buf);

  g_print ("Source message: %s", srcmsg);
  decode_message (srcmsg, GST_BUFFER_SIZE (buf), &decodedmsg, &decodedmsglen);

  g_print ("Decoded message: %s(%lu)", decodedmsg, decodedmsglen);
  outbuf = gst_buffer_new ();
  GST_BUFFER_SIZE (outbuf) = decodedmsglen;
  gst_buffer_set_data (outbuf, decodedmsg, decodedmsglen);

  return outbuf;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_gzdec010_chain (GstPad * pad, GstBuffer * buf)
{
  Gstgzdec010 *filter;
  GstBuffer *outbuf;

  filter = GST_GZDEC010 (GST_OBJECT_PARENT (pad));

  if (filter->silent == FALSE)
    g_print ("Have data of size %u bytes!\n",
                    GST_BUFFER_SIZE (buf));
  outbuf = gst_gzdec_process_data (buf);
  gst_buffer_unref (buf);
  if (!outbuf) {
    GST_ELEMENT_ERROR (GST_ELEMENT (filter), STREAM, FAILED, (NULL), (NULL));
    return GST_FLOW_ERROR;
  }

  /* just push out the incoming buffer without touching it */
  return gst_pad_push (filter->srcpad, outbuf);
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
gzdec010_init (GstPlugin * gzdec010)
{
  /* debug category for fltering log messages */
  GST_DEBUG_CATEGORY_INIT (gst_gzdec010_debug, "gzdec",
      0, "gzip decoder plugin");

  return gst_element_register (gzdec010, "gzdec", GST_RANK_NONE,
      GST_TYPE_GZDEC010);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstgzdec010"
#endif

/* gstreamer looks for this structure to register gzdec010s
 *
 * exchange the string 'Template gzdec010' with your gzdec010 description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "gzdec",
    "gzip decoder plugin",
    gzdec010_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
