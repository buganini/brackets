PREFIX?=/usr/local

all:
	$(CC) -Wall [[[.c -o [[[ -Wall

install:
	install -s -m 555 [[[ ${PREFIX}/bin
	ln -fs [[[ ${PREFIX}/bin/[[

clean:
	rm [[[

test:
	env ARG0=extra [[ \
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
