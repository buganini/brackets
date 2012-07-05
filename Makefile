PREFIX?=/usr/local

all:
	$(CC) -Wall brackets.c -o brackets -Wall

install:
	install -s -m 555 brackets ${PREFIX}/bin
	ln -fs brackets ${PREFIX}/bin/[[
	ln -fs brackets ${PREFIX}/bin/[[[

clean:
	rm -f [[[ [[ brackets

test:
	env ARG0=extra \
	[[ \
		[[ echo a ]] \
		[[[ \
			[[ echo b ]] \
			[[ echo c ]] \
			[[ echo d ]] \
		]]] \
		[[ echo e ]] \
	]]

benchmark:
	for i in `jot 100`; do make test; done
