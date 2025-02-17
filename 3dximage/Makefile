obj = main.o
bin = 3dximage

CFLAGS = -pedantic -Wall -g -I../src
LDFLAGS = ../lib3dxdisp.a -ludev -limago -lm

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
