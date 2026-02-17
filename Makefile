# Compilador e flags
CC = gcc
CFLAGS = -Iinclude -Wall -g

# --- Diretórios ---
OBJDIR = obj
SRCDIR = src

# --- Fontes e Objetos ---
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# --- Detecção de SO Aprimorada ---
IS_WINDOWS :=
ifeq ($(OS),Windows_NT)
    IS_WINDOWS := 1
else
    UNAME_S := $(shell uname -s)
    ifneq (,$(findstring MINGW,$(UNAME_S)))
        IS_WINDOWS := 1
    endif
    ifneq (,$(findstring MSYS,$(UNAME_S)))
        IS_WINDOWS := 1
    endif
endif

# --- Configuração Específica do Sistema Operacional ---
ifdef IS_WINDOWS
    TARGET      := tankwars.exe
    LIBS        := -lfreeglut -lopengl32 -lglu32 -lm
    RM          := del /Q
    RM_DIR      := rmdir /S /Q
    MKDIR_CMD   := if not exist "$(OBJDIR)" mkdir "$(OBJDIR)"
else
    TARGET      := tankwars
    LIBS        := -lGL -lGLU -lglut -lm
    RM          := rm -f
    RM_DIR      := rm -rf
    MKDIR_CMD   := mkdir -p $(OBJDIR)
endif

# --- Regras de Compilação ---

# Regra padrão: compila o alvo e depois limpa o diretório de objetos.
all: $(TARGET)
	@echo "Compilacao concluida. Limpando arquivos temporarios..."
	$(RM_DIR) $(OBJDIR)

# Cria a pasta obj se não existir (necessário para os arquivos .o)
$(OBJDIR):
	$(MKDIR_CMD)

# Linka os objetos para gerar o executável.
# A dependência $(OBJDIR) garante que a pasta seja criada antes de compilar.
$(TARGET): $(OBJDIR) $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

# Regra para criar cada objeto em obj/
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra 'clean' agora apaga apenas o executável,
# pois os objetos já são apagados no processo 'all'.
clean:
	@echo "Limpando o executavel..."
	$(RM) $(TARGET)