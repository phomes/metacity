/*
 * Copyright (C) 2016 Alberts Muktupāvels
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

#include <glib/gi18n.h>

#include "meta-theme-impl-private.h"
#include "meta-theme.h"

typedef struct
{
  gboolean           composited;
  gint               scale;

  MetaFrameStyleSet *style_sets_by_type[META_FRAME_TYPE_LAST];
} MetaThemeImplPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MetaThemeImpl, meta_theme_impl, G_TYPE_OBJECT)

static void
meta_theme_impl_dispose (GObject *object)
{
  MetaThemeImpl *impl;
  MetaThemeImplPrivate *priv;
  gint i;

  impl = META_THEME_IMPL (object);
  priv = meta_theme_impl_get_instance_private (impl);

  for (i = 0; i < META_FRAME_TYPE_LAST; i++)
    {
      if (priv->style_sets_by_type[i])
        {
          meta_frame_style_set_unref (priv->style_sets_by_type[i]);
          priv->style_sets_by_type[i] = NULL;
        }
    }

  G_OBJECT_CLASS (meta_theme_impl_parent_class)->dispose (object);
}

static gboolean
meta_theme_impl_real_load (MetaThemeImpl  *impl,
                           const gchar    *name,
                           GError        **error)
{
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_set_error (error, META_THEME_ERROR, META_THEME_ERROR_FAILED,
               _("MetaThemeImplClass::load not implemented for '%s'"),
               g_type_name (G_TYPE_FROM_INSTANCE (impl)));

  return FALSE;
}

static void
meta_theme_impl_class_init (MetaThemeImplClass *impl_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (impl_class);

  object_class->dispose = meta_theme_impl_dispose;

  impl_class->load = meta_theme_impl_real_load;
}

static void
meta_theme_impl_init (MetaThemeImpl *impl)
{
}

void
meta_theme_impl_set_composited (MetaThemeImpl *impl,
                                gboolean       composited)
{
  MetaThemeImplPrivate *priv;

  priv = meta_theme_impl_get_instance_private (impl);

  priv->composited = composited;
}

gboolean
meta_theme_impl_get_composited (MetaThemeImpl *impl)
{
  MetaThemeImplPrivate *priv;

  priv = meta_theme_impl_get_instance_private (impl);

  return priv->composited;
}

gint
meta_theme_impl_get_scale (MetaThemeImpl *impl)
{
  GValue value = G_VALUE_INIT;
  MetaThemeImplPrivate *priv;
  GdkScreen *screen;

  priv = meta_theme_impl_get_instance_private (impl);

  if (priv->scale != 0)
    return priv->scale;

  screen = gdk_screen_get_default ();

  g_value_init (&value, G_TYPE_INT);

  if (gdk_screen_get_setting (screen, "gdk-window-scaling-factor", &value))
    return g_value_get_int (&value);
  else
    return 1;
}

void
meta_theme_impl_add_style_set (MetaThemeImpl     *impl,
                               MetaFrameType      type,
                               MetaFrameStyleSet *style_set)
{
  MetaThemeImplPrivate *priv;

  priv = meta_theme_impl_get_instance_private (impl);

  if (priv->style_sets_by_type[type])
    meta_frame_style_set_unref (priv->style_sets_by_type[type]);

  priv->style_sets_by_type[type] = style_set;
}

MetaFrameStyleSet *
meta_theme_impl_get_style_set (MetaThemeImpl *impl,
                               MetaFrameType  type)
{
  MetaThemeImplPrivate *priv;

  priv = meta_theme_impl_get_instance_private (impl);

  return priv->style_sets_by_type[type];
}

void
scale_border (GtkBorder *border,
              double     factor)
{
  border->left *= factor;
  border->right *= factor;
  border->top *= factor;
  border->bottom *= factor;
}

gboolean
is_button_visible (MetaButton     *button,
                   MetaFrameFlags  flags)
{
  gboolean visible;

  visible = FALSE;

  switch (button->type)
    {
      case META_BUTTON_TYPE_MENU:
        if (flags & META_FRAME_ALLOWS_MENU)
          visible = TRUE;
        break;

      case META_BUTTON_TYPE_APPMENU:
        if (flags & META_FRAME_ALLOWS_APPMENU)
          visible = TRUE;
        break;

      case META_BUTTON_TYPE_MINIMIZE:
        if (flags & META_FRAME_ALLOWS_MINIMIZE)
          visible = TRUE;
        break;

      case META_BUTTON_TYPE_MAXIMIZE:
        if (flags & META_FRAME_ALLOWS_MAXIMIZE)
          visible = TRUE;
        break;

      case META_BUTTON_TYPE_CLOSE:
        if (flags & META_FRAME_ALLOWS_DELETE)
          visible = TRUE;
        break;

      case META_BUTTON_TYPE_SHADE:
        if ((flags & META_FRAME_ALLOWS_SHADE) && !(flags & META_FRAME_SHADED))
          visible = TRUE;
        break;

      case META_BUTTON_TYPE_ABOVE:
        if (!(flags & META_FRAME_ABOVE))
          visible = TRUE;
        break;

      case META_BUTTON_TYPE_STICK:
        if (!(flags & META_FRAME_STUCK))
          visible = TRUE;
        break;

      case META_BUTTON_TYPE_UNSHADE:
        if ((flags & META_FRAME_ALLOWS_SHADE) && (flags & META_FRAME_SHADED))
          visible = TRUE;
        break;

      case META_BUTTON_TYPE_UNABOVE:
        if (flags & META_FRAME_ABOVE)
          visible = TRUE;
        break;

      case META_BUTTON_TYPE_UNSTICK:
        if (flags & META_FRAME_STUCK)
          visible = TRUE;
        break;

      case META_BUTTON_TYPE_SPACER:
        visible = TRUE;
        break;

      case META_BUTTON_TYPE_LAST:
      default:
        break;
    }

  return visible;
}

gboolean
strip_button (MetaButton     *buttons,
              gint            n_buttons,
              MetaButtonType  type)
{
  gint i;

  for (i = 0; i < n_buttons; i++)
    {
      if (buttons[i].type == type && buttons[i].visible)
        {
          buttons[i].visible = FALSE;
          return TRUE;
        }
    }

  return FALSE;
}

gboolean
strip_buttons (MetaButtonLayout *layout,
               gint             *n_left,
               gint             *n_right)
{
  gint count;
  MetaButtonType types[META_BUTTON_TYPE_LAST];
  gint i;

  count = 0;
  types[count++] = META_BUTTON_TYPE_ABOVE;
  types[count++] = META_BUTTON_TYPE_UNABOVE;
  types[count++] = META_BUTTON_TYPE_STICK;
  types[count++] = META_BUTTON_TYPE_UNSTICK;
  types[count++] = META_BUTTON_TYPE_SHADE;
  types[count++] = META_BUTTON_TYPE_UNSHADE;
  types[count++] = META_BUTTON_TYPE_MINIMIZE;
  types[count++] = META_BUTTON_TYPE_MAXIMIZE;
  types[count++] = META_BUTTON_TYPE_CLOSE;
  types[count++] = META_BUTTON_TYPE_MENU;
  types[count++] = META_BUTTON_TYPE_APPMENU;

  for (i = 0; i < count; i++)
    {
      if (strip_button (layout->left_buttons, layout->n_left_buttons,
                        types[i]))
        {
          *n_left -= 1;
          return TRUE;
        }
      else if (strip_button (layout->right_buttons, layout->n_right_buttons,
                             types[i]))
        {
          *n_left -= 1;
          return TRUE;
        }
    }

  return FALSE;
}
