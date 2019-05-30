#
# @author: Ilker GURCAN - 1777929
#	 (Computer Engineering Dept. - PhD Student)
#
#  Created by vim
#

##################################################
# Add your extra folders for header and libs files
##################################################
INC :=/home/ilithefallen/pybombs/bladeRF/include/ /usr/local/include/liquid
LIB :=/home/ilithefallen/pybombs/bladeRF/lib /usr/lib/x86_64-linux-gnu/
LIBS:=-lstdc++ -lbladeRF -lliquid -lboost_system -lboost_regex -lboost_thread -lpthread
ODIR:=./obj
DDIR:=./deps
CC:=g++
##################################################
# Add your files to compile
##################################################
_CPPS := \
./BladeRFOps.cpp \
./TunTapOps.cpp \
./TxRx.cpp

_OBJS := \
BladeRFOps.o \
TunTapOps.o \
TxRx.o

_DEPS := \
BladeRFOps.d \
TunTapOps.d \
TxRx.d

#################Transmitter Spec files
_CPPS_TRANS := $(_CPPS) \
./Transmitter.cpp \
./Scheduler.cpp

_OBJS_TRANS := $(_OBJS) \
Transmitter.o \
Scheduler.o

_DEPS_TRANS := $(_DEPS) \
Transmitter.d \
Scheduler.d

#################Receiver Spec files
_CPPS_RECV := $(_CPPS) \
./Receiver.cpp

_OBJS_RECV := $(_OBJS) \
Receiver.o

_DEPS_RECV := $(_DEPS) \
Receiver.d


#################Config Spec files
_CPPS_CONF := $(_CPPS) \
./ConfigTun.cpp

_OBJS_CONF := $(_OBJS) \
ConfigTun.o

_DEPS_CONF := $(_DEPS) \
ConfigTun.d

##################################################
# Necessary files and flags for compilation and 
# makefile dependency generation.
##################################################
INC_DIRS=$(INC:%=-I%)
LIB_DIRS=$(LIB:%=-L%)
CFLAGS= $(INC_DIRS) -std=c++11 -g
DEPFLAGS= -MT $@ -MMD -MP -MF $(DDIR)/$*.Td
OBJS_CONF= $(_OBJS_CONF:%=$(ODIR)/%)
DEPS_CONF= $(_DEPS_CONF:%=$(DDIR)/%)
OBJS_TRANS= $(_OBJS_TRANS:%=$(ODIR)/%)
DEPS_TRANS= $(_DEPS_TRANS:%=$(DDIR)/%)
OBJS_RECV= $(_OBJS_RECV:%=$(ODIR)/%)
DEPS_RECV= $(_DEPS_RECV:%=$(DDIR)/%)
DEPS=$(DEPS_CONF) $(DEPS_TRANS) $(DEPS_RECV)
OBJS=$(OBJS_CONF) $(OBJS_TRANS) $(OBJS_RECV)

##################################################
# Compiling and dependency generation
##################################################

$(shell mkdir -p $(ODIR) >/dev/null)
$(shell mkdir -p $(DDIR) >/dev/null)
POSTCOMPILE = @mv -f $(DDIR)/$*.Td $(DDIR)/$*.d && touch $@

# Delete the built-in rules for building object files 
# from .cpp files, so that our rule is used instead. 
# Do the same for the other built-in rules.
$(ODIR)/%.o: %.cpp

# Do the actual compilation with checking 
# dependencies this  time
$(ODIR)/%.o: %.cpp $(DDIR)/%.d
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<
	$(POSTCOMPILE)

# Empty recipe to avoid make from failing
$(DDIR)/%.d: ;

# Mark the dependency files precious to make, 
# so they will not be automatically deleted as 
# intermediate files.
.PRECIOUS: $(DDIR)/%.d

# Include the dependency files that exist: 
# translate each file listed in _CPPS into 
# its dependency file. 
include $(DEPS:%=%)


##################################################
# Linking
##################################################
configTun: $(OBJS_CONF)
	$(CC) $(LIB_DIRS) -o $@ $(OBJS_CONF) $(LIBS)
transmitter: $(OBJS_TRANS)
	$(CC) $(LIB_DIRS) -o $@ $(OBJS_TRANS) $(LIBS)
receiver: $(OBJS_RECV)
	$(CC) $(LIB_DIRS) -o $@ $(OBJS_RECV) $(LIBS)

##################################################
# Cleaning
##################################################
.PHONY: clean

clean:
	rm -f configTun transmitter receiver $(OBJS) $(DEPS)

