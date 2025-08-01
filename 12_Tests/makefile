CC = gcc
OUT = test_runner.exe

CFLAGS = -Wall -Wextra -DUNIT_TEST \
         -include test_stubs/screen_driver.h \
         -IUnity \
         -I. \
         -Itest_stubs \
         -I../10_Source_code/Core/Inc \
         -DUNIT_TEST -include test_stubs/gpio.h -include test_stubs/screen_driver.h

SRC = \
  Unity/unity.c \
  test_runner.c \
  test_midi_logic.c \
  test_stubs/globals.c \
  test_stubs/flash.c \
  ../10_Source_code/Core/Src/midi_modify_transform.c \
  ../10_Source_code/Core/Src/saving.c

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

run: $(OUT)
	.\$(OUT)

clean:
	del /Q $(OUT)