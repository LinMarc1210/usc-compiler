.SUFFIXES: .cpp .o

include ../Makefile.variables

INCPATH =  -I../../llvm/include
INCPATH += -I../parse

OBJS = CopyPropagation.o ConstantBranch.o ConstantOps.o DeadBlocks.o SSABuilder.o LICM.o SpecLICM.o Passes.o InstCombine.o EdgeProfilingUtils.o EdgeProfiling.o EdgeProfilingOpt.o NaturalLoopInfo.o Liveness.o DCE.o AvailableExpressions.o CSE.o RedundantPhi.o Graph.o RegAllocGC.o RegAllocProfileSplit.o ProfileReader.o

SRCS = $(OBJS:.o=.cpp)

CXXFLAGS += $(INCPATH) -fno-rtti 

ifdef DEBUG
CXXFLAGS += -g
endif

all: libopt.a

libopt.a: $(OBJS)
	ar rcs libopt.a $(OBJS)

depend:
	touch libopt.depend
	makedepend -- $(CXXFLAGS) -- $(SRCS) -f libopt.depend

clean:
	-@rm -f $(OBJS) *.depend*
	-@find . -name 'lib*.a' -exec rm {} \;

-include ./libopt.depend
