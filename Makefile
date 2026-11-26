#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:

ifeq ($(strip $(FXCGSDK)),)
export FXCGSDK := $(realpath ../../)
endif

include $(FXCGSDK)/toolchain/prizm_rules

#---------------------------------------------------------------------------------
# Project configuration
#---------------------------------------------------------------------------------
# Keep the output name independent of the checkout directory name. This ensures
# that the generated add-in is always PrizmGBC.g3a.
TARGET          := PrizmGBC
BUILD           := $(CONFIG)
SOURCES         := src src/ptune2_simple src/asm
DATA            := data
INCLUDES        :=

#---------------------------------------------------------------------------------
# Add-in metadata and compiler options
#---------------------------------------------------------------------------------
MKG3AFLAGS      := -n basic:PrizmGBC -i uns:../unselected.bmp -i sel:../selected.bmp

OPTIMIZATION    = -O2

CBASEFLAGS      = $(OPTIMIZATION) \
                  -Wall \
                  -funroll-loops \
                  -fno-trapping-math \
                  -fno-trapv \
                  -Wno-switch \
                  $(MACHDEP) $(INCLUDE) $(DEFINES)

CFLAGS          = $(CBASEFLAGS) \
                  -std=c99

CXXFLAGS        = $(CBASEFLAGS) \
                  -fpermissive \
                  -fno-rtti \
                  -fno-exceptions \
                  -fno-threadsafe-statics \
                  -fno-use-cxa-get-exception-ptr

ASFLAGS         = $(CFLAGS)

LDFLAGS         = -Xlinker -Map=$(CURDIR)/output.map $(MACHDEP) $(OPTIMIZATION) -T$(FXCGSDK)/toolchain/prizm.x -Wl,-static -g

#---------------------------------------------------------------------------------
# Libraries
#---------------------------------------------------------------------------------
SYSTEMLIBS      := -lc -lfxcg -lgcc
LIBS            := -lzx7 -lcalctype -lsnd -lscope_timer -lptune2_simple $(SYSTEMLIBS)
LIBDIRS         :=

#---------------------------------------------------------------------------------
# Build rules
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT       := $(CURDIR)/$(BUILD)/$(TARGET)
export VPATH        := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                       $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR      := $(CURDIR)/$(BUILD)

CFILES              := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES            := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES              := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES            := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

ifeq ($(strip $(CPPFILES)),)
export LD           := $(CC)
else
export LD           := $(CXX)
endif

export OFILES       := $(addsuffix .o,$(BINFILES)) \
                       $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
                       $(SFILES:.S=.o)

export INCLUDE      := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
                       $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                       -I$(CURDIR)/$(BUILD) -I$(LIBFXCG_INC) -I$(UTILS_INC)

export LIBPATHS     := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
                       -L$(FXCGSDK)/utils/lib \
                       -L$(LIBFXCG_LIB)

export OUTPUT       := $(CURDIR)/$(BUILD)/$(TARGET)
export OUTPUT_FINAL := $(CURDIR)/$(TARGET)

.PHONY: $(BUILD) clean

$(BUILD):
	-mkdir $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

export CYGWIN := nodosfilewarning

clean:
	$(call rmdir,$(BUILD))
	$(call rm,$(OUTPUT).bin)
	$(call rm,$(OUTPUT_FINAL).g3a)
	$(call rm,$(OUTPUT_FINAL)_cg10.g3a)

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT_FINAL).g3a: $(OUTPUT).bin
	$(MKG3A) $(MKG3AFLAGS) $< $@

$(OUTPUT).bin: $(OFILES)

-include $(DEPENDS)

endif
