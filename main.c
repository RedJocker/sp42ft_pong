/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maurodri <maurodri@student.42sp...>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/02 15:28:12 by maurodri          #+#    #+#             */
/*   Updated: 2024/02/04 04:21:47 by maurodri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <MLX42/MLX42.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// 32 * 16
#define WIDTH 512
#define HEIGHT 512

typedef enum e_exit_status
{
	OK,
	ERROR
}	t_exit_status;

typedef struct s_color
{
	int32_t	r;
	int32_t	g;
	int32_t	b;
	int32_t	a;
}	t_color;

typedef struct s_position
{
	double	x;
	double	y;
}	t_position;

typedef struct s_velocity
{
	double	x;
	double	y;
}	t_velocity;

typedef struct s_object
{
	char		*name;
	mlx_image_t	*img;
	t_position	pos;
	t_velocity	vel;
}	t_object;

typedef enum e_state
{
	SERVE1,
	SERVE2,
	RUNNING,
	FINISHED,
} t_state;

typedef struct s_score
{
	int 		player1;
	int 		player2;
	mlx_image_t *board_img;
}	t_score;

typedef struct s_game
{
	t_object		paddle1;
	t_object		paddle2;
	t_object        ball;
	mlx_image_t		*field;
	mlx_t      		*mlx;
	t_state     	state;
	t_score          score;	
	t_exit_status	exit_status;
}	t_game;

int32_t	panic(t_game *game)
{
	game->exit_status = ERROR;
	if (game->mlx)
		mlx_close_window(game->mlx);
	puts(mlx_strerror(mlx_errno));
	return (ERROR);
}

int32_t	ft_color_to_int(t_color *color)
{
	u_int32_t c;

	c = 0;
	c = color->r;
	c <<= 8;
	c |= color->g;
	c <<= 8;
	c |= color->b;
	c <<= 8;
	c |= color->a;
	return (int32_t)(c);
}

void	ft_randomize(void *param)
{
	t_color		c;
	mlx_image_t	*image;
	uint32_t	i;
	uint32_t	y;

	image = param;
	i = 0;
	while (i < image->width)
	{
		y = 0;
		while (y < image->height)
		{
			c.a = 0xFF;
			c.r = rand() % 0xFF;
			c.g = rand() % 0xFF;
			c.b = rand() % 0xFF;
			mlx_put_pixel(image, i, y, ft_color_to_int(&c));
			++y;
		}
		++i;
	}
}

int objects_are_coliding(t_object *o1, t_object *o2)
{
	return (!(o1->pos.x > o2->pos.x + o2->img->width
			  || o1->pos.x + + o1->img->width < o2->pos.x
			  || o1->pos.y > o2->pos.y + o2->img->height
			  || o1->pos.y + o1->img->width < o2->pos.y));
}

int32_t x_centralized_on_field(int32_t width, t_game *game)
{
	return (game->field->width - width) / 2.0 + game->field->instances[0].x;
}

int32_t y_centralized_on_field(int32_t height, t_game *game)
{
	return (game->field->height - height) / 2.0 + game->field->instances[0].y;
}

int update_scoreboard(t_game *game)
{
	char score[50];

	if (game->score.board_img)
		mlx_delete_image(game->mlx, game->score.board_img);
	sprintf(score, "%d x %d", game->score.player1, game->score.player2);
	game->score.board_img =
	    mlx_put_string(game->mlx, score, x_centralized_on_field(300, game), 64);
	if (!game->score.board_img)
		return (0);
	mlx_resize_image(game->score.board_img, 300, 100);
	mlx_set_instance_depth(&game->score.board_img->instances[0], 0);
	return (1);
}

void	paddle_key_listener(
	t_object *paddle, int32_t key_up, int32_t key_down, mlx_t *mlx)
{
	double	factor;

	if (mlx_is_key_down(mlx, key_up) && mlx_is_key_down(mlx, key_down))
		paddle->vel.y = 0;
	else if (mlx_is_key_down(mlx, key_up))
		paddle->vel.y = -500;
	else if (mlx_is_key_down(mlx, key_down))
		paddle->vel.y = 500;
	else
	{
		factor = 1000 * mlx->delta_time;
		paddle->vel.y /= factor * (factor > 1.0) + 1.5 * (factor < 1.0);
	}
	paddle->pos.y += paddle->vel.y * mlx->delta_time;
}


#define MAX_ANGLE (4.0 * M_PI / 9.0)

void	paddle_reflection(t_object *paddle, t_object *ball)
{
	double 	ball_center_y;
	int32_t	direction;
	double	relative_intersection;
	double  normalized_intersection;
	double  bounce_angle;

	ball_center_y = ball->pos.y + (ball->img->height / 2.0);
	direction = 1 * (ball->vel.x < 0) + (-1) * (ball->vel.x > 0);	
	relative_intersection =
		paddle->pos.y + (paddle->img->height / 2.0) - ball_center_y;
	normalized_intersection = relative_intersection / paddle->img->height / 2.0;
	bounce_angle = normalized_intersection * MAX_ANGLE;
	normalized_intersection *=
	    (normalized_intersection > 0) + (-1) * (normalized_intersection < 0);  	
	ball->vel.x = 1200 * (normalized_intersection + 0.5) * cos(bounce_angle) * direction;
	ball->vel.y = 1200 * (normalized_intersection + 0.5) * (-sin(bounce_angle));
}

void	ball_update(t_object *ball, t_game *game)
{
	if (objects_are_coliding(ball, &game->paddle1))
	{
		ball->pos.x = game->paddle1.pos.x + game->paddle1.img->width;
		paddle_reflection(&game->paddle1, ball);
	}
	else if (objects_are_coliding(ball, &game->paddle2))
	{
		ball->pos.x = game->paddle2.pos.x - ball->img->width;
		paddle_reflection(&game->paddle2, ball);
	}
	if (ball->pos.y < game->field->instances[0].y)
	{
		ball->vel.y *= -1;
		ball->pos.y = game->field->instances[0].y + 1;
	}
	else if (ball->pos.y + ball->img->height
			 > game->field->instances[0].y + game->field->height)
	{
	  	ball->vel.y *= -1;
		ball->pos.y = game->field->height
			- game->field->instances[0].y - ball->img->height - 1;
	}
	if (ball->pos.x < game->field->instances[0].x)
	{
		game->score.player2++;
		game->state = SERVE2;
		if (!update_scoreboard(game))
			panic(game);
	}
	else if (ball->pos.x + ball->img->width
			 > game->field->instances[0].x + game->field->width)
	{
		game->score.player1++;
		game->state = SERVE1;
		if (!update_scoreboard(game))
			panic(game);
	}
	ball->pos.x += ball->vel.x * game->mlx->delta_time;
	ball->pos.y += ball->vel.y * game->mlx->delta_time;
}

void	ft_hook(void *param)
{
	t_game	*game;

	game = param;
	//printf("delta_time = %.4f\n", game->mlx->delta_time);
	if (mlx_is_key_down(game->mlx, MLX_KEY_ESCAPE))
	{
		game->exit_status = OK;
		mlx_close_window(game->mlx);
	}
	if (game->state == RUNNING)
	{
		paddle_key_listener(&game->paddle1, MLX_KEY_W, MLX_KEY_S, game->mlx);
		paddle_key_listener(&game->paddle2, MLX_KEY_O, MLX_KEY_L, game->mlx);
		ball_update(&game->ball, game);
	}
	else if (game->state == SERVE1 || game->state == SERVE2)
	{
		game->ball.vel.x =
		    300 * (game->state == SERVE1) + (-300) * (game->state == SERVE2);
		game->ball.pos.x = x_centralized_on_field(game->ball.img->width, game);
		game->ball.pos.y = y_centralized_on_field(game->ball.img->height, game);
		game->paddle1.pos.y =
		    y_centralized_on_field(game->paddle1.img->height, game);
		game->paddle2.pos.y =
			y_centralized_on_field(game->paddle2.img->height, game);
		if (mlx_is_key_down(game->mlx, MLX_KEY_SPACE))
			game->state = RUNNING;
	}
	game->ball.img->instances[0].x = (int) game->ball.pos.x;
	game->ball.img->instances[0].y = (int) game->ball.pos.y;
	game->paddle1.img->instances[0].y = (int32_t) game->paddle1.pos.y;
	game->paddle2.img->instances[0].y = (int32_t) game->paddle2.pos.y;
}

void paint_field(mlx_image_t *field)
{
	uint32_t i;
	uint32_t y;

	i = 0;
	while (i < field->width)
	{
		y = 0;
		while (y < field->height)
		{
			mlx_put_pixel(field, i, y, 0x000408FF);
			++y;
		}
		++i;
		mlx_set_instance_depth(&field->instances[0], -1);
	}
}

void close_fun(void* param)
{
	t_game	*game = param;
	game->exit_status = OK;
	if (game->mlx)
		mlx_close_window(game->mlx);
}

int32_t	main(void)
{
	t_game	game;

	game.mlx = mlx_init(WIDTH, HEIGHT, "MLX42", true);
	game.paddle1.img = mlx_new_image(game.mlx, 32, 128);
	game.paddle2.img = mlx_new_image(game.mlx, 32, 128);
	game.ball.img = mlx_new_image(game.mlx, 32, 32);
	game.field = mlx_new_image(game.mlx, HEIGHT, WIDTH);
	//mlx_set_setting(MLX_STRETCH_IMAGE, true);
	if (!game.mlx)
		return (panic(&game));
	if (!game.paddle1.img)
		return (panic(&game));
	if (mlx_image_to_window(
			game.mlx, game.field, 0, 0) == -1)
		return (panic(&game));
	if (mlx_image_to_window(
			game.mlx, game.paddle1.img, 32, (HEIGHT - 128) / 2) == -1)
		return (panic(&game));
	if (!game.paddle2.img)
		return (panic(&game));
	if (mlx_image_to_window(
			game.mlx, game.paddle2.img, WIDTH - 64, (HEIGHT - 128) / 2) == -1)
		return (panic(&game));
	if (!game.ball.img)
		return (panic(&game));
	if (mlx_image_to_window(game.mlx, game.ball.img,
			(WIDTH / 2) - 16, (HEIGHT / 2) - 16) == -1)
		return (panic(&game));
	mlx_set_window_title(game.mlx, "ft_pong");
	game.score.player1 = 0;
	game.score.player2 = 0;
	if (!update_scoreboard(&game))
		return (panic(&game));
	mlx_close_hook(game.mlx, close_fun, &game);
	paint_field(game.field);
	game.state = SERVE1;
	game.paddle1.vel.x = 0;
	game.paddle2.vel.x = 0;
	game.paddle1.vel.y = 0;
	game.paddle2.vel.y = 0;
	game.paddle1.pos.x = game.paddle1.img->instances[0].x;
	game.paddle2.pos.x = game.paddle2.img->instances[0].x;
	game.paddle1.pos.y = game.paddle1.img->instances[0].y;
	game.paddle2.pos.y = game.paddle2.img->instances[0].y;
	game.ball.pos.x = game.ball.img->instances[0].x;
	game.ball.pos.y = game.ball.img->instances[0].y;
	game.ball.vel.x = 300;
	game.ball.vel.y = 100;
	mlx_loop_hook(game.mlx, ft_randomize, game.ball.img);
	mlx_loop_hook(game.mlx, ft_randomize, game.paddle1.img);
	mlx_loop_hook(game.mlx, ft_randomize, game.paddle2.img);
	mlx_loop_hook(game.mlx, ft_hook, &game);
	mlx_loop(game.mlx);
	mlx_terminate(game.mlx);
	return (game.exit_status);
}
