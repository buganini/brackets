all:
	$(CC) [[[.c -g -o [[[
install:
	cp [[[ /usr/local/bin
	ln -fs [[[ /usr/local/bin/[[
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
