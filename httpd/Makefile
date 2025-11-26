CC=gcc
CFLAGS=-std=c99 -Werror -Wall -Wextra -Wvla -pedantic
LDFLAGS=

SRCS=$(shell find src -name "*.c")
OBJS=$(SRCS:src/%.c=obj/%.o)

.PHONY: httpd clean check leak

httpd: $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

leak: CFLAGS += -fsanitize=address
leak: LDFLAGS += -fsanitize=address
leak: clean httpd

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

check: httpd
	python3 -m venv .venv
	@if echo $$SHELL | grep -q fish; then \
		fish -c "source .venv/bin/activate.fish; pip install -r tests/requirements.txt; pytest tests/ -v"; \
	else \
		. .venv/bin/activate && pip install -r tests/requirements.txt && pytest tests/ -v; \
	fi
	rm -rf .pytest_cache/ tests/test_root/ tests/__pycache__/ .venv/

clean:
	rm -rf obj httpd
