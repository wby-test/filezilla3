ACLOCAL_AMFLAGS = -I m4

AUTOMAKE_OPTIONS = 1.6

if !LOCALES_ONLY
MAYBE_LIB = lib
MAYBE_DEMOS = demos
MAYBE_DOC = doc
if HAVE_CPPUNIT
MAYBE_TESTS = tests
endif
endif

if LOCALES
MAYBE_LOCALES = locales
endif

SUBDIRS = . $(MAYBE_LIB) $(MAYBE_DEMOS) $(MAYBE_LOCALES) $(MAYBE_TESTS) $(MAYBE_DOC)

dist_noinst_DATA = libfilezilla.sln
