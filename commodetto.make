.PHONY: archive

ifeq ($(TARGET_SYSTEM), mc)
COMMODETTO_MAKEFILE = makefile
else
COMMODETTO_MAKEFILE = makefile
endif

all archive:
	(cd commodetto; make -f $(COMMODETTO_MAKEFILE) $@)
	touch $(TMP_DIR)/.update

clean:
	(cd commodetto; make -f $(COMMODETTO_MAKEFILE) $@)
