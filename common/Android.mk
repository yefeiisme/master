LOCAL_PATH := $(call my-dir)

#include $(CLEAR_VARS)
#LOCAL_MODULE := libcommon_static
#LOCAL_SRC_FILES := libcommon.a
#include $(PREBUILT_STATIC_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := liblua_static
#LOCAL_SRC_FILES := liblua.a
#include $(PREBUILT_STATIC_LIBRARY)




include $(CLEAR_VARS)
LOCAL_MODULE := libcommon_static
MY_LOGIC_PATH := ../../../common
LOCAL_MODULE_FILENAME := libcommon
LOCAL_SRC_FILES :=$(MY_LOGIC_PATH)/common.cpp \
									$(MY_LOGIC_PATH)/RingBuffer/RingBuffer.cpp \
									$(MY_LOGIC_PATH)/Thread/Thread.cpp \
									$(MY_LOGIC_PATH)/Network/Client/Client.cpp \
									$(MY_LOGIC_PATH)/Network/conn_info/conn_info.cpp \
									$(MY_LOGIC_PATH)/Network/Server/Server.cpp \
									$(MY_LOGIC_PATH)/FileReader/KDebug.cpp \
									$(MY_LOGIC_PATH)/FileReader/KFile.cpp \
									$(MY_LOGIC_PATH)/FileReader/KFilePath.cpp \
									$(MY_LOGIC_PATH)/FileReader/KIniFile.cpp \
									$(MY_LOGIC_PATH)/FileReader/KLinkArray.cpp \
									$(MY_LOGIC_PATH)/FileReader/KMemBase.cpp \
									$(MY_LOGIC_PATH)/FileReader/KMemClass.cpp \
									$(MY_LOGIC_PATH)/FileReader/KMemStack.cpp \
									$(MY_LOGIC_PATH)/FileReader/KPakFile.cpp \
									$(MY_LOGIC_PATH)/FileReader/KStrBase.cpp \
									$(MY_LOGIC_PATH)/FileReader/KTabFile.cpp \
									$(MY_LOGIC_PATH)/Daemon/Daemon.cpp \
									$(MY_LOGIC_PATH)/FileLog/FileLog.cpp




LOCAL_C_INCLUDES := $(LOCAL_PATH)/ \
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../common \
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../common/LuaSrc \
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../Share/Header \
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../Share/Header/Base

include $(BUILD_STATIC_LIBRARY)










include $(CLEAR_VARS)
LOCAL_MODULE := liblua_static
MY_LOGIC_PATH := ../../../common/Lualib
LOCAL_MODULE_FILENAME := liblua
LOCAL_SRC_FILES :=$(MY_LOGIC_PATH)/LuaScript.cpp \
									$(MY_LOGIC_PATH)/LuaSrc/lapi.c \
									$(MY_LOGIC_PATH)/LuaSrc/lcode.c \
									$(MY_LOGIC_PATH)/LuaSrc/ldebug.c \
									$(MY_LOGIC_PATH)/LuaSrc/ldo.c \
									$(MY_LOGIC_PATH)/LuaSrc/ldump.c \
									$(MY_LOGIC_PATH)/LuaSrc/lfunc.c \
									$(MY_LOGIC_PATH)/LuaSrc/lgc.c \
									$(MY_LOGIC_PATH)/LuaSrc/llex.c \
									$(MY_LOGIC_PATH)/LuaSrc/lmem.c \
									$(MY_LOGIC_PATH)/LuaSrc/lobject.c \
									$(MY_LOGIC_PATH)/LuaSrc/lopcodes.c \
									$(MY_LOGIC_PATH)/LuaSrc/lparser.c \
									$(MY_LOGIC_PATH)/LuaSrc/lstate.c \
									$(MY_LOGIC_PATH)/LuaSrc/lstring.c \
									$(MY_LOGIC_PATH)/LuaSrc/ltable.c \
									$(MY_LOGIC_PATH)/LuaSrc/ltests.c \
									$(MY_LOGIC_PATH)/LuaSrc/ltm.c \
									$(MY_LOGIC_PATH)/LuaSrc/lua.c \
									$(MY_LOGIC_PATH)/LuaSrc/lundump.c \
									$(MY_LOGIC_PATH)/LuaSrc/lvm.c \
									$(MY_LOGIC_PATH)/LuaSrc/lzio.c \
									$(MY_LOGIC_PATH)/LuaSrc/lauxlib.c \
									$(MY_LOGIC_PATH)/LuaSrc/lbaselib.c \
									$(MY_LOGIC_PATH)/LuaSrc/ldblib.c \
									$(MY_LOGIC_PATH)/LuaSrc/liolib.c \
									$(MY_LOGIC_PATH)/LuaSrc/lmathlib.c \
									$(MY_LOGIC_PATH)/LuaSrc/loadlib.c \
									$(MY_LOGIC_PATH)/LuaSrc/lstrlib.c \
									$(MY_LOGIC_PATH)/LuaSrc/ltablib.c




LOCAL_C_INCLUDES := $(LOCAL_PATH)/ \
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../common \
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../common/LuaSrc \
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../Share/Header \
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../Share/Header/Base

include $(BUILD_STATIC_LIBRARY)
