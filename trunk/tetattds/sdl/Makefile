#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

export CXX := g++
export LD  := g++

#---------------------------------------------------------------------------------
%.o: %.cpp
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files
# all directories are relative to this makefile
#---------------------------------------------------------------------------------
BUILD       :=	build
SOURCES     :=	source ../network/source ../source
INCLUDES    :=	../network/include ../source source
DATA        :=  data

# applications
PIMPMYBMP   :=  pimpmybmp
LZGB        :=  lzgb
BMP15BIT    :=  bmp15bit
IMA2RAW     :=  ima2raw
CP          :=  cp
MV          :=  mv
BIN2C       :=  bin2c

# other stuff
export PATH := $(CURDIR)/../../tools:$(CURDIR)/../../fwgui/tools:$(PATH)

-include ../version

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CXXFLAGS := $(INCLUDE) -g -Wall -Werror \
	-DVERSION_STRING="\"$(VERSION_STRING)\"" -DABOUT_STRING="\"$(ABOUT_STRING)\""
CXXFLAGS += $(shell sdl-config --cflags)
CXXFLAGS += $(shell libmikmod-config --cflags)

ifneq (,$(findstring CYGWIN,$(shell uname)))
CXXFLAGS += -mno-cygwin
endif

LDFLAGS  := -g
LDFLAGS  += $(shell sdl-config --libs)
LDFLAGS  += $(shell libmikmod-config --libs)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS     := -lfwgui_sdl -lSDL_mixer

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	 := $(CURDIR)/../../fwgui

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export TARGET  := $(CURDIR)/tetattsdl
export DEPSDIR := $(CURDIR)/$(BUILD)

export VPATH   := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                  $(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
                  $(foreach dir,$(IMAGES),$(CURDIR)/$(dir)) \
                  $(foreach dir,$(MUSIC),$(CURDIR)/$(dir)) \
                  $(foreach dir,$(SOUND),$(CURDIR)/$(dir))

CFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES  := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES  := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.s)))

export OFILES   := $(addsuffix .o,$(BINFILES)) \
                   $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET)

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(TARGET)	:	$(OFILES)
	$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
