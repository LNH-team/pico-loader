.PHONY: loader9 loader7 clean

all: checklibtwl loader9 loader7 apList saveList patchList

PICO_PLATFORM ?= DSPICO

checklibtwl:
	$(MAKE) -C libs/libtwl

loader9: checklibtwl
	$(MAKE) -f Makefile.arm9 PLATFORM=$(PICO_PLATFORM)

loader7: checklibtwl
	$(MAKE) -f Makefile.arm7 PLATFORM=$(PICO_PLATFORM)

picoLoaderConverter:
	dotnet build tools/PicoLoaderConverter/PicoLoaderConverter.sln

apList: picoLoaderConverter data/aplist.csv
	dotnet tools/PicoLoaderConverter/PicoLoaderConverter/bin/Debug/net9.0/PicoLoaderConverter.dll aplist -i data/aplist.csv -o data/aplist.bin

saveList: picoLoaderConverter data/savelist.csv
	dotnet tools/PicoLoaderConverter/PicoLoaderConverter/bin/Debug/net9.0/PicoLoaderConverter.dll savelist -i data/savelist.csv -o data/savelist.bin

patchList: picoLoaderConverter data/patchlist.json
	dotnet tools/PicoLoaderConverter/PicoLoaderConverter/bin/Debug/net9.0/PicoLoaderConverter.dll patchlist -i data/patchlist.json -o data/patchlist.bin

clean:
	$(MAKE) -f Makefile.arm7 clean
	$(MAKE) -f Makefile.arm9 clean
	rm -rf build
