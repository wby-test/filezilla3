# Rules for the test code (use `make check` to execute)

TESTS = test
check_PROGRAMS = $(TESTS)

test_SOURCES =  test.cpp \
		cmpnatural.cpp \
		dirparsertest.cpp \
		localpathtest.cpp \
		serverpathtest.cpp

test_CPPFLAGS = -I$(top_builddir)/config
test_CPPFLAGS += $(LIBFILEZILLA_CFLAGS)
test_CPPFLAGS += $(WX_CPPFLAGS)
test_CXXFLAGS = $(WX_CXXFLAGS_ONLY) $(CPPUNIT_CFLAGS)

test_LDFLAGS = ../src/engine/libfzclient-private.la
test_LDFLAGS += $(LIBFILEZILLA_LIBS)
test_LDFLAGS += $(LIBGNUTLS_LIBS)
test_LDFLAGS += $(WX_LIBS)
test_LDFLAGS += $(IDN_LIB)
test_LDFLAGS += $(LIBSQLITE3_LIBS)
test_LDFLAGS += $(CPPUNIT_LIBS)
test_LDFLAGS += $(PUGIXML_LIBS)

test_DEPENDENCIES = ../src/engine/libfzclient-private.la
