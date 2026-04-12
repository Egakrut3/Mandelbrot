INC_DIR = inc/

SRC_DIR	= src/
C_SUF	= .c
make_c_path		= $(addprefix $(SRC_DIR), $(addsuffix $(C_SUF),		$(1)))

OBJ_DIR	= bin/
OBJ_SUF	= .obj
make_obj_path	= $(addprefix $(OBJ_DIR), $(addsuffix $(OBJ_SUF),	$(1)))

SRC			= Mandelbrot main
SLOW_SRC	= Slow_Mandelbrot main

TARGET		= Test
SLOW_TARGET	= Slow_Test

C_OPTIONS	=	-D_DEBUG -D__STDC_WANT_LIB_EXT1__ -ggdb3 -std=c23 -O3 -mavx512f -Wall -Wextra -Waggressive-loop-optimizations -Wmissing-declarations					\
				-Wcast-align -Wcast-qual -Wchar-subscripts -Wconversion -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness			\
				-Wformat=2 -Winline -Wlogical-op -Wopenmp-simd -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion						\
				-Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef		\
				-Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing -Wno-varargs -Wstack-protector -fcheck-new	\
				-fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla			\
				#-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

CPP_OPTIONS	=	-Weffc++ -Wc++14-compat -Woverloaded-virtual -Wconditionally-supported -Wctor-dtor-privacy -Wnon-virtual-dtor -Wsign-promo -Wstrict-null-sentinel	\
				-Wsuggest-override -Wno-literal-suffix -Wno-old-style-cast -fsized-deallocation

.PHONY: all prepare test slow_test clean commit

all: test

prepare:
	@mkdir -p $(OBJ_DIR)

make_c_object = $(call make_obj_path, $(1)): $(call make_c_path, $(1)) | prepare; \
	@gcc $(C_OPTIONS) -I$(INC_DIR) -c $$< -o $$@

$(foreach src, $(SRC), $(eval $(call make_c_object, $(src))))

$(call make_c_object, Slow_Mandelbrot)

$(TARGET): $(call make_obj_path, $(SRC))
	@gcc $(C_OPTIONS) $^ -lglfw -lGL -o $(TARGET)

$(SLOW_TARGET): $(call make_obj_path, $(SLOW_SRC))
	@gcc $(C_OPTIONS) $^ -lglfw -lGL -o $(SLOW_TARGET)

test: $(TARGET)
	@prime-run taskset -c 15 ./$(TARGET)

slow_test: $(SLOW_TARGET)
	@prime-run taskset -c 15 ./$(SLOW_TARGET)

clean:
	@rm -fr	$(OBJ_DIR) $(TARGET)

commit:
	@git add .
	@git commit -m "$(MSG)"
	@git push
