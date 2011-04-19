PREFIX?=/usr/local

all:
	$(CC) [[[.c -o [[[ -Wall
install:
	install -s -m 555 [[[ ${PREFIX}/bin
	ln -fs [[[ ${PREFIX}/bin/[[
test:
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
