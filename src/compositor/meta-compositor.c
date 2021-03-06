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

#include "config.h"

#include "meta-compositor-none.h"
#include "meta-compositor-xrender.h"

typedef struct
{
  MetaDisplay *display;
} MetaCompositorPrivate;

enum
{
  PROP_0,

  PROP_DISPLAY,

  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

static void initable_iface_init (GInitableIface *iface);

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (MetaCompositor, meta_compositor, G_TYPE_OBJECT,
                                  G_ADD_PRIVATE (MetaCompositor)
                                  G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                         initable_iface_init))

static gboolean
meta_compositor_initable_init (GInitable     *initable,
                               GCancellable  *cancellable,
                               GError       **error)
{
  MetaCompositor *compositor;
  MetaCompositorClass *compositor_class;

  compositor = META_COMPOSITOR (initable);
  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  return compositor_class->initable_init (compositor, error);
}

static void
initable_iface_init (GInitableIface *iface)
{
  iface->init = meta_compositor_initable_init;
}

static void
meta_compositor_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  MetaCompositor *compositor;
  MetaCompositorPrivate *priv;

  compositor = META_COMPOSITOR (object);
  priv = meta_compositor_get_instance_private (compositor);

  switch (property_id)
    {
      case PROP_DISPLAY:
        g_value_set_pointer (value, priv->display);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
meta_compositor_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  MetaCompositor *compositor;
  MetaCompositorPrivate *priv;

  compositor = META_COMPOSITOR (object);
  priv = meta_compositor_get_instance_private (compositor);

  switch (property_id)
    {
      case PROP_DISPLAY:
        priv->display = g_value_get_pointer (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
install_properties (GObjectClass *object_class)
{
  properties[PROP_DISPLAY] =
    g_param_spec_pointer ("display", "display", "display",
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
meta_compositor_class_init (MetaCompositorClass *compositor_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (compositor_class);

  object_class->get_property = meta_compositor_get_property;
  object_class->set_property = meta_compositor_set_property;

  install_properties (object_class);
}

static void
meta_compositor_init (MetaCompositor *compositor)
{
}

MetaCompositor *
meta_compositor_new (MetaCompositorType  type,
                     MetaDisplay        *display)
{
  GType gtype;
  MetaCompositor *compositor;
  GError *error;

  switch (type)
    {
      case META_COMPOSITOR_TYPE_NONE:
        gtype = META_TYPE_COMPOSITOR_NONE;
        break;

      case META_COMPOSITOR_TYPE_XRENDER:
        gtype = META_TYPE_COMPOSITOR_XRENDER;
        break;

      default:
        g_assert_not_reached ();
        break;
    }

  compositor = g_object_new (gtype, "display", display, NULL);

  error = NULL;
  if (!g_initable_init (G_INITABLE (compositor), NULL, &error))
    {
      g_warning ("Failed to create %s: %s", g_type_name (gtype), error->message);
      g_error_free (error);

      g_object_unref (compositor);

      if (type != META_COMPOSITOR_TYPE_NONE)
        compositor = meta_compositor_new (META_COMPOSITOR_TYPE_NONE, display);
    }

  g_assert (compositor != NULL);

  return compositor;
}

void
meta_compositor_manage_screen (MetaCompositor *compositor,
                               MetaScreen     *screen)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->manage_screen (compositor, screen);
}

void
meta_compositor_unmanage_screen (MetaCompositor *compositor,
                                 MetaScreen     *screen)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->unmanage_screen (compositor, screen);
}

void
meta_compositor_add_window (MetaCompositor    *compositor,
                            MetaWindow        *window,
                            Window             xwindow,
                            XWindowAttributes *attrs)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->add_window (compositor, window, xwindow, attrs);
}

void
meta_compositor_remove_window (MetaCompositor *compositor,
                               Window          xwindow)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->remove_window (compositor, xwindow);
}

void
meta_compositor_set_updates (MetaCompositor *compositor,
                             MetaWindow     *window,
                             gboolean        updates)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->set_updates (compositor, window, updates);
}

void
meta_compositor_process_event (MetaCompositor *compositor,
                               XEvent         *event,
                               MetaWindow     *window)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->process_event (compositor, event, window);
}

cairo_surface_t *
meta_compositor_get_window_surface (MetaCompositor *compositor,
                                    MetaWindow     *window)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  return compositor_class->get_window_surface (compositor, window);
}

void
meta_compositor_set_active_window (MetaCompositor *compositor,
                                   MetaScreen     *screen,
                                   MetaWindow     *window)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->set_active_window (compositor, screen, window);
}

void
meta_compositor_begin_move (MetaCompositor *compositor,
                            MetaWindow     *window,
                            MetaRectangle  *initial,
                            gint            grab_x,
                            gint            grab_y)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->begin_move (compositor, window, initial, grab_x, grab_y);
}

void
meta_compositor_update_move (MetaCompositor *compositor,
                             MetaWindow     *window,
                             gint            x,
                             gint            y)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->update_move (compositor, window, x, y);
}

void
meta_compositor_end_move (MetaCompositor *compositor,
                          MetaWindow     *window)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->end_move (compositor, window);
}

void
meta_compositor_free_window (MetaCompositor *compositor,
                             MetaWindow     *window)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->free_window (compositor, window);
}

void
meta_compositor_maximize_window (MetaCompositor *compositor,
                                 MetaWindow     *window)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->maximize_window (compositor, window);
}

void
meta_compositor_unmaximize_window (MetaCompositor *compositor,
                                   MetaWindow     *window)
{
  MetaCompositorClass *compositor_class;

  compositor_class = META_COMPOSITOR_GET_CLASS (compositor);

  compositor_class->unmaximize_window (compositor, window);
}

MetaDisplay *
meta_compositor_get_display (MetaCompositor *compositor)
{
  MetaCompositorPrivate *priv;

  priv = meta_compositor_get_instance_private (compositor);

  return priv->display;
}
