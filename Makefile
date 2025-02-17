PREFIX = /usr/local

src = $(wildcard src/*.c) $(wildcard src/hidapi/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)

liba = lib3dxdisp.a

CFLAGS = -pedantic -Wall -g
LDFLAGS = -ludev

$(liba): $(obj)
	$(AR) rcs $@ $(obj)

.PHONY: clean
clean:
	rm -f $(obj) $(liba)

.PHONY: install
install: $(liba)
	mkdir -p $(DESTDIR)$(PREFIX)/include
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp src/3dxdisp.h $(DESTDIR)$(PREFIX)/include/3dxdisp.h
	cp $(liba) $(DESTDIR)$(PREFIX)/lib/$(liba)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/include/3dxdisp.h
	rm -f $(DESTDIR)$(PREFIX)/lib/$(liba)
