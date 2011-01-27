/*
  Copyright (c) 2011        John Darrington

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

#include <config.h>

#include "game.h"

static void on_move (GbkCube *cube, gpointer m, GbkGame *game);

enum  
{
  QUEUE_CHANGED,
  n_SIGNALS
};

static guint signals [n_SIGNALS];

static void
gbk_game_init (GbkGame *game)
{
  game->mode = MODE_RECORD;
  game->animate_complete_id = 0;

  game->head.next = NULL; 
  game->head.prev = NULL;
  game->head.data = NULL;

  game->iter = &game->head;
}

enum
  {
    PROP_0 = 0,
  };

static void
game_set_property (GObject         *object,
		   guint            prop_id,
		   const GValue    *value,
		   GParamSpec      *pspec)
{
  GbkGame *game = GBK_GAME (object);

  switch (prop_id)
    {
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    };
}


static void
game_get_property (GObject         *object,
		   guint            prop_id,
		   GValue          *value,
		   GParamSpec      *pspec)
{
  GbkGame *game  = GBK_GAME (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    };
}


static void
gbk_game_class_init (GbkGameClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = game_set_property;
  gobject_class->get_property = game_get_property;


  signals [QUEUE_CHANGED] =
    g_signal_new ("queue-changed",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  0,
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE,
		  0);
}

G_DEFINE_TYPE (GbkGame, gbk_game, G_TYPE_OBJECT);

GObject*
gbk_game_new (GbkCube *cube)
{
  GObject *g = g_object_new (gbk_game_get_type (),  NULL);

  GBK_GAME (g)->cube = cube;

  g_signal_connect (cube, "move", G_CALLBACK (on_move), g);

  return g;
}

void
gbk_game_set_master_view (GbkGame *game, GbkCubeview *cv)
{
  game->cubeview = cv;
}

gboolean
gbk_game_at_start (GbkGame *game)
{
  return (game->iter == &game->head);
}


gboolean
gbk_game_at_end (GbkGame *game)
{
  return (game->iter->next == NULL); 
}



static void
on_move (GbkCube *cube, gpointer m, GbkGame *game)
{
  struct move_data *move = m;

  if ( game->mode != MODE_RECORD)
    return;

  if ( game->animate_complete_id != 0)
    return;


  GList *d0 = game->head.next;
  GList *n = g_slice_alloc0 (sizeof *n);

  n->next = d0;
  n->prev = &game->head;
  n->data = move_copy (move);

  game->head.next = n;

  if ( d0)
    d0->prev = n;

  game->iter = &game->head;

  g_signal_emit (game, signals [QUEUE_CHANGED], 0);
}


void
gbk_game_rewind (GbkGame *game)
{
  game->mode = MODE_REWIND;

  while (!gbk_game_at_end (game))
    {
      game->iter = game->iter->next;
      const struct move_data *m = game->iter->data;

      struct move_data *mm = move_copy (m);
      mm->dir = ! mm->dir;
      gbk_cube_rotate_slice (game->cube, mm);
    }

  g_signal_emit (game, signals [QUEUE_CHANGED], 0);

  game->mode = MODE_RECORD;
}

typedef gboolean term_pred (GbkGame *);

static void
next_move (GbkGame *game, gboolean backwards)
{
  struct move_data *m;

  term_pred * terminal = backwards ? 
    gbk_game_at_end : gbk_game_at_start ;

  if ( game->mode != MODE_PLAY ||
       terminal (game) )
    {
      g_signal_handler_disconnect (game->cubeview,
				   game->animate_complete_id);
      game->animate_complete_id = 0;
      game->mode = MODE_RECORD;

      g_signal_emit (game, signals [QUEUE_CHANGED], 0);
      return;
    }

  if ( game->animate_complete_id == 0)
    {
      /* Single stepping */
      game->mode = MODE_RECORD;
      game->animate_complete_id = 
	g_signal_connect_swapped (game->cubeview,
			      "animation-complete",
				  G_CALLBACK (next_move), game);
    }

  if ( backwards )
    {
      game->iter = game->iter->next;
      m = game->iter->data;

      m = move_copy (m);
      m->dir = ! m->dir;
    }
  else
    {
      m = game->iter->data;
      game->iter = game->iter->prev;
    }

  gbk_cube_rotate_slice (game->cube, move_copy (m));

  g_signal_emit (game, signals [QUEUE_CHANGED], 0);
}

static void
move_forward (GbkGame *game)
{
  next_move (game, FALSE);
}

void 
gbk_game_stop_replay (GbkGame *game)
{
  game->mode = MODE_RECORD;
}

void
gbk_game_replay (GbkGame *game)
{
  game->mode = MODE_PLAY;

  game->animate_complete_id = 
    g_signal_connect_swapped (game->cubeview,
			      "animation-complete",
		      G_CALLBACK (move_forward), game);

  move_forward (game);
}

void
gbk_game_prev_move (GbkGame *game)
{
  game->mode = MODE_PLAY;

  next_move (game, TRUE);
}

void
gbk_game_next_move (GbkGame *game)
{
  game->mode = MODE_PLAY;

  move_forward (game);
}
