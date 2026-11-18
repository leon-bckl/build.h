#ifndef BUILD_H_INCLUDED
#define BUILD_H_INCLUDED

/*
 * Platform detection
 */

#ifdef __APPLE__
	#define OS_MACOS
#elif defined(_WIN32)
	#define OS_WINDOWS
#elif defined(__linux__)
	#define OS_LINUX
#else
	#error "Unsupported os"
#endif

/*
 * Compiler detection
 */

#ifdef __clang__
	#define COMPILER_CLANG
#elif defined(_MSC_VER)
	#define COMPILER_MSVC
#elif defined(__GNUC__)
	#define COMPILER_GCC
#else
	#error "Unsupported compiler"
#endif

/*
 * Architecture detection
 */

#if defined(_M_IX86) || defined(__i386__)
	#define ARCH_X86
	#define POINTER_SIZE 4
	#define MAX_ALIGN    8
#elif defined(_M_X64) || defined(__amd64__)
	#define ARCH_X86_64
	#define POINTER_SIZE 8
	#define MAX_ALIGN    16
#elif defined(__aarch64__)
	#define ARCH_ARM64
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

#ifdef __cplusplus
	#if __cplusplus >= 201103L
		#define NONE nullptr
	#else
		#define NONE 0
	#endif
#else
	#define NONE (void*)0
#endif

#define _check(expr) \
	do { \
		const int _exitCode = expr; \
		if(_exitCode != 0) \
			return _exitCode; \
	} while((void)0,0)

#ifdef COMPILER_MSVC
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
	#undef  true
	#define true  1
	#undef  false
	#define false 0
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

typedef struct {
#ifdef OS_WINDOWS
	int   argc;
	char* argv[MAX_ARGS + 1];
#endif
	int   len;
	char  buffer[MAX_COMMAND_LINE];
} _cmdlinebuffer;

typedef struct {
	_ulong lo;
	_ulong hi;
} _filetime;

typedef enum {
	CDefault,
	C89,
	C99,
	C11,
	C23
} _clanguageversion;

typedef enum {
	CppDefault,
	Cpp03,
	Cpp11,
	Cpp14,
	Cpp17,
	Cpp20,
	Cpp23,
	Cpp26
} _cpplanguageversion;

typedef enum {
	OptimizeDefault,
	OptimizeNone,
	OptimizeSpeed,
	OptimizeSize
} _optimizationlevel;

typedef enum {
	NotSet,
	Disabled,
	Enabled
} _tristate;

typedef struct {
	_clanguageversion   cVersion;
	_cpplanguageversion cppVersion;
	_optimizationlevel  optimizations;
	_tristate           debugInformation;
	_cmdlinebuffer      _flags;
	_cmdlinebuffer      _cFlags;
	_cmdlinebuffer      _cppFlags;
} _compileoptions;

typedef struct {
	_cmdlinebuffer _flags;    /* Linker flags for executables and shared libraries */
	_cmdlinebuffer _libFlags; /* Librarian flags for static libraries */
} _linkoptions;

typedef enum {
	ObjectFile,
	DebugInformation,
	Executable,
	StaticLibrary,
	SharedLibrary
} _artifact_type;

typedef enum {
	NotLinked,
	Linking,
	Linked
} _linkstate;

typedef struct _target* Target;

struct _target {
	const char*     name;
	str             projectDir;
	str             installDir;
	str             installDirBase; /* Directory of the project file that set installDir */
	_artifact_type  type;
	_compileoptions compileOpt;
	_linkoptions    linkOpt;
	_filetime       _lastModified;
	_bool           _skipBuild;
	_bool           _needsLink;
	_linkstate      _linkState;
	int             _linkDependencyCount;
	Target          _linkDependencies[MAX_TARGET_DEPENDENCIES];
};

typedef enum {
	CSrc,
	CppSrc,
	RcSrc
} _sourcekind;

typedef struct {
	Target      target;
	str         dir; /* Directory that fileName is relative to, from target_sources */
	str         fileName;
	_sourcekind kind;
	int         compiled;
	_filetime   lastModified;
} _source;

typedef struct {
	void*          stdOut; /* Redirected output of a running job, NONE if it writes to the console */
	const _source* source; /* Source of a compile job, NONE for the other jobs */
	Target         target; /* Target of a link job, NONE for the other jobs */
} _job;

typedef struct {
	int  len;
	char data[MAX_PATH];
} _pathbuffer;

/*
 * Globals
 */

_buildconfig           _g_config = Debug;

static const str       _emptyString = {NONE, 0};

static str             _g_buildDir;
static str             _g_installDir;
static str             _g_installDirBase;   /* Directory of the project file that set _g_installDir */
static str             _g_projectFileDir;   /* Directory of the current project file (begin_project) */
static str             _g_currentProjectDir;
static _bool           _g_verbose     = false;
static _bool           _g_incremental = false;
static _bool           _g_install     = false;
static int             _g_maxJobs     = 1;

static char            _g_cwdBuffer[MAX_PATH];
static str             _g_cwd;

static struct _target  _g_targets[MAX_TARGETS];
static const char*     _g_targetsToBuild[MAX_TARGETS + 1];
static int             _g_targetCount = 0;

static _compileoptions _g_compileOptions;
static _linkoptions    _g_linkOptions;

static _source         _g_sources[MAX_SOURCES];
static int             _g_sourceCount = 0;

static char            _g_logBuffer[512];
static int             _g_logBufferLen = 0;
static _bool           _g_logAtNewline = true;

#ifdef OS_WINDOWS
static void*  _g_win32StdoutHandle = NONE;
static int    _g_win32Argc         = 0;
static char*  _g_win32Argv[128];
static char   _g_win32ArgvBuffer[MAX_COMMAND_LINE];
static int    _g_win32CwdLen = 0;
static _wchar _g_win32Cwd[MAX_PATH];
static int    _g_win32ProcessCount = 0;
static void*  _g_win32Processes[MAX_JOBS + 1]; /* +1 so the last entry is always NONE */
static _job   _g_win32Jobs[MAX_JOBS + 1];
#endif

/*
 * Ignore warnings
 */

#ifdef COMPILER_MSVC
	#pragma warning(disable: 4210) /* nonstandard extension used : function given file scope */
	#pragma warning(push)
#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wcast-function-type" /* GetProcAddress return value */
	#pragma GCC diagnostic ignored "-Wunused-function"
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
#ifdef OS_WINDOWS
		|| c == '\\'
#endif
	;
}


/*
 * win32
 */

#ifdef OS_WINDOWS

#if MAX_PATH > 260
	#error "MAX_PATH must not be larger than 260 on windows"
#endif

#if MAX_JOBS > 64
	#error "MAX_JOBS must not be larger than 64 on windows"
#endif

#define _import __declspec(dllimport)
#define _winapi __stdcall

#if defined(NO_CRT) && defined(COMPILER_MSVC)
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

typedef struct {
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

typedef struct {
	_ulong dwLowDateTime;
	_ulong dwHighDateTime;
} FILETIME;

typedef struct {
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

typedef enum {
	GetFileExInfoStandard,
	GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;

typedef struct {
	_ulong nLength;
	void*  lpSecurityDescriptor;
	int    bInheritHandle;
} SECURITY_ATTRIBUTES;

typedef struct OVERLAPPED OVERLAPPED;

_import _ulong _winapi GetLastError(void);

_import void _winapi GetSystemInfo(SYSTEM_INFO* lpSystemInfo);

_import _bool _winapi CloseHandle(void* hObject);

_import _bool _winapi CreateProcessW(const _wchar* lpApplicationName, _wchar* lpCommandLine, void* lpProcessAttributes, void* lpThreadAttributes, int bInheritHandles, _ulong dwCreationFlags, void* lpEnvironment, const _wchar* lpCurrentDirectory, STARTUPINFOW* lpStartupInfo, PROCESS_INFORMATION* lpProcessInformation);
_import _noreturn void _winapi ExitProcess (_uint uExitCode);
_import _bool _winapi GetExitCodeProcess(void* hProcess, _ulong* lpExitCode);

_import void*  _winapi CreateFileW(const _wchar* lpFileName, _ulong dwDesiredAccess, _ulong dwShareMode, SECURITY_ATTRIBUTES* lpSecurityAttributes, _ulong dwCreationDisposition, _ulong dwFlagsAndAttributes, void* hTemplateFile);
_import _ulong _winapi SetFilePointer(void* hFile, long lDistanceToMove, long* lpDistanceToMoveHigh, _ulong dwMoveMethod);
_import _ulong _winapi GetTempPathW(_ulong nBufferLength, _wchar* lpBuffer);
_import _uint  _winapi GetTempFileNameW(const _wchar* lpPathName, const _wchar* lpPrefixString, _uint uUnique, _wchar* lpTempFileName);

_import void* _winapi LoadLibraryA(const char* lpLibFileName);
typedef void(_winapi*_farproc)(void);
_import _farproc _winapi GetProcAddress(void* hModule, const char* lpProcName);

_import void*   _winapi GetStdHandle(_ulong nStdHandle);
_import _wchar* _winapi GetCommandLineW(void);

_import _ulong _winapi GetCurrentDirectoryW(_ulong nBufferLength, _wchar* lpBuffer);

_import _ulong _winapi GetFileAttributesW(const _wchar* lpFileName);
_import _bool _winapi GetFileAttributesExW(const _wchar* lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, void* lpFileInformation);

_import _bool _winapi ReadFile(void* hFile, void* lpBuffer, _ulong nNumberOfBytesToRead, _ulong* lpNumberOfBytesRead, OVERLAPPED* lpOverlapped);
_import _bool _winapi WriteFile(void* hFile, const void* lpBuffer, _ulong nNumberOfBytesToWrite, _ulong* lpNumberOfBytesWritten, void* lpOverlapped);

_import _ulong _winapi GetModuleFileNameW(void* hModule, _wchar* lpFilename, _ulong nSize);

_import _bool _winapi CreateDirectoryW(const _wchar* lpPathName, SECURITY_ATTRIBUTES* lpSecurityAttributes);

_import _bool _winapi CopyFileW(const _wchar* lpExistingFileName, const _wchar* lpNewFileName, _bool bFailIfExists);
_import _bool _winapi MoveFileExW(const _wchar* lpExistingFileName, const _wchar* lpNewFileName, _ulong dwFlags);
_import _bool _winapi DeleteFileW(const _wchar* lpFileName);

#define INFINITE    0xffffffff
#define WAIT_FAILED 0xffffffff
#define INVALID_HANDLE_VALUE ((void*)-1)

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

static void _win32_get_lastmodified(const _wchar* filePath, _filetime* fileTime) {
	WIN32_FILE_ATTRIBUTE_DATA attributes;

	mem_fill(&attributes, 0, sizeof(attributes));

	fileTime->hi = 0;
	fileTime->lo = 0;

	if(GetFileAttributesExW(filePath, GetFileExInfoStandard, &attributes)) {
		fileTime->hi = attributes.ftLastWriteTime.dwHighDateTime;
		fileTime->lo = attributes.ftLastWriteTime.dwLowDateTime;
	}
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
	return _str_ieq(_cstr(s1), _cstr(s2));
}

static _bool _str_contains(str s, str sub) {
	int i;

	for(i = 0; i + sub.len <= s.len; ++i) {
		if(_str_eq(_str(s.data + i, sub.len), sub))
			return 1;
	}

	return 0;
}

static str _str_left(str s, int count) {
	if(count < s.len)
		s.len = count;

	return s;
}

static str _str_right(str s, int count) {
	if(count < s.len) {
		s.data += s.len - count;
		s.len   = count;
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
	int i;

	if(buffer->len + s.len > MAX_PATH) {
		_log_err(_s("Path too long, define MAX_PATH to increase limit (" _stringify(MAX_PATH) ")"), -1, NONE);
		return 1;
	}

	for(i = 0; i < s.len; ++i)
		buffer->data[buffer->len + i] = is_pathsep(s.data[i]) ? '/' : s.data[i];

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
#ifdef OS_WINDOWS
	return path.len >= 2 && ascii_isalpha(path.data[0]) && path.data[1] == ':';
#else
	return path.len > 0 && path.data[0] == '/';
#endif
}

static _bool path_is_abs(const char* path) {
	return _path_is_abs(_cstr(path));
}

/*
 * Adds a project directory to a path. A relative project directory is relative to the working directory
 */
static int _path_add_project_dir(str projectDir, _pathbuffer* buffer) {
	if(!_path_is_abs(projectDir))
		_check(_path_add_segment(_g_cwd, buffer));

	return _path_add_segment(projectDir, buffer);
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
#ifdef OS_WINDOWS
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
#ifdef OS_WINDOWS
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

		if(!curr)
			break; /* Path without trailing separator: don't step over the terminator */
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

static int _copy_file(str src, str dst, int line, const char* file) {
#ifdef OS_WINDOWS
	_wchar srcBuffer[MAX_PATH];
	_wchar dstBuffer[MAX_PATH];

	_check(_create_directory(_path_without_file(dst), -1, NONE));
	_check(_win32_assemble_path(src, srcBuffer));
	_check(_win32_assemble_path(dst, dstBuffer));

	if(!CopyFileW(srcBuffer, dstBuffer, false))
		goto error;
#endif

	return 0;

error:
	_log_err(_s("Failed to copy file '"), line, file);
	_log_raw(src);
	_log_raw(_s("' to '"));
	_log_raw(dst);
	_log_raw(_s("'"));

	return 1;
}

#define copy_file(src, dst) _check(_copy_file(_cstr(src), _cstr(dst), __LINE__, __FILE__))

/*
 * Deletes a file if it exists
 */
static int _delete_file(str path) {
#ifdef OS_WINDOWS
	_wchar pathBuffer[MAX_PATH];

	_check(_win32_assemble_path(path, pathBuffer));
	DeleteFileW(pathBuffer); /* Fails if there is no such file, which is fine */
#endif

	return 0;
}

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

static void _get_lastmodified(str filePath, _filetime* fileTime) {
#ifdef OS_WINDOWS
	_wchar wPath[MAX_PATH];
	_win32_assemble_path(filePath, wPath);
	_win32_get_lastmodified(wPath, fileTime);
#endif
}

/*
 * Path of a source file relative to its target's project directory (or absolute)
 */
static int _assemble_source_rel_path(const _source* source, _pathbuffer* pathBuffer) {
	if(!_path_is_abs(source->fileName))
		_check(_path_add_segment(source->dir, pathBuffer));

	return _path_append_raw(source->fileName, pathBuffer);
}

static int _assemble_source_path(const _source* source, _pathbuffer* pathBuffer) {
	if(!_path_is_abs(source->fileName) && !_path_is_abs(source->dir))
		_check(_path_add_project_dir(source->target->projectDir, pathBuffer));

	return _assemble_source_rel_path(source, pathBuffer);
}

typedef struct {
	Target target;
	str    dir;
} _targetsourcesdata;

static int _target_source_callback(str fileName, void* userData, int line, const char* file) {
	_pathbuffer               pathBuffer;
	const _targetsourcesdata* data    = (const _targetsourcesdata*)userData;
	const Target              target  = data->target;
	const str                 fileExt = _file_ext(fileName);
	_source*                  source;

	if(_g_sourceCount >= MAX_SOURCES) {
		_log_err(_s("Too many sources, define MAX_SOURCES to increase limit (" _stringify(MAX_SOURCES) ")"), line, file);
		return 1;
	}

	source = &_g_sources[_g_sourceCount];

	if(_str_ieq(fileExt, _s(".c")) ||
	   _str_ieq(fileExt, _s(".m"))) {
		source->kind = CSrc;
	} else if(_str_ieq(fileExt, _s(".cpp")) ||
	          _str_ieq(fileExt, _s(".cxx")) ||
	          _str_ieq(fileExt, _s(".cc")) ||
	          _str_ieq(fileExt, _s(".mm"))) {
		source->kind = CppSrc;
	} else if(_str_ieq(fileExt, _s(".rc"))) {
#ifdef OS_WINDOWS
		source->kind = RcSrc;
#else
		return 0;
#endif
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

	source->target   = target;
	source->dir      = data->dir;
	source->fileName = fileName;

	pathBuffer.len = 0;
	_check(_assemble_source_path(source, &pathBuffer));
	_get_lastmodified(_str(pathBuffer.data, pathBuffer.len), &source->lastModified);

	++_g_sourceCount;

	return 0;
}

static int _target_sources(const void* targetOrTargetName, const char* dir, const char* sources, int line, const char* file) {
	_targetsourcesdata data;

	data.target = _get_target(targetOrTargetName, line, file);

	if(data.target == NONE)
		return 1;

	data.dir = dir != NONE ? _cstr(dir) : _emptyString;

	return _parse_list(sources, _target_source_callback, &data, line, file);
}

/*
 * Adds sources in a directory (relative to the project directory, or NONE),
 */
#define target_sources(target_or_target_name, dir, source_list) \
	_target_sources(target_or_target_name, dir, source_list, __LINE__, __FILE__)

/*
 * Target output files
 *
 * Everything goes into a subdirectory of the build directory named after the build config so that artifacts of
 * different configs don't mix:
 *   <build dir>/<config>/<target name>.<ext> (binaries, pdb)
 *   <build dir>/<config>/obj/<target name>/<source path relative to project dir>.obj  (objects, compiler pdb)
 */

static str _config_dirname(_buildconfig buildConfig) {
	switch(buildConfig) {
	case Debug:
		return _s("dbg");
	case Release:
		return _s("rel");
	case Profiling:
		return _s("prf");
	default:
		return _s("unknown");
	}
}

/*
 * Appends a relative or absolute path so that it stays inside the build directory:
 * Absolute paths are made relative to the working directory (or lose their root if they are outside of it),
 * '.' segments are removed and '..' segments become '__'.
 */
static int _path_append_build_rel(str path, _pathbuffer* buffer) {
	if(_path_is_abs(path)) {
		if(_path_prefix(path, _g_cwd)) {
			path.data += _g_cwd.len;
			path.len  -= _g_cwd.len;
		} else {
			while(path.len > 0 && !is_pathsep(path.data[0])) {
				++path.data;
				--path.len;
			}
		}
	}

	while(path.len > 0) {
		str   segment;
		_bool hasPathSep;

		segment = _str(path.data, 0);

		while(segment.len < path.len && !is_pathsep(path.data[segment.len]))
			++segment.len;

		path.data += segment.len;
		path.len  -= segment.len;
		hasPathSep = path.len > 0;

		if(hasPathSep) {
			++path.data;
			--path.len;
		}

		if(segment.len == 0 || _str_eq(segment, _s(".")))
			continue;

		if(_str_eq(segment, _s("..")))
			segment = _s("__");

		_check(hasPathSep ? _path_add_segment(segment, buffer) : _path_append_raw(segment, buffer));
	}

	return 0;
}

static int _make_config_dirpath(_pathbuffer* buffer) {
	if(!_path_is_abs(_g_buildDir))
		_check(_path_add_segment(_g_cwd, buffer));

	_check(_path_add_segment(_g_buildDir, buffer));
	_check(_path_add_segment(_config_dirname(_g_config), buffer));

	return 0;
}

static str _artifact_file_ext(_artifact_type artifactType) {
	switch(artifactType) {
	case ObjectFile:
#ifdef COMPILER_MSVC
		return _s(".obj");
#else
		return _s(".o");
#endif
	case DebugInformation:
#ifdef OS_WINDOWS
		return _s(".pdb");
#else
		return _emptyString;
#endif
	case Executable:
#ifdef OS_WINDOWS
		return _s(".exe");
#else
		return _emptyString;
#endif
	case StaticLibrary:
#ifdef COMPILER_MSVC
		return _s(".lib");
#else
		return _s(".a");
#endif
	case SharedLibrary:
#ifdef OS_WINDOWS
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

/*
 * Output file of a target (binary or linker pdb)
 */
static int _make_target_filepath(str targetName, _artifact_type artifactType, _pathbuffer* buffer) {
	_check(_make_config_dirpath(buffer));
	_check(_path_append_build_rel(_path_without_file(targetName), buffer));
	_check(_path_append_raw(_file_basename(_file_without_path(targetName)), buffer));
	_check(_path_append_raw(_artifact_file_ext(artifactType), buffer));

	return 0;
}

/*
 * Output file of a source (object or compiler pdb).
 * Keeps the source's extension and adds the artifact's extension at the end, so foo.c and foo.cpp don't collide.
 */
static int _make_source_filepath(const _source* source, _artifact_type artifactType, _pathbuffer* buffer) {
	_pathbuffer relPathBuffer;

	relPathBuffer.len = 0;
	_check(_assemble_source_rel_path(source, &relPathBuffer));

	_check(_make_config_dirpath(buffer));
	_check(_path_add_segment(_s(".obj"), buffer));
	_check(_path_append_build_rel(_cstr(source->target->name), buffer));
	_check(_path_add_segment(_emptyString, buffer));
	_check(_path_append_build_rel(_str(relPathBuffer.data, relPathBuffer.len), buffer));
	_check(_path_append_raw(_artifact_file_ext(artifactType), buffer));

	return 0;
}

static _bool _filetime_is_newer(_filetime f1, _filetime f2) {
	if(f1.hi > f2.hi)
		return true;

	if(f1.hi == f2.hi)
		return f1.lo > f2.lo;

	return false;
}

static _bool _filetime_is_zero(_filetime f) {
	return f.hi == 0 && f.lo == 0;
}

/*
 * Buffered file reading and writing
 */

typedef struct {
	void* handle;
	int   pos;
	int   len;
	_bool eof;
	char  buffer[4096];
} _filereader;

typedef struct {
	void* handle;
	int   len;
	_bool failed;
	char  buffer[4096];
} _filewriter;

/*
 * Opens a file for reading or creates (truncates) it for writing. Returns NONE if that fails.
 */
static void* _open_file(str path, _bool forWriting) {
#ifdef OS_WINDOWS
	_wchar pathBuffer[MAX_PATH];
	void*  handle;

	if(path.len <= 0 || _win32_assemble_path(path, pathBuffer) != 0)
		return NONE;

	if(forWriting) /* GENERIC_WRITE, no sharing, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL */
		handle = CreateFileW(pathBuffer, 0x40000000, 0, NONE, 2, FILE_ATTRIBUTE_NORMAL, NONE);
	else /* GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL */
		handle = CreateFileW(pathBuffer, 0x80000000, 0x1, NONE, 3, FILE_ATTRIBUTE_NORMAL, NONE);

	return handle != INVALID_HANDLE_VALUE ? handle : NONE;
#else
	(void)path;
	(void)forWriting;
	return NONE;
#endif
}

static void _close_file(void* handle) {
#ifdef OS_WINDOWS
	if(handle != NONE)
		CloseHandle(handle);
#else
	(void)handle;
#endif
}

static void _filereader_init(_filereader* reader, void* handle) {
	reader->handle = handle;
	reader->pos    = 0;
	reader->len    = 0;
	reader->eof    = false;
}

/*
 * Next byte of the file or -1 at its end
 */
static int _filereader_getc(_filereader* reader) {
	if(reader->pos >= reader->len) {
#ifdef OS_WINDOWS
		_ulong bytesRead = 0;

		if(reader->eof || !ReadFile(reader->handle, reader->buffer, sizeof(reader->buffer), &bytesRead, NONE) || bytesRead == 0) {
			reader->eof = true;
			return -1;
		}

		reader->pos = 0;
		reader->len = (int)bytesRead;
#else
		reader->eof = true;
		return -1;
#endif
	}

	return (_byte)reader->buffer[reader->pos++];
}

/*
 * Puts the byte returned by the last _filereader_getc back
 */
static void _filereader_ungetc(_filereader* reader) {
	if(reader->pos > 0)
		--reader->pos;
}

/*
 * Reads a line into the buffer without its line break (LF or CRLF). Returns the number of bytes or -1 at the end of
 * the file. A line that doesn't fit into the buffer comes in several pieces, *lineEnd tells whether a piece ends the line.
 */
static int _filereader_line(_filereader* reader, char* line, int maxLen, _bool* lineEnd) {
	int len = 0;

	*lineEnd = false;

	while(len < maxLen) {
		const int c = _filereader_getc(reader);

		if(c < 0) {
			if(len == 0)
				return -1;

			*lineEnd = true;
			break;
		}

		if(c == '\n') {
			*lineEnd = true;
			break;
		}

		line[len++] = (char)c;
	}

	if(*lineEnd && len > 0 && line[len - 1] == '\r')
		--len;

	return len;
}

static void _filewriter_init(_filewriter* writer, void* handle) {
	writer->handle = handle;
	writer->len    = 0;
	writer->failed = handle == NONE;
}

static void _filewriter_flush(_filewriter* writer) {
#ifdef OS_WINDOWS
	_ulong bytesWritten = 0;

	if(writer->len > 0 && !writer->failed) {
		if(!WriteFile(writer->handle, writer->buffer, (_ulong)writer->len, &bytesWritten, NONE) || (int)bytesWritten != writer->len)
			writer->failed = true;
	}
#endif

	writer->len = 0;
}

static void _filewriter_write(_filewriter* writer, str s) {
	while(s.len > 0) {
		int copySize = (int)sizeof(writer->buffer) - writer->len;

		if(copySize > s.len)
			copySize = s.len;

		mem_copy(writer->buffer + writer->len, s.data, copySize);
		writer->len += copySize;
		s.data      += copySize;
		s.len       -= copySize;

		if(writer->len >= (int)sizeof(writer->buffer))
			_filewriter_flush(writer);
	}
}

/*
 * Dependency files
 */

#ifndef MSVC_INCLUDE_NOTE
	/* Start of cl's /showIncludes lines. A localized compiler prints a translation, define it to that text */
	#define MSVC_INCLUDE_NOTE "Note: including file:"
#endif

static int _make_depfile_path(const _source* source, _pathbuffer* buffer) {
	_check(_make_source_filepath(source, ObjectFile, buffer));

	return _path_append_raw(_s(".d"), buffer);
}

/*
 * Resource compilers don't report what they include, so .rc sources have no dependency file
 */
static _bool _source_has_depfile(const _source* source) {
	return source->kind != RcSrc;
}

static void _depfile_write_path(_filewriter* writer, str path) {
	int i;

	for(i = 0; i < path.len; ++i) {
		char c = path.data[i];

		if(is_pathsep(c))
			c = '/';
		else if(c == ' ' || c == '#')
			_filewriter_write(writer, _s("\\"));
		else if(c == '$')
			_filewriter_write(writer, _s("$"));

		_filewriter_write(writer, _str(&c, 1));
	}
}

static _bool _dependency_up_to_date(str dependency, str workingDir, _filetime artifactTime) {
	_pathbuffer pathBuffer;
	_filetime   lastModified;

	pathBuffer.len = 0;

	/* What is still relative after this is relative to the working directory of the build program, like any path given to _get_lastmodified */
	if(!_path_is_abs(dependency) && _path_add_segment(workingDir, &pathBuffer) != 0)
		return false;

	if(_path_append_raw(dependency, &pathBuffer) != 0)
		return false;

	_get_lastmodified(_str(pathBuffer.data, pathBuffer.len), &lastModified);

	if(_filetime_is_zero(lastModified) || _filetime_is_newer(lastModified, artifactTime)) {
		if(_g_verbose) {
			_log_msg(_s("Changed: "), -1, NONE);
			_log_raw(dependency);
		}

		return false;
	}

	return true;
}

static _bool _depfile_up_to_date(str depFilePath, str workingDir, _filetime artifactTime, _bool missingIsUpToDate) {
	_filereader reader;
	char        token[MAX_PATH];
	int         tokenLen = 0;
	_bool       result   = true;
	void*       handle   = _open_file(depFilePath, false);

	if(handle == NONE)
		return missingIsUpToDate;

	_filereader_init(&reader, handle);

	/* Skip the target. It ends with the first ':' that is followed by whitespace, which rules out a drive letter's ':' */
	for(;;) {
		int c = _filereader_getc(&reader);

		if(c < 0) {
			result = false; /* No dependencies at all: the compiler didn't finish writing the file */
			goto done;
		}

		if(c == ':') {
			c = _filereader_getc(&reader);

			if(c < 0 || ascii_isspace(c))
				break;
		}
	}

	for(;;) {
		int   c        = _filereader_getc(&reader);
		_bool isEscape = false;

		if(c == '\\') {
			const int next = _filereader_getc(&reader);

			if(next == '\n' || next == '\r') {
				c = ' '; /* Line continuation */
			} else if(next == ' ' || next == '#') {
				c        = next;
				isEscape = true;
			} else if(next >= 0) {
				_filereader_ungetc(&reader); /* A backslash in a path */
			}
		} else if(c == '$') {
			const int next = _filereader_getc(&reader);

			if(next != '$' && next >= 0)
				_filereader_ungetc(&reader);
		}

		if(c < 0 || (!isEscape && ascii_isspace(c))) {
			if(tokenLen > 0 && !_dependency_up_to_date(_str(token, tokenLen), workingDir, artifactTime)) {
				result = false;
				break;
			}

			tokenLen = 0;

			if(c < 0)
				break;

			continue;
		}

		if(tokenLen >= (int)sizeof(token)) {
			result = false; /* Path too long to check, recompile to be safe */
			break;
		}

		token[tokenLen++] = (char)c;
	}

done:
	_close_file(handle);

	return result;
}

static _bool _source_up_to_date(const _source* source, _filetime artifactTime, _bool missingIsUpToDate) {
	_pathbuffer pathBuffer;

	if(!_filetime_is_newer(artifactTime, source->lastModified))
		return false;

	if(!_source_has_depfile(source))
		return true;

	pathBuffer.len = 0;

	if(_make_depfile_path(source, &pathBuffer) != 0)
		return false;

	return _depfile_up_to_date(_str(pathBuffer.data, pathBuffer.len), source->target->projectDir, artifactTime, missingIsUpToDate);
}

static Target _add_target(const char* name, _artifact_type type, int line, const char* file) {
	_pathbuffer pathBuffer;
	Target      target;

	if(_g_targetCount >= MAX_TARGETS) {
		_log_err(_s("Too many targets, define MAX_TARGETS to increase limit (" _stringify(MAX_TARGETS) ")"), line, file); \
		log_flush();
		return NONE;
	}

	target = find_target(name);

	if(target != NONE) {
		_log_err(_s("Target name must be unique"), line, file);
		log_flush();
		return NONE;
	}

	target             = &_g_targets[_g_targetCount++];
	target->name       = name;
	target->projectDir = _g_currentProjectDir;
	target->type       = type;

	pathBuffer.len = 0;
	if(_make_target_filepath(_cstr(target->name), target->type, &pathBuffer) != 0)
		return NONE;

	_get_lastmodified(_str(pathBuffer.data, pathBuffer.len), &target->_lastModified);

	return target;
}

#define executable(executable_name)  _add_target(executable_name, Executable, __LINE__, __FILE__)
#define static_library(library_name) _add_target(library_name, StaticLibrary, __LINE__, __FILE__)
#define shared_library(library_name) _add_target(library_name, SharedLibrary, __LINE__, __FILE__)

/*
 * Executable names
 */

static str _compiler_executable(_sourcekind sourceKind) {
	switch(sourceKind) {
	case CSrc:
#ifdef COMPILER_MSVC
		return _s("cl");
#elif defined(COMPILER_CLANG)
		return _s("clang");
#elif defined(COMPILER_GCC)
		return _s("gcc");
#else
	#error "Unsupported C compiler"
#endif
	case CppSrc:
#ifdef COMPILER_MSVC
		return _s("cl");
#elif defined(COMPILER_CLANG)
		return _s("clang++");
#elif defined(COMPILER_GCC)
		return _s("g++");
#else
	#error "Unsupported C++ compiler"
#endif
#ifdef OS_WINDOWS
	case RcSrc:
#ifdef COMPILER_MSVC
		return _s("rc");
#elif defined(COMPILER_CLANG)
		return _s("llvm-rc");
#elif defined(COMPILER_GCC)
		return _s("windres");
#else
	#error "Unsupported resource compiler"
#endif
#endif
	default:
		return _emptyString;
	}
}

static str _linker_executable(void) {
#ifdef COMPILER_MSVC
	return _s("cl");
#elif defined(COMPILER_CLANG)
	return _s("clang");
#elif defined(COMPILER_GCC)
	return _s("gcc");
#else
	#error "Unsupported linker"
#endif
}

static str _librarian_executable(void) {
#ifdef COMPILER_MSVC
	return _s("lib");
#elif defined(OS_WINDOWS) && defined(COMPILER_CLANG)
	return _s("llvm-ar");
#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
	return _s("ar");
#else
	#error "Unsupported librarian"
#endif
}

/*
 * Command line
 */

static void _log_cmdline(const _cmdlinebuffer* cmdLine) {
#ifdef OS_WINDOWS
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
#ifdef OS_WINDOWS
	int needsEscape = s.len == 0 && !append; /* Escape empty arguments so they are not lost */
#endif
	int i;

	if(append && cmdLine->len > 0)
		--cmdLine->len;

	if(cmdLine->len + s.len >= MAX_COMMAND_LINE)
		goto too_long;

#ifdef OS_WINDOWS
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

#ifdef OS_WINDOWS
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
#ifdef OS_WINDOWS
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

static int _cmdline_add_compile_options(const _source* source, const _compileoptions* defaults, const _compileoptions* overrides, _cmdlinebuffer* cmdLine) {
	const _clanguageversion   cVersion      = overrides->cVersion != CDefault ? overrides->cVersion : defaults->cVersion;
	const _cpplanguageversion cppVersion    = overrides->cppVersion != CppDefault ? overrides->cppVersion : defaults->cppVersion;
	_optimizationlevel        optimizations = overrides->optimizations != OptimizeDefault ? overrides->optimizations : defaults->optimizations;
	const _tristate           debugInfo     = overrides->debugInformation != NotSet ? overrides->debugInformation : defaults->debugInformation;

	if(optimizations == OptimizeDefault)
		optimizations = _g_config == Debug ? OptimizeNone : OptimizeSpeed;

#ifdef COMPILER_MSVC
	#define _stdopt "/std:"
#else
	#define _stdopt "-std="
#endif

	if(source->kind == CSrc) {
		switch(cVersion) {
#ifndef COMPILER_MSVC
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
#ifdef COMPILER_MSVC
		_check(_cmdline_add_arg(_s("/Od"), cmdLine));
#else
		_check(_cmdline_add_arg(_s("-O0"), cmdLine));
#endif
	} else if(optimizations == OptimizeSpeed) {
#ifdef COMPILER_MSVC
		_check(_cmdline_add_arg(_s("/O2"), cmdLine));
#else
		_check(_cmdline_add_arg(_s("-O2"), cmdLine));
#endif
	} else if(optimizations == OptimizeSize) {
#ifdef COMPILER_MSVC
		_check(_cmdline_add_arg(_s("/O1"), cmdLine));
#else
		_check(_cmdline_add_arg(_s("-Os"), cmdLine));
#endif
	}

	if(_g_config != Debug) {
#ifdef COMPILER_MSVC
		_check(_cmdline_add_arg(_s("/DNDEBUG"), cmdLine));
#else
		_check(_cmdline_add_arg(_s("-DNDEBUG"), cmdLine));
#endif
	}

	if(debugInfo == Enabled) {
#ifdef COMPILER_MSVC
		if(source->target->type == StaticLibrary) {
			_check(_cmdline_add_arg(_s("/Z7"), cmdLine));
		} else {
			_pathbuffer pathBuffer;

			_check(_cmdline_add_arg(_s("/Zi"), cmdLine));
			_check(_cmdline_add_arg(_s("/Fd"), cmdLine));
			pathBuffer.len = 0;
			_check(_make_source_filepath(source, DebugInformation, &pathBuffer));
			_check(_cmdline_append_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));
		}
#else
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

static int _build_compiler_cmdline(const _source* source, str objFilePath, _cmdlinebuffer* cmdLine) {
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

#ifdef COMPILER_MSVC
	if(source->kind != RcSrc) /* Later versions of rc do support /nologo but better be safe and not use it */
		_check(_cmdline_add_arg(_s("/nologo"), cmdLine));
#endif

	if(source->kind != RcSrc) {
#ifdef COMPILER_MSVC
		_check(_cmdline_add_arg(_s("/c"), cmdLine));
#else
		_check(_cmdline_add_arg(_s("-c"), cmdLine));
#endif
	}

	if(source->kind != RcSrc) /* TODO: Separate include paths and defines and pass them to rc */
		_check(_cmdline_add_compile_options(source, &_g_compileOptions, &source->target->compileOpt, cmdLine));

	/* Have the compiler list the included files for the dependency file */
	if(_source_has_depfile(source)) {
#ifdef COMPILER_MSVC
		_check(_cmdline_add_arg(_s("/showIncludes"), cmdLine));
#else
		pathBuffer.len = 0;
		_check(_make_depfile_path(source, &pathBuffer));
		_check(_cmdline_add_arg(_s("-MD"), cmdLine));
		_check(_cmdline_add_arg(_s("-MF"), cmdLine));
		_check(_cmdline_add_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));
#endif
	}

#ifdef COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/Fo"), cmdLine));
#else
	#ifdef COMPILER_CLANG
	if(source->kind == RcSrc)
		_check(_cmdline_add_arg(_s("/Fo"), cmdLine)); /* llvm-rc uses /Fo like msvc rc */
	else
	#endif
	_check(_cmdline_add_arg(_s("-o"), cmdLine));
#endif

	_check(_cmdline_append_arg(objFilePath, cmdLine));

	pathBuffer.len = 0;
	_check(_assemble_source_path(source, &pathBuffer));
	_check(_cmdline_add_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));
	return 0;
}

static int _cmdline_add_link_options(const _linkoptions* defaults, const _linkoptions* overrides, _cmdlinebuffer* cmdLine) {
	_check(_cmdline_combine(cmdLine, &defaults->_flags));
	_check(_cmdline_combine(cmdLine, &overrides->_flags));

	return 0;
}

static int _cmdline_add_object_files(Target target, _cmdlinebuffer* cmdLine) {
	_pathbuffer pathBuffer;
	int         i;

	for(i = 0; i < _g_sourceCount; ++i) {
		_source* source = &_g_sources[i];

		if(source->target != target)
			continue;

		pathBuffer.len = 0;
		_check(_make_source_filepath(source, ObjectFile, &pathBuffer));
		_check(_cmdline_add_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));
	}

	return 0;
}

/*
 * Whether linking the target writes a pdb (MSVC with debug information. Static libraries have it embedded)
 */
static _bool _target_has_linker_pdb(Target target) {
#ifdef COMPILER_MSVC
	if(target->type == StaticLibrary)
		return false;

	if(target->compileOpt.debugInformation != NotSet)
		return target->compileOpt.debugInformation == Enabled;

	return _g_compileOptions.debugInformation == Enabled;
#else
	(void)target;
	return false;
#endif
}

static int _build_librarian_cmdline(Target target, _cmdlinebuffer* cmdLine) {
	_pathbuffer pathBuffer;

	cmdLine->len   = 0;
	pathBuffer.len = 0;

	_check(_cmdline_add_arg(_librarian_executable(), cmdLine));

#ifdef COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/NOLOGO"), cmdLine));
#else
	_check(_cmdline_add_arg(_s("rcs"), cmdLine)); /* Replace or add members, create the archive, write a symbol index */
#endif

	/* Static libraries aren't linked: they get the lib flags instead of the link flags */
	_check(_cmdline_combine(cmdLine, &_g_linkOptions._libFlags));
	_check(_cmdline_combine(cmdLine, &target->linkOpt._libFlags));

	_check(_make_target_filepath(_cstr(target->name), StaticLibrary, &pathBuffer));

#ifdef COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/OUT:"), cmdLine));
	_check(_cmdline_append_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));
#else
	_check(_cmdline_add_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));
#endif

	return _cmdline_add_object_files(target, cmdLine);
}

static int _build_linker_cmdline(Target target, _cmdlinebuffer* cmdLine) {
	_pathbuffer pathBuffer;

	if(target->type == StaticLibrary)
		return _build_librarian_cmdline(target, cmdLine);

	cmdLine->len   = 0;
	pathBuffer.len = 0;

	_check(_cmdline_add_arg(_linker_executable(), cmdLine));

	/*
	 * Object files come before the libraries in the link options: GNU ld only takes symbols from a library that are
	 * needed by the inputs before it.
	 */
#ifdef COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/nologo"), cmdLine));
#endif

	_check(_cmdline_add_object_files(target, cmdLine));

#ifdef COMPILER_MSVC
	/* cl passes the object files on to the linker, everything after /link are linker options */
	_check(_cmdline_add_arg(_s("/link"), cmdLine));
	_check(_cmdline_add_arg(_s("/NOLOGO"), cmdLine));
	_check(_cmdline_add_arg(_s("/INCREMENTAL:NO"), cmdLine));
#endif

	if(target->type == SharedLibrary) {
#ifdef COMPILER_MSVC
		_check(_cmdline_add_arg(_s("/DLL"), cmdLine));
#else
		_check(_cmdline_add_arg(_s("--shared"), cmdLine));
#endif
	}

	_check(_cmdline_add_link_options(&_g_linkOptions, &target->linkOpt, cmdLine));

#ifdef COMPILER_MSVC
	if(_target_has_linker_pdb(target)) {
		_check(_cmdline_add_arg(_s("/DEBUG:FULL"), cmdLine));
		_check(_cmdline_add_arg(_s("/PDB:"), cmdLine));

		pathBuffer.len = 0;
		_check(_make_target_filepath(_cstr(target->name), DebugInformation, &pathBuffer));
		_check(_cmdline_append_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));
	}
#endif

#ifdef COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/OUT:"), cmdLine));
#else
	_check(_cmdline_add_arg(_s("-o"), cmdLine));
#endif

	pathBuffer.len = 0;
	_check(_make_target_filepath(_cstr(target->name), target->type, &pathBuffer));
	_check(_cmdline_append_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine));

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

/*
 * Appends a path from a project file to a command line as the rest of the current argument. Relative paths are
 * relative to the project file's directory. The path is assembled first so it is quoted as a whole, if at all.
 */
static int _cmdline_append_project_path(str path, _cmdlinebuffer* cmdLine) {
	_pathbuffer pathBuffer;

	pathBuffer.len = 0;

	if(!_path_is_abs(path))
		_check(_path_add_project_dir(_g_currentProjectDir, &pathBuffer));

	_check(_path_append_raw(path, &pathBuffer));

	return _cmdline_append_arg(_str(pathBuffer.data, pathBuffer.len), cmdLine);
}

static int _add_include_paths_callback(str includePath, void* userData, int line, const char* file) {
	_cmdlinebuffer* cmdLine = (_cmdlinebuffer*)userData;
	(void)line;
	(void)file;

#ifdef COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/I"), cmdLine));
#else
	_check(_cmdline_add_arg(_s("-I"), cmdLine));
#endif

	return _cmdline_append_project_path(includePath, cmdLine);
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

#ifdef COMPILER_MSVC
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

/*
 * Librarian options, used instead of the linker options when creating static libraries (lib, llvm-ar, ar)
 */

#define lib_flags(flag_list) \
	_check(_parse_list(flag_list, _add_flags_callback, &_g_linkOptions._libFlags, __LINE__, __FILE__))

#define target_lib_flags(target_or_target_name, flag_list) \
	do { \
		Target _target = _get_target(target_or_target_name, __LINE__, __FILE__); \
		if(_target == NONE) \
			return 1; \
		_check(_parse_list(flag_list, _add_flags_callback, &_target->linkOpt._libFlags, __LINE__, __FILE__)); \
	} while((void)0,0)

static int _target_lib_path(str libPath, _cmdlinebuffer* cmdLine) {
#ifdef COMPILER_MSVC
	_check(_cmdline_add_arg(_s("/LIBPATH:"), cmdLine));
#else
	_check(_cmdline_add_arg(_s("-L"), cmdLine));
#endif

	return _cmdline_append_project_path(libPath, cmdLine);
}

static int _target_link_lib(str libName, _cmdlinebuffer* cmdLine) {
	if(libName.len > 0) {
#ifdef COMPILER_MSVC
		_check(_cmdline_add_arg(libName, cmdLine));

		if(_file_ext(libName).len == 0)
			_check(_cmdline_append_arg(_s(".lib"), cmdLine));
#else
		_check(_cmdline_add_arg(_s("-l"), cmdLine));
		_check(_cmdline_append_arg(libName, cmdLine));
#endif
	}

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

static _bool _link_dependencies_done(Target target) {
	int i;

	for(i = 0; i < target->_linkDependencyCount; ++i) {
		if(target->_linkDependencies[i]->_linkState != Linked)
			return false;
	}

	return true;
}

static int _target_link(Target target, Target linkedTarget, int line, const char* file) {
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

	targetLibDirBuffer.len = 0;
	_check(_make_target_filepath(_cstr(linkedTarget->name), linkedTarget->type, &targetLibDirBuffer));

	/* Static libraries by path because -l<name> looks for <name>.lib or lib<name>.a, not the <name>.a created here */
	if(linkedTarget->type == StaticLibrary) {
		_check(_cmdline_add_arg(_str(targetLibDirBuffer.data, targetLibDirBuffer.len), &target->linkOpt._flags));
	} else {
		_check(_target_lib_path(_path_without_file(_str(targetLibDirBuffer.data, targetLibDirBuffer.len)), &target->linkOpt._flags));
		_check(_target_link_lib(_cstr(linkedTarget->name), &target->linkOpt._flags));
	}

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
			_check(_parse_list((const char*)library_list_or_target, _add_libs_callback, target, __LINE__, __FILE__)); \
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

/* A relative install directory is relative to the directory of the project file */
#define install_directory(install_dir) \
	do { \
		_g_installDir     = _cstr(install_dir); \
		_g_installDirBase = _g_projectFileDir; \
	} while((void)0,0)

#define target_install_directory(target_or_target_name, install_dir) \
	do { \
		Target target = _get_target(target_or_target_name, __LINE__, __FILE__); \
		if(target == NONE) \
			return 1; \
		target->installDir     = _cstr(install_dir); \
		target->installDirBase = _g_projectFileDir; \
	} while((void)0,0)

/*
 * Process creation
 */

#ifdef OS_WINDOWS
/*
 * Whether a line of compiler or linker output is a diagnostic worth showing
 */
static _bool _line_is_diagnostic(str line) {
#ifdef COMPILER_MSVC
	/* cl prints the source file name and link prints informational lines, so the output is never empty. Warnings and errors always contain " warning " or " error " */
	return _str_contains(line, _s(" warning ")) || _str_contains(line, _s(" error "));
#else
	/* gcc, clang and their linkers print nothing on success */
	return line.len > 0;
#endif
}

/*
 * Whether a line of compiler output is one of cl's /showIncludes notes. *path receives the included file.
 */
static _bool _line_is_include_note(str line, str* path) {
#ifdef COMPILER_MSVC
	const str note = _s(MSVC_INCLUDE_NOTE);

	if(!_str_prefix(line, note))
		return false;

	*path = _str_right(line, line.len - note.len);

	while(path->len > 0 && path->data[0] == ' ') { /* Indented by nesting depth */
		++path->data;
		--path->len;
	}

	while(path->len > 0 && ascii_isspace(path->data[path->len - 1]))
		--path->len;

	return true;
#else
	(void)line;
	(void)path;
	return false;
#endif
}

/*
 * Shows the output of a finished job if it failed or contains diagnostics. cl's include notes are left out of the
 * output and, if a dependency file writer is given, written to the dependency file.
 */
static void _finish_job_output(void* outputHandle, _bool failed, _filewriter* depFile) {
	_filereader reader;
	char        line[1024];
	_bool       hasDiagnostics = false;
	_bool       isIncludeLine  = false;
	_bool       atLineStart;
	_bool       lineEnd;
	int         lineLen;
	int         pass;

	/* The first pass collects the dependencies and decides whether there is anything to show, the second shows it */
	for(pass = 0; pass < 2; ++pass) {
		if(pass == 1) {
			if(!failed && !hasDiagnostics)
				break;

			if(failed)
				_log_err(_emptyString, -1, NONE);
			else
				_log_msg(_emptyString, -1, NONE);
		}

		SetFilePointer(outputHandle, 0, NONE, 0); /* FILE_BEGIN. The file position is shared with the finished job */
		_filereader_init(&reader, outputHandle);
		atLineStart = true;

		while((lineLen = _filereader_line(&reader, line, sizeof(line), &lineEnd)) >= 0) {
			const str piece       = _str(line, lineLen);
			str       includePath = _emptyString;

			if(atLineStart)
				isIncludeLine = _line_is_include_note(piece, &includePath);

			if(isIncludeLine) {
				if(pass == 0 && atLineStart && depFile != NONE) {
					_filewriter_write(depFile, _s(" \\\n "));
					_depfile_write_path(depFile, includePath);
				}
			} else if(pass == 0) {
				if(_line_is_diagnostic(piece))
					hasDiagnostics = true;
			} else {
				_log_raw(piece);

				if(lineEnd)
					_log_raw(_s("\n"));
			}

			atLineStart = lineEnd;
		}
	}
}

/*
 * Output and dependency file of a finished compile job. gcc and clang write the dependency file themselves but may
 * have left it incomplete when they failed, for cl it is assembled from the include notes in the output.
 */
static void _finish_compile_job(const _source* source, void* outputHandle, _bool failed) {
	_pathbuffer pathBuffer;
	str         depFilePath;

	pathBuffer.len = 0;

	if(_make_depfile_path(source, &pathBuffer) != 0) {
		_finish_job_output(outputHandle, failed, NONE);
		return;
	}

	depFilePath = _str(pathBuffer.data, pathBuffer.len);

	if(failed) {
		_delete_file(depFilePath);
		_finish_job_output(outputHandle, true, NONE);
		return;
	}

#ifdef COMPILER_MSVC
	{
		_filewriter depFile;

		_filewriter_init(&depFile, _open_file(depFilePath, true));
		_depfile_write_path(&depFile, _file_basename(depFilePath)); /* The object file */
		_filewriter_write(&depFile, _s(":"));

		_finish_job_output(outputHandle, false, &depFile);

		_filewriter_write(&depFile, _s("\n"));
		_filewriter_flush(&depFile);
		_close_file(depFile.handle);

		if(depFile.failed) {
			_log_err(_s("Failed to write dependency file '"), -1, NONE);
			_log_raw(depFilePath);
			_log_raw(_s("'"));
			_delete_file(depFilePath);
		}
	}
#else
	_finish_job_output(outputHandle, false, NONE);
#endif
}

/*
 * Handles the output of a finished job and closes its handles
 */
static void _close_job(int jobIdx, _bool failed) {
	const _job* job = &_g_win32Jobs[jobIdx];

	if(job->stdOut != NONE) { /* The relaunched build program writes to the console directly */
		if(job->source != NONE && _source_has_depfile(job->source))
			_finish_compile_job(job->source, job->stdOut, failed);
		else
			_finish_job_output(job->stdOut, failed, NONE);

		CloseHandle(job->stdOut);
	}

	if(job->target != NONE)
		job->target->_linkState = Linked;

	CloseHandle(_g_win32Processes[jobIdx]);
}

#endif

/*
 * Waits for any running job to finish and removes it from the job list. Fails if no job is running.
 */
static int _wait_any_job(void) {
#ifdef OS_WINDOWS
	_ulong exitCode = 0;
	_ulong wait;

	log_flush(); /* Show the pending log line before blocking */

	wait = WaitForMultipleObjects((_ulong)_g_win32ProcessCount, _g_win32Processes, 0, INFINITE);

	if(wait == WAIT_FAILED) {
		/* No way to tell which job the wait failed for, give up on all of them */
		const int error = (int)GetLastError();
		int       i;

		for(i = 0; i < _g_win32ProcessCount; ++i)
			_close_job(i, true);

		mem_fill(_g_win32Processes, 0, sizeof(_g_win32Processes));
		mem_fill(_g_win32Jobs, 0, sizeof(_g_win32Jobs));
		_g_win32ProcessCount = 0;

		return error;
	}

	GetExitCodeProcess(_g_win32Processes[wait], &exitCode);
	_close_job((int)wait, exitCode != 0);

	--_g_win32ProcessCount;
	_g_win32Processes[wait]                 = _g_win32Processes[_g_win32ProcessCount];
	_g_win32Jobs[wait]                      = _g_win32Jobs[_g_win32ProcessCount];
	_g_win32Processes[_g_win32ProcessCount] = NONE;
	mem_fill(&_g_win32Jobs[_g_win32ProcessCount], 0, sizeof(_job));

	return (int)exitCode;
#else
	return 1;
#endif
}

/*
 * Waits for all running jobs to finish. Returns the exit code of the last job that failed.
 */
static int _wait_jobs(void) {
	int exitCode = 0;

#ifdef OS_WINDOWS
	while(_g_win32ProcessCount > 0) {
		const int jobExit = _wait_any_job();

		if(jobExit != 0)
			exitCode = jobExit;
	}
#endif

	return exitCode;
}

/*
 * Starts a process. source is the source of a compile job (for its dependency file and output), target the target of a
 * link job (marked done when the job finishes), NONE for other jobs.
 */
static int _add_job(const _cmdlinebuffer* cmdLine, str workingDir, _bool redirectOutput, const _source* source, Target target) {
#ifdef OS_WINDOWS
	_wchar              workingDirBuffer[MAX_PATH];
	_wchar              cmdLineBuffer[MAX_COMMAND_LINE + 1];
	const int           workingDirLen = MultiByteToWideChar(CP_UTF8, 0, workingDir.data, workingDir.len, workingDirBuffer, MAX_PATH);
	const int           cmdLineLen    = MultiByteToWideChar(CP_UTF8, 0, cmdLine->buffer, cmdLine->len, cmdLineBuffer, MAX_COMMAND_LINE + 1);
	STARTUPINFOW        startupInfo;
	PROCESS_INFORMATION processInfo;
	SECURITY_ATTRIBUTES securityAttributes;
	void*               outputFile = NONE;

	/* A log line is only written once it ends, i.e. with the next message. Show what is about to run now. */
	log_flush();

	workingDirBuffer[workingDirLen] = '\0';
	cmdLineBuffer[cmdLineLen]       = '\0';

	mem_fill(&startupInfo, 0, sizeof(startupInfo));
	mem_fill(&processInfo, 0, sizeof(processInfo));
	mem_fill(&securityAttributes, 0, sizeof(securityAttributes));

	/* Max job slots used, wait for one to finish */
	if(_g_win32ProcessCount >= _g_maxJobs)
		_check(_wait_any_job());

	securityAttributes.nLength        = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = 1;

	startupInfo.cb      = sizeof(startupInfo);
	startupInfo.dwFlags = 0x00000100; /* STARTF_USESTDHANDLES */

	if(redirectOutput) {
		/*
		 * Job output goes to a temporary file instead of a pipe.
		 * It is only read after the job has finished, which would deadlock as soon as a job fills up the pipe buffer.
		 */
		static const _wchar prefix[] = {'b', 'l', 'd', '\0'};
		_wchar              tempDir[MAX_PATH];
		_wchar              tempFile[MAX_PATH];

		if(GetTempPathW(MAX_PATH, tempDir) == 0 || GetTempFileNameW(tempDir, prefix, 0, tempFile) == 0) {
			_log_err(_s("Failed to create temporary file for job output"), -1, NONE);
			return (int)GetLastError();
		}

		/* GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE */
		outputFile = CreateFileW(tempFile, 0xc0000000, 0x7, &securityAttributes, 2, 0x04000100, NONE);

		if(outputFile == INVALID_HANDLE_VALUE) {
			_log_err(_s("Failed to open temporary file for job output"), -1, NONE);
			return (int)GetLastError();
		}

		startupInfo.hStdError  = outputFile;
		startupInfo.hStdOutput = outputFile;
	} else {
		startupInfo.hStdError  = _g_win32StdoutHandle;
		startupInfo.hStdOutput = _g_win32StdoutHandle;
	}

	if(!CreateProcessW(NONE, cmdLineBuffer, NONE, NONE, 1, 0, NONE, workingDirLen > 0 ? workingDirBuffer : _g_win32Cwd, &startupInfo, &processInfo)) {
		if(outputFile)
			CloseHandle(outputFile);

		_log_err(_s("Failed to start process '"), -1, NONE);
		_log_raw(_str(cmdLine->buffer, cmdLine->len));
		_log_raw(_s("'"));
		return (int)GetLastError();
	}

	CloseHandle(processInfo.hThread);

	_g_win32Processes[_g_win32ProcessCount]   = processInfo.hProcess;
	_g_win32Jobs[_g_win32ProcessCount].stdOut = outputFile;
	_g_win32Jobs[_g_win32ProcessCount].source = source;
	_g_win32Jobs[_g_win32ProcessCount].target = target;
	++_g_win32ProcessCount;
#else
	(void)source;
	(void)target;
#endif

	return 0;
}

static int _add_compile_job(_source* source) {
	_cmdlinebuffer cmdLine;
	_pathbuffer    pathBuffer;
	_filetime      objLastModified;
	str            objFilePath;

	pathBuffer.len = 0;
	_check(_make_source_filepath(source, ObjectFile, &pathBuffer));
	objFilePath = _str(pathBuffer.data, pathBuffer.len);

	_get_lastmodified(objFilePath, &objLastModified);

	if(_g_incremental && _source_up_to_date(source, objLastModified, false))
		return 0;

	_check(_build_compiler_cmdline(source, objFilePath, &cmdLine));

	_log_msg(_s("["), -1, NONE);
	_log_raw(_cstr(source->target->name));
	_log_raw(_s("] "));
	_log_raw(_file_without_path(source->fileName));

	if(_g_verbose)
		_log_cmdline(&cmdLine);

	_check(_create_directory(_path_without_file(objFilePath), -1, NONE));

	return _add_job(&cmdLine, source->target->projectDir, true, source, NONE);
}

static int _add_link_job(Target target) {
	_cmdlinebuffer cmdLine;
	_pathbuffer    pathBuffer;
	int            i;

	target->_needsLink = !_g_incremental;

	if(!target->_needsLink) {
		for(i = 0; i < target->_linkDependencyCount; ++i) {
			if(target->_linkDependencies[i]->_needsLink) {
				target->_needsLink = true;
				break;
			}
		}
	}

	if(!target->_needsLink) {
		for(i = 0; i < _g_sourceCount; ++i) {
			_source*  source = &_g_sources[i];
			_filetime lastModified;

			if(source->target != target)
				continue;

			pathBuffer.len = 0;
			_check(_make_source_filepath(source, ObjectFile, &pathBuffer));
			_get_lastmodified(_str(pathBuffer.data, pathBuffer.len), &lastModified);

			if(_filetime_is_newer(lastModified, target->_lastModified)) {
				target->_needsLink = true;
				break;
			}
		}
	}

	if(!target->_needsLink) {
		target->_linkState = Linked;
		return 0;
	}

	_check(_build_linker_cmdline(target, &cmdLine));

#ifndef COMPILER_MSVC
	/* ar adds to an existing archive, which would keep the objects of removed sources */
	if(target->type == StaticLibrary) {
		pathBuffer.len = 0;
		_check(_make_target_filepath(_cstr(target->name), StaticLibrary, &pathBuffer));
		_check(_delete_file(_str(pathBuffer.data, pathBuffer.len)));
	}
#endif

	if(target->type == StaticLibrary)
		_log_msg(_s("Creating "), -1, NONE);
	else
		_log_msg(_s("Linking "), -1, NONE);

	_log_raw(_cstr(target->name));
	_log_raw(_artifact_file_ext(target->type));

	if(_g_verbose)
		_log_cmdline(&cmdLine);

	target->_linkState = Linking;

	return _add_job(&cmdLine, target->projectDir, true, NONE, target);
}

/*
 * Build
 */

static _buildconfig config(void) {
	return _g_config;
}

static int _install(void) {
	int i;

	for(i = 0; i < _g_targetCount; ++i) {
		const Target target = &_g_targets[i];
		str          installDir;
		str          installDirBase;
		_pathbuffer  artifactPathBuffer;
		_pathbuffer  installPathBuffer;
		int          pathLen;

		if(target->_skipBuild)
			continue;

		installDir     = target->installDir;
		installDirBase = target->installDirBase;

		/* Static libraries are only intermediate artifacts, install them only if the target asks for it explicitly */
		if(installDir.len == 0 && target->type != StaticLibrary) {
			installDir     = _g_installDir;
			installDirBase = _g_installDirBase;
		}

		if(installDir.len == 0)
			continue;

		_log_msg(_s("Installing "), -1, NONE);
		_log_raw(_cstr(target->name));
		_log_raw(_s(" to "));
		_log_raw(installDir);

		installPathBuffer.len = 0;

		/* Relative install directories are relative to the project file that set them */
		if(!_path_is_abs(installDir))
			_check(_path_add_project_dir(installDirBase, &installPathBuffer));

		_check(_path_add_segment(installDir, &installPathBuffer));

		artifactPathBuffer.len = 0;
		_check(_make_target_filepath(_cstr(target->name), target->type, &artifactPathBuffer));

		pathLen = installPathBuffer.len;

		_check(_path_append_raw(_file_without_path(_str(artifactPathBuffer.data, artifactPathBuffer.len)), &installPathBuffer));
		_check(_copy_file(_str(artifactPathBuffer.data, artifactPathBuffer.len), _str(installPathBuffer.data, installPathBuffer.len), -1, NONE));

		if(_target_has_linker_pdb(target)) {
			artifactPathBuffer.len = 0;
			_check(_make_target_filepath(_cstr(target->name), DebugInformation, &artifactPathBuffer));

			installPathBuffer.len = pathLen;
			_check(_path_append_raw(_file_without_path(_str(artifactPathBuffer.data, artifactPathBuffer.len)), &installPathBuffer));
			_check(_copy_file(_str(artifactPathBuffer.data, artifactPathBuffer.len), _str(installPathBuffer.data, installPathBuffer.len), -1, NONE));
		}
	}

	return 0;
}

static int _build(void) {
	int exitCode = 0;
	int i;

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
				return 1;
			}

			for(i = 0; i < _g_targetCount; ++i) {
				Target dependency = &_g_targets[i];

				if(dependency != target && dependency->_skipBuild && _target_depends_on(target, dependency))
					_g_targets[i]._skipBuild = false;
			}

			target->_skipBuild = false;
		}
	}

	for(i = 0; i < _g_sourceCount; ++i) {
		_source*  source = &_g_sources[i];

		if(source->compiled || source->target->_skipBuild)
			continue;

		exitCode = _add_compile_job(source);

		if(exitCode != 0) {
			_wait_jobs();
			break;
		}
	}

	if(exitCode == 0)
		exitCode = _wait_jobs();

	if(exitCode != 0)
		return exitCode;

	/*
	 * Link. A target is linked as soon as every target it links against is done, so targets which don't depend on each
	 * other link in parallel. Skipped targets count as done; their artifacts are expected to exist.
	 */
	for(i = 0; i < _g_targetCount; ++i)
		_g_targets[i]._linkState = _g_targets[i]._skipBuild ? Linked : NotLinked;

	for(;;) {
		_bool started = false;
		_bool pending = false;

		for(i = 0; i < _g_targetCount; ++i) {
			Target target = &_g_targets[i];

			if(target->_linkState == Linked)
				continue;

			pending = true;

			if(target->_linkState == Linking || !_link_dependencies_done(target))
				continue;

			exitCode = _add_link_job(target);

			if(exitCode != 0) {
				_wait_jobs();
				return exitCode;
			}

			started = true;
		}

		if(!pending)
			break;

		if(!started) /* Otherwise a target which didn't need linking may have unblocked one scanned before it */
			_check(_wait_any_job());
	}

	if(_g_install)
		_check(_install());

	return 0;
}

static int _rebuild_if_needed(const char* srcFile, int argc, char** argv, _bool* didRebuild) {
	static _pathbuffer tmpBuildDirBuffer;
	const str          prevBuildDir = _g_buildDir;
	const _buildconfig prevConfig   = _g_config;
	char               targetNameBuffer[64];
	Target             target;
	_source*           source;
	str                targetName = _file_without_path(_file_basename(_cstr(srcFile)));
#ifdef OS_WINDOWS
	_wchar             wExePathBuffer[MAX_PATH];
#endif

	if(targetName.len >= (int)sizeof(targetNameBuffer) - 1) {
		_log_err(_s("Build project file name too long"), -1, NONE);
		return 1;
	}

	mem_copy(targetNameBuffer, targetName.data, targetName.len);
	targetNameBuffer[targetName.len] = '\0';

	target = _add_target(targetNameBuffer, Executable, -1, NONE);

	if(!target)
		return 1;

	target_sources(target, NONE, srcFile);

	target = &_g_targets[_g_targetCount - 1];
	source = &_g_sources[_g_sourceCount - 1];
	target->compileOpt.optimizations = OptimizeSpeed;

	/*
	 * Get lastmodified timestamp from actual executable file which might not be the same as the target.
	 */

#ifdef OS_WINDOWS
	if(GetModuleFileNameW(NONE, wExePathBuffer, _array_count(wExePathBuffer)) == 0) {
		_log_err(_s("GetModuleFileNameW failed"), -1, NONE);
		return (int)GetLastError();
	}

	_win32_get_lastmodified(wExePathBuffer, &target->_lastModified);
#endif

	/*
	 * The build program is rebuilt in the release config into _g_buildDir + ".tmp", where the dependency file of its
	 * last rebuild is as well
	 */
	tmpBuildDirBuffer.len = 0;
	_check(_path_add_segment(_g_buildDir, &tmpBuildDirBuffer));
	_check(_path_add_segment(_s(".tmp"), &tmpBuildDirBuffer));
	_g_buildDir = _str(tmpBuildDirBuffer.data, tmpBuildDirBuffer.len);
	_g_config   = Release;

	if(!_source_up_to_date(source, target->_lastModified, false)) {
#ifdef OS_WINDOWS
		_pathbuffer pathBuffer;
		_wchar      wTmpExePathBuffer[MAX_PATH];
		str         artifactPath;

		_g_incremental       = false;
		_g_targetsToBuild[0] = targetNameBuffer;
		_g_targetsToBuild[1] = NONE;
		_g_installDir        = _emptyString; /* Don't install */
		_log_msg(_s("Recreating build program because the source has changed"), -1, NONE);

		_check(_create_directory(_g_buildDir, -1, NONE));

		/*
		 * Windows doesn't allow deleting the currently running executable but moving the file is fine.
		 * It is moved to a file in the build directory with the .old extension so the compiler can create the new executable in place.
		 */

		pathBuffer.len = 0;
		_check(_make_target_filepath(targetName, target->type, &pathBuffer));
		artifactPath = _str(pathBuffer.data, pathBuffer.len);
		_check(_create_directory(_path_without_file(artifactPath), -1, NONE));
		_check(_path_append_raw(_s(".old"), &pathBuffer));
		_check(_win32_assemble_path(_str(pathBuffer.data, pathBuffer.len), wTmpExePathBuffer));

		if(!MoveFileExW(wExePathBuffer, wTmpExePathBuffer, 0x1 | 0x8)) { /* MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH */
			_log_err(_s("Failed to move to temporary file: "), -1, NONE);
			_log_raw(_str(pathBuffer.data, pathBuffer.len));
			return (int)GetLastError();
		}
#endif

		_check(_build());

#ifdef OS_WINDOWS
		/*
		 * After the build, move the newly created executable back to the old location.
		 */

		_check(_win32_assemble_path(artifactPath, wTmpExePathBuffer));

		if(!MoveFileExW(wTmpExePathBuffer, wExePathBuffer, 0x1 | 0x8)) { /* MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH */
			_log_err(_s("Failed to move build executable to final location"), -1, NONE);
			return (int)GetLastError();
		}
#endif

		*didRebuild = true;

		/*
		 * Relaunch build with the same args
		 */
		{
			_cmdlinebuffer cmdLine;
			int            i;
			cmdLine.len = 0;

			for(i = 0; i < argc; ++i) {
				const str argStr = _cstr(argv[i]);

				/* Project file has changed so don't do an incremental build even if it was requested */
				if(_str_ieq(argStr, _s("--incremental")))
					continue;

				_check(_cmdline_add_arg(argStr, &cmdLine));
			}

			_log_raw(_s("\n"));
			_check(_add_job(&cmdLine, _g_cwd, false, NONE, NONE));
			_check(_wait_jobs());
		}
	} else {
		_g_buildDir = prevBuildDir;
		_g_config   = prevConfig;
		*didRebuild = false;
	}

	--_g_targetCount;
	--_g_sourceCount;

	return 0;
}

/*
 * Project
 */

#define begin_project(project_identifier) \
	int _main(int argc, char** argv) { \
		_g_projectFileDir    = _path_without_file(_s(__FILE__)); \
		_g_currentProjectDir = _g_projectFileDir; \
		do { \
			_bool didRebuild = false; \
			_check(_rebuild_if_needed(__FILE__, argc, argv, &didRebuild)); \
			if(didRebuild) \
				return 0; \
		} while((void)0,0); \
		do { \

#define end_project \
		} while((void)0,0); \
		return _build(); \
	}

#define include_project(project_identifier) \
	do { \
		int       project_identifier ## _main(int argc, char** argv); \
		const str _prevProjectFileDir    = _g_projectFileDir; \
		const str _prevCurrentProjectDir = _g_currentProjectDir; \
		_check(project_identifier ## _main(argc, argv)); \
		_g_projectFileDir    = _prevProjectFileDir; \
		_g_currentProjectDir = _prevCurrentProjectDir; \
	} while((void)0,0)

#define project_directory(directory_name) \
	_g_currentProjectDir = _cstr(directory_name)

/*
 * Init/deinit
 */

static int _init(void) {
#ifdef OS_WINDOWS
	typedef _wchar**(_winapi*CmdLineToArgv)(const _wchar*, int*);
	void*         shell32            = LoadLibraryA("Shell32.dll");
	CmdLineToArgv CommandLineToArgvW = (CmdLineToArgv)GetProcAddress(shell32, "CommandLineToArgvW");
	_wchar**      argvW              = CommandLineToArgvW(GetCommandLineW(), &_g_win32Argc);
	int           bufferOffset       = 0;
	SYSTEM_INFO   systemInfo;
	int i;

	mem_fill(&systemInfo, 0, sizeof(systemInfo));
	GetSystemInfo(&systemInfo);

	_g_maxJobs           = (int)systemInfo.dwNumberOfProcessors;
	_g_win32StdoutHandle = GetStdHandle(0xfffffff5);

	if(_g_win32Argc >= _array_count(_g_win32Argv)) {
		_log_err(_s("Too many arguments"), -1, NONE);
		return 1;
	}

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
#ifdef OS_WINDOWS
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
						_g_config = Debug;
						break;
					} else if(_str_ieq(argStr, _s("--release"))) {
						_g_config = Release;
						break;
					} else if(_str_ieq(argStr, _s("--profiling"))) {
						_g_config = Profiling;
						break;
					} else if(_str_ieq(argStr, _s("--incremental"))) {
						_g_incremental = true;
						break;
					} else if(_str_ieq(argStr, _s("--install"))) {
						_g_install = true;
						break;
					}
				}
				/* fallthrough */
			default:
				_log_err(_s("Unknown argument '"), -1, NONE);
				_log_raw(_cstr(arg));
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

int _main(int argc, char** argv);

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

		if(exitCode == 0) {
			_log_msg(_s("Compiler: "), -1, NONE);
			_log_raw(_compiler_executable(CSrc));

			_log_msg(_s("Config: "), -1, NONE);

			switch(_g_config) {
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

			exitCode = _main(argc, argv);
		}
	}

	_deinit_and_exit(exitCode);
#undef argc
#undef argv
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

#ifdef COMPILER_MSVC
	#pragma warning(pop)
#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
	#pragma GCC diagnostic pop
#endif

#else
/*
 * If build.h was already included, change the begin_project and end_project macros.
 * Subproject files must be included after the main begin_project/end_project or compiled as separate compilation units.
 */

#undef begin_project
#define begin_project(project_identifier) \
	int project_identifier ## _main(int argc, char** argv) { \
		(void)argc; \
		(void)argv; \
		_g_projectFileDir    = _path_without_file(_s(__FILE__)); \
		_g_currentProjectDir = _g_projectFileDir; \
		do {

#undef end_project
#define end_project \
		} while((void)0,0); \
		return 0; \
	}

#endif /* BUILD_H_INCLUDED */
