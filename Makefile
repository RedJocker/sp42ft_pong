# cc main.c ... libmlx42.a -Iinclude -ldl -lglfw -pthread -lm

NAME := ft_pong
LIBMLX_DIR := ./MLX42
LIBMLX := $(LIBMLX_DIR)/build/libmlx42.a
SRCS := main.c
OBJS := $(patsubst %.c, %.o, $(SRCS))
DEP_FLAGS := -MP -MD -MF
DEP_DIR := ./dep/
DEP_FILES := $(addprefix $(DEP_DIR), $(addsuffix .d, $(OBJS)))
INCLUDES := -I./ -I$(LIBMLX_DIR)/include
CFLAGS := -Wall -Wextra -Werror -g -fsanitize=address -fsanitize=undefined
LFLAGS := -ldl -lglfw -pthread -lm
CC := cc


all: $(NAME)

$(NAME): $(OBJS) $(LIBMLX)
	$(CC) $(CFLAGS) $^ $(INCLUDES) $(LFLAGS) -o $@
	etags $$(find . -name '*.[ch]')

$(OBJS): %.o : %.c | $(DEP_DIR)
	$(CC) $(CFLAGS) -c $< $(INCLUDES) -o $@ $(DEP_FLAGS) "$(DEP_DIR)$@.d"

$(DEP_DIR):
	@mkdir -p $@

$(LIBMLX):
	$(MAKE) -C $(LIBMLX_DIR) build #--no-print-directory

