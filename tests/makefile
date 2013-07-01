OUTDIR=$(MSYSTEM)_debug
CC=gcc

all:    $(OUTDIR) $(OUTDIR)/test1.exe

$(OUTDIR):
	mkdir $(OUTDIR)

$(OUTDIR)/test1.exe: test1.c
	$(CC) $< -std=gnu99 -o $@

clean:
	$(RM) $(OUTDIR)/*.exe
