/*
 * Copyright (C) 2008 Iain Holmes
 * Copyright (C) 2017 Alberts Muktupāvels
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef META_COMPOSITOR_PRIVATE_H
#define META_COMPOSITOR_PRIVATE_H

#include "meta-compositor.h"

G_BEGIN_DECLS

struct _MetaCompositorClass
{
  GObjectClass parent_class;

  gboolean          (* initable_init)      (MetaCompositor     *compositor,
                                            GError            **error);

  void              (* manage_screen)      (MetaCompositor     *compositor,
                                            MetaScreen         *screen);

  void              (* unmanage_screen)    (MetaCompositor     *compositor,
                                            MetaScreen         *screen);

  void              (* add_window)         (MetaCompositor     *compositor,
                                            MetaWindow         *window,
                                            Window              xwindow,
                                            XWindowAttributes  *attrs);

  void              (* remove_window)      (MetaCompositor     *compositor,
                                            Window              xwindow);

  void              (* set_updates)        (MetaCompositor     *compositor,
                                            MetaWindow         *window,
                                            gboolean            update);

  void              (*process_event)       (MetaCompositor     *compositor,
                                            XEvent             *event,
                                            MetaWindow         *window);

  cairo_surface_t * (* get_window_surface) (MetaCompositor     *compositor,
                                            MetaWindow         *window);

  void              (* set_active_window)  (MetaCompositor     *compositor,
                                            MetaScreen         *screen,
                                            MetaWindow         *window);

  void              (* begin_move)         (MetaCompositor     *compositor,
                                            MetaWindow         *window,
                                            MetaRectangle      *initial,
                                            gint                grab_x,
                                            gint                grab_y);

  void              (* update_move)        (MetaCompositor     *compositor,
                                            MetaWindow         *window,
                                            gint                x,
                                            gint                y);

  void              (* end_move)           (MetaCompositor     *compositor,
                                            MetaWindow         *window);

  void              (* free_window)        (MetaCompositor     *compositor,
                                            MetaWindow         *window);

  void              (* maximize_window)    (MetaCompositor     *compositor,
                                            MetaWindow         *window);

  void              (* unmaximize_window)  (MetaCompositor     *compositor,
                                            MetaWindow         *window);
};

G_END_DECLS

#endif
