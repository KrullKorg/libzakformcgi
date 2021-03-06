/*
 * Copyright (C) 2015-2017 Andrea Zagli <azagli@libero.it>
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

#include <libzakcgi/libzakcgi.h>
#include "formelementtext.h"

static void zak_form_cgi_form_element_text_class_init (ZakFormCgiFormElementTextClass *class);
static void zak_form_cgi_form_element_text_init (ZakFormCgiFormElementText *zak_form_cgi_form_element_text);

static gchar *zak_form_cgi_form_element_text_render (ZakFormCgiFormElement *element);

static void zak_form_cgi_form_element_text_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec);
static void zak_form_cgi_form_element_text_get_property (GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec);

static void zak_form_cgi_form_element_text_dispose (GObject *gobject);
static void zak_form_cgi_form_element_text_finalize (GObject *gobject);

#define ZAK_FORM_CGI_FORM_ELEMENT_TEXT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), ZAK_FORM_CGI_TYPE_FORM_ELEMENT_TEXT, ZakFormCgiFormElementTextPrivate))

typedef struct _ZakFormCgiFormElementTextPrivate ZakFormCgiFormElementTextPrivate;
struct _ZakFormCgiFormElementTextPrivate
	{
		gpointer empty;
	};

G_DEFINE_TYPE (ZakFormCgiFormElementText, zak_form_cgi_form_element_text, ZAK_FORM_CGI_TYPE_FORM_ELEMENT)

static void
zak_form_cgi_form_element_text_class_init (ZakFormCgiFormElementTextClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	ZakFormElementClass *form_elem_class = ZAK_FORM_ELEMENT_CLASS (klass);
	ZakFormCgiFormElementClass *elem_class = ZAK_FORM_CGI_FORM_ELEMENT_CLASS (klass);

	object_class->set_property = zak_form_cgi_form_element_text_set_property;
	object_class->get_property = zak_form_cgi_form_element_text_get_property;
	object_class->dispose = zak_form_cgi_form_element_text_dispose;
	object_class->finalize = zak_form_cgi_form_element_text_finalize;

	form_elem_class->xml_parsing = zak_form_cgi_form_element_text_xml_parsing;

	elem_class->render = zak_form_cgi_form_element_text_render;

	g_type_class_add_private (object_class, sizeof (ZakFormCgiFormElementTextPrivate));
}

static void
zak_form_cgi_form_element_text_init (ZakFormCgiFormElementText *zak_form_cgi_form_element_text)
{
	ZakFormCgiFormElementTextPrivate *priv = ZAK_FORM_CGI_FORM_ELEMENT_TEXT_GET_PRIVATE (zak_form_cgi_form_element_text);

}

/**
 * zak_form_cgi_form_element_text_new:
 *
 * Returns: the newly created #ZakFormCgiFormElementText object.
 */
ZakFormCgiFormElement
*zak_form_cgi_form_element_text_new ()
{
	ZakFormCgiFormElementText *zak_form_cgi_form_element_text;

	zak_form_cgi_form_element_text = ZAK_FORM_CGI_FORM_ELEMENT_TEXT (g_object_new (zak_form_cgi_form_element_text_get_type (), NULL));

	return ZAK_FORM_CGI_FORM_ELEMENT (zak_form_cgi_form_element_text);
}

/**
 * zak_form_cgi_form_element_text_new_attrs:
 * @id:
 * @...:
 *
 * Returns: the newly created #ZakFormCgiFormElementText object.
 */
ZakFormCgiFormElement
*zak_form_cgi_form_element_text_new_attrs (const gchar *id,
									  ...)
{
	va_list ap;

	ZakFormCgiFormElement *zak_form_cgi_form_element_text;

	zak_form_cgi_form_element_text = zak_form_cgi_form_element_text_new ();

	va_start (ap, id);

	ZAK_FORM_CGI_FORM_ELEMENT_CLASS (zak_form_cgi_form_element_text_parent_class)->construct (zak_form_cgi_form_element_text,
																					id,
																					zak_cgi_commons_valist_to_ghashtable (ap));

	return zak_form_cgi_form_element_text;
}

void
zak_form_cgi_form_element_text_xml_parsing (ZakFormElement *element, xmlNodePtr xmlnode)
{
	gchar *id;

	GHashTable *ht_attrs;

	xmlNode *cur;

	id = NULL;

	ht_attrs = g_hash_table_new (g_str_hash, g_str_equal);

	cur = xmlnode->children;
	while (cur != NULL)
		{
			if (xmlStrcmp (cur->name, (const xmlChar *)"id") == 0)
				{
					id = (gchar *)xmlNodeGetContent (cur);
				}
			else if (xmlStrcmp (cur->name, (const xmlChar *)"label") == 0)
				{
					zak_form_cgi_form_element_set_label (ZAK_FORM_CGI_FORM_ELEMENT (element), (gchar *)xmlNodeGetContent (cur), NULL);
				}
			else if (xmlStrcmp (cur->name, (const xmlChar *)"text") == 0)
				{
				}
			else
				{
					g_hash_table_replace (ht_attrs, g_strdup (cur->name), (gchar *)xmlNodeGetContent (cur));
				}

			cur = cur->next;
		}

	if (id != NULL)
		{
			ZAK_FORM_CGI_FORM_ELEMENT_CLASS (zak_form_cgi_form_element_text_parent_class)->construct (ZAK_FORM_CGI_FORM_ELEMENT (element),
																							id,
																							ht_attrs);
		}
}

static gchar
*zak_form_cgi_form_element_text_render (ZakFormCgiFormElement *element)
{
	gchar *ret;

	GHashTable *ht_attrs;

	gchar *value;

	ZakFormCgiFormElementClass *klass;

	klass = (ZakFormCgiFormElementClass *)g_type_class_peek_parent (ZAK_FORM_CGI_FORM_ELEMENT_TEXT_GET_CLASS (ZAK_FORM_CGI_FORM_ELEMENT_TEXT (element)));

	ht_attrs = klass->get_ht_attrs (element);

	value = zak_form_element_get_value (ZAK_FORM_ELEMENT (element));
	if (value != NULL)
		{
			g_hash_table_insert (ht_attrs, (gpointer)"value", (gpointer)g_strdup (value));
		}

	ret = zak_cgi_tag_text_ht (zak_form_cgi_form_element_get_id (element), ht_attrs);

	return ret;
}

/* PRIVATE */
static void
zak_form_cgi_form_element_text_set_property (GObject *object,
                   guint property_id,
                   const GValue *value,
                   GParamSpec *pspec)
{
	ZakFormCgiFormElementText *zak_form_cgi_form_element_text = (ZakFormCgiFormElementText *)object;
	ZakFormCgiFormElementTextPrivate *priv = ZAK_FORM_CGI_FORM_ELEMENT_TEXT_GET_PRIVATE (zak_form_cgi_form_element_text);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
zak_form_cgi_form_element_text_get_property (GObject *object,
                   guint property_id,
                   GValue *value,
                   GParamSpec *pspec)
{
	ZakFormCgiFormElementText *zak_form_cgi_form_element_text = (ZakFormCgiFormElementText *)object;
	ZakFormCgiFormElementTextPrivate *priv = ZAK_FORM_CGI_FORM_ELEMENT_TEXT_GET_PRIVATE (zak_form_cgi_form_element_text);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
zak_form_cgi_form_element_text_dispose (GObject *gobject)
{
	ZakFormCgiFormElementText *zak_form_cgi_form_element_text = (ZakFormCgiFormElementText *)gobject;
	ZakFormCgiFormElementTextPrivate *priv = ZAK_FORM_CGI_FORM_ELEMENT_TEXT_GET_PRIVATE (zak_form_cgi_form_element_text);



	GObjectClass *parent_class = g_type_class_peek_parent (G_OBJECT_GET_CLASS (gobject));
	parent_class->dispose (gobject);
}

static void
zak_form_cgi_form_element_text_finalize (GObject *gobject)
{
	ZakFormCgiFormElementText *zak_form_cgi_form_element_text = (ZakFormCgiFormElementText *)gobject;
	ZakFormCgiFormElementTextPrivate *priv = ZAK_FORM_CGI_FORM_ELEMENT_TEXT_GET_PRIVATE (zak_form_cgi_form_element_text);



	GObjectClass *parent_class = g_type_class_peek_parent (G_OBJECT_GET_CLASS (gobject));
	parent_class->finalize (gobject);
}
