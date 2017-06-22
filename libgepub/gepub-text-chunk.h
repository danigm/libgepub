/* GepubTextChunk
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
#define GEPUB_TEXT_CHUNK(obj)           (G_TYPE_CHECK_INSTANCE_CAST (obj, GEPUB_TYPE_TEXT_CHUNK, GepubTextChunk))
#define GEPUB_TEXT_CHUNK_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST (cls, GEPUB_TYPE_TEXT_CHUNK, GepubTextChunkClass))
#define GEPUB_IS_TEXT_CHUNK(obj)        (G_TYPE_CHECK_INSTANCE_TYPE (obj, GEPUB_TYPE_TEXT_CHUNK))
#define GEPUB_IS_TEXT_CHUNK_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE (obj, GEPUB_TYPE_TEXT_CHUNK))
#define GEPUB_TEXT_CHUNK_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GEPUB_TYPE_TEXT_CHUNK, GepubTextChunkClass))

typedef enum {
    GEPUBTextHeader,
    GEPUBTextBold,
    GEPUBTextItalic,
    GEPUBTextNormal
} GepubTextChunkType;

struct _GepubTextChunk {
    GObject parent;

    GepubTextChunkType type;
    gchar *text;
};

struct _GepubTextChunkClass {
    GObjectClass parent_class;
};

typedef struct _GepubTextChunk      GepubTextChunk;
typedef struct _GepubTextChunkClass GepubTextChunkClass;

GType               gepub_text_chunk_get_type     (void) G_GNUC_CONST;
GepubTextChunk     *gepub_text_chunk_new          (GepubTextChunkType type, const gchar *text);
const char         *gepub_text_chunk_type_str     (GepubTextChunk *chunk);
const char         *gepub_text_chunk_text         (GepubTextChunk *chunk);
GepubTextChunkType  gepub_text_chunk_type         (GepubTextChunk *chunk);

G_END_DECLS

#endif /* __GEPUB_TEXT_CHUNK_H__ */
