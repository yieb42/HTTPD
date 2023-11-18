export CC = gcc
export CPPFLAGS = -Isrc
export CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic

BINARY=httpd

all: $(BINARY)

$(BINARY):
	$(MAKE) -C src
	mv src/$(BINARY) .

clean:
	$(RM) $(OBJ) $(LIB) $(TEST_BIN) $(TEST_OBJ) $(addprefix $(SRC_DIR), $(LIBS))

.PHONY: all
