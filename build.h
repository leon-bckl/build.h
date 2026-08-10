#ifndef BUILD_H_INCLUDED
#define BUILD_H_INCLUDED

/*
 * Platform detection
 */

#define OS_MACOS   1
#define OS_WINDOWS 2
#define OS_LINUX   3

#ifdef __APPLE__
	#define OS OS_MACOS
#elif defined(_WIN32)
	#define OS OS_WINDOWS
#elif defined(__linux__)
	#define OS OS_LINUX
#else
	#error "Unsupported os"
#endif

/*
 * Compiler detection
 */

#define COMPILER_CLANG 1
#define COMPILER_MSVC  2
#define COMPILER_GCC   3

#ifdef __clang__
	#define COMPILER COMPILER_CLANG
#elif defined(_MSC_VER)
	#define COMPILER COMPILER_MSVC
#elif defined(__GNUC__)
	#define COMPILER COMPILER_GCC
#else
	#error "Unsupported compiler"
#endif

/*
 * Architecture detection
 */

#define ARCH_X86    1
#define ARCH_X86_64 2
#define ARCH_ARM64  3

#if defined(_M_IX86) || defined(__i386__)
	#define ARCH         ARCH_X86
	#define POINTER_SIZE 4
	#define MAX_ALIGN    8
#elif defined(_M_X64) || defined(__amd64__)
	#define ARCH         ARCH_X86_64
	#define POINTER_SIZE 8
	#define MAX_ALIGN    16
#elif defined(__aarch64__)
	#define ARCH         ARCH_ARM64
	#define POINTER_SIZE 8
	#define MAX_ALIGN    16
#else
	#error "Unsupported architecture"
#endif

/*
 * Macros
 */

#define _stringify_helper(x) #x
#define _stringify(x) _stringify_helper(x)

#define _array_count(a) (int)(sizeof(a) / sizeof(a[0]))

#undef  NONE
#define NONE (void*)0

#define _check(expr) \
	do { \
		const int _exitCode = expr; \
		if(_exitCode != 0) \
			return _exitCode; \
	} while((void)0,0)

#if COMPILER == COMPILER_MSVC
	#define _noreturn __declspec(noreturn)
#else
	#define _noreturn __attribute__((noreturn))
#endif

/*
 * Constants
 */

#ifndef MAX_TARGETS
	#define MAX_TARGETS 16
#endif

#ifndef MAX_TARGET_DEPENDENCIES
	#define MAX_TARGET_DEPENDENCIES 8
#endif

#if MAX_TARGET_DEPENDENCIES > MAX_TARGETS
	#error "MAX_TARGET_DEPENDENCIES cannot be larger than MAX_TARGETS"
#endif

#ifndef MAX_SOURCES
	#define MAX_SOURCES 128
#endif

#ifndef MAX_COMMAND_LINE
	#define MAX_COMMAND_LINE 4096
#endif

#ifndef MAX_ARGS
	#define MAX_ARGS 128
#endif

#ifndef MAX_JOBS
	#define MAX_JOBS 64
#endif

#ifndef MAX_PATH
	#define MAX_PATH 260
#endif

/*
 * Types
 */

typedef unsigned char  _byte;
typedef unsigned short _ushort;
typedef unsigned int   _uint;
typedef unsigned long  _ulong;
typedef unsigned short _wchar;
typedef int            _bool;

#ifndef __cplusplus
	#ifndef true
		#define true 1
	#endif
	#ifndef false
		#define false 0
	#endif
#endif

typedef struct {
	const char* data;
	int         len;
} str;

typedef enum {
	Debug,
	Release,
	Profiling
} _buildconfig;

typedef struct{
#if OS != OS_WINDOWS
	int   argc;
	char* argv[MAX_ARGS + 1];
#endif
	int   len;
	char  buffer[MAX_COMMAND_LINE];
} _cmdlinebuffer;

typedef enum{
	CDefault,
	C89,
	C99,
	C11,
	C23
} _clanguageversion;

typedef enum{
	CppDefault,
	Cpp03,
	Cpp11,
	Cpp14,
	Cpp17,
	Cpp20,
	Cpp23,
	Cpp26
} _cpplanguageversion;

typedef enum{
	OptimizeDefault,
	OptimizeNone,
	OptimizeSpeed,
	OptimizeSize
} _optimizationlevel;

typedef enum{
	NotSet,
	Disabled,
	Enabled
} _tristate;

typedef struct{
	_clanguageversion   cVersion;
	_cpplanguageversion cppVersion;
	_optimizationlevel  optimizations;
	_tristate           debugInformation;
	_cmdlinebuffer      _flags;
	_cmdlinebuffer      _cFlags;
	_cmdlinebuffer      _cppFlags;
} _compileoptions;

typedef struct{
	_cmdlinebuffer _flags;
} _linkoptions;

typedef enum _target_type {
	ObjectFile,
	DebugInformation,
	Executable,
	StaticLibrary,
	SharedLibrary
} _target_type;

typedef struct _target* Target;

struct _target {
	const char*     name;
	str             projectDir;
	_target_type    type;
	_compileoptions compileOpt;
	_linkoptions    linkOpt;
	_bool           _skipBuild;
	_bool           _isLinkTarget;
	int             _sourceCount;
	int             _linkDependencyCount;
	Target          _linkDependencies[MAX_TARGET_DEPENDENCIES];
};

typedef enum{
	CSrc,
	CppSrc,
	RcSrc
} _sourcekind;

typedef struct {
	Target      target;
	str         fileName;
	_sourcekind kind;
	int         compiled;
} _source;

typedef struct {
	int  len;
	char data[MAX_PATH];
} _pathbuffer;

/*
 * Globals
 */

_buildconfig           buildConfiguration = Debug;

static const str       _emptyString = {0};

static str             _g_buildDir;
static str             _g_currentProjectDir;
static _bool           _g_verbose = false;
static int             _g_maxJobs = 1;

static char            _g_cwdBuffer[MAX_PATH];
static str             _g_cwd;

static struct _target  _g_targets[MAX_TARGETS];
static const char*     _g_targetsToBuild[MAX_TARGETS + 1];
static int             _g_targetCount = 0;
static Target          _g_nextTarget  = &_g_targets[0];

static _compileoptions _g_compileOptions;
static _linkoptions    _g_linkOptions;

static _source         _g_sources[MAX_SOURCES];
static int             _g_sourceCount = 0;

static char            _g_logBuffer[512];
static int             _g_logBufferLen = 0;
static _bool           _g_logAtNewline = true;

#if OS == OS_WINDOWS
static void*  _g_win32StdoutHandle = NONE;
static int    _g_win32Argc         = 0;
static char*  _g_win32Argv[128];
static char   _g_win32ArgvBuffer[512];
static int    _g_win32CwdLen = 0;
static _wchar _g_win32Cwd[MAX_PATH];
static void*  _g_win32Processes[MAX_JOBS + 1]; /* +1 so the last entry is always NONE */
static int    _g_win32ProcessCount = 0;
#endif

/*
 * Ignore warnings
 */

#if COMPILER == COMPILER_MSVC
	#pragma warning(push)
	#pragma warning(disable: 4210) /* nonstandard extension used : function given file scope */
#elif COMPILER == COMPILER_GCC || COMPILER == COMPILER_CLANG
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wcast-function-type" /* GetProcAddress return value */
#endif

/*
 * Memory functions
 */

static void* mem_fill(void* dest, int fill, int size) {
	const _byte val = (_byte)fill;
	_byte* d;

	for(d = (_byte*)dest; size; --size, ++d)
		*d = val;

	return dest;
}

static void* mem_copy(void* dest, const void* src, int size) {
	const _byte* srcByte  = (const _byte*)src;
	_byte*       destByte = (_byte*)dest;

	while(size) {
		*destByte = *srcByte;
		++destByte;
		++srcByte;
		--size;
	}

	return dest;
}

void* mem_move(void* dest, const void* src, int size) {
	if(dest == src || size == 0)
		return dest;

	if((_byte*)dest >= (_byte*)src + size || dest > src) {
		mem_copy(dest, src, size);
	} else {
		while(size) {
			--size;
			((_byte*)dest)[size] = ((_byte*)src)[size];
		}
	}

	return dest;
}

/*
 * Char functions
 */

static char ascii_lower(int c) {
	return (char)(c >= 'A' && c <= 'Z' ? c + 32 : c);
}

static char ascii_upper(int c) {
	return (char)(c >= 'a' && c <= 'z' ? c - 32 : c);
}

static _bool ascii_isalpha(int c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static _bool ascii_isdigit(int c) {
	return c >= '0' && c <= '9';
}

static _bool ascii_isspace(int c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

static _bool is_pathsep(int c) {
	return c == '/'
#if OS == OS_WINDOWS
		|| c == '\\'
#endif
	;
}


/*
 * win32
 */

#if OS == OS_WINDOWS

#if MAX_PATH > 260
	#error "MAX_PATH must not be larger than 260 on windows"
#endif

#if MAX_JOBS > 64
	#error "MAX_JOBS must not be larger than 64 on windows"
#endif

#define _import __declspec(dllimport)
#define _winapi __stdcall

#if defined(NO_CRT) && COMPILER == COMPILER_MSVC
	#pragma comment(lib, "kernel32.lib")
#endif

#ifndef _WINBASE_

typedef struct {
	void*  hProcess;
	void*  hThread;
	_ulong dwProcessId;
	_ulong dwThreadId;
} PROCESS_INFORMATION;

typedef struct {
	_ulong        cb;
	const _wchar* lpReserved;
	const _wchar* lpDesktop;
	const _wchar* lpTitle;
	_ulong        dwX;
	_ulong        dwY;
	_ulong        dwXSize;
	_ulong        dwYSize;
	_ulong        dwXCountChars;
	_ulong        dwYCountChars;
	_ulong        dwFillAttribute;
	_ulong        dwFlags;
	_ushort       wShowWindow;
	_ushort       cbReserved2;
	_byte*        lpReserved2;
	void*         hStdInput;
	void*         hStdOutput;
	void*         hStdError;
} STARTUPINFOW;

typedef struct{
	union {
		_ulong dwOemId;
		struct {
			_ushort wProcessorArchitecture;
			_ushort wReserved;
		} DUMMYSTRUCTNAME;
	} DUMMYUNIONNAME;
	_ulong  dwPageSize;
	void*   lpMinimumApplicationAddress;
	void*   lpMaximumApplicationAddress;
	_ulong* dwActiveProcessorMask;
	_ulong  dwNumberOfProcessors;
	_ulong  dwProcessorType;
	_ulong  dwAllocationGranularity;
	_ushort wProcessorLevel;
	_ushort wProcessorRevision;
} SYSTEM_INFO;

typedef struct{
	_ulong dwLowDateTime;
	_ulong dwHighDateTime;
} FILETIME;

typedef struct{
	_ulong   dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	_ulong   nFileSizeHigh;
	_ulong   nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA;

#define INVALID_FILE_ATTRIBUTES  ((_ulong)-1)
#define FILE_ATTRIBUTE_READONLY  0x00000001
#define FILE_ATTRIBUTE_HIDDEN    0x00000002
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_NORMAL    0x00000080

typedef enum{
	GetFileExInfoStandard,
	GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;

typedef struct SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES;

_import _ulong _winapi GetLastError(void);

_import void _winapi GetSystemInfo(SYSTEM_INFO* lpSystemInfo);

_import int _winapi CloseHandle(void* hObject);

_import int _winapi CreateProcessW(const _wchar* lpApplicationName, _wchar* lpCommandLine, void* lpProcessAttributes, void* lpThreadAttributes, int bInheritHandles, _ulong dwCreationFlags, void* lpEnvironment, const _wchar* lpCurrentDirectory, STARTUPINFOW* lpStartupInfo, PROCESS_INFORMATION* lpProcessInformation);
_import _noreturn void _winapi ExitProcess (_uint uExitCode);
_import int _winapi GetExitCodeProcess(void* hProcess, _ulong* lpExitCode);

_import void* _winapi LoadLibraryW(const _wchar* lpLibFileName);
typedef void(_winapi*_farproc)(void);
_import _farproc _winapi GetProcAddress(void* hModule, const char* lpProcName);

_import void*   _winapi GetStdHandle(_ulong nStdHandle);
_import _wchar* _winapi GetCommandLineW(void);

_import _ulong _winapi GetCurrentDirectoryW(_ulong nBufferLength, _wchar* lpBuffer);

_import _ulong _winapi GetFileAttributesW(const _wchar* lpFileName);
_import int _winapi GetFileAttributesExW(const _wchar* lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, void* lpFileInformation);
_import int _winapi WriteFile(void* hFile, const void* lpBuffer, _ulong nNumberOfBytesToWrite, _ulong* lpNumberOfBytesWritten, void* lpOverlapped);

_import int _winapi CreateDirectoryW(const _wchar* lpPathName, SECURITY_ATTRIBUTES* lpSecurityAttributes);

#define INFINITE    0xffffffff
#define WAIT_FAILED 0xffffffff

_import _ulong _winapi WaitForSingleObject(void* hHandle, _ulong dwMilliseconds);
_import _ulong _winapi WaitForMultipleObjects(_ulong nCount, void* const* lpHandles, int bWaitAll, _ulong dwMilliseconds);

#define CP_UTF8 65001

_import int _winapi MultiByteToWideChar(_uint CodePage, _ulong dwFlags, const char* lpMultiByteStr, int cbMultiByte, _wchar* lpWideCharStr, int cchWideChar);
_import int _winapi WideCharToMultiByte(_uint CodePage, _ulong dwFlags, const _wchar* lpWideCharStr, int cchWideChar, char* lpMultiByteStr, int cbMultiByte, const char* lpDefaultChar, int* lpUsedDefaultChar);
#endif

static int _win32_assemble_path(str path, _wchar* buffer) {
	int len;

	if(path.len <= 0)
		return 0;

	if(path.len > 2 && ascii_isalpha(path.data[0]) && path.data[1] == ':') {
		len = MultiByteToWideChar(CP_UTF8, 0, path.data, path.len, buffer, MAX_PATH);
	} else {
		if(path.len + _g_win32CwdLen + 1 >= MAX_PATH)
			return 1;

		len = _g_win32CwdLen;
		mem_copy(buffer, _g_win32Cwd, len * sizeof(_wchar));
		len += MultiByteToWideChar(CP_UTF8, 0, path.data, path.len, buffer + len, MAX_PATH - len);
	}

	buffer[len] = '\0';

	for(len -= 1; len >= 0; --len) {
		if(buffer[len] == '/')
			buffer[len] = '\\';
	}

	return 0;
}
#endif

/*
 * str functions
 */

static int str_len(const char* s) {
	int len = 0;

	if(s == NONE)
		return 0;

	while(s[len])
		++len;

	return len;
}

static str _str(const char* s, int len) {
	str result;
	result.data = s;
	result.len  = len;
	return result;
}
#define _s(str) _str(str, sizeof(str) - 1)

static str _cstr(const char* cstr) {
	return _str(cstr, str_len(cstr));
}

static _bool _str_eq(str s1, str s2) {
	int i;

	if(s1.len != s2.len)
		return 0;

	for(i = 0; i < s1.len; ++i) {
		if(s1.data[i] != s2.data[i])
			return 0;
	}

	return 1;
}

static _bool _str_ieq(str s1, str s2) {
	int i;

	if(s1.len != s2.len)
		return 0;

	for(i = 0; i < s1.len; ++i) {
		if(ascii_lower(s1.data[i]) != ascii_lower(s2.data[i]))
			return 0;
	}

	return 1;
}

static _bool str_ieq(const char* s1, const char* s2) {
	return _str_eq(_cstr(s1), _cstr(s2));
}

static str _str_left(str s, int count) {
	if(count < s.len)
		s.len = count;

	return s;
}

static str _str_right(str s, int count) {
	if(count < s.len) {
		s.len -= count;
		s.data += s.len;
	}

	return s;
}

static _bool _str_prefix(str s, str start) {
	if(start.len > s.len)
		return 0;

	return start.len <= 0 || _str_eq(_str_left(s, start.len), start);
}

static _bool str_prefix(const char* s, const char* start) {
	return _str_prefix(_cstr(s), _cstr(start));
}

static _bool _str_iprefix(str s, str start) {
	if(start.len > s.len)
		return 0;

	return start.len <= 0 || _str_ieq(_str_left(s, start.len), start);
}

static _bool str_iprefix(const char* s, const char* start) {
	return _str_iprefix(_cstr(s), _cstr(start));
}

static _bool _str_suffix(str s, str start) {
	if(start.len > s.len)
		return 0;

	return start.len <= 0 || _str_eq(_str_right(s, start.len), start);
}

static _bool str_suffix(const char* s, const char* start) {
	return _str_suffix(_cstr(s), _cstr(start));
}

static _bool _str_isuffix(str s, str start) {
	if(start.len > s.len)
		return 0;

	return start.len <= 0 || _str_ieq(_str_right(s, start.len), start);
}

static _bool str_isuffix(const char* s, const char* start) {
	return _str_isuffix(_cstr(s), _cstr(start));
}

static int _str_to_int(str s) {
	const char* c     = s.data;
	const char* end   = s.data + s.len;
	int         isNeg = 0;
	int         val   = 0;

	while(c < end && ascii_isspace(*c))
		++c;

	if(c >= end)
		return 0;

	if(*c == '-') {
		isNeg = 1;
		++c;
	}

	while(c < end && ascii_isdigit(*c)) {
		val *= 10;
		val += *c - '0';
		++c;
	}

	if(isNeg)
		val = -val;

	return val;
}

static str _int_to_string(int intValue, char* buffer) {
	int   digitCount = 1;
	int   i          = 10;
	char* dest       = buffer;

	if(intValue < 0) {
		*dest = '-';
		++dest;
	}

	for(i = 10; i <= intValue; i *= 10)
		++digitCount;

	for(i = digitCount; i > 0; --i) {
		dest[i - 1] = (char)('0' + (intValue % 10));
		intValue /= 10;
	}

	return _str(buffer, digitCount);
}

/*
 * Path functions
 */

static void _log_err(str msg, int line, const char* file);

static int _path_append_raw(str s, _pathbuffer* buffer) {
	if(buffer->len + s.len > MAX_PATH) {
		_log_err(_s("Path too long, define MAX_PATH to increase limit (" _stringify(MAX_PATH) ")"), -1, NONE);
		return 1;
	}

	mem_copy(&buffer->data[buffer->len], s.data, s.len);
	buffer->len += s.len;

	return 0;
}

static int _path_add_segment(str s, _pathbuffer* buffer) {
	const int hasPathSep = s.len > 0 && is_pathsep(s.data[s.len - 1]);

	_check(_path_append_raw(s, buffer));

	/*
	 * Only add path separator for empty strs if the path doesn't already have one to avoid double separators '//'
	 */
	if(!hasPathSep && (s.len > 0 || (buffer->len > 0 && !is_pathsep(buffer->data[buffer->len - 1]))))
		_check(_path_append_raw(_s("/"), buffer));

	return 0;
}

static _bool _path_is_abs(str path) {
#if OS == OS_WINDOWS
	return path.len >= 2 && ascii_isalpha(path.data[0]) && path.data[1] == ':';
#else
	return path.len > 0 && path.data[0] == '/';
#endif
}

static _bool path_is_abs(const char* path) {
	return _path_is_abs(_cstr(path));
}

static char _normalized_path_char(int c) {
	if(is_pathsep(c))
		return '/';

	return ascii_lower(c); /* FIXME: Linux */
}

static _bool _path_eq(str path1, str path2) {
	int i;

	if(path1.len != path2.len)
		return 0;

	for(i = 0; i < path1.len; ++i) {
		if(_normalized_path_char(path1.data[i]) != _normalized_path_char(path2.data[i]))
			return 0;
	}

	return 1;
}

static _bool path_eq(const char* path1, const char* path2) {
	return _path_eq(_cstr(path1), _cstr(path2));
}

static _bool _path_prefix(str path, str start) {
	if(start.len > path.len)
		return 0;

	return start.len <= 0 || _path_eq(_str_left(path, start.len), start);
}

static _bool path_prefix(const char* path, const char* start) {
	return _path_prefix(_cstr(path), _cstr(start));
}

static str _file_without_path(str filePath) {
	int i = filePath.len;

	while(i > 0 && !is_pathsep(filePath.data[i - 1]))
		--i;

	filePath.len  -= i;
	filePath.data += i;

	return filePath;
}

static str _path_without_file(str filePath) {
	while(filePath.len > 0 && !is_pathsep(filePath.data[filePath.len - 1]))
		--filePath.len;

	return filePath;
}

static str _file_basename(str fileName) {
	int i = fileName.len - 1;

	while(i > 0) {
		if(fileName.data[i] == '.') {
			fileName.len = i;
			break;
		}

		if(is_pathsep(fileName.data[i]))
			break;

		--i;
	}

	return fileName;
}

static str _file_ext(str fileName) {
	int i = fileName.len - 1;

	while(i > 0) {
		if(fileName.data[i] == '.')
			return _str(fileName.data + i, fileName.len - i);

		if(is_pathsep(fileName.data[i]))
			break;

		--i;
	}

	return _emptyString;
}

/*
 * Console output
 */

static void log_flush(void) {
	if(_g_logBufferLen > 0) {
#if OS == OS_WINDOWS
		_ulong numWritten;
		WriteFile(_g_win32StdoutHandle, _g_logBuffer, (_ulong)_g_logBufferLen, &numWritten, NONE);
#endif

		_g_logBufferLen = 0;
	}
}

static void _log_raw(str msg) {
	if(msg.len > 0)
		_g_logAtNewline = msg.data[msg.len - 1] == '\n';

	while(msg.len > 0) {
		const int freeSize = sizeof(_g_logBuffer) - _g_logBufferLen;
		int       copySize = msg.len;

		if(msg.len > freeSize)
			copySize = freeSize;

		mem_copy(_g_logBuffer + _g_logBufferLen, msg.data, copySize);
		_g_logBufferLen += copySize;
		msg.data        += copySize;
		msg.len         -= copySize;

		if(_g_logBufferLen >= (int)sizeof(_g_logBuffer))
			log_flush();
	}

	if(_g_logAtNewline)
		log_flush();
}

static void log_raw(const char* msg) {
	_log_raw(_cstr(msg));
}

static void _log_msg(str msg, int line, const char* file) {
	if(!_g_logAtNewline)
		_log_raw(_s("\n"));

	if(line > 0 && file != NONE) {
		char buffer[32];

		_log_raw(_cstr(file));
		_log_raw(_s(":"));
		_log_raw(_int_to_string(line, buffer));
		_log_raw(_s(" - "));
	}

	_log_raw(msg);
}
#define log_msg(msg) _log_msg(_cstr(msg), _g_verbose ? __LINE__ : -1, __FILE__)

static void _log_err(str msg, int line, const char* file) {
	_log_msg(_s("ERROR: "), line, file);
	_log_raw(msg);
}
#define log_err(msg) _log_err(_cstr(msg),  __LINE__, __FILE__)

/*
 * Filesystem
 */

static int _create_directory(str path, int line, const char* file) {
#if OS == OS_WINDOWS
	_wchar  pathBuffer[MAX_PATH];
	_wchar* p;

	_check(_win32_assemble_path(path, pathBuffer));

	for(p = pathBuffer; *p; ++p) {
		_wchar curr;
		_ulong attributes;

		while(*p && !is_pathsep(*p))
			++p;

		curr = *p;
		*p   = '\0';

		attributes = GetFileAttributesW(pathBuffer);

		if(attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			if(!CreateDirectoryW(pathBuffer, NONE))
				goto error;
		}

		*p = curr;
	}
#endif

	return 0;

error:
	_log_err(_s("Failed to create directory '"), line, file);
	_log_raw(path);
	_log_raw(_s("'"));

	return 1;
}

#define create_directory(path) _check(_create_directory(_cstr(path), __LINE__, __FILE__))

/*
 * ParseList
 */

static int _parse_list(const char* list, int(*callback)(str, void*, int, const char*), void* userData, int line, const char* file) {
	const char* start;
	const char* c;

	if(list == NONE)
		return 0;

	c = list;

	do {
		int quoted;
		int callbackResult;

		while(*c && (ascii_isspace(*c) || *c == ','))
			++c;

		if(!*c)
			break;

		start  = c;
		quoted = *c == '"' && (c == list || c[-1] != '\\');

		if(quoted) {
			start = ++c;

			for(; c[0] != '\"' || c[-1] == '\\'; ++c) {
				if(!*c) {
					_log_err(_s("Unmatched '\"'"), line, file);
					return 1;
				}
			}
		} else {
			while(*c && !ascii_isspace(*c) && *c != ',')
				++c;
		}

		callbackResult = callback(_str(start, (int)(c - start)), userData, line, file);

		if(callbackResult != 0)
			return callbackResult;

		if(*c)
			++c; /* Skip separator if not at \0 */
	} while(*c);

	return 0;
}

/*
 * Target
 */

static Target _find_target(str name) {
	int i;

	for(i = 0; i < _g_targetCount; ++i) {
		if(_str_ieq(name, _cstr(_g_targets[i].name)))
			return &_g_targets[i];
	}

	return NONE;
}

static Target find_target(const char* name) {
	return _find_target(_cstr(name));
}

static _bool _is_target_ptr(const void* ptr) {
	return ptr != NONE && (Target)ptr >= &_g_targets[0] && (Target)ptr < _g_targets + MAX_TARGETS;
}

static Target _get_target(const void* targetOrTargetName, int line, const char* file) {
	Target target = (Target)targetOrTargetName;

	if(!_is_target_ptr(targetOrTargetName))
		target = find_target((const char*)targetOrTargetName);

	if(target == NONE)
		_log_err(_s("Invalid target"), line, file);

	return target;
}

static int _target_source_callback(str fileName, void* userData, int line, const char* file) {
	Target    target  = (Target)userData;
	const str fileExt = _file_ext(fileName);
	_source*  source;

	if(_g_sourceCount >= MAX_SOURCES) {
		_log_err(_s("Too many sources, define MAX_SOURCES to increase limit (" _stringify(MAX_SOURCES) ")"), line, file);
		return 1;
	}

	source           = &_g_sources[_g_sourceCount];
	source->target   = target;
	source->fileName = fileName;

	if(_str_ieq(fileExt, _s(".c")) ||
	   _str_ieq(fileExt, _s(".m"))) {
		source->kind = CSrc;
	} else if(_str_ieq(fileExt, _s(".cpp")) ||
	          _str_ieq(fileExt, _s(".cxx")) ||
	          _str_ieq(fileExt, _s(".cc")) ||
	          _str_ieq(fileExt, _s(".mm"))) {
		source->kind = CppSrc;
	} else if(_str_ieq(fileExt, _s(".rc"))) {
		source->kind = RcSrc;
	} else if(_str_ieq(fileExt, _s(".h")) ||
	          _str_ieq(fileExt, _s(".hpp")) ||
	          _str_ieq(fileExt, _s(".hxx")) ||
	          _str_ieq(fileExt, _s(".inl"))) {
		return 0; /* Ignore header files */
	} else {
		_log_err(_s("Unsupported file extension '"), line, file);
		_log_raw(fileExt);
		_log_raw(_s("'"));
		return 1;
	}

	++_g_sourceCount;
	++target->_sourceCount;

	return 0;
}

static int _target_sources(const void* targetOrTargetName, const char* sources, int line, const char* file) {
	Target target = _get_target(targetOrTargetName, line, file);

	if(target == NONE)
		return 1;

	return _parse_list(sources, _target_source_callback, target, line, file);
}

#define target_sources(target_or_target_name, source_list) _target_sources(target_or_target_name, source_list, __LINE__, __FILE__)

static int _add_target(const char* name, _target_type type, const char* sources, int line, const char* file) {
	Target       target;

	if(_g_targetCount >= MAX_TARGETS) {
		_log_err(_s("Too many targets, define MAX_TARGETS to increase limit (" _stringify(MAX_TARGETS) ")"), line, file); \
		 return 1;
	}

	target = find_target(name);

	if(target != NONE) {
		_log_err(_s("Target name must be unique"), line, file); \
		 return 1;
	}

	target             = &_g_targets[_g_targetCount++];
	target->name       = name;
	target->projectDir = _g_currentProjectDir;
	target->type       = type;

	target_sources(target, sources);

	return 0;
}

static Target _nextTarget(void){
	return _g_nextTarget++;
}

#define add_target(target_name, target_type, source_list) \
	_nextTarget(); \
	_check(_add_target(target_name, target_type, source_list, __LINE__, __FILE__))

#define add_executable(executable_name, source_list)  add_target(executable_name, Executable, source_list)
#define add_static_library(library_name, source_list) add_target(library_name, StaticLibrary, source_list)
#define add_shared_library(library_name, source_list) add_target(library_name, SharedLibrary, source_list)

/*
 * Target output files
 */

static str _rel_build_path(str path) {
	path = _path_without_file(path);

	if(_path_is_abs(path)) {
		if(_path_prefix(path, _g_cwd)) {
			path.data += _g_cwd.len;
			path.len  -= _g_cwd.len;
		} else {
			do {
				++path.data;
				--path.len;
			} while(path.len > 0 && !is_pathsep(path.data[-1]));
		}
	}

	return path;
}

static str _target_file_ext(_target_type targetType) {
	switch(targetType) {
	case ObjectFile:
#if COMPILER == COMPILER_MSVC
		return _s(".obj");
#else
		return _s(".o");
#endif
	case DebugInformation:
#if OS == OS_WINDOWS
		return _s(".pdb");
#else
		return _emptyString;
#endif
	case Executable:
#if OS == OS_WINDOWS
		return _s(".exe");
#else
		return _emptyString;
#endif
	case StaticLibrary:
#if COMPILER == COMPILER_MSVC
		return _s(".lib");
#else
		return _s(".a");
#endif
	case SharedLibrary:
#if OS == OS_WINDOWS
		return _s(".dll");
#elif OS == OS_MACOS
		return _s(".dylib");
#else
		return _s(".so");
#endif
	default:
		return _emptyString;
	}
}

static int _make_target_filepath(str baseName, str buildDir, _target_type targetType, _pathbuffer* buffer) {
	if(!_path_is_abs(buildDir))
		_check(_path_add_segment(_g_cwd, buffer));

	_check(_path_add_segment(buildDir, buffer));
	_check(_path_add_segment(_rel_build_path(baseName), buffer));

	/*
	 * Keep original extension for object and pbd files and add target extension at
	 * the end, to avoid ambiguities if a source file has the same name as the target
	 * */
	if(targetType == ObjectFile || targetType == DebugInformation)
		_check(_path_append_raw(_file_without_path(baseName), buffer));
	else
		_check(_path_append_raw(_file_basename(_file_without_path(baseName)), buffer));

	_check(_path_append_raw(_target_file_ext(targetType), buffer));

	return 0;
}

/*
 * Executable names
 */

static str _compiler_executable(_sourcekind sourceKind) {
	switch(sourceKind) {
	case CSrc:
#if COMPILER == COMPILER_MSVC
		return _s("cl");
#elif COMPILER == COMPILER_CLANG
		return _s("clang");
#elif COMPILER == COMPILER_GCC
		return _s("gcc");
#else
	#error "Unsupported C compiler"
#endif
	case CppSrc:
#if COMPILER == COMPILER_MSVC
		return _s("cl");
#elif COMPILER == COMPILER_CLANG
		return _s("clang++");
#elif COMPILER == COMPILER_GCC
		return _s("g++");
#else
	#error "Unsupported C++ compiler"
#endif
#if OS == OS_WINDOWS
	case RcSrc:
#if COMPILER == COMPILER_MSVC
		return _s("rc");
#elif COMPILER == COMPILER_CLANG
		return _s("llvm-rc");
#elif COMPILER == COMPILER_GCC
		return _s("windres");
#else
	#error "Unsupported resource compiler"
#endif
#endif
	default:
		return _emptyString;
	}
}

static str _linker_executable(_target_type targetType) {
	(void)targetType;
#if COMPILER == COMPILER_MSVC
	if(targetType == StaticLibrary)
		return _s("lib");

	return _s("link");
#elif COMPILER == COMPILER_CLANG
	return _s("clang");
#elif COMPILER == COMPILER_GCC
	return _s("gcc");
#else
	#error "Unsupported linker"
#endif
}

/*
 * Command line
 */

static void _log_cmdline(const _cmdlinebuffer* cmdLine) {
#if OS == OS_WINDOWS
	_log_msg(_str(cmdLine->buffer, cmdLine->len), -1, NONE);
#else
	int i;
	_log_msg(_cstr(cmdLine.argv[0]), -1, NONE);
	for(i = 1; i < cmdLine.argc; ++i) {
		_log_raw(_cstr(cmdLine.argv[i]), -1, NONE);
		_log_raw(_s(" "), -1, NONE);
	}
#endif
}

static int _cmdline_arg_push(str s, _cmdlinebuffer* cmdLine, int append) {
#if OS == OS_WINDOWS
	int needsEscape = s.len == 0 && !append; /* Escape empty arguments so they are not lost */
#endif
	int i;

	if(append && cmdLine->len > 0)
		--cmdLine->len;

	if(cmdLine->len + s.len >= MAX_COMMAND_LINE)
		goto too_long;

#if OS == OS_WINDOWS
	for(i = 0; i < s.len; ++i) {
		if(ascii_isspace(s.data[i]) || s.data[i] == '\\' || s.data[i] == ',') {
			needsEscape = 1;
			break;
		}
	}

	if(needsEscape) {
		const char* srcEnd = s.data + s.len;
		const char* src    = s.data;

		cmdLine->buffer[cmdLine->len] = '\"';
		++cmdLine->len;

		for(;; ++src) {
			int backslashCount = 0;

			while(src < srcEnd && *src == '\\') {
				++backslashCount;
				++src;
			}

			if(src == srcEnd) {
				if(cmdLine->len + backslashCount * 2 >= MAX_COMMAND_LINE)
					goto too_long;

				mem_fill(cmdLine->buffer + cmdLine->len, '\\', backslashCount * 2);
				cmdLine->len += backslashCount * 2;
				break;
			}

			if(*src == '\"') {
				const int count = backslashCount * 2 + 2;

				if(cmdLine->len + count >= MAX_COMMAND_LINE)
					goto too_long;

				mem_fill(cmdLine->buffer + cmdLine->len, '\\', count - 1);
				cmdLine->buffer[cmdLine->len + count - 1] = '\"';
				cmdLine->len += count;
			} else {
				if(cmdLine->len + backslashCount + 1 >= MAX_COMMAND_LINE)
					goto too_long;

				mem_fill(cmdLine->buffer + cmdLine->len, '\\', backslashCount);
				cmdLine->buffer[backslashCount + cmdLine->len] = *src;
				cmdLine->len += backslashCount + 1;
			}
		}

		if(cmdLine->len >= MAX_COMMAND_LINE)
			goto too_long;

		cmdLine->buffer[cmdLine->len] = '\"';
		++cmdLine->len;
	} else
#endif
	{
		mem_copy(cmdLine->buffer + cmdLine->len, s.data, s.len);
		cmdLine->len += s.len;
	}

	if(cmdLine->len >= MAX_COMMAND_LINE)
		goto too_long;

#if OS == OS_WINDOWS
	if(cmdLine->len < MAX_COMMAND_LINE) {
		cmdLine->buffer[cmdLine->len] = ' ';
		++cmdLine->len;
	}
#else
	cmdLine->buffer[cmdLine->len] = '\0';
	++cmdLine->len;
	#error "Update argc and argv here"
#endif

	return 0;

too_long:
	log_err("Command line too long, define MAX_COMMAND_LINE to increase limit (" _stringify(MAX_COMMAND_LINE) ")");
	return 1;
}

#define _cmdline_add_arg(arg, cmd_line) _cmdline_arg_push((arg), (cmd_line), 0)
#define _cmdline_append_arg(arg, cmd_line) _cmdline_arg_push((arg), (cmd_line), 1)

static int _cmdline_combine(_cmdlinebuffer* cmdLine, const _cmdlinebuffer* other) {
#if OS == OS_WINDOWS
	if(cmdLine->len + other->len > MAX_COMMAND_LINE) {
		log_err("Command line too long, define MAX_COMMAND_LINE to increase limit (" _stringify(MAX_COMMAND_LINE) ")");
		return 1;
	}

	mem_copy(cmdLine->buffer + cmdLine->len, other->buffer, other->len);
	cmdLine->len += other->len;
#else
	int i;
	for(i = 0; i < other.argc; ++i)
		_check(_cmdline_add_arg(_cstr(other.argv[i]), cmdLine)));
#endif

	return 0;
}

static int _cmdline_add_compile_options(const _source* source, str buildDir, const _compileoptions* defaults, const _compileoptions* overrides, _cmdlinebuffer* cmdLine) {
	const _clanguageversion   cVersion      = overrides->cVersion != CDefault ? overrides->cVersion : defaults->cVersion;
	const _cpplanguageversion cppVersion    = overrides->cppVersion != CppDefault ? overrides->cppVersion : defaults->cppVersion;
	const _optimizationlevel  optimizations = overrides->optimizations != OptimizeDefault ? overrides->optimizations : defaults->optimizations;
	const _tristate           debugInfo     = overrides->debugInformation != NotSet ? overrides->debugInformation : defaults->debugInformation;

#if COMPILER == COMPILER_MSVC
	#define _stdopt "/std:"
#else
	#define _stdopt "-std="
#endif

	if(source->kind == CSrc) {
		switch(cVersion) {
#if COMPILER != COMPILER_MSVC
		case C89:
			_check(_cmdline_add_arg(_s(_stdopt"c89"), cmdLine));
			break;
		case C99:
			_check(_cmdline_add_arg(_s(_stdopt"c99"), cmdLine));
			break;
#endif
		case C11:
			_check(_cmdline_add_arg(_s(_stdopt"c11"), cmdLine));
			break;
		case C23:
			_check(_cmdline_add_arg(_s(_stdopt"c23"), cmdLine));
			break;
		default:
			break;
		}
	}

	if(source->kind == CppSrc) {
		switch(cppVersion) {
		case Cpp03:
			_check(_cmdline_add_arg(_s(_stdopt"c++03"), cmdLine));
			break;
		case Cpp11:
			_check(_cmdline_add_arg(_s(_stdopt"c++11"), cmdLine));
			break;
		case Cpp14:
			_check(_cmdline_add_arg(_s(_stdopt"c++14"), cmdLine));
			break;
		case Cpp17:
			_check(_cmdline_add_arg(_s(_stdopt"c++17"), cmdLine));
			break;
		case Cpp20:
			_check(_cmdline_add_arg(_s(_stdopt"c++20"), cmdLine));
			break;
		case Cpp23:
			_check(_cmdline_add_arg(_s(_stdopt"c++23"), cmdLine));
			break;
		case Cpp26:
			_check(_cmdline_add_arg(_s(_stdopt"c++26"), cmdLine));
			break;
		default:
			break;
		}
	}
#undef _stdopt

	if(optimizations == OptimizeNone) {
#if COMPILER == COMPILER_MSVC
		_check(_cmdline_add_arg(_s("/Od"), cmdLine));
#else
		_check(_cmdline_add_arg(_s("-O0"), cmdLine));
#endif
	} else if(optimizations == OptimizeNone) {
#if COMPILER == COMPILER_MSVC
		_check(_cmdline_add_arg(_s("/O2"), cmdLine));
#else
		_check(_cmdline_add_arg(_s("-O2"), cmdLine));
#endif
	}

	if(debugInfo == Enabled) {
#if COMPILER == COMPILER_MSVC
		if(source->target->type == StaticLibrary) {
			_check(_cmdline_add_arg(_s("/Z7"), cmdLine));
		} else {
			_pathbuffer pathBuffer;

			_check(_cmdline_add_arg(_s("/Zi"), cmdLine));
			_check(_cmdline_add_arg(_s("/Fd"), cmdLine));
			pathBuffer.len = 0;
			_check(_make_target_filepath(source->fileName, buildDir, DebugInformation, &pathBuffer));
			_check(_cmdline_append_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));
		}
#else
		(void)buildDir;
		_check(_cmdline_add_arg(_s("-g"), cmdLine));
#endif
	}

	_check(_cmdline_combine(cmdLine, &defaults->_flags));
	_check(_cmdline_combine(cmdLine, &overrides->_flags));

	if(source->kind == CSrc)
		_check(_cmdline_combine(cmdLine, &overrides->_cFlags));
	else if(source->kind == CppSrc)
		_check(_cmdline_combine(cmdLine, &overrides->_cppFlags));

	return 0;
}

static int _build_compiler_cmdline(const _source* source, str buildDir, _cmdlinebuffer* cmdLine) {
	_pathbuffer pathBuffer;
	const str   compiler = _compiler_executable(source->kind);

	if(compiler.len == 0) {
		_log_err(_s("No matching compiler executable could be identified for '"), -1, NONE);
		_log_raw(source->fileName);
		_log_raw(_s("'"));
		return 1;
	}

	cmdLine->len   = 0;
	pathBuffer.len = 0;

	_check(_cmdline_add_arg(compiler, cmdLine));

#if COMPILER == COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/nologo"), cmdLine));
#endif

#if COMPILER == COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/c"), cmdLine));
#else
	_check(_cmdline_add_arg(_s("-c"), cmdLine));
#endif

	_check(_cmdline_add_compile_options(source, buildDir, &_g_compileOptions, &source->target->compileOpt, cmdLine));

#if COMPILER == COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/Fo"), cmdLine));
#else
	_check(_cmdline_add_arg(_s("-o"), cmdLine));
#endif

	pathBuffer.len = 0;
	_check(_make_target_filepath(source->fileName, buildDir, ObjectFile, &pathBuffer));
	_check(_cmdline_append_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));

	pathBuffer.len = 0;

	if(!_path_is_abs(source->target->projectDir))
		_check(_path_add_segment(_g_cwd, &pathBuffer));

	_check(_path_add_segment(source->target->projectDir, &pathBuffer));
	_check(_path_append_raw(source->fileName, &pathBuffer));
	_check(_cmdline_add_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));

	return 0;
}

static int _cmdline_add_link_options(const _linkoptions* defaults, const _linkoptions* overrides, _cmdlinebuffer* cmdLine) {
	_check(_cmdline_combine(cmdLine, &defaults->_flags));
	_check(_cmdline_combine(cmdLine, &overrides->_flags));

	return 0;
}

static int _build_linker_cmdline(Target target, str buildDir, _cmdlinebuffer* cmdLine) {
	_pathbuffer pathBuffer;
	str         linker = _linker_executable(target->type);
	int         i;

	if(linker.len == 0) {
		_log_err(_s("No matching linker executable could be identified for this compiler and platform"), -1, NONE);
		return 1;
	}

	cmdLine->len   = 0;
	pathBuffer.len = 0;

	_check(_cmdline_add_arg(linker, cmdLine));

#if COMPILER == COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/NOLOGO"), cmdLine));

	if(target->type != StaticLibrary)
		_check(_cmdline_add_arg(_s("/INCREMENTAL:NO"), cmdLine));
#endif

	if(target->type == SharedLibrary) {
#if COMPILER == COMPILER_MSVC
		_check(_cmdline_add_arg(_s("/DLL"), cmdLine));
#else
		_check(_cmdline_add_arg(_s("--shared"), cmdLine));
#endif
	} else if(target->type == StaticLibrary) {
#if COMPILER != COMPILER_MSVC
		_check(_cmdline_add_arg(_s("--static"), cmdLine));
#endif
	}

	_cmdline_add_link_options(&_g_linkOptions, &target->linkOpt, cmdLine);

	if(target->compileOpt.debugInformation == Enabled ||
	   (target->compileOpt.debugInformation == NotSet && _g_compileOptions.debugInformation == Enabled )) {
#if COMPILER == COMPILER_MSVC
		if(target->type != StaticLibrary) {
			_check(_cmdline_add_arg(_s("/DEBUG:FULL"), cmdLine));
			_check(_cmdline_add_arg(_s("/PDB:"), cmdLine));

			pathBuffer.len = 0;
			_check(_make_target_filepath(_file_basename(_cstr(target->name)), buildDir, DebugInformation, &pathBuffer));
			_check(_cmdline_append_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));
		}
#endif
	}

#if COMPILER == COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/OUT:"), cmdLine));
#else
	_check(_cmdline_add_arg(_s("-o"), cmdLine));
#endif

	pathBuffer.len = 0;
	_check(_make_target_filepath(_cstr(target->name), buildDir, target->type, &pathBuffer));
	_check(_cmdline_append_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));

	for(i = 0; i < _g_sourceCount; ++i) {
		_source* source = &_g_sources[i];

		if(source->target != target)
			continue;

		pathBuffer.len = 0;
		_check(_make_target_filepath(source->fileName, buildDir, ObjectFile, &pathBuffer));
		_check(_cmdline_add_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));
	}

	return 0;
}

/*
 * Compiler options
 */

#define compile_option(option_name, option_value) _g_compileOptions.option_name = option_value
#define target_compile_option(target_or_target_name, option_name, option_value) \
	do { \
		Target _target = _get_target(target_or_target_name, __LINE__, __FILE__); \
		if(_target == NONE) \
			return 1; \
		_target->compileOpt.option_name = option_value; \
	} while((void)0,0)

static int _add_flags_callback(str flag, void* userData, int line, const char* file) {
	_cmdlinebuffer* cmdLine = (_cmdlinebuffer*)userData;
	(void)line;
	(void)file;
	_check(_cmdline_add_arg(flag, cmdLine));

	return 0;
}

#define compile_flags(flag_list) \
	_check(_parse_list(flag_list, _add_flags_callback, &_g_compileOptions._flags, __LINE__, __FILE__))

#define target_compile_flags(target_or_target_name, flag_list) \
	do { \
		Target _target = _get_target(target_or_target_name, __LINE__, __FILE__); \
		if(_target == NONE) \
			return 1; \
		_check(_parse_list(flag_list, _add_flags_callback, &_target->compileOpt._flags, __LINE__, __FILE__)); \
	} while((void)0,0)

#define c_flags(flag_list) \
	_check(_parse_list(flag_list, _add_flags_callback, &_g_compileOptions._cFlags, __LINE__, __FILE__))

#define target_c_flags(target_or_target_name, flag_list) \
	do { \
		Target _target = _get_target(target_or_target_name, __LINE__, __FILE__); \
		if(_target == NONE) \
			return 1; \
		_check(_parse_list(flag_list, _add_flags_callback, &_target->compileOpt._cFlags, __LINE__, __FILE__)); \
	} while((void)0,0)

#define cpp_flags(flag_list) \
	_check(_parse_list(flag_list, _add_flags_callback, &_g_compileOptions._cppFlags, __LINE__, __FILE__))

#define target_cpp_flags(target_or_target_name, flag_list) \
	do { \
		Target _target = _get_target(target_or_target_name, __LINE__, __FILE__); \
		if(_target == NONE) \
			return 1; \
		_check(_parse_list(flag_list, _add_flags_callback, &_target->compileOpt._cppFlags, __LINE__, __FILE__)); \
	} while((void)0,0)

static int _add_include_paths_callback(str includePath, void* userData, int line, const char* file) {
	_cmdlinebuffer* cmdLine = (_cmdlinebuffer*)userData;
	(void)line;
	(void)file;

#if COMPILER == COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/I"), cmdLine));
#else
	_check(_cmdline_add_arg(_s("-I"), cmdLine));
#endif

	if(!_path_is_abs(includePath)) {
		_check(_cmdline_append_arg(_g_currentProjectDir, cmdLine));
		_check(_cmdline_append_arg(includePath, cmdLine));
	} else {
		_check(_cmdline_append_arg(includePath, cmdLine));
	}

	return 0;
}

#define include_paths(path_list) \
	_check(_parse_list(path_list, _add_include_paths_callback, &_g_compileOptions._flags, __LINE__, __FILE__))

#define target_include_paths(target_or_target_name, path_list) \
	do { \
		Target _target = _get_target(target_or_target_name, __LINE__, __FILE__); \
		if(_target == NONE) \
			return 1; \
		_check(_parse_list(path_list, _add_include_paths_callback, &_target->compileOpt._flags, __LINE__, __FILE__)); \
	} while((void)0,0)

static int _add_defines_callback(str defineStr, void* userData, int line, const char* file) {
	_cmdlinebuffer* cmdLine = (_cmdlinebuffer*)userData;
	(void)line;
	(void)file;

#if COMPILER == COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/D"), cmdLine));
#else
	_check(_cmdline_add_arg(_s("-D"), cmdLine));
#endif

	_check(_cmdline_append_arg(defineStr, cmdLine));

	return 0;
}

#define defines(define_list) \
	_check(_parse_list(define_list, _add_defines_callback, &_g_compileOptions._flags, __LINE__, __FILE__))

#define target_defines(target_or_target_name, define_list) \
	do { \
		Target _target = _get_target(target_or_target_name, __LINE__, __FILE__); \
		if(_target == NONE) \
			return 1; \
		_check(_parse_list(define_list, _add_defines_callback, &_target->compileOpt._flags, __LINE__, __FILE__)); \
	} while((void)0,0)

/*
 * Linker options
 */

#define link_flags(flag_list) \
	_check(_parse_list(flag_list, _add_flags_callback, &_g_linkOptions._flags, __LINE__, __FILE__))

#define target_link_flags(target_or_target_name, flag_list) \
	do { \
		Target _target = _get_target(target_or_target_name, __LINE__, __FILE__); \
		if(_target == NONE) \
			return 1; \
		_check(_parse_list(flag_list, _add_flags_callback, &_target->linkOpt._flags, __LINE__, __FILE__)); \
	} while((void)0,0)

static int _target_lib_path(str libPath, _cmdlinebuffer* cmdLine) {
	if(libPath.len >= 0) {
#if COMPILER == COMPILER_MSVC
		_check(_cmdline_add_arg(_s("/LIBPATH:"), cmdLine));
#else
		_check(_cmdline_add_arg(_s("-L"), cmdLine));
#endif
	}

	_check(_cmdline_append_arg(libPath, cmdLine));

	return 0;
}

static int _target_link_lib(str libName, _cmdlinebuffer* cmdLine) {
#if COMPILER == COMPILER_MSVC
	_check(_cmdline_add_arg(libName, cmdLine));

	if(_file_ext(libName).len == 0)
		_check(_cmdline_append_arg(_s(".lib"), cmdLine));
#else
	_check(_cmdline_add_arg(_s("-l"), cmdLine));
	_check(_cmdline_append_arg(libName, cmdLine));
#endif

	return 0;
}

static _bool _target_depends_on(Target target, Target dependency) {
	Target stack[MAX_TARGETS];
	_bool  visited[MAX_TARGETS] = {0};
	int    stackSize;
	int    i;

	stack[0]  = target;
	stackSize = 1;

	while(stackSize > 0) {
		Target currentTarget = stack[--stackSize];

		for(i = 0; i < currentTarget->_linkDependencyCount; ++i) {
			Target dependencyTarget = currentTarget->_linkDependencies[i];

			if(visited[dependencyTarget - _g_targets])
				continue;

			stack[stackSize++] = dependencyTarget;

			if(dependencyTarget == dependency)
				return true;
		}

		visited[currentTarget - _g_targets] = true;
	}

	return false;
}

static int _target_link(Target target, Target linkedTarget, int line, const char* file) {
	_pathbuffer targetBuildDirBuffer;
	_pathbuffer targetLibDirBuffer;
	int         i;

	if(linkedTarget == NONE)
		return 0;

	for(i = 0; i < target->_linkDependencyCount; ++i) {
		if(target->_linkDependencies[i] == linkedTarget)
			return 0;
	}

	if(target->_linkDependencyCount >= _array_count(target->_linkDependencies)) {
		_log_err(_s("Too many dependencies for target. Define MAX_TARGET_DEPENDENCIES to increase limit (" _stringify(MAX_TARGET_DEPENDENCIES) ")"), line, file);
		return 1;
	}

	if(target == linkedTarget || _target_depends_on(linkedTarget, target)) {
		_log_err(_s("Target dependency cycle"), line, file);
		return 1;
	}

	target->_linkDependencies[target->_linkDependencyCount++] = linkedTarget;
	linkedTarget->_isLinkTarget = true;

	targetBuildDirBuffer.len = 0;
	_check(_path_add_segment(_g_buildDir, &targetBuildDirBuffer));
	_check(_path_add_segment(_rel_build_path(linkedTarget->projectDir), &targetBuildDirBuffer));

	targetLibDirBuffer.len = 0;
	_check(_make_target_filepath(_cstr(linkedTarget->name), _str(targetBuildDirBuffer.data, targetBuildDirBuffer.len), linkedTarget->type, &targetLibDirBuffer));

	_check(_target_lib_path(_path_without_file(_str(targetLibDirBuffer.data, targetLibDirBuffer.len)), &target->linkOpt._flags));
	_check(_target_link_lib(_cstr(linkedTarget->name), &target->linkOpt._flags));

	return 0;
}

static int _add_libs_callback(str lib, void* userData, int line, const char* file) {
	Target target     = NONE;
	Target linkTarget = _find_target(lib);
	(void)line;
	(void)file;

	if(_is_target_ptr(userData))
		target = (Target)userData;

	if(linkTarget) {
		if(target)
			_check(_target_link(target, linkTarget, line, file));
	} else {
		_check(_target_link_lib(lib, target ? &target->linkOpt._flags : (_cmdlinebuffer*)userData));
	}

	return 0;
}

#define link_libraries(library_list) \
	_check(_parse_list(library_list, _add_libs_callback, &_g_linkOptions._flags, __LINE__, __FILE__))

#define target_link_libraries(target_or_target_name, library_list_or_target) \
	do { \
		Target target = _get_target(target_or_target_name, __LINE__, __FILE__); \
		if(target == NONE) \
			return 1; \
		if(_is_target_ptr(library_list_or_target)) \
			_check(_target_link(target, (Target)library_list_or_target, __LINE__, __FILE__)); \
		else \
			_check(_parse_list(library_list_or_target, _add_libs_callback, target, __LINE__, __FILE__)); \
	} while((void)0,0)

static int _add_lib_paths_callback(str libPath, void* userData, int line, const char* file) {
	_cmdlinebuffer* cmdLine = (_cmdlinebuffer*)userData;
	(void)line;
	(void)file;
	_target_lib_path(libPath, cmdLine);

	return 0;
}

#define library_paths(path_list) \
	_check(_parse_list(path_list, _add_lib_paths_callback, &_g_linkOptions._flags, __LINE__, __FILE__))

#define target_library_paths(target_or_target_name, path_list) \
	do { \
		Target _target = _get_target(target_or_target_name, __LINE__, __FILE__); \
		if(_target == NONE) \
			return 1; \
		_check(_parse_list(path_list, _add_lib_paths_callback, &_target->linkOpt._flags, __LINE__, __FILE__)); \
	} while((void)0,0)

/*
 * Process creation
 */

static int _wait_jobs(void) {
	int exitCode = 0;
	int i;

	for(i = 0; i < _g_win32ProcessCount; ++i) {
		void*  hProcess = _g_win32Processes[i];
		_ulong wait     = WaitForSingleObject(hProcess, INFINITE);

		if(wait == WAIT_FAILED) {
			exitCode = (int)GetLastError();
		} else {
			_ulong procExit = 0;
			GetExitCodeProcess(hProcess, &procExit);

			if(procExit != 0)
				exitCode = procExit;
		}

		CloseHandle(hProcess);
	}

	mem_fill(_g_win32Processes, 0, sizeof(_g_win32Processes));
	_g_win32ProcessCount = 0;

	return exitCode;
}

static int _add_job(_cmdlinebuffer cmdLine, str workingDir) {
#if OS == OS_WINDOWS
	_wchar              workingDirBuffer[MAX_PATH];
	_wchar              cmdLineBuffer[MAX_COMMAND_LINE + 1];
	const int           workingDirLen = MultiByteToWideChar(CP_UTF8, 0, workingDir.data, workingDir.len, workingDirBuffer, MAX_PATH);
	const int           cmdLineLen    = MultiByteToWideChar(CP_UTF8, 0, cmdLine.buffer, cmdLine.len, cmdLineBuffer, MAX_COMMAND_LINE + 1);
	STARTUPINFOW        startupInfo   = {0};
	PROCESS_INFORMATION processInfo   = {0};

	workingDirBuffer[workingDirLen] = '\0';
	cmdLineBuffer[cmdLineLen]       = '\0';

	startupInfo.cb         = sizeof(startupInfo);
	startupInfo.dwFlags    = 0x00000100; /* STARTF_USESTDHANDLES */
	/* Redirect all output to stdout */
	startupInfo.hStdError  = _g_win32StdoutHandle;
	startupInfo.hStdOutput = _g_win32StdoutHandle;

	/* Max job slots used, wait for one to finish */
	if(_g_win32ProcessCount >= _g_maxJobs) {
		_ulong exitCode = 0;
		_ulong wait     = WaitForMultipleObjects((_ulong)_g_win32ProcessCount, _g_win32Processes, 0, INFINITE);

		if(wait == WAIT_FAILED)
			return (int)GetLastError();

		GetExitCodeProcess(_g_win32Processes[wait], &exitCode);
		CloseHandle(_g_win32Processes[wait]);
		_g_win32Processes[wait] = _g_win32Processes[_g_win32ProcessCount - 1];
		_g_win32Processes[_g_win32ProcessCount - 1] = NONE;
		--_g_win32ProcessCount;

		if(exitCode != 0)
			return (int)exitCode;
	}

	if(!CreateProcessW(NONE, cmdLineBuffer, NONE, NONE, 1, 0, NONE, workingDirLen > 0 ? workingDirBuffer : _g_win32Cwd, &startupInfo, &processInfo)) {
		_log_err(_s("Failed to start process '"), -1, NONE);
		_log_raw(_str(cmdLine.buffer, cmdLine.len));
		_log_raw(_s("'"));
		return (int)GetLastError();
	}

	CloseHandle(processInfo.hThread);
	_g_win32Processes[_g_win32ProcessCount++] = processInfo.hProcess;
#endif

	return 0;
}

static int _add_compile_job(_source* source, str buildDir) {
	_cmdlinebuffer cmdLine;
	_check(_build_compiler_cmdline(source, buildDir, &cmdLine));

	if(_g_verbose)
		_log_cmdline(&cmdLine);

	/*
	 * msvc always prints the file name.
	 * This can't be turned off so we just don't print the file and target name.
	 * In the future process output might be filtered and not just written to stdout.
	 */
#if COMPILER != COMPILER_MSVC
	_log_msg(_s("["), -1, NONE);
	_log_raw(_cstr(source->target->name));
	_log_raw(_s("] "));
	_log_raw(source->target->projectDir);
	_log_raw(source->fileName);
#endif

	return _add_job(cmdLine, source->target->projectDir);
}

static int _add_link_job(Target target, str outPath) {
	_cmdlinebuffer cmdLine;

	_check(_build_linker_cmdline(target, outPath, &cmdLine));

	if(_g_verbose)
		_log_cmdline(&cmdLine);

	if(target->type == StaticLibrary)
		_log_msg(_s("Creating "), -1, NONE);
	else
		_log_msg(_s("Linking "), -1, NONE);

	_log_raw(_cstr(target->name));
	_log_raw(_target_file_ext(target->type));

	return _add_job(cmdLine, target->projectDir);
}

/*
 * Build
 */

static void _sort_targets_by_link_order(Target* targets, int count) {
	int j;

	for(j = 1; j < count; ++j) {
		_bool swapped = false;
		int   i;

		for(i = 0; i < count - j; ++i) {
			if(_target_depends_on(targets[i], targets[i + 1])) {
				Target tmp = targets[i];
				targets[i]     = targets[i + 1];
				targets[i + 1] = tmp;
				swapped        = true;
			}
		}

		if(!swapped)
			break;
	}
}

static int _build(void) {
	_pathbuffer pathBuffer = {0};
	str         buildDirBase;
	int         exitCode = 0;
	int         i;

	if(_g_targetsToBuild[0] != NONE) {
		const char** nameIt;

		for(i = 0; i < _g_targetCount; ++i)
			_g_targets[i]._skipBuild = true;

		for(nameIt = _g_targetsToBuild; *nameIt != NONE; ++nameIt) {
			Target target = find_target(*nameIt);

			if(!target) {
				_log_err(_s("No target named '"), -1, NONE);
				_log_raw(_cstr(*nameIt));
				_log_raw(_s("'"));
				return 0;
			}

			for(i = 0; i < _g_targetCount; ++i) {
				Target dependency = &_g_targets[i];

				if(dependency != target && dependency->_skipBuild && _target_depends_on(target, dependency))
					_g_targets[i]._skipBuild = false;
			}

			target->_skipBuild = false;
		}
	}

	_check(_path_add_segment(_g_buildDir, &pathBuffer));

	buildDirBase = _str(pathBuffer.data, pathBuffer.len);

	for(i = 0; i < _g_sourceCount; ++i) {
		_source* source = &_g_sources[i];
		str   projectBuildDir;

		if(source->compiled || source->target->_skipBuild)
			continue;

		--source->target->_sourceCount;

		_check(_path_add_segment(_rel_build_path(source->target->projectDir), &pathBuffer));
		projectBuildDir = _str(pathBuffer.data, pathBuffer.len);
		_check(_path_append_raw(_rel_build_path(source->fileName), &pathBuffer));
		_check(_create_directory(_str(pathBuffer.data, pathBuffer.len), -1, NONE));

		exitCode       = _add_compile_job(source, projectBuildDir);
		pathBuffer.len = buildDirBase.len;

		if(exitCode != 0) {
			_wait_jobs();
			break;
		}
	}

	if(exitCode == 0)
		exitCode = _wait_jobs();

	if(exitCode != 0)
		return exitCode;

	/* TODO: Move compilation and link steps into separate functions */
	{
		Target    targets[MAX_TARGETS];
		const int targetCount = _g_targetCount;

		for(i = 0; i < targetCount; ++i)
			targets[i] = &_g_targets[i];

		_sort_targets_by_link_order(targets, targetCount);

		for(i = 0; i < targetCount; ++i) {
			Target target = targets[i];

			if(target->_skipBuild)
				continue;

			pathBuffer.len = buildDirBase.len;
			_check(_path_add_segment(_rel_build_path(target->projectDir), &pathBuffer));

			exitCode       = _add_link_job(target, _str(pathBuffer.data, pathBuffer.len));
			pathBuffer.len = buildDirBase.len;

			if(exitCode != 0) {
				_wait_jobs();
				break;
			}

			if(target->_isLinkTarget)
				_check(_wait_jobs()); /* Can't link targets which are linked against by others in parallel */
		}

		if(exitCode == 0)
			exitCode = _wait_jobs();
	}

	return exitCode;
}

/*
 * Project
 */

#define begin_project(project_identifier) \
	int _main(void) { \
		_g_currentProjectDir = _path_without_file(_s(__FILE__)); \
		do { \

#define end_project \
		} while((void)0,0); \
		return _build(); \
	}

#define add_project(project_identifier) \
	do { \
		int project_identifier ## _main(void); \
		project_identifier ## _main(); \
	} while((void)0,0)

/*
 * Init/deinit
 */

static int _init(void) {
#if OS == OS_WINDOWS
	typedef _wchar**(_winapi*CmdLineToArgv)(const _wchar*, int*);
	void*         shell32            = LoadLibraryW(L"Shell32.dll");
	CmdLineToArgv CommandLineToArgvW = (CmdLineToArgv)GetProcAddress(shell32, "CommandLineToArgvW");
	_wchar**      argvW              = CommandLineToArgvW(GetCommandLineW(), &_g_win32Argc);
	int           bufferOffset       = 0;
	SYSTEM_INFO   systemInfo         = {0};
	int i;

	GetSystemInfo(&systemInfo);

	_g_maxJobs           = (int)systemInfo.dwNumberOfProcessors;
	_g_win32StdoutHandle = GetStdHandle(0xfffffff5);

	for(i = 0; i < _g_win32Argc; ++i)
	{
		_g_win32Argv[i] = _g_win32ArgvBuffer + bufferOffset;
		bufferOffset += WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, _g_win32Argv[i], sizeof(_g_win32ArgvBuffer) - bufferOffset, NONE, NONE);
		_g_win32ArgvBuffer[bufferOffset] = '\0';
	}

	_g_win32CwdLen = GetCurrentDirectoryW(MAX_PATH, _g_win32Cwd);

	if(_g_win32CwdLen == 0 || _g_win32CwdLen >= MAX_PATH)
		return (int)GetLastError();

	if(!is_pathsep(_g_win32Cwd[_g_win32CwdLen - 1]))
		_g_win32Cwd[_g_win32CwdLen++] = '\\';

	_g_cwd.data = _g_cwdBuffer;
	_g_cwd.len  = WideCharToMultiByte(CP_UTF8, 0, _g_win32Cwd, _g_win32CwdLen, _g_cwdBuffer, MAX_PATH, NONE, NONE);
#endif

	return 0;
}

static _noreturn void _deinit_and_exit(int exitCode) {
	if(!_g_logAtNewline)
		_log_raw(_s("\n"));

	log_flush();
#if OS == OS_WINDOWS
	ExitProcess((_uint)exitCode);
#endif
}

/*
 * Argument parsing and entry point
 */

static int _parse_args(int argc, char** argv) {
	int targetNameIdx = 0;
	int i;

	for(i = 1; i < argc; ++i) {
		const char* arg    = argv[i];
		const int   argLen = str_len(arg);

		if(argLen <= 0)
			continue;

		if(arg[0] == '-') {
			switch(arg[1]) {
			case 'j':
				{
					const char* numJobs = NONE;

					if(argLen > 2) {
						numJobs = arg + 2;
					} else if(i + 1 < argc) {
						numJobs = argv[i + 1];
						++i;
					}

					if(numJobs == NONE || !ascii_isdigit(*numJobs)) {
						_log_err(_s("-j must be followed by the number of jobs"), -1, NONE);
						return 1;
					}

					_g_maxJobs = _str_to_int(_cstr(numJobs));

					if(_g_maxJobs <= 0)
						_g_maxJobs = 1;
					else if(_g_maxJobs > MAX_JOBS)
						_g_maxJobs = MAX_JOBS;

					break;
				}
			case 'v':
				_g_verbose = true;
				break;
			case 'b':
				{
					const char* buildDir = NONE;

					if(argLen > 2) {
						buildDir = arg + 2;
					} else {
						buildDir = argv[i + 1];
						++i;
					}

					if(buildDir == NONE) {
						_log_err(_s("-b must be followed by the build directory path"), -1, NONE);
						return 1;
					}

					_g_buildDir = _cstr(buildDir);
					break;
				}
			case '-':
				{
					const str argStr = _cstr(arg);

					if(_str_ieq(argStr, _s("--debug"))) {
						buildConfiguration = Debug;
						break;
					} else if(_str_ieq(argStr, _s("--release"))) {
						buildConfiguration = Release;
						break;
					} else if(_str_ieq(argStr, _s("--profiling"))) {
						buildConfiguration = Profiling;
						break;
					}
				}
				/* fallthrough */
			default:
				_log_err(_s("Unknown argument '"), -1, NONE);
				_log_raw(_s(arg));
				_log_raw(_s("'"));
				return 1;
			}
		} else {
			if(targetNameIdx >= MAX_TARGETS) {
				_log_err(_s("Specified more than MAX_TARGETS to build"), -1, NONE);
				return 1;
			}

			_g_targetsToBuild[targetNameIdx++] = arg;
		}
	}

	return 0;
}

int _main(void);

#ifdef NO_CRT
int mainCRTStartup(void) {
#define argc _g_win32Argc
#define argv _g_win32Argv
#else
int main(int argc, char** argv) {
#endif
	int exitCode;

	exitCode = _init();

	if(exitCode == 0) {
		_g_buildDir = _s(".build/");
		exitCode    = _parse_args(argc, argv);
#undef argc
#undef argv

		if(exitCode == 0) {
			_log_msg(_s("Compiler: "), -1, NONE);
			_log_raw(_compiler_executable(CSrc));

			_log_msg(_s("Config: "), -1, NONE);

			switch(buildConfiguration) {
			case Debug:
				_log_raw(_s("debug"));
				_g_compileOptions.debugInformation = Enabled;
				break;
			case Release:
				_log_raw(_s("release"));
				break;
			case Profiling:
				_log_raw(_s("profiling"));
				_g_compileOptions.debugInformation = Enabled;
				break;
			default:
				_log_raw(_s("unknown"));
			}

			exitCode = _main();
		}
	}

	_deinit_and_exit(exitCode);
}

/*
 * crt
 */

#ifdef NO_CRT
void* __cdecl memset(void* dest, int fill, unsigned long long size);
void* __cdecl memcpy(void* dest, const void* src, unsigned long long size);
void* __cdecl memmove(void* dest, const void* src, unsigned long long size);

int _fltused;

#pragma intrinsic(memset,memcpy,memmove)

#pragma function(memset,memcpy,memmove)

void* __cdecl memset(void* dest, int fill, unsigned long long size) {
	return mem_fill(dest, fill, (int)size);
}

void* __cdecl memcpy(void* dest, const void* src, unsigned long long size) {
	return mem_copy(dest, src, (int)size);
}

void* __cdecl memmove(void* dest, const void* src, unsigned long long size) {
	return mem_move(dest, src, (int)size);
}
#endif

#if COMPILER == COMPILER_MSVC
	#pragma warning(pop)
#elif COMPILER == COMPILER_GCC || COMPILER == COMPILER_CLANG
	#pragma GCC diagnostic pop
#endif

#else
/*
 * If build.h was already included, change the begin_project and end_project macros.
 * Subproject files must be included after the main begin_project/end_project or compiled as separate compilation units.
 */

#undef begin_project
#define begin_project(project_identifier) \
	int project_identifier ## _main(void) { \
		_g_currentProjectDir = _path_without_file(_s(__FILE__)); \
		do {

#undef end_project
#define end_project \
		} while((void)0,0); \
		return 0; \
	}

#endif /* BUILD_H_INCLUDED */
