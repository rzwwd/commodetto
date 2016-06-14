#
#    Copyright (C) 2016 Moddable Tech, Inc.
#
#   This Source Code Form is subject to the terms of the Mozilla Public
#   License, v. 2.0. If a copy of the MPL was not distributed with this
#   file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

COMMODETTO_JS_SRCS = commodetto_Bitmap.js commodetto_BMPOut.js commodetto_parseBMF.js commodetto_parseBMP.js commodetto_parseNFNT.js commodetto_PixelsOut.js commodetto_Poco.js commodetto_readJPEG.js commodetto_Render.js commodetto_RLEOut.js commodetto_SPIDisplay.js commodetto_BufferOut.js
COMMODETTO_C_SRCS = commodetto_Bitmap.c commodetto_BufferOut.c commodetto_Poco.c commodetto_PocoBlit.c commodetto_host.c picojpeg.c commodetto_readJPEG.c

COMMODETTO_BINARIES = $(addprefix $(DEST_DIR)/commodetto/, $(addsuffix .xsb, $(patsubst commodetto_%,%,$(basename $(COMMODETTO_JS_SRCS)))))
COMMODETTO_OBJS = $(addprefix $(TMP_DIR)/commodetto/, $(addsuffix .o, $(basename $(COMMODETTO_C_SRCS))))

ifndef XS_ARCHIVE
COMMODETTO_OBJS += $(addprefix $(TMP_DIR)/commodetto/, $(addsuffix .xs.o, $(basename $(COMMODETTO_C_SRCS))))
endif

ifndef COMMODETTO_DIR
COMMODETTO_DIR = $(F_HOME)/xs6/sources/mc/extensions/commodetto
endif

.PHONY: archive

all archive: $(TMP_DIR)/commodetto $(DEST_DIR)/commodetto $(DEST_DIR)/commodetto.xsb $(COMMODETTO_BINARIES) $(COMMODETTO_OBJS)
	$(AR) cr $(TMP_DIR)/$(LIBMODULE) $(COMMODETTO_OBJS)

$(TMP_DIR)/commodetto:
	mkdir -p $(TMP_DIR)/commodetto
$(DEST_DIR)/commodetto:
	mkdir -p $(DEST_DIR)/commodetto

# rules
$(DEST_DIR)/commodetto.xsb: $(TMP_DIR)/commodetto_host.xsb
	cp -p $< $@
$(DEST_DIR)/commodetto/%.xsb: $(TMP_DIR)/commodetto/commodetto_%.xsb
	cp -p $< $@
$(TMP_DIR)/%.xsb: $(COMMODETTO_DIR)/%.js
	$(XS6_TOOL_DIR)/xsc6 $(XSC_OPTIONS) -c -o $(TMP_DIR) $<
$(TMP_DIR)/commodetto/%.xsb: $(COMMODETTO_DIR)/%.js
	$(XS6_TOOL_DIR)/xsc6 $(XSC_OPTIONS) -c -o $(TMP_DIR)/commodetto $<
$(TMP_DIR)/commodetto/%.o: $(COMMODETTO_DIR)/%.c
	$(CC) -c $(C_OPTIONS) -I ${TMP_DIR} $< -o $@

clean:
	rm -f $(COMMODETTO_OBJS) $(addsuffix .d, $(basename $(COMMODETTO_OBJS))) $(TMP_DIR)/commodetto_host.xsb
