# these are the headers for your project

noinst_LIBRARIES = libfzdbus.a

libfzdbus_a_CPPFLAGS = -I$(top_builddir)/config
libfzdbus_a_CPPFLAGS += $(LIBDBUS_CFLAGS) $(WX_CPPFLAGS) $(LIBFILEZILLA_CFLAGS)
libfzdbus_a_CXXFLAGS = $(WX_CXXFLAGS_ONLY)
libfzdbus_a_CFLAGS = $(WX_CFLAGS_ONLY)


libfzdbus_a_SOURCES = \
		dbushandler.cpp \
		desktop_notification.cpp \
		power_management_inhibitor.cpp \
		session_manager.cpp \
		wxdbusconnection.cpp \
		wxdbusmessage.cpp

noinst_HEADERS = \
		dbushandler.h \
		desktop_notification.h \
		power_management_inhibitor.h \
		session_manager.h \
		wxdbusconnection.h \
		wxdbusmessage.h

