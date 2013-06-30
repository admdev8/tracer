CC=gcc

test1_BPF.exe: test1_BPF.c
	$(CC) $< -std=gnu99  -Xlinker -Map=test1_BPF.map -o $@

clean:
	$(RM) *.exe
