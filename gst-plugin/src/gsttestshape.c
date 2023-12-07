/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2023 Vinay Kesarwani <<user@hostname.org>>
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
 * SECTION:element-testshape
 *
 * FIXME:Describe testshape here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! testshape ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include "gsttestshape.h"

/* Enum to define a set of properties for plugin. */
enum
{
	PROP_0,
	PROP_SHAPE,
	PROP_COLOR,
	PROP_X,
	PROP_Y,
	PROP_COORDINATE,
	PROP_RECT_L,
	PROP_RECT_H,
	PROP_TRIANGLE_SIZE,
	PROP_SQUARE_SIZE,
	PROP_CIRCLE_RADIUS,
};

/* Capabilities of input and output pads */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, format=(string)NV12, width=(int)[1, 3840], height=(int)[1, 2160], framerate=(fraction)[0/1, 30/1]")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, format=(string)NV12, width=(int)[1, 3840], height=(int)[1, 2160], framerate=(fraction)[0/1, 30/1]")
    );

/* Defines a new GObject-derived type with name Gsttestshape that inherits from the GST_TYPE_ELEMENT type. */ 
G_DEFINE_TYPE (Gsttestshape, gst_testshape, GST_TYPE_ELEMENT);

/* To register an element called as testshape. */
GST_ELEMENT_REGISTER_DEFINE (testshape, "testshape", GST_RANK_NONE, GST_TYPE_TESTSHAPE);

/* Declaration of _set _get and chain function. */
static void gst_testshape_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_testshape_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
static GstFlowReturn gst_testshape_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);

/* gst_testshape_class_init function used to initialize class for Gsttestshape element.
 * It is used to configure the behavior of the Gsttestshape class when it's instantiated 
 * and used within GStreamer pipelines. 
 */
static void gst_testshape_class_init (GsttestshapeClass * klass)    //sets class structure for Gsttestshape element
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;
    
    gobject_class = (GObjectClass *) klass;
    gstelement_class = (GstElementClass *) klass;
    
    gobject_class->set_property = gst_testshape_set_property;
    gobject_class->get_property = gst_testshape_get_property;
    
    /* g_object_class_install_property is called to install properties for testshape element. */
    g_object_class_install_property(gobject_class, PROP_SHAPE,
    	g_param_spec_string("shape", "Shape", "SHAPE: Rectangle, Triangle, Square, Circle", "rectangle", G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, PROP_COLOR,
        g_param_spec_string("color", "Color", "COLOR: Red, Green, Blue", "blue", G_PARAM_READWRITE));
    
    g_object_class_install_property(gobject_class, PROP_X,
        g_param_spec_int("x", "x", "X-corodinate", INT_MIN, INT_MAX, 200, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, PROP_Y,
        g_param_spec_int("y", "y", "Y-corodinate", INT_MIN, INT_MAX, 200, G_PARAM_READWRITE));
    
    g_object_class_install_property (gobject_class, PROP_COORDINATE,
        g_param_spec_boolean ("coordinate", "Coordinate", "set when need to pass the coordinate for shape", FALSE, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, PROP_RECT_L,
        g_param_spec_int("rect_l", "Rectangle_length", "length of rectangle", INT_MIN, INT_MAX, 200, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, PROP_RECT_H,
        g_param_spec_int("rect_h", "Rectangle_height", "height of rectangle", INT_MIN, INT_MAX, 100, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, PROP_TRIANGLE_SIZE,
        g_param_spec_int("triangle_size", "Triangle_Size", "Size of Triangle", INT_MIN, INT_MAX, 200, G_PARAM_READWRITE));
    
    g_object_class_install_property(gobject_class, PROP_SQUARE_SIZE,
        g_param_spec_int("square_size", "Square_size", "Size of Square ", INT_MIN, INT_MAX, 200, G_PARAM_READWRITE));
    
    g_object_class_install_property(gobject_class, PROP_CIRCLE_RADIUS,
        g_param_spec_int("circle_radius", "Circle_radius", "Radius of Circle", INT_MIN, INT_MAX, 100, G_PARAM_READWRITE));

	/* Used to set details of Gsttestshape element.
	 * It is used to provide essential information about the element's properties, 
	 * signals, and other characteristics.
	 */
    gst_element_class_set_details_simple (gstelement_class,
    	"testshape",
   	   	"FIXME:Generic",
   		"This plugin is used to draw different shapes like square, circle rectangle, etc                                                        with different colors such as red, green and blue", 
   		"Vinay Kesarwani vinay.kesarwani@softnautics.com");

	/* This is used to add pad templates to source and sink of Gsttestshape element. */
    gst_element_class_add_pad_template (gstelement_class, gst_static_pad_template_get (&src_factory));
    gst_element_class_add_pad_template (gstelement_class, gst_static_pad_template_get (&sink_factory));
}

/* gst_testshape_init function initializes the instance-specific data, 
 * sets default property values, and creates and adds input and output pads 
 * to Gsttestshape element
 */
static void gst_testshape_init (Gsttestshape * filter)
{
    /* Create new pads from the static pad templates*/
    filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
    
    /* gst_pad_set_chain_function function sets the chain function for the sinkpad.*/
    gst_pad_set_chain_function (filter->sinkpad, GST_DEBUG_FUNCPTR (gst_testshape_chain));
    
    /* Used to set proxy caps on a sink pad of an element.
     * Enables dynamic negotiation of data formats between elements in pipeline.
     */
    GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
    
    filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
    GST_PAD_SET_PROXY_CAPS (filter->srcpad);
	
	/* It add pads to Gsttestshape element.
	 * Allows flow of data in and out of element through pads.
	 */
    gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);
    gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
    
    /* Properties of an element which can be set by the user to control behavior.*/
    strcpy(filter->shape, "square");
    strcpy(filter->color, "red");
    filter->x = 200;
    filter->y = 200;
    filter->coordinate = FALSE;
    filter->rect_l = 200;
    filter->rect_h = 100;
    filter->triangle_size = 200;
    filter->square_size = 200;
    filter->circle_radius = 100;
    
}

/* gst_testshape_set_property funtion sets property of Gsttestshape element. */
static void gst_testshape_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec)
{
    Gsttestshape *filter = GST_TESTSHAPE (object);
    
    /* To check prop_id value and determine which property is being set. */
    switch (prop_id) {
        case PROP_SHAPE:
            strcpy(filter->shape, g_value_get_string(value));
            break;
        case PROP_COLOR:
            strcpy(filter->color, g_value_get_string(value));
            break;
        case PROP_X:
            filter->x = g_value_get_int(value);
            break;
        case PROP_Y:
            filter->y = g_value_get_int(value);
            break;
        case PROP_COORDINATE:
            filter->coordinate = g_value_get_boolean (value);
            break;
        case PROP_RECT_L:
            filter->rect_l = g_value_get_int(value);
            break;
        case PROP_RECT_H:
            filter->rect_h = g_value_get_int(value);
            break;
        case PROP_TRIANGLE_SIZE:
            filter->triangle_size = g_value_get_int(value);
            break;
        case PROP_SQUARE_SIZE:
            filter->square_size = g_value_get_int(value);
            break;
        case PROP_CIRCLE_RADIUS:
            filter->circle_radius = g_value_get_int(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

/* Called after setting Gsttestshape element property to get properrty. */
static void gst_testshape_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
    Gsttestshape *filter = GST_TESTSHAPE (object);
    
    /* To check prop_id value and determine which property is being retrieved.*/
    switch (prop_id) {
        case PROP_SHAPE:
            g_value_set_string(value, filter->shape);
            break;
        case PROP_COLOR:
            g_value_set_string(value, filter->color);
            break;
        case PROP_X:
            g_value_set_int(value, filter->x);
            break;
        case PROP_Y:
            g_value_set_int(value, filter->y);
            break;
        case PROP_COORDINATE:
            g_value_set_boolean (value, filter->coordinate);
            break;
        case PROP_RECT_L:
            g_value_set_int(value, filter->rect_l);
            break;
        case PROP_RECT_H:
            g_value_set_int(value, filter->rect_h);
            break;
        case PROP_TRIANGLE_SIZE:
            g_value_set_int(value, filter->triangle_size);
            break;
        case PROP_SQUARE_SIZE:
            g_value_set_int(value, filter->square_size);
            break;
        case PROP_CIRCLE_RADIUS:
            g_value_set_int(value, filter->circle_radius);
            break;
		default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

/* Function to draw SQUARE shape. */
static void drawSquare(GstVideoFrame *frame, gint x1, gint y1, gint size, guint8 *y_data, guint8 *u_data, guint8 *v_data, gchar *color, gint square_x1, gint square_y1, gint square_x2, gint square_y2) {

	/* examines the position of each pixel in the frame and 
	 * checks whether it falls within the boundaries of the square.
	 */
    if (((x1 == square_x1 || x1 == square_x2 - 1) && (y1 >= square_y1 && y1 < square_y2)) ||
        ((y1 == square_y1 || y1 == square_y2 - 1) && (x1 >= square_x1 && x1 < square_x2))) {
        /* pixel values are set to red. */
        if (strcmp(color, "red") == 0) {
            *y_data = 76;
            *u_data = 84;
            *v_data = 255;
        } 
        /* pixel values are set to green. */
        else if (strcmp(color, "green") == 0) {
            *y_data = 149;
            *u_data = 43;
            *v_data = 21;
        } 
        /* pixel values are set to blue. */
        else if (strcmp(color, "blue") == 0) {
            *y_data = 29;
            *u_data = 255;
            *v_data = 107;
        }
    }
}

/* Function to draw CIRCLE shape. */
static void drawCircle(GstVideoFrame *frame, gint x1, gint y1, gint radius, guint8 *y_data, guint8 *u_data, guint8 *v_data, gchar *color, gint circle_x, gint circle_y){
    gint distance_from_center_squared = (x1 - circle_x) * (x1 - circle_x) + (y1 - circle_y) * (y1 - circle_y);
    
    /* examines the position of each pixel in the frame and 
	 * checks whether it falls within the boundaries of the circle.
	 */
    if (distance_from_center_squared <= radius * radius &&
        distance_from_center_squared >= (radius - 1) * (radius - 1)) {
        /* pixel values are set to red. */
        if (strcmp(color, "red") == 0) {
            *y_data = 76;
            *u_data = 84;
            *v_data = 255;
        } 
        /* pixel values are set to green. */
        else if (strcmp(color, "green") == 0) {
            *y_data = 149;
            *u_data = 43;
            *v_data = 21;
        } 
        /* pixel values are set to blue. */
        else if (strcmp(color, "blue") == 0) {
            *y_data = 29;
            *u_data = 255;
            *v_data = 107;
        }
    }
}

/* Function to draw RECTANGLE shape. */
static void drawRectangle(GstVideoFrame *frame, gint x1, gint y1, gint width, gint height, guint8 *y_data, guint8 *u_data, guint8 *v_data, gchar *color, gint rect_x1, gint rect_y1, gint rect_x2, gint rect_y2){
    
    /* examines the position of each pixel in the frame and 
	 * checks whether it falls within the boundaries of the rectangle.
	 */
    if (((x1 == rect_x1 || x1 == rect_x2 - 1) && (y1 >= rect_y1 && y1 < rect_y2)) ||
        ((y1 == rect_y1 || y1 == rect_y2 - 1) && (x1 >= rect_x1 && x1 < rect_x2))) {
        /* pixel values are set to red. */
        if (strcmp(color, "red") == 0) {
            *y_data = 76;
            *u_data = 84;
            *v_data = 255;
        } 
        /* pixel values are set to green. */
        else if (strcmp(color, "green") == 0) {
            *y_data = 149;
            *u_data = 43;
            *v_data = 21;
        } 
        /* pixel values are set to blue. */
        else if (strcmp(color, "blue") == 0) {
            *y_data = 29;
            *u_data = 255;
            *v_data = 107;
        }
    }
}

/* Function to draw TRIANGLE shape. */
static void drawTriangle(GstVideoFrame *frame, gint x1, gint y1, gint size, guint8 *y_data, guint8 *u_data, guint8 *v_data, gchar *color, gint triangle_x1, gint triangle_y1, gint triangle_x2, gint triangle_y2, gint triangle_x3){
    
    /* examines the position of each pixel in the frame and 
	 * checks whether it falls within the boundaries of the traingle.
	 */
    if ((y1 >= triangle_y1 && y1 <= triangle_y2) &&
        ((x1 <= triangle_x1 && x1 >= triangle_x2) || (x1 >= triangle_x1 && x1 <= triangle_x3))) {
        if ((x1 + y1 == triangle_y1 + triangle_x1) || (x1 - y1 == triangle_x1 - triangle_y1)) {
            /* pixel values are set to red. */
            if (strcmp(color, "red") == 0) {
                *y_data = 76;
                *u_data = 84;
                *v_data = 255;
            } 
            /* pixel values are set to green. */
            else if (strcmp(color, "green") == 0) {
                *y_data = 149;
                *u_data = 43;
                *v_data = 21;
            } 
            /* pixel values are set to blue. */
            else if (strcmp(color, "blue") == 0) {
                *y_data = 29;
                *u_data = 255;
                *v_data = 107;
            }
        }
    }
	
	/* examines the position of each pixel in the frame and 
	 * checks whether it falls within the boundaries of the triangle.
	 */
    if (y1 == triangle_y2) {
        if (x1 >= triangle_x2 && x1 <= triangle_x3) {
            /* pixel values are set to red. */
            if (strcmp(color, "red") == 0) {
                *y_data = 76;
                *u_data = 84;
                *v_data = 255;
            } 
            /* pixel values are set to green. */
            else if (strcmp(color, "green") == 0) {
                *y_data = 149;
                *u_data = 43;
                *v_data = 21;
            } 
            /* pixel values are set to blue. */
            else if (strcmp(color, "blue") == 0) {
                *y_data = 29;
                *u_data = 255;
                *v_data = 107;
            }
        }
    }
}

/* gst_testshape_chain function is used to process actual data in sink pad.
 * In this, it processes the video frames and is responsible to apply visual effects to video frames.
 */
static GstFlowReturn gst_testshape_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
    Gsttestshape *filter;
    GstCaps *caps;
    GstVideoInfo in_info;
    GstVideoFrame frame;
    gint x1, y1;
    
    guint8 *y_data, *uv_data;
    gint width, height, rowstride, uv_rowstride;
    
    gint rect_width, rect_height, rect_x1, rect_y1, rect_x2, rect_y2, center_rect_x1, center_rect_y1;
    gint t_square_size, square_x1, square_y1, square_x2, square_y2, center_square_x1, center_square_y1;
    gint t_circle_radius, circle_x, circle_y;
    gint triangle_x1, triangle_y1, triangle_x2, triangle_y2, triangle_x3, t_triangle_size, center_triangle_x1, center_triangle_y1;
    
    filter = GST_TESTSHAPE (parent);
    
    /*is used to obtain the current capabilities (format and properties) of the input pad, 
     * which represents the format of the video data that is currently being processed by the element.
     */
    caps = gst_pad_get_current_caps (pad);
    if (caps) {
        if (gst_video_info_from_caps (&in_info, caps)) {
            width = in_info.width;
            height = in_info.height;
            rowstride = in_info.stride[0];
        }
        gst_caps_unref (caps);
    }
    
    /* gst_video_frame_map function is used for mapping a video frame for efficient and safe access to its pixel data. */
    if (gst_video_frame_map (&frame, &in_info, buf, GST_MAP_READWRITE)) {
        y_data = GST_VIDEO_FRAME_PLANE_DATA (&frame, 0);
        uv_data = GST_VIDEO_FRAME_PLANE_DATA (&frame, 1);
        uv_rowstride = GST_VIDEO_FRAME_COMP_STRIDE (&frame, 1);
        
        /* If the user does not provide coordinates 'x' and 'y',
         * then by default it is set to FALSE and following will be implemented.
         */
        if (filter->coordinate == FALSE){
            /* Calculates the coordinates of the square */
            t_square_size = MIN(width, height) / 4;
            square_x1 = (width - t_square_size) / 2;
            square_y1 = (height - t_square_size) / 2;
            square_x2 = square_x1 + t_square_size;
            square_y2 = square_y1 + t_square_size;
            
            /* Calculates the coordinates of the circle */
            t_circle_radius = MIN(width, height) / 4;
            circle_x = width / 2;
            circle_y = height / 2;
            
            /* Calculates the coordinates of the rectangle */
            rect_width = width / 2;
            rect_height = height / 2;
            rect_x1 = (width - rect_width) / 2;
            rect_y1 = (height - rect_height) / 2;
            rect_x2 = rect_x1 + rect_width;
            rect_y2 = rect_y1 + rect_height;
            
            /* Calculates the coordinates of the triangle */
            triangle_x1 = width / 2;
            triangle_y1 = (height / 2) - 25;
            triangle_x2 = triangle_x1 - 50;
            triangle_y2 = triangle_y1 + 50;
            triangle_x3 = triangle_x1 + 50;
        }
        
        /* If user sets the coordinate to TRUE and provides 'x' and 'y' coordinates,
         * then following will be implemented.
         */
        if (filter->coordinate == TRUE){
            /* Calculates the coordinates of the square */
            t_square_size = filter->square_size;
            center_square_x1 = filter->x;
            center_square_y1 = filter->y;
            square_x1 = center_square_x1 - (t_square_size / 2);
            square_y1 = center_square_y1 - (t_square_size / 2);
            square_x2 = square_x1 + t_square_size;
            square_y2 = square_y1 + t_square_size;
            
            /* Calculates the coordinates of the circle */
            t_circle_radius = filter->circle_radius;
            circle_x = filter->x;
            circle_y = filter->y;
            
            /* Calculates the coordinates of the rectangle */
            rect_width = filter->rect_l;
            rect_height = filter->rect_h;
            center_rect_x1 = filter->x;
            center_rect_y1 = filter->y;
            rect_x1 = center_rect_x1 - (rect_width / 2);
            rect_y1 = center_rect_y1 - (rect_height / 2);
            rect_x2 = rect_x1 + rect_width;
            rect_y2 = rect_y1 + rect_height;
            
            /* Calculates the coordinates of the triangle */
            t_triangle_size = filter->triangle_size;
            center_triangle_x1 = filter->x;
            center_triangle_y1 = filter->y;
            triangle_x1 = center_triangle_x1;
            triangle_y1 = center_triangle_y1 - (t_triangle_size / 2);
            triangle_x2 = triangle_x1 - t_triangle_size;
            triangle_y2 = triangle_y1 + t_triangle_size;
            triangle_x3 = triangle_x1 + t_triangle_size;
        }
        
		/* Define min and max coordinates for the shape */
		int min_x, max_x, min_y, max_y;

		/* Calculate min and max coordinates based on shape type. */
		if (strcmp(filter->shape, "rectangle") == 0) {
			// Calculate min and max coordinates for a rectangle
			min_x = rect_x1;
			max_x = rect_x2;
			min_y = rect_y1;
			max_y = rect_y2;
		} else if (strcmp(filter->shape, "circle") == 0) {
			// Calculate min and max coordinates for a circle
			min_x = circle_x - t_circle_radius;
			max_x = circle_x + t_circle_radius;
			min_y = circle_y - t_circle_radius;
			max_y = circle_y + t_circle_radius;
		} else if (strcmp(filter->shape, "triangle") == 0) {
			// Calculate min and max coordinates for a triangle
			int min_x1 = triangle_x1 - t_triangle_size;
			int max_x1 = triangle_x1 + t_triangle_size;
			int min_x2 = triangle_x2 - t_triangle_size;
			int max_x2 = triangle_x2 + t_triangle_size;
			int min_x3 = triangle_x3 - t_triangle_size;
			int max_x3 = triangle_x3 + t_triangle_size;

			min_x = min_x1 < min_x2 ? min_x1 : min_x2;
			min_x = min_x < min_x3 ? min_x : min_x3;
			max_x = max_x1 > max_x2 ? max_x1 : max_x2;
			max_x = max_x > max_x3 ? max_x : max_x3;

			min_y = triangle_y1 - t_triangle_size;
			max_y = triangle_y2 + t_triangle_size;
		} else if (strcmp(filter->shape, "square") == 0) {
			// Calculate min and max coordinates for a square
			min_x = square_x1 - t_square_size / 2;
			max_x = square_x2 + t_square_size / 2;
			min_y = square_y1 - t_square_size / 2;
			max_y = square_y2 + t_square_size / 2;
		}

		/* Looping over pixels for getting exact location to draw shape. */
		for (y1 = min_y; y1 <= max_y; y1++) {
			for (x1 = min_x; x1 <= max_x; x1++) {
				    // Get pixels at the particular location
				    guint8 *y_pixel = y_data + y1 * rowstride + x1;
				    guint8 *u_pixel = uv_data + (y1 / 2) * uv_rowstride + (x1 / 2) * 2;
				    guint8 *v_pixel = u_pixel + 1;

				    /*If shape is set to rectangle, drawRectangle function is called. */
				    if (strcmp(filter->shape, "rectangle") == 0) {
				        drawRectangle(&frame, x1, y1, rect_width, rect_height, y_pixel, u_pixel, v_pixel, filter->color, rect_x1, rect_y1, rect_x2, rect_y2);
				    } 
				    /*If shape is set to circle, drawCircle function is called. */
				    else if (strcmp(filter->shape, "circle") == 0) {
				        drawCircle(&frame, x1, y1, t_circle_radius, y_pixel, u_pixel, v_pixel, filter->color, circle_x, circle_y);
				    } 
				    /*If shape is set to traingle, drawTriangle function is called.*/
				    else if (strcmp(filter->shape, "triangle") == 0) {
				        drawTriangle(&frame, x1, y1, t_triangle_size, y_pixel, u_pixel, v_pixel, filter->color, triangle_x1, triangle_y1, triangle_x2, triangle_y2, triangle_x3);
				    } 
				    /*If shape is set to square, drawSquare function is called.*/
				    else if (strcmp(filter->shape, "square") == 0) {
				        drawSquare(&frame, x1, y1, t_square_size, y_pixel, u_pixel, v_pixel, filter->color, square_x1, square_y1, square_x2, square_y2);
				    }
			}
		}
        
        /* unmap the frame */
		gst_video_frame_unmap (&frame);
    }
    
    /* pushing buffer to a downstream element */
    return gst_pad_push (filter->srcpad, buf);
}

/* To initialize the plug-in and
 * register the element factories and other features.
 */
static gboolean testshape_init (GstPlugin * testshape)
{
    return GST_ELEMENT_REGISTER (testshape, testshape);  //passes as pointer to an argument and name of element to be registered.
}

/* GST_PLUGIN_DEFINE macro is used to define a plugin and 
 * provide various metadata and initialization information about the plugin.
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    testshape,
    "This plugin is used to draw different shapes like square, circle rectangle, etc                                                            with different colors such as red, green and blue",
    testshape_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
