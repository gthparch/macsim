CXX=g++
AM_CPPFLAGS = -Iinterfaces -I$(includedir)/manifold -I../../../ -I../../../kernel/include
# I want to install the library under $(libdir)/manifold, not the default $(libdir).
# However I can't use pkglib_LIBRARIES either, because that is $(libdir)/manifold/iris,
# since the package name is manifold/iris.
# Creating my own manifoldlib prefix solves this.
manifoldlibdir = ../../../kernel #$(libdir)/manifold
manifoldlib_LIBRARIES = libiris.a
IRIS_SOURCES = \
	    data_types/networkPacket.cc \
	    data_types/flit.cc \
	    data_types/linkData.cc \
	    \
	    topology/twoNode.cc \
	    topology/ring.cc \
	    topology/util.cc \
	    \
	    components/genericBuffer.cc \
	    components/genericRC.cc \
	    components/genericSwitchArbiter.cc \
	    components/genericVcAllocator.cc \
	    components/ninterface.cc \
	    components/pktgen.cc \
	    components/manifoldProcessor.cc \
	    components/simpleMC.cc \
	    components/simpleRouter.cc 

pkginclude_interfacesdir = $(includedir)/manifold/iris/interfaces
pkginclude_iris_componentsdir = $(includedir)/manifold/iris/iris/components
pkginclude_iris_data_typesdir = $(includedir)/manifold/iris/iris/data_types
pkginclude_interfaces_HEADERS = \
	    ../interfaces/genericHeader.h \
	    ../interfaces/irisInterface.h \
	    ../interfaces/irisRouter.h \
	    ../interfaces/irisTerminal.h 
	    ../interfaces/topology.h 

pkginclude_iris_components_HEADERS = \
	    topology/twoNode.h \
	    topology/ring.h \
	    \
	    components/genericBuffer.h \
	    components/genericRC.h \
	    components/genericSwitchArbiter.h \
	    components/genericVcAllocator.h \
	    components/ninterface.h \
	    components/pktgen.h \
	    components/manifoldProcessor.h \
	    components/simpleMC.h \
	    components/simpleRouter.h

pkginclude_iris_data_types_HEADERS = \
	    data_types/flit.h \
	    data_types/linkData.h \
	    data_types/networkPacket.h

##libIris
libiris_a_CFLAGS= -lgsl -lgslcblas $(CXX_FLAGS)
libiris_a_SOURCES = $(IRIS_SOURCES)
