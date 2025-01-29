CPUS ?= $(shell (nproc --all || sysctl -n hw.ncpu) 2>/dev/null || echo 1)
MAKEFLAGS += --jobs=$(CPUS)

appname ?= simulator_smash

CXX ?= g++
CXXFLAGS ?= -std=c++20 -pipe -m64 -Ofast

srcfiles := $(shell find . -mindepth 2 -name "*.cpp" && find . -maxdepth 1 -name "$(appname).cpp")
hdrfiles := $(shell find . -mindepth 2 -name "*.hpp" -or -name "*.h")
objects  := $(patsubst %.cpp, %.o, $(srcfiles))
app_sources := $(shell find . -maxdepth 1 -name "*.cpp" | sort)
app_objects := $(patsubst %.cpp, %.o, $(app_sources))
app_names := $(patsubst ./%.cpp, %, $(app_sources))

COLOR_RESET = \033[0m
GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[0;33m
BOLD_GREEN = \033[1;32m
BOLD_RED = \033[1;31m
BOLD_YELLOW = \033[1;33m


.PHONY: format list build depend help run dist-clean clean clean-all depends-all all test

build: $(appname)
	strip -s $(appname)

format:
	clang-format -i $(srcfiles) $(hdrfiles)

list:
	@echo available: $(app_names)
	@echo current: $(appname)

run: build
	@mkdir -p Results/$(appname)
	./$(appname) ${ARGS}

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets available:"
	@echo "  build        - build the main target [default]"
	@echo "  format       - format the source code"
	@echo "  list         - list all available main targets"
	@echo "  run ARGS=... - run the main target with arguments"
	@echo "  depend       - regenerate dependency tree"
	@echo "  dist-clean   - remove all dependency trees"
	@echo "  clean        - remove the main target and object files"
	@echo "  clean-all    - remove all targets"
	@echo "  clean-out    - remove all output files"
	@echo "  depend-all   - generate dependency trees for all targets"
	@echo "  all          - build all targets"
	@echo "  test         - run standardised tests"
	@echo "  Results/%    - run the main target with arguments and save the output"
	@echo "  help         - print this help message"
	@echo ""
	@echo "Variables available:"
	@echo "  appname      - the name of the main target"
	@echo "  CPUS         - the number of parallel jobs to use for building"
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
	rm -rf Results/**.csv Results/**.txt

.clean_%:
	@$(MAKE) appname=$(patsubst .clean_%,%,$@) clean

.depends_%:
	$(MAKE) appname=$(patsubst .depends_%,%,$@) depend

.app_%:
	@$(MAKE) appname=$(patsubst .app_%,%,$@)

clean-all: $(foreach T,$(app_names),.clean_$T) dist-clean
depend-all: dist-clean $(foreach T,$(app_names),.depends_$T)
all: $(foreach T,$(app_names),.app_$T)

test: build
	@mkdir -p Results/$(appname)
	$(MAKE) CPUS=1 Results/overLambdas-nClasses2-N50-Win1-Exponential-oneOrAll-test1.csv \
	Results/overLambdas-nClasses4-N16-Win0-Exponential-testing_4C_16.csv \
	Results/overLambdas-nClasses4-N16-Win1-Exponential-testing_4C_16.csv \
	Results/overLambdas-nClasses4-N16-Win4-Exponential-testing_4C_16.csv \
	Results/overLambdas-nClasses4-N16-Win-2-Exponential-testing_4C_16.csv \
	Results/overLambdas-nClasses4-N16-Win-3-Exponential-testing_4C_16.csv \
	Results/overLambdas-nClasses4-N16-Win1-Frechet-testing_4C_16.csv \
	Results/overLambdas-nClasses4-N16-Win1-Uniform-testing_4C_16.csv

define compare_results
	@python3 ensure_same_results.py Results/$(appname)/$(@F) test/expected/$(@F) > Results/$(appname)/$(@F).diff.txt 2>&1
	@grep -q "Data is the same" Results/$(appname)/$(@F).diff.txt && echo -e "$(GREEN)$(appname) $(BOLD_GREEN)PASSED$(GREEN) test $(@F)$(COLOR_RESET)" || echo -e "$(RED)$(appname) $(BOLD_RED)FAILED$(RED) test $(@F)$(COLOR_RESET)"
endef

Results/overLambdas-nClasses2-N50-Win1-Exponential-oneOrAll-test1.csv: $(appname)
	$(MAKE) run ARGS="oneOrAll-test1 50 1 exp 100000 10" > Results/$(appname)/$(@F).out.txt 2>&1
	$(compare_results)

Results/overLambdas-nClasses4-N16-Win0-Exponential-testing_4C_16.csv: $(appname)
	$(MAKE) run ARGS="testing_4C_16 16 0 exp 100000 20" > Results/$(appname)/$(@F).out.txt 2>&1
	$(compare_results)

Results/overLambdas-nClasses4-N16-Win1-Exponential-testing_4C_16.csv: $(appname)
	$(MAKE) run ARGS="testing_4C_16 16 1 exp 100000 20" > Results/$(appname)/$(@F).out.txt 2>&1
	$(compare_results)

Results/overLambdas-nClasses4-N16-Win4-Exponential-testing_4C_16.csv: $(appname)
	$(MAKE) run ARGS="testing_4C_16 16 4 exp 100000 20" > Results/$(appname)/$(@F).out.txt 2>&1
	$(compare_results)

Results/overLambdas-nClasses4-N16-Win-2-Exponential-testing_4C_16.csv: $(appname)
	$(MAKE) run ARGS="testing_4C_16 16 -2 exp 100000 20" > Results/$(appname)/$(@F).out.txt 2>&1
	$(compare_results)

Results/overLambdas-nClasses4-N16-Win-3-Exponential-testing_4C_16.csv: $(appname)
	$(MAKE) run ARGS="testing_4C_16 16 -3 exp 100000 20" > Results/$(appname)/$(@F).out.txt 2>&1
	$(compare_results)

Results/overLambdas-nClasses4-N16-Win1-Frechet-testing_4C_16.csv: $(appname)
	$(MAKE) run ARGS="testing_4C_16 16 1 fre 100000 20" > Results/$(appname)/$(@F).out.txt 2>&1
	$(compare_results)

Results/overLambdas-nClasses4-N16-Win1-Uniform-testing_4C_16.csv: $(appname)
	$(MAKE) run ARGS="testing_4C_16 16 1 uni 100000 20" > Results/$(appname)/$(@F).out.txt 2>&1
	$(compare_results)

