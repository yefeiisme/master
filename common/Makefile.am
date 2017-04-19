bin_PROGRAMS = main
main_SOURCES = main.cpp
SUBDIRS = common LuaLib
main_LDFLAGS = -static
main_LDADD =  -lpthread
main_LDADD+= ../Share/llib/libcommon.a
main_LDADD+= ../Share/llib/liblua.a

INCLUDES = -I@top_srcdir@/Share/Header/
INCLUDES += -I../Share/Header/
INCLUDES += -I../Share/Header/Base/
INCLUDES += -I../Share/Header/GameServer/
