appname ?= simulator_smash

CXX ?= g++
CXXFLAGS ?= -std=c++20 -pipe -m64 -O3

srcfiles := $(shell find . -mindepth 2 -name "*.cpp" && find . -maxdepth 1 -name "$(appname).cpp")
hdrfiles := $(shell find . -mindepth 2 -name "*.hpp" -or -name "*.h")
objects  := $(patsubst %.cpp, %.o, $(srcfiles))
app_sources := $(shell find . -maxdepth 1 -name "*.cpp" | sort)
app_objects := $(patsubst %.cpp, %.o, $(app_sources))
app_names := $(patsubst ./%.cpp, %, $(app_sources))

.PHONY: list build depend help run dist-clean clean clean-all depends-all all

build: $(appname)
	#strip -s $(appname)

list:
	@echo available: $(app_names)
	@echo current: $(appname)

run: build
	@mkdir -p Results
	./$(appname) ${ARGS}

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets available:"
	@echo "  build        - build the main target [default]"
	@echo "  list         - list all available main targets"
	@echo "  run ARGS=... - run the main target with arguments"
	@echo "  depend       - regenerate dependency tree"
	@echo "  dist-clean   - remove all dependency trees"
	@echo "  clean        - remove the main target and object files"
	@echo "  clean-all    - remove all targets"
	@echo "  depend-all   - generate dependency trees for all targets"
	@echo "  all          - build all targets"
	@echo "  help         - print this help message"
	@echo ""
	@echo "Variables available:"
	@echo "  appname      - the name of the main target"
	@echo ""
	@echo "Examples:"
	@echo "  make         # same as make build"
	@echo "  make list"
	@echo "  make run ARGS=\"arg1 arg2\""
	@echo "  make appname=main1 build"
	@echo "  make appname=main1 clean"


$(appname): depend $(objects)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(appname) $(objects) $(LDLIBS)

depend: .depends-$(appname)

.depends-$(appname): $(srcfiles) $(hdrfiles)
	@rm -f .depends-$(appname)
	$(CXX) $(CXXFLAGS) -MM $^>>./.depends-$(appname);

-include .depends-$(appname)

dist-clean:
	rm -f *~ .depends-*

clean:
	rm -f $(appname) $(objects)

clean-out:
	rm -rf Results/*.csv

.clean_%:
	@$(MAKE) appname=$(patsubst .clean_%,%,$@) clean

.depends_%:
	$(MAKE) appname=$(patsubst .depends_%,%,$@) depend

.app_%:
	@$(MAKE) appname=$(patsubst .app_%,%,$@)

clean-all: $(foreach T,$(app_names),.clean_$T) dist-clean
depend-all: dist-clean $(foreach T,$(app_names),.depends_$T)
all: $(foreach T,$(app_names),.app_$T)