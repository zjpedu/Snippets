ROOT_DIR=${CURDIR}/../
TARGETS=show array-access conflict move-pages memcpy

all: $(TARGETS)

$(TARGETS):
	make -f makefile.template PROJECT=$(ROOT_DIR) DNAME=$@

