/* GEPUBTextChunk
 *
 * Copyright (C) 2011  Daniel Garcia <danigm@wadobo.com>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GEPUB_TEXT_CHUNK_H__
#define __GEPUB_TEXT_CHUNK_H__

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define GEPUB_TYPE_TEXT_CHUNK           (gepub_text_chunk_get_type ())
#define GEPUB_TEXT_CHUNK(obj)           (G_TYPE_CHECK_INSTANCE_CAST (obj, GEPUB_TYPE_TEXT_CHUNK, GEPUBTextChunk))
#define GEPUB_TEXT_CHUNK_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST (cls, GEPUB_TYPE_TEXT_CHUNK, GEPUBTextChunkClass))
#define GEPUB_IS_TEXT_CHUNK(obj)        (G_TYPE_CHECK_INSTANCE_TYPE (obj, GEPUB_TYPE_TEXT_CHUNK))
#define GEPUB_IS_TEXT_CHUNK_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE (obj, GEPUB_TYPE_TEXT_CHUNK))
#define GEPUB_TEXT_CHUNK_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GEPUB_TYPE_TEXT_CHUNK, GEPUBTextChunkClass))

enum _GEPUBTextChunkType {
    GEPUBTextHeader,
    GEPUBTextBold,
    GEPUBTextItalic,
    GEPUBTextNormal
};

typedef enum _GEPUBTextChunkType     GEPUBTextChunkType;

struct _GEPUBTextChunk {
    GObject parent;

    GEPUBTextChunkType type;
    guchar *text;
};

struct _GEPUBTextChunkClass {
    GObjectClass parent_class;
};

typedef struct _GEPUBTextChunk      GEPUBTextChunk;
typedef struct _GEPUBTextChunkClass GEPUBTextChunkClass;

GType             gepub_text_chunk_get_type     (void) G_GNUC_CONST;
GEPUBTextChunk   *gepub_text_chunk_new          (GEPUBTextChunkType type, const guchar *path);
gchar            *gepub_text_chunk_type_str     (GEPUBTextChunk *chunk);

G_END_DECLS

#endif /* __GEPUB_TEXT_CHUNK_H__ */
