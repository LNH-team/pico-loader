PICO_PLATFORM ?= DSPICO

PICO_LOADER_CONVERTER	:=	tools/PicoLoaderConverter/bin/PicoLoaderConverter.dll
PICO_LOADER_CONVERTER_DEPS	:=	\
	$(shell find -L tools/PicoLoaderConverter/PicoLoaderConverter -type f) \
	tools/PicoLoaderConverter/Directory.Build.props \
	tools/PicoLoaderConverter/PicoLoaderConverter.sln

.PHONY: loader9 loader7 clean

all: checklibtwl loader9 loader7 apList saveList patchList

checklibtwl:
	$(MAKE) -C libs/libtwl

loader9: checklibtwl
	$(MAKE) -f Makefile.arm9 PLATFORM=$(PICO_PLATFORM)

loader7: checklibtwl
	$(MAKE) -f Makefile.arm7

$(PICO_LOADER_CONVERTER): $(PICO_LOADER_CONVERTER_DEPS) 
	dotnet build tools/PicoLoaderConverter/PicoLoaderConverter.sln

apList: $(PICO_LOADER_CONVERTER) data/aplist.csv
	dotnet $(PICO_LOADER_CONVERTER) aplist -i data/aplist.csv -o data/aplist.bin

saveList: $(PICO_LOADER_CONVERTER) data/savelist.csv
	dotnet $(PICO_LOADER_CONVERTER) savelist -i data/savelist.csv -o data/savelist.bin

patchList: $(PICO_LOADER_CONVERTER) data/patchlist.json
	dotnet $(PICO_LOADER_CONVERTER) patchlist -i data/patchlist.json -o data/patchlist.bin

clean:
	$(MAKE) -f Makefile.arm7 clean
	$(MAKE) -f Makefile.arm9 clean
	rm -rf build tools/PicoLoaderConverter/bin tools/PicoLoaderConverter/artifacts
