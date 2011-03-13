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

static void on_move (GbkCube * cube, gpointer m, GbkGame * game);

enum
{
  QUEUE_CHANGED,
  MARK_SET,
  n_SIGNALS
};

static guint signals[n_SIGNALS];

static void
gbk_game_init (GbkGame * game)
{
  game->mesg_id = 0;
  game->start_of_moves.next = &game->end_of_moves;

  gbk_game_reset (game);
}


void
gbk_game_reset (GbkGame * game)
{
  game->animate_complete_id = 0;
  game->mode = MODE_RECORD;

  gbk_game_delete_moves (game, game->start_of_moves.next);

  game->end_of_moves.prev = &game->start_of_moves;
  game->end_of_moves.next = NULL;
  game->end_of_moves.data = NULL;

  game->start_of_moves.prev = NULL;
  game->start_of_moves.next = &game->end_of_moves;
  game->start_of_moves.data = NULL;

  game->iter = &game->start_of_moves;

  game->posn = 0;
  game->total = 0;

  g_signal_emit (game, signals[QUEUE_CHANGED], 0);
}



enum
{
  PROP_0 = 0,
  PROP_CUBE
};

static void
game_set_property (GObject * object,
		   guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GbkGame *game = GBK_GAME (object);

  switch (prop_id)
    {
    case PROP_CUBE:
      game->cube = GBK_CUBE (g_value_get_object (value));
      g_signal_connect (game->cube, "move", G_CALLBACK (on_move), game);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    };
}


static void
game_get_property (GObject * object,
		   guint prop_id, GValue * value, GParamSpec * pspec)
{
  GbkGame *game = GBK_GAME (object);

  switch (prop_id)
    {
    case PROP_CUBE:
      g_value_set_object (value, game->cube);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    };
}


static void
gbk_game_class_init (GbkGameClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = game_set_property;
  gobject_class->get_property = game_get_property;

  GParamSpec *gbk_param_spec = g_param_spec_object ("cube",
						    "Set the cube",
						    "The cube for this game",
						    GBK_TYPE_CUBE,
						    G_PARAM_READWRITE);

  g_object_class_install_property (gobject_class, PROP_CUBE, gbk_param_spec);



  signals[QUEUE_CHANGED] =
    g_signal_new ("queue-changed",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  0,
		  NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  signals[MARK_SET] =
    g_signal_new ("mark-set",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  0,
		  NULL, NULL,
		  g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
}

G_DEFINE_TYPE (GbkGame, gbk_game, G_TYPE_OBJECT);

GObject *
gbk_game_new (GbkCube * cube)
{
  return g_object_new (gbk_game_get_type (), "cube", cube, NULL);
}

void
gbk_game_remove_view (GbkGame * game, GbkCubeview * cv)
{
  GSList *element = g_slist_find (game->views, cv);

  if (element->data == game->masterview)
    g_critical ("Removing the master view");

  game->views = g_slist_remove_link (game->views, element);
}

void
gbk_game_add_view (GbkGame * game, GbkCubeview * cv, gboolean master)
{
  game->views = g_slist_prepend (game->views, cv);
  if (master)
    game->masterview = cv;
  else
    {
      g_signal_connect_swapped (cv, "destroy",
				G_CALLBACK (gbk_game_remove_view), game);
    }
}

void
gbk_game_set_mark (GbkGame * game)
{
  game->iter->marked = TRUE;

  g_signal_emit (game, signals[MARK_SET], 0, game->posn);
}


gboolean
gbk_game_at_end (GbkGame * game)
{
  return (game->iter->next == &game->end_of_moves);
}


gboolean
gbk_game_at_start (GbkGame * game)
{
  return (game->iter == &game->start_of_moves);
}





/* Delete all the moves beginning at FROM */
void
gbk_game_delete_moves (GbkGame * game, struct GbkList *from)
{
  struct GbkList *n = from;
  struct GbkList *prev = from->prev;
  while (n != &game->end_of_moves)
    {
      struct GbkList *next = n->next;

      if (n == game->iter)
	game->iter = prev;

      move_unref (n->data);
      g_slice_free (struct GbkList, n);
      game->total--;

      n = next;
    }

  if (prev)
    prev->next = &game->end_of_moves;
  game->end_of_moves.prev = prev;
}

/* Insert MOVE into the queue before WHERE */
struct GbkList *
gbk_game_insert_move (GbkGame * game, struct move_data *move,
		      struct GbkList *where)
{
  struct GbkList *before = where->prev;
  struct GbkList *n = g_slice_alloc0 (sizeof *n);

  if (where != &game->end_of_moves)
    {
      g_warning ("Inserting in middle of moves");
      gbk_game_delete_moves (game, where);
    }

  n->prev = before;
  n->next = where;
  n->data = move_ref (move);
  n->marked = FALSE;


  if (before)
    before->next = n;

  if (game->start_of_moves.next == NULL)
    {
      game->start_of_moves.next = n;
    }

  where->prev = n;

  game->total++;

  return n;
}


static void
on_move (GbkCube * cube, gpointer m, GbkGame * game)
{
  struct move_data *move = m;

  if (game->mode != MODE_RECORD)
    return;

  if (game->animate_complete_id != 0)
    return;

  if (game->iter->next != &game->end_of_moves)
    gbk_game_delete_moves (game, game->iter->next);

  game->iter = gbk_game_insert_move (game, move, game->iter->next);

  game->posn++;

  g_signal_emit (game, signals[QUEUE_CHANGED], 0);
}



void
gbk_game_rewind (GbkGame * game)
{
  game->mode = MODE_REWIND;

  while (!gbk_game_at_start (game))
    {
      const struct move_data *m = game->iter->data;

      struct move_data *mm = move_copy (m);
      mm->dir = !mm->dir;
      gbk_cube_rotate_slice (game->cube, mm);
      move_unref (mm);
      game->posn--;
      game->iter = game->iter->prev;
      if (game->iter->marked)
	break;
    }

  g_signal_emit (game, signals[QUEUE_CHANGED], 0);

  game->mode = MODE_RECORD;
}

typedef gboolean term_pred (GbkGame *);

static void
next_move (GbkGame * game, gboolean backwards)
{
  struct move_data *m;

  /* Choose the terminal predicate. That is, when to stop moving. */
  term_pred *terminal = backwards ? gbk_game_at_start : gbk_game_at_end;

  if (game->mode != MODE_PLAY || terminal (game))
    {
      if (game->animate_complete_id != 0)
	g_signal_handler_disconnect (game->masterview,
				     game->animate_complete_id);

      game->animate_complete_id = 0;
      game->mode = MODE_RECORD;

      g_signal_emit (game, signals[QUEUE_CHANGED], 0);
      return;
    }

  if (game->animate_complete_id == 0)
    {
      /* Single stepping */
      game->mode = MODE_RECORD;
      game->animate_complete_id =
	g_signal_connect_swapped (game->masterview,
				  "animation-complete",
				  G_CALLBACK (next_move), game);
    }

  if (backwards)
    {
      m = move_copy (game->iter->data);
      m->dir = !m->dir;
      game->iter = game->iter->prev;
      game->posn--;
    }
  else
    {
      game->iter = game->iter->next;
      m = move_copy (game->iter->data);

      game->posn++;
    }

  gbk_cube_rotate_slice (game->cube, m);

  move_unref (m);

  g_signal_emit (game, signals[QUEUE_CHANGED], 0);
}

static void
move_forward (GbkGame * game)
{
  next_move (game, FALSE);
}

void
gbk_game_stop_replay (GbkGame * game)
{
  game->mode = MODE_RECORD;
}

void
gbk_game_replay (GbkGame * game)
{
  game->mode = MODE_PLAY;

  game->animate_complete_id =
    g_signal_connect_swapped (game->masterview,
			      "animation-complete",
			      G_CALLBACK (move_forward), game);

  move_forward (game);
}

void
gbk_game_prev_move (GbkGame * game)
{
  game->mode = MODE_PLAY;

  next_move (game, TRUE);
}

void
gbk_game_next_move (GbkGame * game)
{
  game->mode = MODE_PLAY;

  move_forward (game);
}

void
gbk_game_dump_moves (GbkGame * game)
{
  struct GbkList *n = game->start_of_moves.next;
  g_print ("Start %p\n", n);

  while (n != &game->end_of_moves)
    {
      if (n == game->iter)
	printf ("* ");
      else
	printf ("  ");
      move_dump (n->data);
      n = n->next;
    }
  printf ("\n");
}
