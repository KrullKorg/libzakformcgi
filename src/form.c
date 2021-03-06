/*
 * Copyright (C) 2015-2016 Andrea Zagli <azagli@libero.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include <libzakutils/libzakutils.h>

#include "form.h"
#include "formelementstring.h"

static void zak_form_cgi_form_class_init (ZakFormCgiFormClass *class);
static void zak_form_cgi_form_init (ZakFormCgiForm *zak_form_cgi_form);

static void zak_form_cgi_form_set_property (GObject *object,
                                            guint property_id,
                                            const GValue *value,
                                            GParamSpec *pspec);
static void zak_form_cgi_form_get_property (GObject *object,
                                            guint property_id,
                                            GValue *value,
                                            GParamSpec *pspec);

static void zak_form_cgi_form_dispose (GObject *gobject);
static void zak_form_cgi_form_finalize (GObject *gobject);

#define ZAK_FORM_CGI_FORM_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), ZAK_FORM_CGI_TYPE_FORM, ZakFormCgiFormPrivate))

typedef struct _ZakFormCgiFormPrivate ZakFormCgiFormPrivate;
struct _ZakFormCgiFormPrivate
	{
		ZakCgiMain *zakcgimain;
		GHashTable *ht_attrs;
		GPtrArray *ar_elems;
	};

G_DEFINE_TYPE (ZakFormCgiForm, zak_form_cgi_form, ZAK_FORM_TYPE_FORM)

#ifdef G_OS_WIN32
static HMODULE backend_dll = NULL;

BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

BOOL WINAPI
DllMain (HINSTANCE hinstDLL,
         DWORD     fdwReason,
         LPVOID    lpReserved)
{
	switch (fdwReason)
		{
		case DLL_PROCESS_ATTACH:
			backend_dll = (HMODULE) hinstDLL;
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
		}
	return TRUE;
}
#endif

static void
zak_form_cgi_form_class_init (ZakFormCgiFormClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	object_class->set_property = zak_form_cgi_form_set_property;
	object_class->get_property = zak_form_cgi_form_get_property;
	object_class->dispose = zak_form_cgi_form_dispose;
	object_class->finalize = zak_form_cgi_form_finalize;

	g_type_class_add_private (object_class, sizeof (ZakFormCgiFormPrivate));
}

static void
zak_form_cgi_form_init (ZakFormCgiForm *zak_form_cgi_form)
{
	ZakFormCgiFormPrivate *priv = ZAK_FORM_CGI_FORM_GET_PRIVATE (zak_form_cgi_form);

	priv->zakcgimain = NULL;
	priv->ht_attrs = NULL;
	priv->ar_elems = ZAK_FORM_FORM_CLASS (zak_form_cgi_form_parent_class)->get_elements (ZAK_FORM_FORM (zak_form_cgi_form));
}

/**
 * zak_form_cgi_form_new:
 * @zakcgimain:
 * @...:
 *
 * Returns: the newly created #ZakFormCgiForm object.
 */
ZakFormCgiForm
*zak_form_cgi_form_new (ZakCgiMain *zakcgimain, ...)
{
	gchar *localedir;

	ZakFormCgiForm *zak_form_cgi_form;
	ZakFormCgiFormPrivate *priv;

	va_list ap;

#ifdef G_OS_WIN32

	gchar *moddir;
	gchar *p;

	moddir = g_win32_get_package_installation_directory_of_module (backend_dll);

	p = g_strrstr (moddir, g_strdup_printf ("%c", G_DIR_SEPARATOR));
	if (p != NULL
	    && (g_ascii_strcasecmp (p + 1, "src") == 0
	        || g_ascii_strcasecmp (p + 1, ".libs") == 0))
		{
			localedir = g_strdup (LOCALEDIR);
		}
	else
		{
			localedir = g_build_filename (moddir, "share", "locale", NULL);
		}

	g_free (moddir);

#else

	localedir = g_strdup (LOCALEDIR);

#endif

	bindtextdomain (GETTEXT_PACKAGE, localedir);
	textdomain (GETTEXT_PACKAGE);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

	zak_form_cgi_form = ZAK_FORM_CGI_FORM (g_object_new (zak_form_cgi_form_get_type (), NULL));

	priv = ZAK_FORM_CGI_FORM_GET_PRIVATE (zak_form_cgi_form);
	priv->zakcgimain = zakcgimain;

	va_start (ap, zakcgimain);
	priv->ht_attrs = zak_cgi_commons_valist_to_ghashtable (ap);

	return zak_form_cgi_form;
}

static guint
get_idx (ZakFormCgiForm *zakcgiform, const gchar *id)
{
	guint idx;
	gchar *element_id;

	ZakFormCgiFormPrivate *priv;

	priv = ZAK_FORM_CGI_FORM_GET_PRIVATE (zakcgiform);

	for (idx = 0; idx < priv->ar_elems->len; idx++)
		{
			element_id = zak_form_cgi_form_element_get_id ((ZakFormCgiFormElement *)g_ptr_array_index (priv->ar_elems, idx));
			if (g_strcmp0 (element_id, id) == 0)
				{
					break;
				}
			g_free (element_id);
		}
	if (idx == priv->ar_elems->len)
		{
			idx = -1;
		}

	return idx;
}

static void
_zak_form_cgi_form_bind (ZakFormCgiFormPrivate *priv, ZakFormCgiFormElement *element)
{
	gchar *id;
	GValue *gval;

	id = zak_form_cgi_form_element_get_id (element);
	if (id != NULL)
		{
			gval = zak_cgi_main_get_stdin_field (priv->zakcgimain, id);
			if (gval == NULL)
				{
					gval = zak_utils_gvalue_new_string ("");
				}
			zak_form_cgi_form_element_bind (element, gval);
			g_free (id);
		}
	else
		{
			g_warning (_("Element without id."));
		}
}

/**
 * zak_form_cgi_form_bind:
 * @zakcgiform:
 *
 */
void
zak_form_cgi_form_bind (ZakFormCgiForm *zakcgiform)
{
	guint i;

	ZakFormCgiFormPrivate *priv;

	g_return_if_fail (ZAK_FORM_CGI_IS_FORM (zakcgiform));

	priv = ZAK_FORM_CGI_FORM_GET_PRIVATE (zakcgiform);

	for (i = 0; i < priv->ar_elems->len; i++)
		{
			ZakFormCgiFormElement *element = (ZakFormCgiFormElement *)g_ptr_array_index (priv->ar_elems, i);
			if (!ZAK_FORM_CGI_IS_FORM_ELEMENT_STRING (element))
				{
					if (ZAK_FORM_IS_ELEMENT_ARRAY ((ZAK_FORM_ELEMENT (element))))
						{
							GPtrArray *ar_elements;
							guint l;

							ar_elements = zak_form_element_array_get_elements (ZAK_FORM_ELEMENT (element));
							for (l = 0; l < ar_elements->len; l++)
								{
									_zak_form_cgi_form_bind (priv, (ZakFormCgiFormElement *)g_ptr_array_index (ar_elements, l));
								}
						}
					else
						{
							_zak_form_cgi_form_bind (priv, element);
						}
				}
		}
}

/**
 * zak_form_cgi_form_render_start:
 * @zakcgiform:
 *
 * Returns:
 */
gchar
*zak_form_cgi_form_render_start (ZakFormCgiForm *zakcgiform)
{
	gchar *ret;

	GString *str;

	ZakFormCgiFormPrivate *priv;

	g_return_val_if_fail (ZAK_FORM_CGI_IS_FORM (zakcgiform), g_strdup (""));

	priv = ZAK_FORM_CGI_FORM_GET_PRIVATE (zakcgiform);

	str = g_string_new ("<form");

	ret = zak_cgi_commons_ghashtable_to_str_attrs (priv->ht_attrs);
	g_string_append_printf (str, "%s>", ret);
	g_free (ret);

	ret = g_strdup (str->str);

	return ret;
}

/**
 * zak_form_cgi_form_render:
 * @zakcgiform:
 *
 * Returns:
 */
gchar
*zak_form_cgi_form_render (ZakFormCgiForm *zakcgiform)
{
	GString *str;

	guint i;

	gchar *tmp;

	ZakFormCgiFormPrivate *priv;

	priv = ZAK_FORM_CGI_FORM_GET_PRIVATE (zakcgiform);

	str = g_string_new ("");

	tmp = zak_form_cgi_form_render_start (zakcgiform);
	g_string_append (str, tmp);
	g_free (tmp);

	for (i = 0; i < priv->ar_elems->len; i++)
		{
			ZakFormCgiFormElement *element = (ZakFormCgiFormElement *)g_ptr_array_index (priv->ar_elems, i);

			if (ZAK_FORM_IS_ELEMENT_ARRAY (element))
				{
					GPtrArray *ar_elements;
					guint l;

					ar_elements = zak_form_element_array_get_elements (ZAK_FORM_ELEMENT (element));
					for (l = 0; l < ar_elements->len; l++)
						{
							tmp = zak_form_cgi_form_element_render ((ZakFormCgiFormElement *)g_ptr_array_index (ar_elements, l));
							g_string_append_printf (str, "\n%s", tmp);
							g_free (tmp);
						}
				}
			else
				{
					tmp = zak_form_cgi_form_element_render (element);
					g_string_append_printf (str, "\n%s", tmp);
					g_free (tmp);
				}
		}

	g_string_append (str, "\n</form>");

	return g_strdup (str->str);
}

/* PRIVATE */
static void
zak_form_cgi_form_set_property (GObject *object,
                   guint property_id,
                   const GValue *value,
                   GParamSpec *pspec)
{
	ZakFormCgiForm *zak_form_cgi_form = (ZakFormCgiForm *)object;
	ZakFormCgiFormPrivate *priv = ZAK_FORM_CGI_FORM_GET_PRIVATE (zak_form_cgi_form);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
zak_form_cgi_form_get_property (GObject *object,
                   guint property_id,
                   GValue *value,
                   GParamSpec *pspec)
{
	ZakFormCgiForm *zak_form_cgi_form = (ZakFormCgiForm *)object;
	ZakFormCgiFormPrivate *priv = ZAK_FORM_CGI_FORM_GET_PRIVATE (zak_form_cgi_form);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
zak_form_cgi_form_dispose (GObject *gobject)
{
	ZakFormCgiForm *zak_form_cgi_form = (ZakFormCgiForm *)gobject;
	ZakFormCgiFormPrivate *priv = ZAK_FORM_CGI_FORM_GET_PRIVATE (zak_form_cgi_form);


	GObjectClass *parent_class = g_type_class_peek_parent (G_OBJECT_GET_CLASS (gobject));
	parent_class->dispose (gobject);
}

static void
zak_form_cgi_form_finalize (GObject *gobject)
{
	ZakFormCgiForm *zak_form_cgi_form = (ZakFormCgiForm *)gobject;
	ZakFormCgiFormPrivate *priv = ZAK_FORM_CGI_FORM_GET_PRIVATE (zak_form_cgi_form);


	GObjectClass *parent_class = g_type_class_peek_parent (G_OBJECT_GET_CLASS (gobject));
	parent_class->finalize (gobject);
}
