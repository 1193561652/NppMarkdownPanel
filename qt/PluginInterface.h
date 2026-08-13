#ifndef NPP_CROSS_PLATFORM_PLUGIN_INTERFACE_H
#define NPP_CROSS_PLATFORM_PLUGIN_INTERFACE_H

#include <stddef.h>
#include <stdint.h>

#define NPP_PLUGIN_ABI_VERSION 1u

#if defined(_WIN32)
#define NPP_PLUGIN_CALL __cdecl
#if defined(NPP_PLUGIN_BUILD)
#define NPP_PLUGIN_EXPORT __declspec(dllexport)
#else
#define NPP_PLUGIN_EXPORT
#endif
#else
#define NPP_PLUGIN_CALL
#if defined(NPP_PLUGIN_BUILD)
#define NPP_PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#define NPP_PLUGIN_EXPORT
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NppPluginSystemType {
    NPP_PLUGIN_SYSTEM_UNKNOWN = 0,
    NPP_PLUGIN_SYSTEM_WINDOWS = 1,
    NPP_PLUGIN_SYSTEM_LINUX = 2,
    NPP_PLUGIN_SYSTEM_MACOS = 3
} NppPluginSystemType;

typedef enum NppPluginCpuArchitecture {
    NPP_PLUGIN_CPU_UNKNOWN = 0,
    NPP_PLUGIN_CPU_X86 = 1,
    NPP_PLUGIN_CPU_X64 = 2,
    NPP_PLUGIN_CPU_ARM64 = 3
} NppPluginCpuArchitecture;

typedef enum NppPluginLogLevel {
    NPP_PLUGIN_LOG_DEBUG = 0,
    NPP_PLUGIN_LOG_INFO = 1,
    NPP_PLUGIN_LOG_WARNING = 2,
    NPP_PLUGIN_LOG_ERROR = 3
} NppPluginLogLevel;

typedef enum NppPluginNotificationCode {
    NPP_PLUGIN_NOTIFICATION_READY = 1,
    NPP_PLUGIN_NOTIFICATION_SHUTDOWN = 2,
    NPP_PLUGIN_NOTIFICATION_FILE_BEFORE_LOAD = 3,
    NPP_PLUGIN_NOTIFICATION_FILE_BEFORE_OPEN = 4,
    NPP_PLUGIN_NOTIFICATION_FILE_OPENED = 5,
    NPP_PLUGIN_NOTIFICATION_FILE_LOAD_FAILED = 6,
    NPP_PLUGIN_NOTIFICATION_FILE_BEFORE_CLOSE = 7,
    NPP_PLUGIN_NOTIFICATION_FILE_CLOSED = 8,
    NPP_PLUGIN_NOTIFICATION_FILE_BEFORE_SAVE = 9,
    NPP_PLUGIN_NOTIFICATION_FILE_SAVED = 10,
    NPP_PLUGIN_NOTIFICATION_BUFFER_ACTIVATED = 11,
    NPP_PLUGIN_NOTIFICATION_LANGUAGE_CHANGED = 12,
    NPP_PLUGIN_NOTIFICATION_TEXT_MODIFIED = 13,
    NPP_PLUGIN_NOTIFICATION_UPDATE_UI = 14,
    NPP_PLUGIN_NOTIFICATION_DARK_MODE_CHANGED = 15
} NppPluginNotificationCode;

typedef size_t (NPP_PLUGIN_CALL *NppPluginGetCurrentFilePath)(
    void* host_context, char* output_utf8, size_t output_capacity);
typedef int (NPP_PLUGIN_CALL *NppPluginOpenFile)(
    void* host_context, const char* path_utf8);
typedef void (NPP_PLUGIN_CALL *NppPluginLog)(
    void* host_context, uint32_t level, const char* message_utf8);
typedef uint64_t (NPP_PLUGIN_CALL *NppPluginGetCurrentBufferId)(
    void* host_context);
typedef int32_t (NPP_PLUGIN_CALL *NppPluginGetCurrentView)(
    void* host_context);
typedef size_t (NPP_PLUGIN_CALL *NppPluginGetCurrentDocument)(
    void* host_context, uint8_t* output, size_t output_capacity);
typedef int (NPP_PLUGIN_CALL *NppPluginReplaceCurrentDocument)(
    void* host_context, const uint8_t* data, size_t data_size);
typedef size_t (NPP_PLUGIN_CALL *NppPluginGetCurrentSelection)(
    void* host_context, uint8_t* output, size_t output_capacity,
    int64_t* start, int64_t* end);
typedef int (NPP_PLUGIN_CALL *NppPluginReplaceCurrentSelection)(
    void* host_context, const uint8_t* data, size_t data_size);
typedef int (NPP_PLUGIN_CALL *NppPluginSetCurrentSelection)(
    void* host_context, int64_t start, int64_t end);
typedef int (NPP_PLUGIN_CALL *NppPluginCreateDocument)(
    void* host_context, const uint8_t* data, size_t data_size);
typedef size_t (NPP_PLUGIN_CALL *NppPluginGetClipboardText)(
    void* host_context, char* output_utf8, size_t output_capacity);
typedef int (NPP_PLUGIN_CALL *NppPluginSetClipboardText)(
    void* host_context, const char* text_utf8);
typedef void (NPP_PLUGIN_CALL *NppPluginSetStatusText)(
    void* host_context, const char* text_utf8);
typedef size_t (NPP_PLUGIN_CALL *NppPluginGetViewDocument)(
    void* host_context, int32_t view, uint8_t* output, size_t output_capacity);
typedef int (NPP_PLUGIN_CALL *NppPluginShowBufferInView)(
    void* host_context, uint64_t buffer_id, int32_t view);
typedef void (NPP_PLUGIN_CALL *NppPluginClearCompareMarks)(
    void* host_context, int32_t view);
typedef int (NPP_PLUGIN_CALL *NppPluginAddCompareMark)(
    void* host_context, int32_t view, int64_t line, uint32_t kind);
typedef int64_t (NPP_PLUGIN_CALL *NppPluginGetFirstVisibleLine)(
    void* host_context, int32_t view);
typedef int (NPP_PLUGIN_CALL *NppPluginSetFirstVisibleLine)(
    void* host_context, int32_t view, int64_t line);
typedef int (NPP_PLUGIN_CALL *NppPluginGotoLine)(
    void* host_context, int32_t view, int64_t line);
typedef intptr_t (NPP_PLUGIN_CALL *NppPluginSendScintilla)(
    void* host_context, int32_t view, uint32_t message,
    uintptr_t w_param, intptr_t l_param);
typedef size_t (NPP_PLUGIN_CALL *NppPluginGetBufferFilePath)(
    void* host_context, uint64_t buffer_id, char* output_utf8,
    size_t output_capacity);
typedef int (NPP_PLUGIN_CALL *NppPluginSaveCurrentFile)(
    void* host_context);
typedef int (NPP_PLUGIN_CALL *NppPluginExecuteMenuCommand)(
    void* host_context, int32_t command_id);

typedef struct NppPluginHostInfo {
    uint32_t struct_size;
    uint32_t abi_version;
    void* host_context;
    uint32_t system_type;
    uint32_t cpu_architecture;
    const char* system_name_utf8;
    const char* system_version_utf8;
    const char* application_name_utf8;
    const char* application_version_utf8;
    const char* plugin_home_path_utf8;
    const char* plugin_config_path_utf8;
    NppPluginGetCurrentFilePath get_current_file_path;
    NppPluginOpenFile open_file;
    NppPluginLog log;
    NppPluginGetCurrentBufferId get_current_buffer_id;
    NppPluginGetCurrentView get_current_view;
    NppPluginGetCurrentDocument get_current_document;
    NppPluginReplaceCurrentDocument replace_current_document;
    NppPluginGetCurrentSelection get_current_selection;
    NppPluginReplaceCurrentSelection replace_current_selection;
    NppPluginSetCurrentSelection set_current_selection;
    NppPluginCreateDocument create_document;
    NppPluginGetClipboardText get_clipboard_text;
    NppPluginSetClipboardText set_clipboard_text;
    NppPluginSetStatusText set_status_text;
    NppPluginGetViewDocument get_view_document;
    NppPluginShowBufferInView show_buffer_in_view;
    NppPluginClearCompareMarks clear_compare_marks;
    NppPluginAddCompareMark add_compare_mark;
    NppPluginGetFirstVisibleLine get_first_visible_line;
    NppPluginSetFirstVisibleLine set_first_visible_line;
    NppPluginGotoLine goto_line;
    NppPluginSendScintilla send_scintilla;
    NppPluginGetBufferFilePath get_buffer_file_path;
    NppPluginSaveCurrentFile save_current_file;
    NppPluginExecuteMenuCommand execute_menu_command;
} NppPluginHostInfo;

typedef struct NppPluginShortcutKey {
    uint8_t is_ctrl;
    uint8_t is_alt;
    uint8_t is_shift;
    uint8_t reserved;
    uint32_t key;
} NppPluginShortcutKey;

typedef void (NPP_PLUGIN_CALL *NppPluginCommandProc)(void* user_data);

typedef struct NppPluginFuncItem {
    uint32_t struct_size;
    const char* item_name_utf8;
    NppPluginCommandProc command;
    void* user_data;
    uint8_t initially_checked;
    uint8_t reserved[7];
    NppPluginShortcutKey shortcut;
} NppPluginFuncItem;

typedef struct NppPluginNotification {
    uint32_t struct_size;
    uint32_t code;
    uint64_t buffer_id;
    int32_t source_view;
    int32_t reserved;
    int64_t position;
    int64_t length;
    uint32_t modification_type;
    uint32_t updated;
    const char* text_utf8;
} NppPluginNotification;

typedef uint32_t (NPP_PLUGIN_CALL *NppGetPluginAbiVersionFn)(void);
typedef const char* (NPP_PLUGIN_CALL *NppGetNameFn)(void);
typedef int (NPP_PLUGIN_CALL *NppSetInfoFn)(const NppPluginHostInfo* host_info);
typedef const NppPluginFuncItem* (NPP_PLUGIN_CALL *NppGetFuncsArrayFn)(
    uint32_t* count);
typedef void (NPP_PLUGIN_CALL *NppBeNotifiedFn)(
    const NppPluginNotification* notification);
typedef intptr_t (NPP_PLUGIN_CALL *NppMessageProcFn)(
    uint32_t message, uintptr_t w_param, intptr_t l_param);

NPP_PLUGIN_EXPORT uint32_t NPP_PLUGIN_CALL nppGetPluginAbiVersion(void);
NPP_PLUGIN_EXPORT const char* NPP_PLUGIN_CALL nppGetName(void);
NPP_PLUGIN_EXPORT int NPP_PLUGIN_CALL nppSetInfo(
    const NppPluginHostInfo* host_info);
NPP_PLUGIN_EXPORT const NppPluginFuncItem* NPP_PLUGIN_CALL nppGetFuncsArray(
    uint32_t* count);
NPP_PLUGIN_EXPORT void NPP_PLUGIN_CALL nppBeNotified(
    const NppPluginNotification* notification);
NPP_PLUGIN_EXPORT intptr_t NPP_PLUGIN_CALL nppMessageProc(
    uint32_t message, uintptr_t w_param, intptr_t l_param);

#ifdef __cplusplus
}
#endif

#endif
