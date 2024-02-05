/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maurodri <maurodri@student.42sp...>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/02 15:28:12 by maurodri          #+#    #+#             */
/*   Updated: 2024/02/05 09:04:57 by maurodri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <MLX42/MLX42.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "scratch.h"

int32_t	system_panic(t_game *game)
{
	game->exit_status = ERROR;
	if (game->mlx)
		mlx_close_window(game->mlx);
	puts(mlx_strerror(mlx_errno));
	return (ERROR);
}

void	system_exit_ok(void* param)
{
	t_game	*game = param;
	game->exit_status = OK;
	if (game->mlx)
		mlx_close_window(game->mlx);
}

int	colision_check(int32_t x, int32_t y, t_entity *e)
{
	return (x == e->x && y == e->y);
}

void loop(t_game *game)
{
	(void) game;
}

int32_t	init(t_game *game)
{
	t_drawable hero_drawable;
	
	game->mlx = mlx_init(WIDTH, HEIGHT, "So Long", true);
	if (!game->mlx)
		return (system_panic(game));
	game->hero_texture = mlx_load_png("./assets/hero.png");
	if (!game->hero_texture)
		(system_panic(game));
	hero_drawable.img = mlx_texture_to_image(game->mlx, game->hero_texture);
	if (!hero_drawable.img)
		 (system_panic(game));
	hero_drawable.i = mlx_image_to_window(game->mlx, hero_drawable.img, 10 * 32, 10 * 32);
	if (hero_drawable.i < 0)
		(system_panic(game));
	mlx_loop_hook(game->mlx, (void(*)(void *))loop, game);
	mlx_close_hook(game->mlx, system_exit_ok, game);
	return (OK);
}

int32_t	main(void)
{
	t_game			game;

	if (init(&game) == OK)
		mlx_loop(game.mlx);
	mlx_delete_texture(game.hero_texture);
	mlx_terminate(game.mlx);
	return (game.exit_status);
}
