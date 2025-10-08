run:
	clang -Iinclude -D NOPICO src/*.c && ./a.out && rm a.out

upload:
	pio run --target upload --target monitor --environment proton

.PHONY: run upload
