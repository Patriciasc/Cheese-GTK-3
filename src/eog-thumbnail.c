/* Eye Of Gnome - Thumbnailing functions 
 *
 * Copyright (C) 2000-2007 The Free Software Foundation
 *
 * Author: Lucas Rocha <lucasr@gnome.org>
 *
 * Based on eel code (eel/eel-graphic-effects.c) by: 
 * 	- Andy Hertzfeld <andy@eazel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* NOTE this is a stripped version of eog-thumbnail which only contains the 
 * functions necessary for cheese
 */
#ifdef HAVE_CONFIG_H
#include <cheese-config.h>
#endif

#include <libart_lgpl/art_rgb.h>

#include "eog-thumbnail.h"


static GdkPixbuf *frame = NULL;


static void
draw_frame_row (GdkPixbuf *frame_image, 
		gint target_width, 
		gint source_width, 
		gint source_v_position, 
		gint dest_v_position, 
		GdkPixbuf *result_pixbuf, 
		gint left_offset, 
		gint height)
{
	gint remaining_width, h_offset, slab_width;
	
	remaining_width = target_width;
	h_offset = 0;

	while (remaining_width > 0) {	
		slab_width = remaining_width > source_width ? 
			     source_width : remaining_width;

		gdk_pixbuf_copy_area (frame_image, 
				      left_offset, 
				      source_v_position, 
				      slab_width, 
				      height, 
				      result_pixbuf, 
				      left_offset + h_offset, 
				      dest_v_position);

		remaining_width -= slab_width;
		h_offset += slab_width; 
	}
}

static void
draw_frame_column (GdkPixbuf *frame_image, 
		   gint target_height, 
		   gint source_height, 
		   gint source_h_position, 
		   gint dest_h_position, 
		   GdkPixbuf *result_pixbuf, 
		   gint top_offset, 
		   gint width)
{
	gint remaining_height, v_offset, slab_height;
	
	remaining_height = target_height;
	v_offset = 0;

	while (remaining_height > 0) {	
		slab_height = remaining_height > source_height ? 
			      source_height : remaining_height;

		gdk_pixbuf_copy_area (frame_image, 
				      source_h_position, 
				      top_offset, 
				      width, 
				      slab_height, 
				      result_pixbuf, 
				      dest_h_position, 
				      top_offset + v_offset);

		remaining_height -= slab_height;
		v_offset += slab_height; 
	}
}

static GdkPixbuf *
eog_thumbnail_stretch_frame_image (GdkPixbuf *frame_image, 
				   gint left_offset, 
				   gint top_offset, 
				   gint right_offset, 
				   gint bottom_offset,
                                   gint dest_width, 
				   gint dest_height, 
				   gboolean fill_flag)
{
        GdkPixbuf *result_pixbuf;
        guchar *pixels_ptr;
        gint frame_width, frame_height;
        gint y, row_stride;
        gint target_width, target_frame_width;
        gint target_height, target_frame_height;

        frame_width  = gdk_pixbuf_get_width  (frame_image);
        frame_height = gdk_pixbuf_get_height (frame_image);

        if (fill_flag) {
		result_pixbuf = gdk_pixbuf_scale_simple (frame_image, 
							 dest_width, 
							 dest_height, 
							 GDK_INTERP_NEAREST);
        } else {
                result_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, 
						TRUE, 
						8, 
						dest_width, 
						dest_height);
        }

        row_stride = gdk_pixbuf_get_rowstride (result_pixbuf);
        pixels_ptr = gdk_pixbuf_get_pixels (result_pixbuf);

        if (!fill_flag) {
        	for (y = 0; y < dest_height; y++) {
			art_rgb_run_alpha (pixels_ptr, 
					   255, 255, 
					   255, 255, 
					   dest_width);
			pixels_ptr += row_stride;
        	}
        }

        target_width  = dest_width - left_offset - right_offset;
        target_frame_width = frame_width - left_offset - right_offset;

        target_height  = dest_height - top_offset - bottom_offset;
        target_frame_height = frame_height - top_offset - bottom_offset;

        /* Draw the left top corner  and top row */
        gdk_pixbuf_copy_area (frame_image, 
			      0, 0, 
			      left_offset, 
			      top_offset, 
			      result_pixbuf, 
			      0, 0);

        draw_frame_row (frame_image, 
			target_width, 
			target_frame_width, 
			0, 0, 
			result_pixbuf, 
			left_offset, 
			top_offset);

        /* Draw the right top corner and left column */
        gdk_pixbuf_copy_area (frame_image, 
			      frame_width - right_offset, 
			      0, 
			      right_offset, 
			      top_offset, 
			      result_pixbuf, 
			      dest_width - right_offset,  
			      0);

        draw_frame_column (frame_image, 
			   target_height, 
			   target_frame_height, 
			   0, 0, 
			   result_pixbuf, 
			   top_offset, 
			   left_offset);

        /* Draw the bottom right corner and bottom row */
        gdk_pixbuf_copy_area (frame_image, 
			      frame_width - right_offset, 
			      frame_height - bottom_offset, 
			      right_offset, 
			      bottom_offset, 
			      result_pixbuf, 
			      dest_width - right_offset,  
			      dest_height - bottom_offset);

        draw_frame_row (frame_image, 
			target_width, 
			target_frame_width, 
			frame_height - bottom_offset, 
			dest_height - bottom_offset, 
			result_pixbuf, 
			left_offset, bottom_offset);

        /* Draw the bottom left corner and the right column */
        gdk_pixbuf_copy_area (frame_image, 
			      0, 
			      frame_height - bottom_offset, 
			      left_offset, 
			      bottom_offset, 
			      result_pixbuf, 
			      0,
			      dest_height - bottom_offset);

        draw_frame_column (frame_image, 
			   target_height, 
			   target_frame_height, 
			   frame_width - right_offset, 
			   dest_width - right_offset, 
			   result_pixbuf, top_offset, 
			   right_offset);

        return result_pixbuf;
}

void 
eog_thumbnail_add_frame (GdkPixbuf **thumbnail)
{
	GdkPixbuf *result_pixbuf;
	gint source_width, source_height;
	gint dest_width, dest_height;

	source_width  = gdk_pixbuf_get_width  (*thumbnail);
	source_height = gdk_pixbuf_get_height (*thumbnail);
	
	dest_width  = source_width  + 9;
	dest_height = source_height + 9;

	result_pixbuf = eog_thumbnail_stretch_frame_image (frame, 
							   3, 3, 6, 6,
	                                	           dest_width, 
							   dest_height,
							   FALSE);

	gdk_pixbuf_copy_area (*thumbnail, 
			      0, 0, 
			      source_width, 
			      source_height, 
			      result_pixbuf, 
			      3, 3);

	g_object_unref (*thumbnail);

	*thumbnail = result_pixbuf;
}


void
eog_thumbnail_init (void)
{
	if (frame == NULL) {
		frame = gdk_pixbuf_new_from_file (PACKAGE_DATADIR "/pixmaps/thumbnail-frame.png", NULL);
	}
}
