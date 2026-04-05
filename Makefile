INC_DIR = inc/

SRC_DIR	= src/
C_SUF	= .cpp
make_c_path		= $(addprefix $(SRC_DIR), $(addsuffix $(C_SUF),		$(1)))

OBJ_DIR	= bin/
OBJ_SUF	= .obj
make_obj_path	= $(addprefix $(OBJ_DIR), $(addsuffix $(OBJ_SUF),	$(1)))

SRC = Mandelbrot main

TARGET = Test

C_FLAGS	=	-D_DEBUG -ggdb3 -std=c23 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat								\
			-Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy	\
			-Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op			\
			-Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow			\
			-Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn							\
			-Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef				\
			-Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers				\
			-Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector			\
			-fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla	\
			-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

.PHONY: all prepare test clean commit

all: test

prepare:
	@mkdir -p $(OBJ_DIR)

make_c_object = $(call make_obj_path, $(1)): $(call make_c_path, $(1)) | prepare; \
	@gcc $(C_OPTIONS) -I$(INC_DIR) -c $$< -o $$@

$(foreach src, $(SRC), $(eval $(call make_c_object, $(src))))

$(TARGET): $(call make_obj_path, $(SRC))
	@gcc $(C_OPTIONS) $^ -lglfw -lGL -o $(TARGET)

test: $(TARGET)
	@./$(TARGET)

clean:
	@rm -fr	$(OBJ_DIR) $(TARGET)

commit:
	@git add .
	@git commit -m "$(MSG)"
