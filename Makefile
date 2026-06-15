.PHONY: server client test clean

server:
	$(MAKE) -C src/server

client:
	$(MAKE) -C src/client

test:
	$(MAKE) -C tests/unit

clean:
	$(MAKE) -C src/server clean
	$(MAKE) -C src/client clean
	$(MAKE) -C tests/unit clean
