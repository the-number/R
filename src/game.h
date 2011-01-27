/*
    Copyright (c) 2011   John Darrington

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License,  or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __GBK_GAME_H__
#define __GBK_GAME_H__


#include <glib-object.h>
#include <gtk/gtkwindow.h>
#include "cube.h"
#include "cubeview.h"

#define GBK_TYPE_GAME                  (gbk_game_get_type ())
#define GBK_GAME(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GBK_TYPE_GAME, GbkGame))
#define GBK_IS_GAME(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GBK_TYPE_GAME))
#define GBK_GAME_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GBK_TYPE_GAME, GbkGameClass))
#define GBK_IS_GAME_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GBK_TYPE_GAME))
#define GBK_GAME_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GBK_TYPE_GAME, GbkGameClass))

typedef struct _GbkGame        GbkGame;
typedef struct _GbkGameClass   GbkGameClass;

enum mode
  {
    MODE_NONE = 0,
    MODE_RECORD,
    MODE_PLAY,
    MODE_REWIND,
  };


struct _GbkGame
{
  GObject parent_instance;

  /* instance members */

  /* The toplevel window in which the game is played */
  GtkWindow *toplevel;

  /* The cube */
  GbkCube *cube;

  /* The cubeview which we'll consider as the master one */
  GbkCubeview *cubeview;

  enum mode mode;

  GList head;
  GList *iter;

  //  struct deque queue;

  /* Id of the animate complete signal */
  gulong animate_complete_id;
};

struct _GbkGameClass
{
  GObjectClass parent_class;

  /* class members */
};

/* used by GBK_TYPE_GAME */
GType gbk_game_get_type (void);

/*
 * Method definitions.
 */

GObject* gbk_game_new (GbkCube *cube);

void gbk_game_set_master_view (GbkGame *game, GbkCubeview *cv);

void gbk_game_rewind (GbkGame *game);
void gbk_game_replay (GbkGame *game);
void gbk_game_stop_replay (GbkGame *game);

void gbk_game_next_move (GbkGame *game);
void gbk_game_prev_move (GbkGame *game);

gboolean gbk_game_at_start (GbkGame *game);
gboolean gbk_game_at_end (GbkGame *game);

#endif /* __GAME_H__ */
