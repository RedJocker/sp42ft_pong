// -----------------------------------------------------------------------------
// Codam Coding College, Amsterdam @ 2022-2023 by W2Wizard.
// See README in the root project for more information.
// -----------------------------------------------------------------------------

#include <MLX42/MLX42.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 512
#define HEIGHT 512

typedef struct s_color
{
	char	r;
	char    g;
	char    b;
	char	a;
}	t_color;

typedef struct s_position
{
	double	x;
	double	y;
}	t_position;

typedef struct s_velocity
{
	double x;
	double y;
}	t_velocity;

typedef struct s_object
{
	char		*name;
	mlx_image_t	*img;
	t_position	pos;
	t_velocity	vel;	
}	t_object;

typedef struct	s_game
{
	t_object	paddle1;
	t_object	paddle2;
	t_object	ball;
	mlx_t		*mlx;
}	t_game;

int32_t	ft_color_to_int(t_color *color)
{
	return (color->r << 24 
			| color->g << 16
			| color->b << 8
	        | color->a);
}

void	ft_randomize(void *param)
{
	t_color		c;
	mlx_image_t *image;

	image = param;	
	for (uint32_t i = 0; i < image->width; ++i)
	{
		for (uint32_t y = 0; y < image->height; ++y)
		{
			c.a = 0xFF;
			c.r = rand() % 0xFF;
			c.g = rand() % 0xFF;
			c.b = rand() % 0xFF;
			uint32_t color = ft_color_to_int(&c);
			mlx_put_pixel(image, i, y, color);
		}
	}
}

void paddle_key_listener(
    t_object *paddle, int32_t key_up, int32_t key_down, mlx_t *mlx)
{
	if (mlx_is_key_down(mlx, key_up) && mlx_is_key_down(mlx, key_down))
		paddle->vel.y = 0;
	else if (mlx_is_key_down(mlx, key_up))
		paddle->vel.y = -500;
	else if (mlx_is_key_down(mlx, key_down))
		paddle->vel.y = 500;
	else
		paddle->vel.y /= 1.04;
	paddle->pos.y += paddle->vel.y * mlx->delta_time;	
	paddle->img->instances[0].y = (int) paddle->pos.y;
}    

void ft_hook(void *param)
{
	t_game	*game;

	game = param;
	printf("delta_time = %.4f\n", game->mlx->delta_time);
	if (mlx_is_key_down(game->mlx, MLX_KEY_ESCAPE))
		mlx_close_window(game->mlx);		
	paddle_key_listener(&game->paddle1, MLX_KEY_W, MLX_KEY_S, game->mlx);
	paddle_key_listener(&game->paddle2, MLX_KEY_O, MLX_KEY_L, game->mlx);
}

int32_t panic(t_game *game)
{
	if (game->mlx)
		mlx_close_window(game->mlx);
	puts(mlx_strerror(mlx_errno));
	return (EXIT_FAILURE);
}    

int32_t main(void)
{
	t_game 		game;

	if (!(game.mlx = mlx_init(WIDTH, HEIGHT, "MLX42", true)))
		return (panic(&game));
	if (!(game.paddle1.img = mlx_new_image(game.mlx, 32, 128)))
		return (panic(&game));
	if (mlx_image_to_window(game.mlx, game.paddle1.img, 32, (HEIGHT - 128) / 2) == -1)
		return (panic(&game));
	if (!(game.paddle2.img = mlx_new_image(game.mlx, 32, 128)))
		return (panic(&game));
	if (mlx_image_to_window(game.mlx, game.paddle2.img, WIDTH - 64, (HEIGHT - 128) / 2) == -1)
		return (panic(&game));
	if (!(game.ball.img = mlx_new_image(game.mlx, 32, 32)))
		return (panic(&game));
	if (mlx_image_to_window(game.mlx, game.ball.img,
	        (WIDTH / 2) - 16, (HEIGHT / 2) - 16 ) == -1)
		return (panic(&game));
	game.paddle1.vel.x = 0;
	game.paddle2.vel.x = 0;
	game.paddle1.pos.y = game.paddle1.img->instances[0].y;
	game.paddle2.pos.y = game.paddle2.img->instances[0].y;		
	mlx_loop_hook(game.mlx, ft_randomize, game.paddle1.img);
	mlx_loop_hook(game.mlx, ft_randomize, game.paddle2.img);
	mlx_loop_hook(game.mlx, ft_randomize, game.ball.img);	
	mlx_loop_hook(game.mlx, ft_hook, &game);
	mlx_loop(game.mlx);
	mlx_terminate(game.mlx);
	return (EXIT_SUCCESS);
}
