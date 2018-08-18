src = $(wildcard src/*.c)
obj = $(src:.c=.o)
bin = example

CFLAGS = -pedantic -Wall -g -I../src
LDFLAGS = ../lib3dxdisp.a -ludev

$(bin): $(obj) ../lib3dxdisp.a
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
