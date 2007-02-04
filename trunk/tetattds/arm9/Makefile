#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files
# all directories are relative to this makefile
#---------------------------------------------------------------------------------
BUILD		:=	build
SOURCES		:=	source ../network/source 
INCLUDES	:=	include ../network/include
DATA		:=  data
IMAGES      :=  ../images
MUSIC       :=  ../music
SOUND       :=  ../sound

# applications
PIMPMYBMP   :=  pimpmybmp
LZGB        :=  lzgb
BMP15BIT    :=  bmp15bit
IMA2RAW     :=  ima2raw
CP          :=  cp

# other stuff
export PATH := $(TOPDIR)/tools:$(TOPDIR)/../fwgui/tools:$(PATH)

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb -mthumb-interwork

# note: arm9tdmi isn't the correct CPU arch, but anything newer and LD
# *insists* it has a FPU or VFP, and it won't take no for an answer!
CFLAGS	:=	-g -Wall -O2\
			-mcpu=arm9tdmi -mtune=arm9tdmi -fomit-frame-pointer\
			-ffast-math \
			$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM9  -DVERSION_STRING="\"$(VERSION_STRING)\"" -DABOUT_STRING="\"$(ABOUT_STRING)\""
CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=ds_arm9.specs -g $(ARCH) -mno-fpu -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= -lfat -lcartreset9 -lmikmod9 -lfwgui -lnds9 -ldswifi9

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(TOPDIR)/libcartreset $(TOPDIR)/../mikmod $(TOPDIR)/../fwgui $(LIBNDS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export ARM9BIN	:=	$(TOPDIR)/$(TARGET).arm9
export ARM9ELF	:=	$(CURDIR)/$(TARGET).arm9.elf
export DEPSDIR := $(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
					$(foreach dir,$(IMAGES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(MUSIC),$(CURDIR)/$(dir)) \
					$(foreach dir,$(SOUND),$(CURDIR)/$(dir))

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=  $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.s))) \
                $(patsubst %.bmp,%_map.bin,$(notdir $(wildcard $(IMAGES)/bg_tile/*.bmp))) \
                $(patsubst %.bmp,%_pal.bin,$(foreach dir,$(IMAGES)/bg_tile $(IMAGES)/tile8 $(IMAGES)/tile16,$(notdir $(wildcard $(dir)/*.bmp)))) \
                $(patsubst %.bmp,%.bin,$(foreach dir,$(IMAGES)/bg_tile $(IMAGES)/tile8 $(IMAGES)/tile16 $(IMAGES)/bg,$(notdir $(wildcard $(dir)/*.bmp)))) \
				$(patsubst %.xm,%.bin,$(notdir $(wildcard $(MUSIC)/*.xm))) \
				$(patsubst %.wav,%.bin,$(notdir $(wildcard $(SOUND)/*.wav)))

                

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) *.elf *.nds* *.bin 


#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(ARM9BIN)	:	$(ARM9ELF)
	@$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)

$(ARM9ELF)	:	$(OFILES)
	@echo linking $(notdir $@)
	@$(LD)  $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data 
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
# graphics
#---------------------------------------------------------------------------------

%.raw %.pal: tile16/%.bmp
	$(PIMPMYBMP) -T16 $^

%.raw %.pal %.map: bg_tile/%.bmp
	$(PIMPMYBMP) -m $^

%.raw %.pal: tile8/%.bmp
	$(PIMPMYBMP) $^

%.raw: bg/%.bmp
	$(BMP15BIT) --dither $^ $@

%.bin: %.wav
	$(IMA2RAW) $^
	mv $^.raw $*.bin

%.bin: %.xm
	$(LZGB) $^ $@

%_map.bin: %.map
	$(LZGB) $^ $@

%_pal.bin: %.pal
	$(LZGB) $^ $@

%.bin: %.raw
	$(LZGB) $^ $@


-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
