# =========================
# Variables de compilación
# =========================
SRCDIR = src
SRCLIBDIR = srclib
LIBDIR = lib
INCLUDEDIR = include
OBJDIR = obj
TEST_OBJDIR = tests/obj

CC = gcc
CFLAGS = -Wall -Wextra -I$(INCLUDEDIR) -L$(LIBDIR) -lserver -O3 -g

# =========================
# Objetos servidor
# =========================
API_CLIENT_OBJ = $(OBJDIR)/api_client.o
CONCURRENCY_OBJ = $(OBJDIR)/concurrency.o
CONF_OBJ = $(OBJDIR)/conf.o
FILE_HANDLER_OBJ = $(OBJDIR)/file_handler.o
HTTP_PARSER_OBJ = $(OBJDIR)/http_parser.o
HTTP_UTILS_OBJ = $(OBJDIR)/http_utils.o
LIB_OBJ = $(OBJDIR)/net_lib.o
SCRIPTS_OBJ = $(OBJDIR)/scripts.o

SERVER_OBJS = \
	$(LIB_OBJ) \
	$(CONCURRENCY_OBJ) \
	$(CONF_OBJ) \
	$(FILE_HANDLER_OBJ) \
	$(SCRIPTS_OBJ) \
	$(HTTP_PARSER_OBJ) \
	$(HTTP_UTILS_OBJ) \
	$(API_CLIENT_OBJ)

# =========================
# Objetos tests
# =========================
TEST_MUNDO_OBJ = $(TEST_OBJDIR)/server_mundo_test.o
TEST_CONC_OBJ = $(TEST_OBJDIR)/server_concurrency_test.o
TEST_PARSE_OBJ = $(TEST_OBJDIR)/test_parse_request.o
TEST_GETARGS_OBJ = $(TEST_OBJDIR)/test_get_url_args.o

# =========================
# Binarios
# =========================
MUNDO_BIN = server_test_mundo
CONCURRENCY_BIN = server_concurrency_test
SERVER_BIN = server
PARSE_BIN = test_parse_request
GETARGS_BIN = test_get_url_args

# =========================
# Regla por defecto
# =========================
all: $(OBJDIR) \
     $(SERVER_BIN) 

tests: $(TEST_OBJDIR) \
	$(MUNDO_BIN) $(CONCURRENCY_BIN) $(PARSE_BIN) $(GETARGS_BIN)

# =========================
# Crear carpetas
# =========================
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TEST_OBJDIR):
	mkdir -p $(TEST_OBJDIR)

# =========================
# Compilación servidor (.o)
# =========================
$(OBJDIR)/net_lib.o: $(SRCLIBDIR)/net_lib.c $(INCLUDEDIR)/net_lib.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/concurrency.o: $(SRCDIR)/concurrency.c $(INCLUDEDIR)/concurrency.h $(INCLUDEDIR)/net_lib.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/conf.o: $(SRCDIR)/conf.c $(INCLUDEDIR)/conf.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/file_handler.o: $(SRCDIR)/file_handler.c $(INCLUDEDIR)/file_handler.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/scripts.o: $(SRCDIR)/scripts.c $(INCLUDEDIR)/scripts.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/http_parser.o: $(SRCLIBDIR)/http_parser.c $(INCLUDEDIR)/http_parser.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/http_utils.o: $(SRCLIBDIR)/http_utils.c $(INCLUDEDIR)/http_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/api_client.o: $(SRCDIR)/api_client.c $(INCLUDEDIR)/api_client.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBDIR)/libserver.a: $(OBJDIR)/http_utils.o $(OBJDIR)/http_parser.o $(OBJDIR)/net_lib.o
	ar rcs $@ $^
# =========================
# Compilación tests (.o)
# =========================
$(TEST_OBJDIR)/server_mundo_test.o: test/server_mundo_test.c | $(TEST_OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_OBJDIR)/server_concurrency_test.o: test/server_concurrency_test.c | $(TEST_OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_OBJDIR)/test_parse_request.o: test/test_parse_request.c | $(TEST_OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_OBJDIR)/test_get_url_args.o: test/test_get_url_args.c | $(TEST_OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# =========================
# Enlazado tests
# =========================
$(MUNDO_BIN): $(TEST_MUNDO_OBJ) $(LIB_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(CONCURRENCY_BIN): $(TEST_CONC_OBJ) $(LIB_OBJ) $(CONCURRENCY_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(PARSE_BIN): $(TEST_PARSE_OBJ) $(HTTP_PARSER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(GETARGS_BIN): $(TEST_GETARGS_OBJ) $(HTTP_PARSER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# =========================
# Servidor completo
# =========================
$(SERVER_BIN): $(SRCDIR)/server.c $(SERVER_OBJS) $(LIBDIR)/libserver.a
	$(CC) $(CFLAGS) $(SRCDIR)/server.c $(SERVER_OBJS) -o $@

# =========================
# Limpieza
# =========================
clean:
	rm -rf $(OBJDIR) $(TEST_OBJDIR) *.o \
	       $(MUNDO_BIN) $(CONCURRENCY_BIN) \
	       $(SERVER_BIN) $(PARSE_BIN) $(GETARGS_BIN)

# =========================
# Valgrind
# =========================
check_mundo: $(MUNDO_BIN)
	valgrind --leak-check=full ./$<

check_concurrency: $(CONCURRENCY_BIN)
	valgrind --leak-check=full --child-silent-after-fork=yes ./$<

check_server: $(SERVER_BIN)
	valgrind --leak-check=full --child-silent-after-fork=yes ./$<

check_parse: $(PARSE_BIN)
	valgrind --leak-check=full ./$<

check_getargs: $(GETARGS_BIN)
	valgrind --leak-check=full ./$<