/*
 Formatting library for C++

 Copyright (c) 2012 - 2016, Victor Zverovich
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FMT_FORMAT_H_
#define FMT_FORMAT_H_

#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <locale>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>
#include <stdint.h>

#ifdef _SECURE_SCL
# define FMT_SECURE_SCL _SECURE_SCL
#else
# define FMT_SECURE_SCL 0
#endif

#if FMT_SECURE_SCL
# include <iterator>
#endif

#if !defined(FMT_HEADER_ONLY) && defined(_WIN32)
# ifdef FMT_EXPORT
#  define FMT_API __declspec(dllexport)
# elif defined(FMT_SHARED)
#  define FMT_API __declspec(dllimport)
# endif
#endif
#ifndef FMT_API
# define FMT_API
#endif

#ifdef __GNUC__
# define FMT_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
# if FMT_GCC_VERSION >= 406
#  pragma GCC diagnostic push

// Disable the warning about declaration shadowing because it affects too
// many valid cases.
#  pragma GCC diagnostic ignored "-Wshadow"

// Disable the warning about implicit conversions that may change the sign of
// an integer; silencing it otherwise would require many explicit casts.
#  pragma GCC diagnostic ignored "-Wsign-conversion"
# endif
#endif

#ifdef __clang__
# define FMT_CLANG_VERSION (__clang_major__ * 100 + __clang_minor__)
#endif

#if defined(__INTEL_COMPILER)
# define FMT_ICC_VERSION __INTEL_COMPILER
#elif defined(__ICL)
# define FMT_ICC_VERSION __ICL
#endif

#if defined(__clang__) && !defined(FMT_ICC_VERSION)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
# pragma clang diagnostic ignored "-Wpadded"
#endif

#ifdef __GNUC_LIBSTD__
# define FMT_GNUC_LIBSTD_VERSION (__GNUC_LIBSTD__ * 100 + __GNUC_LIBSTD_MINOR__)
#endif

#ifdef _MSC_VER
# define FMT_MSC_VER _MSC_VER
#else
# define FMT_MSC_VER 0
#endif

#ifdef __has_feature
# define FMT_HAS_FEATURE(x) __has_feature(x)
#else
# define FMT_HAS_FEATURE(x) 0
#endif

#ifdef __has_builtin
# define FMT_HAS_BUILTIN(x) __has_builtin(x)
#else
# define FMT_HAS_BUILTIN(x) 0
#endif

#ifdef __has_cpp_attribute
# define FMT_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
# define FMT_HAS_CPP_ATTRIBUTE(x) 0
#endif

// Use the compiler's attribute noreturn.
#if defined(__MINGW32__) || defined(__MINGW64__)
# define FMT_NORETURN __attribute__((noreturn))
#elif FMT_HAS_CPP_ATTRIBUTE(noreturn)
# define FMT_NORETURN [[noreturn]]
#else
# define FMT_NORETURN
#endif

// Check if exceptions are disabled.
#if defined(__GNUC__) && !defined(__EXCEPTIONS)
# define FMT_EXCEPTIONS 0
#endif
#if FMT_MSC_VER && !_HAS_EXCEPTIONS
# define FMT_EXCEPTIONS 0
#endif
#ifndef FMT_EXCEPTIONS
# define FMT_EXCEPTIONS 1
#endif

// Define FMT_USE_NOEXCEPT to make fmt use noexcept (C++11 feature).
#ifndef FMT_USE_NOEXCEPT
# define FMT_USE_NOEXCEPT 0
#endif

#ifndef FMT_NOEXCEPT
# if FMT_EXCEPTIONS
#  if FMT_USE_NOEXCEPT || FMT_HAS_FEATURE(cxx_noexcept) || \
    FMT_GCC_VERSION >= 408 || FMT_MSC_VER >= 1900
#   define FMT_NOEXCEPT noexcept
#  else
#   define FMT_NOEXCEPT throw()
#  endif
# else
#  define FMT_NOEXCEPT
# endif
#endif

// A macro to disallow the copy construction and assignment.
#define FMT_DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete; \
    TypeName& operator=(const TypeName&) = delete
#define FMT_DELETED_OR_UNDEFINED = delete

#ifndef FMT_USE_USER_DEFINED_LITERALS
// All compilers which support UDLs also support variadic templates. This
// makes the fmt::literals implementation easier. However, an explicit check
// for variadic templates is added here just in case.
// For Intel's compiler both it and the system gcc/msc must support UDLs.
# if (FMT_HAS_FEATURE(cxx_user_literals) || \
      FMT_GCC_VERSION >= 407 || FMT_MSC_VER >= 1900) && \
      (!defined(FMT_ICC_VERSION) || FMT_ICC_VERSION >= 1500)
#  define FMT_USE_USER_DEFINED_LITERALS 1
# else
#  define FMT_USE_USER_DEFINED_LITERALS 0
# endif
#endif

#if FMT_USE_USER_DEFINED_LITERALS && \
    (FMT_GCC_VERSION >= 600 || FMT_CLANG_VERSION >= 304)
# define FMT_UDL_TEMPLATE 1
#else
# define FMT_UDL_TEMPLATE 0
#endif

#ifndef FMT_ASSERT
# define FMT_ASSERT(condition, message) assert((condition) && message)
#endif

#if FMT_GCC_VERSION >= 400 || FMT_HAS_BUILTIN(__builtin_clz)
# define FMT_BUILTIN_CLZ(n) __builtin_clz(n)
#endif

#if FMT_GCC_VERSION >= 400 || FMT_HAS_BUILTIN(__builtin_clzll)
# define FMT_BUILTIN_CLZLL(n) __builtin_clzll(n)
#endif

// Some compilers masquerade as both MSVC and GCC-likes or otherwise support
// __builtin_clz and __builtin_clzll, so only define FMT_BUILTIN_CLZ using the
// MSVC intrinsics if the clz and clzll builtins are not available.
#if FMT_MSC_VER && !defined(FMT_BUILTIN_CLZLL)
# include <intrin.h>  // _BitScanReverse, _BitScanReverse64

namespace fmt {
namespace internal {
# pragma intrinsic(_BitScanReverse)
inline uint32_t clz(uint32_t x) {
  unsigned long r = 0;
  _BitScanReverse(&r, x);

  assert(x != 0);
  // Static analysis complains about using uninitialized data
  // "r", but the only way that can happen is if "x" is 0,
  // which the callers guarantee to not happen.
# pragma warning(suppress: 6102)
  return 31 - r;
}
# define FMT_BUILTIN_CLZ(n) fmt::internal::clz(n)

# ifdef _WIN64
#  pragma intrinsic(_BitScanReverse64)
# endif

inline uint32_t clzll(uint64_t x) {
  unsigned long r = 0;
# ifdef _WIN64
  _BitScanReverse64(&r, x);
# else
  // Scan the high 32 bits.
  if (_BitScanReverse(&r, static_cast<uint32_t>(x >> 32)))
    return 63 - (r + 32);

  // Scan the low 32 bits.
  _BitScanReverse(&r, static_cast<uint32_t>(x));
# endif

  assert(x != 0);
  // Static analysis complains about using uninitialized data
  // "r", but the only way that can happen is if "x" is 0,
  // which the callers guarantee to not happen.
# pragma warning(suppress: 6102)
  return 63 - r;
}
# define FMT_BUILTIN_CLZLL(n) fmt::internal::clzll(n)
}
}
#endif

namespace fmt {
namespace internal {
struct dummy_int {
  int data[2];
  operator int() const { return 0; }
};
typedef std::numeric_limits<fmt::internal::dummy_int> fputil;

// Dummy implementations of system functions such as signbit and ecvt called
// if the latter are not available.
inline dummy_int signbit(...) { return dummy_int(); }
inline dummy_int _ecvt_s(...) { return dummy_int(); }
inline dummy_int isinf(...) { return dummy_int(); }
inline dummy_int _finite(...) { return dummy_int(); }
inline dummy_int isnan(...) { return dummy_int(); }
inline dummy_int _isnan(...) { return dummy_int(); }

// A helper function to suppress bogus "conditional expression is constant"
// warnings.
template <typename T>
inline T const_check(T value) { return value; }
}
}  // namespace fmt

namespace std {
// Standard permits specialization of std::numeric_limits. This specialization
// is used to resolve ambiguity between isinf and std::isinf in glibc:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=48891
// and the same for isnan and signbit.
template <>
class numeric_limits<fmt::internal::dummy_int> :
    public std::numeric_limits<int> {
 public:
  // Portable version of isinf.
  template <typename T>
  static bool isinfinity(T x) {
    using namespace fmt::internal;
    // The resolution "priority" is:
    // isinf macro > std::isinf > ::isinf > fmt::internal::isinf
    if (const_check(sizeof(isinf(x)) == sizeof(bool) ||
                    sizeof(isinf(x)) == sizeof(int))) {
      return isinf(x) != 0;
    }
    return !_finite(static_cast<double>(x));
  }

  // Portable version of isnan.
  template <typename T>
  static bool isnotanumber(T x) {
    using namespace fmt::internal;
    if (const_check(sizeof(isnan(x)) == sizeof(bool) ||
                    sizeof(isnan(x)) == sizeof(int))) {
      return isnan(x) != 0;
    }
    return _isnan(static_cast<double>(x)) != 0;
  }

  // Portable version of signbit.
  static bool isnegative(double x) {
    using namespace fmt::internal;
    if (const_check(sizeof(signbit(x)) == sizeof(int)))
      return signbit(x) != 0;
    if (x < 0) return true;
    if (!isnotanumber(x)) return false;
    int dec = 0, sign = 0;
    char buffer[2];  // The buffer size must be >= 2 or _ecvt_s will fail.
    _ecvt_s(buffer, sizeof(buffer), x, 0, &dec, &sign);
    return sign != 0;
  }
};
}  // namespace std

namespace fmt {

template <typename Char>
class basic_writer;

template <typename Context>
class basic_arg;

template <typename Char>
class basic_context;

typedef basic_context<char> context;
typedef basic_context<wchar_t> wcontext;

// A formatter for objects of type T.
template <typename T, typename Char = char, typename Enable = void>
struct formatter;

/**
  \rst
  An implementation of ``std::basic_string_view`` for pre-C++17. It provides a
  subset of the API.
  \endrst
 */
template <typename Char>
class basic_string_view {
 private:
  const Char *data_;
  std::size_t size_;

 public:
  using char_type = Char;
  using iterator = const Char *;

  constexpr basic_string_view() noexcept : data_(0), size_(0) {}

  /** Constructs a string reference object from a C string and a size. */
  constexpr basic_string_view(const Char *s, std::size_t size) noexcept
    : data_(s), size_(size) {}

  /**
    \rst
    Constructs a string reference object from a C string computing
    the size with ``std::char_traits<Char>::length``.
    \endrst
   */
  basic_string_view(const Char *s)
    : data_(s), size_(std::char_traits<Char>::length(s)) {}

  /**
    \rst
    Constructs a string reference from an ``std::string`` object.
    \endrst
   */
  constexpr basic_string_view(const std::basic_string<Char> &s) noexcept
  : data_(s.c_str()), size_(s.size()) {}

  /**
    \rst
    Converts a string reference to an ``std::string`` object.
    \endrst
   */
  std::basic_string<Char> to_string() const {
    return std::basic_string<Char>(data_, size_);
  }

  /** Returns a pointer to the string data. */
  const Char *data() const { return data_; }

  /** Returns the string size. */
  constexpr std::size_t size() const { return size_; }

  constexpr iterator begin() const { return data_; }
  constexpr iterator end() const { return data_ + size_; }

  constexpr void remove_prefix(size_t n) {
    data_ += n;
    size_ -= n;
  }

  // Lexicographically compare this string reference to other.
  int compare(basic_string_view other) const {
    std::size_t size = size_ < other.size_ ? size_ : other.size_;
    int result = std::char_traits<Char>::compare(data_, other.data_, size);
    if (result == 0)
      result = size_ == other.size_ ? 0 : (size_ < other.size_ ? -1 : 1);
    return result;
  }

  friend bool operator==(basic_string_view lhs, basic_string_view rhs) {
    return lhs.compare(rhs) == 0;
  }
  friend bool operator!=(basic_string_view lhs, basic_string_view rhs) {
    return lhs.compare(rhs) != 0;
  }
  friend bool operator<(basic_string_view lhs, basic_string_view rhs) {
    return lhs.compare(rhs) < 0;
  }
  friend bool operator<=(basic_string_view lhs, basic_string_view rhs) {
    return lhs.compare(rhs) <= 0;
  }
  friend bool operator>(basic_string_view lhs, basic_string_view rhs) {
    return lhs.compare(rhs) > 0;
  }
  friend bool operator>=(basic_string_view lhs, basic_string_view rhs) {
    return lhs.compare(rhs) >= 0;
  }
};

typedef basic_string_view<char> string_view;
typedef basic_string_view<wchar_t> wstring_view;

/** A formatting error such as invalid format string. */
class format_error : public std::runtime_error {
 public:
  explicit format_error(const char *message)
  : std::runtime_error(message) {}

  explicit format_error(const std::string &message)
  : std::runtime_error(message) {}

  ~format_error() throw();
};

namespace internal {

// Casts nonnegative integer to unsigned.
template <typename Int>
constexpr typename std::make_unsigned<Int>::type to_unsigned(Int value) {
  FMT_ASSERT(value >= 0, "negative value");
  return static_cast<typename std::make_unsigned<Int>::type>(value);
}

// The number of characters to store in the basic_memory_buffer object itself
// to avoid dynamic memory allocation.
enum { INLINE_BUFFER_SIZE = 500 };

#if FMT_SECURE_SCL
// Use checked iterator to avoid warnings on MSVC.
template <typename T>
inline stdext::checked_array_iterator<T*> make_ptr(T *ptr, std::size_t size) {
  return stdext::checked_array_iterator<T*>(ptr, size);
}
#else
template <typename T>
inline T *make_ptr(T *ptr, std::size_t) { return ptr; }
#endif

#ifndef FMT_THROW
# if FMT_EXCEPTIONS
#  define FMT_THROW(x) throw x
# else
#  define FMT_THROW(x) assert(false)
# endif
#endif

struct error_handler {
  constexpr error_handler() {}
  constexpr error_handler(const error_handler &) {}

  // This function is intentionally not constexpr to give a compile-time error.
  void on_error(const char *message) {
    FMT_THROW(format_error(message));
  }
};
}  // namespace internal

/**
  \rst
  A contiguous memory buffer with an optional growing ability.
  \endrst
 */
template <typename T>
class basic_buffer {
 private:
  FMT_DISALLOW_COPY_AND_ASSIGN(basic_buffer);

  T *ptr_;
  std::size_t size_;
  std::size_t capacity_;

 protected:
  basic_buffer() FMT_NOEXCEPT : ptr_(0), size_(0), capacity_(0) {}

  /** Sets the buffer data and capacity. */
  void set(T* data, std::size_t capacity) FMT_NOEXCEPT {
    ptr_ = data;
    capacity_ = capacity;
  }

  /**
    \rst
    Increases the buffer capacity to hold at least *capacity* elements.
    \endrst
   */
  virtual void grow(std::size_t capacity) = 0;

 public:
  virtual ~basic_buffer() {}

  /** Returns the size of this buffer. */
  std::size_t size() const FMT_NOEXCEPT { return size_; }

  /** Returns the capacity of this buffer. */
  std::size_t capacity() const FMT_NOEXCEPT { return capacity_; }

  /** Returns a pointer to the buffer data. */
  T *data() FMT_NOEXCEPT { return ptr_; }

  /** Returns a pointer to the buffer data. */
  const T *data() const FMT_NOEXCEPT { return ptr_; }

  /**
    Resizes the buffer. If T is a POD type new elements may not be initialized.
   */
  void resize(std::size_t new_size) {
    reserve(new_size);
    size_ = new_size;
  }

  /**
    \rst
    Reserves space to store at least *capacity* elements.
    \endrst
   */
  void reserve(std::size_t capacity) {
    if (capacity > capacity_)
      grow(capacity);
  }

  void push_back(const T &value) {
    reserve(size_ + 1);
    ptr_[size_++] = value;
  }

  /** Appends data to the end of the buffer. */
  template <typename U>
  void append(const U *begin, const U *end);

  T &operator[](std::size_t index) { return ptr_[index]; }
  const T &operator[](std::size_t index) const { return ptr_[index]; }

  virtual std::locale locale() const { return std::locale(); }
};

typedef basic_buffer<char> buffer;
typedef basic_buffer<wchar_t> wbuffer;

template <typename T>
template <typename U>
void basic_buffer<T>::append(const U *begin, const U *end) {
  std::size_t new_size = size_ + internal::to_unsigned(end - begin);
  reserve(new_size);
  std::uninitialized_copy(begin, end,
                          internal::make_ptr(ptr_, capacity_) + size_);
  size_ = new_size;
}

template <typename Char>
inline std::basic_string<Char> to_string(const basic_buffer<Char>& buffer) {
  return std::basic_string<Char>(buffer.data(), buffer.size());
}

/**
  \rst
  A dynamically growing memory buffer for trivially copyable/constructible types
  with the first SIZE elements stored in the object itself.

  You can use one of the following typedefs for common character types:

  +----------------+------------------------------+
  | Type           | Definition                   |
  +================+==============================+
  | memory_buffer  | basic_memory_buffer<char>    |
  +----------------+------------------------------+
  | wmemory_buffer | basic_memory_buffer<wchar_t> |
  +----------------+------------------------------+

  **Example**::

     memory_buffer out;
     format_to(out, "The answer is {}.", 42);

  This will write the following output to the ``out`` object:

  .. code-block:: none

     The answer is 42.

  The output can be converted to an ``std::string`` with ``to_string(out)``.
  \endrst
 */
template <typename T, std::size_t SIZE = internal::INLINE_BUFFER_SIZE,
          typename Allocator = std::allocator<T> >
class basic_memory_buffer : private Allocator, public basic_buffer<T> {
 private:
  T store_[SIZE];

  // Deallocate memory allocated by the buffer.
  void deallocate() {
    T* data = this->data();
    if (data != store_) Allocator::deallocate(data, this->capacity());
  }

 protected:
  void grow(std::size_t size);

 public:
  explicit basic_memory_buffer(const Allocator &alloc = Allocator())
      : Allocator(alloc) {
    this->set(store_, SIZE);
  }
  ~basic_memory_buffer() { deallocate(); }

 private:
  // Move data from other to this buffer.
  void move(basic_memory_buffer &other) {
    Allocator &this_alloc = *this, &other_alloc = other;
    this_alloc = std::move(other_alloc);
    T* data = other.data();
    std::size_t size = other.size(), capacity = other.capacity();
    if (data == other.store_) {
      this->set(store_, capacity);
      std::uninitialized_copy(other.store_, other.store_ + size,
                              internal::make_ptr(store_, capacity));
    } else {
      this->set(data, capacity);
      // Set pointer to the inline array so that delete is not called
      // when deallocating.
      other.set(other.store_, 0);
    }
    this->resize(size);
  }

 public:
  /**
    \rst
    Constructs a :class:`fmt::basic_memory_buffer` object moving the content
    of the other object to it.
    \endrst
   */
  basic_memory_buffer(basic_memory_buffer &&other) {
    move(other);
  }

  /**
    \rst
    Moves the content of the other ``basic_memory_buffer`` object to this one.
    \endrst
   */
  basic_memory_buffer &operator=(basic_memory_buffer &&other) {
    assert(this != &other);
    deallocate();
    move(other);
    return *this;
  }

  // Returns a copy of the allocator associated with this buffer.
  Allocator get_allocator() const { return *this; }
};

template <typename T, std::size_t SIZE, typename Allocator>
void basic_memory_buffer<T, SIZE, Allocator>::grow(std::size_t size) {
  std::size_t old_capacity = this->capacity();
  std::size_t new_capacity = old_capacity + old_capacity / 2;
  if (size > new_capacity)
      new_capacity = size;
  T *old_data = this->data();
  T *new_data = this->allocate(new_capacity);
  // The following code doesn't throw, so the raw pointer above doesn't leak.
  std::uninitialized_copy(old_data, old_data + this->size(),
                          internal::make_ptr(new_data, new_capacity));
  this->set(new_data, new_capacity);
  // deallocate must not throw according to the standard, but even if it does,
  // the buffer already uses the new storage and will deallocate it in
  // destructor.
  if (old_data != store_)
    Allocator::deallocate(old_data, old_capacity);
}

typedef basic_memory_buffer<char> memory_buffer;
typedef basic_memory_buffer<wchar_t> wmemory_buffer;

/**
  \rst
  A fixed-size memory buffer. For a dynamically growing buffer use
  :class:`fmt::basic_memory_buffer`.

  Trying to increase the buffer size past the initial capacity will throw
  ``std::runtime_error``.
  \endrst
 */
template <typename Char>
class basic_fixed_buffer : public basic_buffer<Char> {
 public:
  /**
   \rst
   Constructs a :class:`fmt::basic_fixed_buffer` object for *array* of the
   given size.
   \endrst
   */
  basic_fixed_buffer(Char *array, std::size_t size) {
    this->set(array, size);
  }

  /**
   \rst
   Constructs a :class:`fmt::basic_fixed_buffer` object for *array* of the
   size known at compile time.
   \endrst
   */
  template <std::size_t SIZE>
  explicit basic_fixed_buffer(Char (&array)[SIZE]) {
    this->set(array, SIZE);
  }

 protected:
  FMT_API void grow(std::size_t size);
};

namespace internal {

template <typename Char>
class basic_char_traits {
 public:
  static Char cast(int value) { return static_cast<Char>(value); }
};

template <typename Char>
class char_traits;

template <>
class char_traits<char> : public basic_char_traits<char> {
 private:
  // Conversion from wchar_t to char is not allowed.
  static char convert(wchar_t);

 public:
  static char convert(char value) { return value; }

  // Formats a floating-point number.
  template <typename T>
  FMT_API static int format_float(char *buffer, std::size_t size,
      const char *format, unsigned width, int precision, T value);
};

template <>
class char_traits<wchar_t> : public basic_char_traits<wchar_t> {
 public:
  static wchar_t convert(char value) { return value; }
  static wchar_t convert(wchar_t value) { return value; }

  template <typename T>
  FMT_API static int format_float(wchar_t *buffer, std::size_t size,
      const wchar_t *format, unsigned width, int precision, T value);
};

template <typename Char>
class null_terminating_iterator;

template <typename Char>
constexpr const Char *pointer_from(null_terminating_iterator<Char> it);

// An iterator that produces a null terminator on *end. This simplifies parsing
// and allows comparing the performance of processing a null-terminated string
// vs string_view.
template <typename Char>
class null_terminating_iterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = Char;
  using pointer = const Char*;
  using reference = const Char&;
  using iterator_category = std::random_access_iterator_tag;

  null_terminating_iterator() : ptr_(0), end_(0) {}

  constexpr null_terminating_iterator(const Char *ptr, const Char *end)
    : ptr_(ptr), end_(end) {}

  template <typename Range>
  constexpr explicit null_terminating_iterator(const Range &r)
    : ptr_(r.begin()), end_(r.end()) {}

  null_terminating_iterator &operator=(const Char *ptr) {
    assert(ptr <= end_);
    ptr_ = ptr;
    return *this;
  }

  constexpr Char operator*() const {
    return ptr_ != end_ ? *ptr_ : 0;
  }

  constexpr null_terminating_iterator operator++() {
    ++ptr_;
    return *this;
  }

  constexpr null_terminating_iterator operator++(int) {
    null_terminating_iterator result(*this);
    ++ptr_;
    return result;
  }

  constexpr null_terminating_iterator operator--() {
    --ptr_;
    return *this;
  }

  constexpr null_terminating_iterator operator+(difference_type n) {
    return null_terminating_iterator(ptr_ + n, end_);
  }

  constexpr null_terminating_iterator operator-(difference_type n) {
    return null_terminating_iterator(ptr_ - n, end_);
  }

  constexpr null_terminating_iterator operator+=(difference_type n) {
    ptr_ += n;
    return *this;
  }

  constexpr difference_type operator-(null_terminating_iterator other) const {
    return ptr_ - other.ptr_;
  }

  constexpr bool operator!=(null_terminating_iterator other) const {
    return ptr_ != other.ptr_;
  }

  bool operator>=(null_terminating_iterator other) const {
    return ptr_ >= other.ptr_;
  }

  friend constexpr const Char *pointer_from<Char>(null_terminating_iterator it);

 private:
  const Char *ptr_;
  const Char *end_;
};

template <typename T>
constexpr const T *pointer_from(const T *p) { return p; }

template <typename Char>
constexpr const Char *pointer_from(null_terminating_iterator<Char> it) {
  return it.ptr_;
}

// Returns true if value is negative, false otherwise.
// Same as (value < 0) but doesn't produce warnings if T is an unsigned type.
template <typename T>
constexpr typename std::enable_if<
    std::numeric_limits<T>::is_signed, bool>::type is_negative(T value) {
  return value < 0;
}
template <typename T>
constexpr typename std::enable_if<
    !std::numeric_limits<T>::is_signed, bool>::type is_negative(T) {
  return false;
}

template <typename T>
struct int_traits {
  // Smallest of uint32_t and uint64_t that is large enough to represent
  // all values of T.
  typedef typename std::conditional<
    std::numeric_limits<T>::digits <= 32, uint32_t, uint64_t>::type main_type;
};

FMT_API FMT_NORETURN void report_unknown_type(char code, const char *type);

// Static data is placed in this class template to allow header-only
// configuration.
template <typename T = void>
struct FMT_API basic_data {
  static const uint32_t POWERS_OF_10_32[];
  static const uint64_t POWERS_OF_10_64[];
  static const char DIGITS[];
};

#ifndef FMT_USE_EXTERN_TEMPLATES
// Clang doesn't have a feature check for extern templates so we check
// for variadic templates which were introduced in the same version.
# define FMT_USE_EXTERN_TEMPLATES (__clang__)
#endif

#if FMT_USE_EXTERN_TEMPLATES && !defined(FMT_HEADER_ONLY)
extern template struct basic_data<void>;
#endif

typedef basic_data<> data;

#ifdef FMT_BUILTIN_CLZLL
// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
inline unsigned count_digits(uint64_t n) {
  // Based on http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
  // and the benchmark https://github.com/localvoid/cxx-benchmark-count-digits.
  int t = (64 - FMT_BUILTIN_CLZLL(n | 1)) * 1233 >> 12;
  return to_unsigned(t) - (n < data::POWERS_OF_10_64[t]) + 1;
}
#else
// Fallback version of count_digits used when __builtin_clz is not available.
inline unsigned count_digits(uint64_t n) {
  unsigned count = 1;
  for (;;) {
    // Integer division is slow so do it for a group of four digits instead
    // of for every digit. The idea comes from the talk by Alexandrescu
    // "Three Optimization Tips for C++". See speed-test for a comparison.
    if (n < 10) return count;
    if (n < 100) return count + 1;
    if (n < 1000) return count + 2;
    if (n < 10000) return count + 3;
    n /= 10000u;
    count += 4;
  }
}
#endif

#ifdef FMT_BUILTIN_CLZ
// Optional version of count_digits for better performance on 32-bit platforms.
inline unsigned count_digits(uint32_t n) {
  int t = (32 - FMT_BUILTIN_CLZ(n | 1)) * 1233 >> 12;
  return to_unsigned(t) - (n < data::POWERS_OF_10_32[t]) + 1;
}
#endif

// A functor that doesn't add a thousands separator.
struct no_thousands_sep {
  template <typename Char>
  void operator()(Char *) {}
};

// A functor that adds a thousands separator.
template <typename Char>
class add_thousands_sep {
 private:
  basic_string_view<Char> sep_;

  // Index of a decimal digit with the least significant digit having index 0.
  unsigned digit_index_;

 public:
  explicit add_thousands_sep(basic_string_view<Char> sep)
    : sep_(sep), digit_index_(0) {}

  void operator()(Char *&buffer) {
    if (++digit_index_ % 3 != 0)
      return;
    buffer -= sep_.size();
    std::uninitialized_copy(sep_.data(), sep_.data() + sep_.size(),
                            internal::make_ptr(buffer, sep_.size()));
  }
};

// Formats a decimal unsigned integer value writing into buffer.
// thousands_sep is a functor that is called after writing each char to
// add a thousands separator if necessary.
template <typename UInt, typename Char, typename ThousandsSep>
inline void format_decimal(Char *buffer, UInt value, unsigned num_digits,
                           ThousandsSep thousands_sep) {
  buffer += num_digits;
  while (value >= 100) {
    // Integer division is slow so do it for a group of two digits instead
    // of for every digit. The idea comes from the talk by Alexandrescu
    // "Three Optimization Tips for C++". See speed-test for a comparison.
    unsigned index = static_cast<unsigned>((value % 100) * 2);
    value /= 100;
    *--buffer = data::DIGITS[index + 1];
    thousands_sep(buffer);
    *--buffer = data::DIGITS[index];
    thousands_sep(buffer);
  }
  if (value < 10) {
    *--buffer = static_cast<char>('0' + value);
    return;
  }
  unsigned index = static_cast<unsigned>(value * 2);
  *--buffer = data::DIGITS[index + 1];
  thousands_sep(buffer);
  *--buffer = data::DIGITS[index];
}

template <typename UInt, typename Char>
inline void format_decimal(Char *buffer, UInt value, unsigned num_digits) {
  return format_decimal(buffer, value, num_digits, no_thousands_sep());
}

#ifndef _WIN32
# define FMT_USE_WINDOWS_H 0
#elif !defined(FMT_USE_WINDOWS_H)
# define FMT_USE_WINDOWS_H 1
#endif

// Define FMT_USE_WINDOWS_H to 0 to disable use of windows.h.
// All the functionality that relies on it will be disabled too.
#if FMT_USE_WINDOWS_H
// A converter from UTF-8 to UTF-16.
// It is only provided for Windows since other systems support UTF-8 natively.
class utf8_to_utf16 {
 private:
  wmemory_buffer buffer_;

 public:
  FMT_API explicit utf8_to_utf16(string_view s);
  operator wstring_view() const { return wstring_view(&buffer_[0], size()); }
  size_t size() const { return buffer_.size() - 1; }
  const wchar_t *c_str() const { return &buffer_[0]; }
  std::wstring str() const { return std::wstring(&buffer_[0], size()); }
};

// A converter from UTF-16 to UTF-8.
// It is only provided for Windows since other systems support UTF-8 natively.
class utf16_to_utf8 {
 private:
  memory_buffer buffer_;

 public:
  utf16_to_utf8() {}
  FMT_API explicit utf16_to_utf8(wstring_view s);
  operator string_view() const { return string_view(&buffer_[0], size()); }
  size_t size() const { return buffer_.size() - 1; }
  const char *c_str() const { return &buffer_[0]; }
  std::string str() const { return std::string(&buffer_[0], size()); }

  // Performs conversion returning a system error code instead of
  // throwing exception on conversion error. This method may still throw
  // in case of memory allocation error.
  FMT_API int convert(wstring_view s);
};

FMT_API void format_windows_error(fmt::buffer &out, int error_code,
                                  fmt::string_view message) FMT_NOEXCEPT;
#endif

template <typename T = void>
struct null {};

typedef char yes[1];
typedef char no[2];

yes &convert(unsigned long long);
no &convert(...);

template<typename T, bool ENABLE_CONVERSION>
struct convert_to_int_impl {
  enum { value = ENABLE_CONVERSION };
};

template<typename T, bool ENABLE_CONVERSION>
struct convert_to_int_impl2 {
  enum { value = false };
};

template<typename T>
struct convert_to_int_impl2<T, true> {
  enum {
    // Don't convert numeric types.
    value = convert_to_int_impl<
      T, !std::numeric_limits<T>::is_specialized>::value
  };
};

template<typename T>
struct convert_to_int {
  enum {
    enable_conversion = sizeof(convert(std::declval<T>())) == sizeof(yes)
  };
  enum { value = convert_to_int_impl2<T, enable_conversion>::value };
};

#define FMT_DISABLE_CONVERSION_TO_INT(Type) \
  template <> \
  struct convert_to_int<Type> { enum { value = 0 }; }

// Silence warnings about convering float to int.
FMT_DISABLE_CONVERSION_TO_INT(float);
FMT_DISABLE_CONVERSION_TO_INT(double);
FMT_DISABLE_CONVERSION_TO_INT(long double);

enum type {
  NONE, NAMED_ARG,
  // Integer types should go first,
  INT, UINT, LONG_LONG, ULONG_LONG, BOOL, CHAR, LAST_INTEGER_TYPE = CHAR,
  // followed by floating-point types.
  DOUBLE, LONG_DOUBLE, LAST_NUMERIC_TYPE = LONG_DOUBLE,
  CSTRING, STRING, POINTER, CUSTOM
};

constexpr bool is_integral(type t) {
  FMT_ASSERT(t != internal::NAMED_ARG, "invalid argument type");
  return t > internal::NONE && t <= internal::LAST_INTEGER_TYPE;
}

constexpr bool is_numeric(type t) {
  FMT_ASSERT(t != internal::NAMED_ARG, "invalid argument type");
  return t > internal::NONE && t <= internal::LAST_NUMERIC_TYPE;
}

template <typename Char>
struct string_value {
  const Char *value;
  std::size_t size;
};

template <typename Char>
struct custom_value {
  typedef void (*format_func)(
      basic_buffer<Char> &buffer, const void *arg, void *ctx);

  const void *value;
  format_func format;
};

template <typename Char>
struct named_arg;

template <typename T>
struct is_named_arg : std::false_type {};

template <typename Char>
struct is_named_arg<named_arg<Char>> : std::true_type {};

template <typename T>
constexpr type get_type() {
  return std::is_reference<T>::value || std::is_array<T>::value ?
        get_type<typename std::decay<T>::type>() :
        (is_named_arg<T>::value ?
           NAMED_ARG : (convert_to_int<T>::value ? INT : CUSTOM));
}

template <> constexpr type get_type<bool>() { return BOOL; }
template <> constexpr type get_type<short>() { return INT; }
template <> constexpr type get_type<unsigned short>() { return UINT; }
template <> constexpr type get_type<int>() { return INT; }
template <> constexpr type get_type<unsigned>() { return UINT; }
template <> constexpr type get_type<long>() {
  return sizeof(long) == sizeof(int) ? INT : LONG_LONG;
}
template <> constexpr type get_type<unsigned long>() {
  return sizeof(unsigned long) == sizeof(unsigned) ? UINT : ULONG_LONG;
}
template <> constexpr type get_type<long long>() { return LONG_LONG; }
template <> constexpr type get_type<unsigned long long>() { return ULONG_LONG; }
template <> constexpr type get_type<float>() { return DOUBLE; }
template <> constexpr type get_type<double>() { return DOUBLE; }
template <> constexpr type get_type<long double>() { return LONG_DOUBLE; }
template <> constexpr type get_type<signed char>() { return INT; }
template <> constexpr type get_type<unsigned char>() { return UINT; }
template <> constexpr type get_type<char>() { return CHAR; }

#if !defined(_MSC_VER) || defined(_NATIVE_WCHAR_T_DEFINED)
template <> constexpr type get_type<wchar_t>() { return CHAR; }
#endif

template <> constexpr type get_type<char *>() { return CSTRING; }
template <> constexpr type get_type<const char *>() { return CSTRING; }
template <> constexpr type get_type<signed char *>() { return CSTRING; }
template <> constexpr type get_type<const signed char *>() { return CSTRING; }
template <> constexpr type get_type<unsigned char *>() { return CSTRING; }
template <> constexpr type get_type<const unsigned char *>() { return CSTRING; }
template <> constexpr type get_type<std::string>() { return STRING; }
template <> constexpr type get_type<string_view>() { return STRING; }
template <> constexpr type get_type<wchar_t *>() { return CSTRING; }
template <> constexpr type get_type<const wchar_t *>() { return CSTRING; }
template <> constexpr type get_type<std::wstring>() { return STRING; }
template <> constexpr type get_type<wstring_view>() { return STRING; }
template <> constexpr type get_type<void *>() { return POINTER; }
template <> constexpr type get_type<const void *>() { return POINTER; }
template <> constexpr type get_type<std::nullptr_t>() { return POINTER; }

// Formatting of wide characters and strings into a narrow output is disallowed:
//   fmt::format("{}", L"test"); // error
// To fix this, use a wide format string:
//   fmt::format(L"{}", L"test");
template <typename Char>
inline void require_wchar() {
  static_assert(
      std::is_same<wchar_t, Char>::value,
      "formatting of wide characters into a narrow output is disallowed");
}

template <typename T>
inline const T *as_const(T *p) { return p; }

// A formatting argument value.
template <typename Context>
class value {
 public:
  using char_type = typename Context::char_type;

  union {
    int int_value;
    unsigned uint_value;
    long long long_long_value;
    unsigned long long ulong_long_value;
    double double_value;
    long double long_double_value;
    const void *pointer;
    string_value<char_type> string;
    string_value<signed char> sstring;
    string_value<unsigned char> ustring;
    custom_value<char_type> custom;
  };

  constexpr value() : int_value(0) {}
  value(bool val) { set<BOOL>(int_value, val); }
  value(short val) { set<INT>(int_value, val); }
  value(unsigned short val) { set<UINT>(uint_value, val); }
  constexpr value(int val) : int_value(val) {}
  value(unsigned val) { set<UINT>(uint_value, val); }

  value(long val) {
    // To minimize the number of types we need to deal with, long is
    // translated either to int or to long long depending on its size.
    if (const_check(sizeof(val) == sizeof(int)))
      int_value = static_cast<int>(val);
    else
      long_long_value = val;
  }

  value(unsigned long val) {
    if (const_check(sizeof(val) == sizeof(unsigned)))
      uint_value = static_cast<unsigned>(val);
    else
      ulong_long_value = val;
  }

  value(long long val) { set<LONG_LONG>(long_long_value, val); }
  value(unsigned long long val) { set<ULONG_LONG>(ulong_long_value, val); }
  value(float val) { set<DOUBLE>(double_value, val); }
  value(double val) { set<DOUBLE>(double_value, val); }
  value(long double val) { set<LONG_DOUBLE>(long_double_value, val); }
  value(signed char val) { set<INT>(int_value, val); }
  value(unsigned char val) { set<UINT>(uint_value, val); }
  value(char val) { set<CHAR>(int_value, val); }

#if !defined(_MSC_VER) || defined(_NATIVE_WCHAR_T_DEFINED)
  value(wchar_t value) {
    require_wchar<char_type>();
    set<CHAR>(int_value, value);
  }
#endif

  // Formatting of wide strings into a narrow buffer and multibyte strings
  // into a wide buffer is disallowed (https://github.com/fmtlib/fmt/pull/606).
  value(char_type *s) { set<CSTRING>(string.value, s); }
  value(const char_type *s) { set<CSTRING>(string.value, s); }
  value(signed char *s) { set_cstring(sstring.value, s); }
  value(const signed char *s) { set_cstring(sstring.value, s); }
  value(unsigned char *s) { set_cstring(ustring.value, s); }
  value(const unsigned char *s) { set_cstring(ustring.value, s); }
  value(basic_string_view<char_type> s) { set_string(s); }
  value(const std::basic_string<char_type> &s) { set_string(s); }

  // Formatting of arbitrary pointers is disallowed. If you want to output a
  // pointer cast it to "void *" or "const void *". In particular, this forbids
  // formatting of "[const] volatile char *" which is printed as bool by
  // iostreams.
  template <typename T>
  value(const T *p) {
    static_assert(std::is_same<T, void>::value,
                  "formatting of non-void pointers is disallowed");
    set<POINTER>(pointer, p);
  }

  template <typename T>
  value(T *p) : value(as_const(p)) {}

  value(std::nullptr_t) { pointer = nullptr; }

  template <typename T>
  value(const T &value,
        typename std::enable_if<convert_to_int<T>::value, int>::type = 0) {
    static_assert(get_type<T>() == INT, "invalid type");
    int_value = value;
  }

  template <typename T>
  value(const T &value,
        typename std::enable_if<!convert_to_int<T>::value, int>::type = 0) {
    static_assert(get_type<T>() == CUSTOM, "invalid type");
    custom.value = &value;
    custom.format = &format_custom_arg<T>;
  }

  // Additional template param `Char` is needed here because get_type always
  // uses char.
  template <typename Char>
  value(const named_arg<Char> &value) {
    static_assert(
      get_type<const named_arg<Char> &>() == NAMED_ARG, "invalid type");
    pointer = &value;
  }

 private:
  template <type TYPE, typename T, typename U>
  constexpr void set(T &field, const U &value) {
    static_assert(get_type<U>() == TYPE, "invalid type");
    field = value;
  }

  template <typename T>
  void set_string(const T &value) {
    static_assert(get_type<T>() == STRING, "invalid type");
    string.value = value.data();
    string.size = value.size();
  }

  template <typename T, typename U>
  constexpr void set_cstring(T &field, const U *str) {
    static_assert(std::is_same<char, char_type>::value,
                  "incompatible string types");
    set<CSTRING>(field, str);
  }

  // Formats an argument of a custom type, such as a user-defined class.
  template <typename T>
  static void format_custom_arg(
      basic_buffer<char_type> &buffer, const void *arg, void *context) {
    Context &ctx = *static_cast<Context*>(context);
    // Get the formatter type through the context to allow different contexts
    // have different extension points, e.g. `formatter<T>` for `format` and
    // `printf_formatter<T>` for `printf`.
    typename Context::template formatter_type<T> f;
    auto &&parse_ctx = ctx.parse_context();
    parse_ctx.advance_to(f.parse(parse_ctx));
    f.format(buffer, *static_cast<const T*>(arg), ctx);
  }
};

template <typename Context>
class arg_map;

template <typename Context, typename T>
constexpr basic_arg<Context> make_arg(const T &value);
}  // namespace internal

struct monostate {};

template <typename Context>
class basic_args;

// A formatting argument. It is a trivially copyable/constructible type to
// allow storage in basic_memory_buffer.
template <typename Context>
class basic_arg {
 private:
  internal::value<Context> value_;
  internal::type type_;

  template <typename ContextType, typename T>
  friend constexpr basic_arg<ContextType> internal::make_arg(const T &value);

  template <typename Visitor, typename Ctx>
  friend constexpr typename std::result_of<Visitor(int)>::type
    visit(Visitor &&vis, basic_arg<Ctx> arg);

  friend class basic_args<Context>;
  friend class internal::arg_map<Context>;

 public:
  constexpr basic_arg() : type_(internal::NONE) {}

  explicit operator bool() const noexcept { return type_ != internal::NONE; }

  internal::type type() const { return type_; }

  bool is_integral() const { return internal::is_integral(type_); }
  bool is_numeric() const { return internal::is_numeric(type_); }
  bool is_pointer() const { return type_ == internal::POINTER; }
};

/**
  \rst
  Visits an argument dispatching to the appropriate visit method based on
  the argument type. For example, if the argument type is ``double`` then
  ``vis(value)`` will be called with the value of type ``double``.
  \endrst
 */
template <typename Visitor, typename Context>
constexpr typename std::result_of<Visitor(int)>::type
    visit(Visitor &&vis, basic_arg<Context> arg) {
  typedef typename Context::char_type Char;
  switch (arg.type_) {
  case internal::NONE:
    return vis(monostate());
  case internal::NAMED_ARG:
    FMT_ASSERT(false, "invalid argument type");
    break;
  case internal::INT:
    return vis(arg.value_.int_value);
  case internal::UINT:
    return vis(arg.value_.uint_value);
  case internal::LONG_LONG:
    return vis(arg.value_.long_long_value);
  case internal::ULONG_LONG:
    return vis(arg.value_.ulong_long_value);
  case internal::BOOL:
    return vis(arg.value_.int_value != 0);
  case internal::CHAR:
    return vis(static_cast<Char>(arg.value_.int_value));
  case internal::DOUBLE:
    return vis(arg.value_.double_value);
  case internal::LONG_DOUBLE:
    return vis(arg.value_.long_double_value);
  case internal::CSTRING:
    return vis(arg.value_.string.value);
  case internal::STRING:
    return vis(basic_string_view<Char>(
                 arg.value_.string.value, arg.value_.string.size));
  case internal::POINTER:
    return vis(arg.value_.pointer);
  case internal::CUSTOM:
    return vis(arg.value_.custom);
  }
  return typename std::result_of<Visitor(int)>::type();
}

namespace internal {

template <typename Context, typename T>
constexpr basic_arg<Context> make_arg(const T &value) {
  basic_arg<Context> arg;
  arg.type_ = get_type<T>();
  arg.value_ = value;
  return arg;
}

#if FMT_GCC_VERSION >= 407
# define FMT_UNUSED __attribute__((unused))
#else
# define FMT_UNUSED
#endif

template <typename Context>
struct named_arg : basic_arg<Context> {
  typedef typename Context::char_type Char;

  basic_string_view<Char> name;

  template <typename T>
  named_arg(basic_string_view<Char> argname, const T &value)
  : basic_arg<Context>(make_arg<Context>(value)), name(argname) {}
};

template <typename Arg, typename... Args>
constexpr uint64_t get_types() {
  return get_type<Arg>() | (get_types<Args...>() << 4);
}

template <>
constexpr uint64_t get_types<void>() { return 0; }

// Maximum number of arguments with packed types.
enum { MAX_PACKED_ARGS = 15 };

template <bool IS_PACKED, typename Context, typename T>
inline typename std::enable_if<IS_PACKED, value<Context>>::type
    make_arg(const T& value) {
  return value;
}

template <bool IS_PACKED, typename Context, typename T>
inline typename std::enable_if<!IS_PACKED, basic_arg<Context>>::type
    make_arg(const T& value) {
  return make_arg<Context>(value);
}
}  // namespace internal

template <typename Context, typename ...Args>
class arg_store {
 private:
  static const size_t NUM_ARGS = sizeof...(Args);

  // Packed is a macro on MinGW so use IS_PACKED instead.
  static const bool IS_PACKED = NUM_ARGS < internal::MAX_PACKED_ARGS;

  typedef typename Context::char_type char_type;

  typedef typename std::conditional<IS_PACKED,
    internal::value<Context>, basic_arg<Context>>::type value_type;

  // If the arguments are not packed, add one more element to mark the end.
  typedef std::array<value_type, NUM_ARGS + (IS_PACKED ? 0 : 1)> Array;
  Array data_;

 public:
  static const uint64_t TYPES = IS_PACKED ?
      internal::get_types<Args..., void>() : -static_cast<int64_t>(NUM_ARGS);

  arg_store(const Args &... args)
    : data_(Array{{internal::make_arg<IS_PACKED, Context>(args)...}}) {}

  const value_type *data() const { return data_.data(); }
};

template <typename Context, typename ...Args>
inline arg_store<Context, Args...> make_args(const Args & ... args) {
  return arg_store<Context, Args...>(args...);
}

template <typename ...Args>
inline arg_store<context, Args...> make_args(const Args & ... args) {
  return arg_store<context, Args...>(args...);
}

/** Formatting arguments. */
template <typename Context>
class basic_args {
 public:
  typedef unsigned size_type;
  typedef basic_arg<Context> format_arg;

 private:
  // To reduce compiled code size per formatting function call, types of first
  // MAX_PACKED_ARGS arguments are passed in the types_ field.
  uint64_t types_;
  union {
    // If the number of arguments is less than MAX_PACKED_ARGS, the argument
    // values are stored in values_, otherwise they are stored in args_.
    // This is done to reduce compiled code size as storing larger objects
    // may require more code (at least on x86-64) even if the same amount of
    // data is actually copied to stack. It saves ~10% on the bloat test.
    const internal::value<Context> *values_;
    const format_arg *args_;
  };

  typename internal::type type(unsigned index) const {
    unsigned shift = index * 4;
    uint64_t mask = 0xf;
    return static_cast<typename internal::type>(
      (types_ & (mask << shift)) >> shift);
  }

  friend class internal::arg_map<Context>;

  void set_data(const internal::value<Context> *values) { values_ = values; }
  void set_data(const format_arg *args) { args_ = args; }

  format_arg get(size_type index) const {
    int64_t signed_types = static_cast<int64_t>(types_);
    if (signed_types < 0) {
      uint64_t num_args = -signed_types;
      return index < num_args ? args_[index] : format_arg();
    }
    format_arg arg;
    if (index > internal::MAX_PACKED_ARGS)
      return arg;
    arg.type_ = type(index);
    if (arg.type_ == internal::NONE)
      return arg;
    internal::value<Context> &val = arg.value_;
    val = values_[index];
    return arg;
  }

 public:
  basic_args() : types_(0) {}

  template <typename... Args>
  basic_args(const arg_store<Context, Args...> &store)
  : types_(store.TYPES) {
    set_data(store.data());
  }

  /** Returns the argument at specified index. */
  format_arg operator[](size_type index) const {
    format_arg arg = get(index);
    return arg.type_ == internal::NAMED_ARG ?
      *static_cast<const format_arg*>(arg.value_.pointer) : arg;
  }
};

typedef basic_args<context> args;
typedef basic_args<wcontext> wargs;

enum alignment {
  ALIGN_DEFAULT, ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER, ALIGN_NUMERIC
};

// Flags.
enum {
  SIGN_FLAG = 1, PLUS_FLAG = 2, MINUS_FLAG = 4, HASH_FLAG = 8,
  CHAR_FLAG = 0x10  // Argument has char type - used in error reporting.
};

enum format_spec_tag {fill_tag, align_tag, width_tag, type_tag};

// Format specifier.
template <typename T, format_spec_tag>
class format_spec {
 private:
  T value_;

 public:
  typedef T value_type;

  explicit format_spec(T value) : value_(value) {}

  T value() const { return value_; }
};

// template <typename Char>
// using fill_spec = format_spec<Char, fill_tag>;
template <typename Char>
class fill_spec : public format_spec<Char, fill_tag> {
 public:
  explicit fill_spec(Char value) : format_spec<Char, fill_tag>(value) {}
};

typedef format_spec<unsigned, width_tag> width_spec;
typedef format_spec<char, type_tag> type_spec;

// An empty format specifier.
struct empty_spec {};

// An alignment specifier.
struct align_spec : empty_spec {
  unsigned width_;
  // Fill is always wchar_t and cast to char if necessary to avoid having
  // two specialization of AlignSpec and its subclasses.
  wchar_t fill_;
  alignment align_;

  constexpr align_spec(
      unsigned width, wchar_t fill, alignment align = ALIGN_DEFAULT)
  : width_(width), fill_(fill), align_(align) {}

  constexpr unsigned width() const { return width_; }
  constexpr wchar_t fill() const { return fill_; }
  constexpr alignment align() const { return align_; }

  int precision() const { return -1; }
};

// Format specifiers.
template <typename Char>
class basic_format_specs : public align_spec {
 private:
  template <typename FillChar>
  typename std::enable_if<std::is_same<FillChar, Char>::value ||
                          std::is_same<FillChar, char>::value, void>::type
      set(fill_spec<FillChar> fill) {
    fill_ = fill.value();
  }

  void set(width_spec width) {
    width_ = width.value();
  }

  void set(type_spec type) {
    type_ = type.value();
  }

  template <typename Spec, typename... Specs>
  void set(Spec spec, Specs... tail) {
    set(spec);
    set(tail...);
  }

 public:
  unsigned flags_;
  int precision_;
  Char type_;

  constexpr basic_format_specs(
      unsigned width = 0, char type = 0, wchar_t fill = ' ')
  : align_spec(width, fill), flags_(0), precision_(-1), type_(type) {}

  template <typename... FormatSpecs>
  explicit basic_format_specs(FormatSpecs... specs)
    : align_spec(0, ' '), flags_(0), precision_(-1), type_(0){
    set(specs...);
  }

  constexpr bool flag(unsigned f) const { return (flags_ & f) != 0; }
  constexpr int precision() const { return precision_; }
  constexpr Char type() const { return type_; }
};

typedef basic_format_specs<char> format_specs;

// Parsing context consisting of a format string range being parsed and an
// argument counter for automatic indexing.
template <typename Char, typename ErrorHandler = internal::error_handler>
class basic_parse_context : private ErrorHandler {
 private:
  basic_string_view<Char> format_str_;
  int next_arg_index_;

 protected:
  constexpr bool check_no_auto_index() {
    if (next_arg_index_ > 0) {
      on_error("cannot switch from automatic to manual argument indexing");
      return false;
    }
    next_arg_index_ = -1;
    return true;
  }

 public:
  using char_type = Char;
  using iterator = typename basic_string_view<Char>::iterator;

  explicit constexpr basic_parse_context(
      basic_string_view<Char> format_str, ErrorHandler eh = ErrorHandler())
    : ErrorHandler(eh), format_str_(format_str), next_arg_index_(0) {}

  // Returns an iterator to the beginning of the format string range being
  // parsed.
  constexpr iterator begin() const { return format_str_.begin(); }

  // Returns an iterator past the end of the format string range being parsed.
  constexpr iterator end() const { return format_str_.end(); }

  // Advances the begin iterator to ``it``.
  constexpr void advance_to(iterator it) {
    format_str_.remove_prefix(it - begin());
  }

  // Returns the next argument index.
  constexpr unsigned next_arg_index() {
    if (next_arg_index_ >= 0)
      return internal::to_unsigned(next_arg_index_++);
    on_error("cannot switch from manual to automatic argument indexing");
    return 0;
  }

  constexpr void check_arg_id(unsigned) { check_no_auto_index(); }
  void check_arg_id(basic_string_view<Char>) {}

  constexpr void on_error(const char *message) {
    ErrorHandler::on_error(message);
  }

  constexpr ErrorHandler error_handler() const { return *this; }
};

using parse_context = basic_parse_context<char>;
using wparse_context = basic_parse_context<wchar_t>;

namespace internal {

template <typename Handler>
constexpr void handle_integral_type_spec(char c, Handler &&handler) {
  switch (c) {
  case 0: case 'd':
    handler.on_dec();
    break;
  case 'x': case 'X':
    handler.on_hex();
    break;
  case 'b': case 'B':
    handler.on_bin();
    break;
  case 'o':
    handler.on_oct();
    break;
  case 'n':
    handler.on_num();
    break;
  default:
    handler.on_error();
  }
}

template <typename ErrorHandler>
class int_type_checker : private ErrorHandler {
 public:
  constexpr int_type_checker(ErrorHandler eh) : ErrorHandler(eh) {}

  constexpr void on_dec() {}
  constexpr void on_hex() {}
  constexpr void on_bin() {}
  constexpr void on_oct() {}
  constexpr void on_num() {}

  constexpr void on_error() {
    ErrorHandler::on_error("invalid type specifier");
  }
};

template <typename Context>
class arg_map {
 private:
  typedef typename Context::char_type Char;
  typedef std::vector<
    std::pair<fmt::basic_string_view<Char>, basic_arg<Context> > > MapType;
  typedef typename MapType::value_type Pair;

  MapType map_;

 public:
  void init(const basic_args<Context> &args);

  const basic_arg<Context>
      *find(const fmt::basic_string_view<Char> &name) const {
    // The list is unsorted, so just return the first matching name.
    for (typename MapType::const_iterator it = map_.begin(), end = map_.end();
         it != end; ++it) {
      if (it->first == name)
        return &it->second;
    }
    return 0;
  }
};

template <typename Context>
void arg_map<Context>::init(const basic_args<Context> &args) {
  if (!map_.empty())
    return;
  typedef internal::named_arg<Context> NamedArg;
  const NamedArg *named_arg = 0;
  bool use_values =
  args.type(MAX_PACKED_ARGS - 1) == internal::NONE;
  if (use_values) {
    for (unsigned i = 0;/*nothing*/; ++i) {
      internal::type arg_type = args.type(i);
      switch (arg_type) {
        case internal::NONE:
          return;
        case internal::NAMED_ARG:
          named_arg = static_cast<const NamedArg*>(args.values_[i].pointer);
          map_.push_back(Pair(named_arg->name, *named_arg));
          break;
        default:
          /*nothing*/;
      }
    }
    return;
  }
  for (unsigned i = 0; i != MAX_PACKED_ARGS; ++i) {
    internal::type arg_type = args.type(i);
    if (arg_type == internal::NAMED_ARG) {
      named_arg = static_cast<const NamedArg*>(args.args_[i].value_.pointer);
      map_.push_back(Pair(named_arg->name, *named_arg));
    }
  }
  for (unsigned i = MAX_PACKED_ARGS; ; ++i) {
    switch (args.args_[i].type_) {
      case internal::NONE:
        return;
      case internal::NAMED_ARG:
        named_arg = static_cast<const NamedArg*>(args.args_[i].value_.pointer);
        map_.push_back(Pair(named_arg->name, *named_arg));
        break;
      default:
        /*nothing*/;
    }
  }
}

template <typename Char>
class arg_formatter_base {
 public:
  typedef basic_format_specs<Char> format_specs;

 private:
  basic_writer<Char> writer_;
  format_specs &spec_;

  FMT_DISALLOW_COPY_AND_ASSIGN(arg_formatter_base);

  void write_pointer(const void *p) {
    spec_.flags_ = HASH_FLAG;
    spec_.type_ = 'x';
    writer_.write_int(reinterpret_cast<uintptr_t>(p), spec_);
  }

  template <typename StrChar>
  typename std::enable_if<
    std::is_same<Char, wchar_t>::value &&
    std::is_same<StrChar, wchar_t>::value>::type
      write_str(basic_string_view<StrChar> value) {
    writer_.write_str(value, spec_);
  }

  template <typename StrChar>
  typename std::enable_if<
    !std::is_same<Char, wchar_t>::value ||
    !std::is_same<StrChar, wchar_t>::value>::type
      write_str(basic_string_view<StrChar> ) {
    // Do nothing.
  }

 protected:
  basic_writer<Char> &writer() { return writer_; }
  format_specs &spec() { return spec_; }

  void write(bool value) {
    writer_.write_str(string_view(value ? "true" : "false"), spec_);
  }

  void write(const char *value) {
    writer_.write_str(
          string_view(value, value != 0 ? std::strlen(value) : 0), spec_);
  }

 public:
  typedef Char char_type;

  arg_formatter_base(basic_buffer<Char> &b, format_specs &s)
  : writer_(b), spec_(s) {}

  void operator()(monostate) {
    FMT_ASSERT(false, "invalid argument type");
  }

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value>::type
      operator()(T value) { writer_.write_int(value, spec_); }

  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value>::type
      operator()(T value) { writer_.write_double(value, spec_); }

  void operator()(bool value) {
    if (spec_.type_)
      return (*this)(value ? 1 : 0);
    write(value);
  }

  void operator()(Char value) {
    if (spec_.type_ && spec_.type_ != 'c') {
      spec_.flags_ |= CHAR_FLAG;
      writer_.write_int(value, spec_);
      return;
    }
    if (spec_.align_ == ALIGN_NUMERIC || spec_.flags_ != 0)
      FMT_THROW(format_error("invalid format specifier for char"));
    typedef typename basic_writer<Char>::pointer_type pointer_type;
    Char fill = internal::char_traits<Char>::cast(spec_.fill());
    pointer_type out = pointer_type();
    const unsigned CHAR_WIDTH = 1;
    if (spec_.width_ > CHAR_WIDTH) {
      out = writer_.grow_buffer(spec_.width_);
      if (spec_.align_ == ALIGN_RIGHT) {
        std::uninitialized_fill_n(out, spec_.width_ - CHAR_WIDTH, fill);
        out += spec_.width_ - CHAR_WIDTH;
      } else if (spec_.align_ == ALIGN_CENTER) {
        out = writer_.fill_padding(out, spec_.width_,
                                   internal::const_check(CHAR_WIDTH), fill);
      } else {
        std::uninitialized_fill_n(out + CHAR_WIDTH,
                                  spec_.width_ - CHAR_WIDTH, fill);
      }
    } else {
      out = writer_.grow_buffer(CHAR_WIDTH);
    }
    *out = internal::char_traits<Char>::cast(value);
  }

  void operator()(const char *value) {
    if (spec_.type_ == 'p')
      return write_pointer(value);
    write(value);
  }

  void operator()(string_view value) {
    writer_.write_str(value, spec_);
  }

  void operator()(basic_string_view<wchar_t> value) {
    write_str(value);
  }

  void operator()(const void *value) {
    if (spec_.type_ && spec_.type_ != 'p')
      report_unknown_type(spec_.type_, "pointer");
    write_pointer(value);
  }
};

template <typename Char, typename Context>
class context_base : public basic_parse_context<Char>{
 private:
  basic_args<Context> args_;

 protected:
  typedef basic_arg<Context> format_arg;

  context_base(basic_string_view<Char> format_str, basic_args<Context> args)
  : basic_parse_context<Char>(format_str), args_(args) {}
  ~context_base() {}

  basic_args<Context> args() const { return args_; }

  // Returns the argument with specified index.
  format_arg do_get_arg(unsigned arg_index) {
    format_arg arg = args_[arg_index];
    if (!arg)
      this->on_error("argument index out of range");
    return arg;
  }

  // Checks if manual indexing is used and returns the argument with
  // specified index.
  format_arg get_arg(unsigned arg_index) {
    return this->check_no_auto_index() ?
      this->do_get_arg(arg_index) : format_arg();
  }

 public:
  basic_parse_context<Char> &parse_context() { return *this; }
};

struct format_string {};

template <typename Char>
constexpr bool is_name_start(Char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || '_' == c;
}

// Parses the input as an unsigned integer. This function assumes that the
// first character is a digit and presence of a non-digit character at the end.
// it: an iterator pointing to the beginning of the input range.
template <typename Iterator, typename ErrorHandler>
constexpr unsigned parse_nonnegative_int(Iterator &it, ErrorHandler &&eh) {
  assert('0' <= *it && *it <= '9');
  unsigned value = 0;
  // Convert to unsigned to prevent a warning.
  unsigned max_int = (std::numeric_limits<int>::max)();
  unsigned big = max_int / 10;
  do {
    // Check for overflow.
    if (value > big) {
      value = max_int + 1;
      break;
    }
    value = value * 10 + (*it - '0');
    // Workaround for MSVC "setup_exception stack overflow" error:
    auto next = it;
    ++next;
    it = next;
  } while ('0' <= *it && *it <= '9');
  if (value > max_int)
    eh.on_error("number is too big");
  return value;
}

template <typename Char, typename Context>
class custom_formatter {
 private:
  basic_buffer<Char> &buffer_;
  Context &ctx_;

 public:
  custom_formatter(basic_buffer<Char> &buffer, Context &ctx)
  : buffer_(buffer), ctx_(ctx) {}

  bool operator()(internal::custom_value<Char> custom) {
    custom.format(buffer_, custom.value, &ctx_);
    return true;
  }

  template <typename T>
  bool operator()(T) { return false; }
};

template <typename T>
struct is_integer {
  enum {
    value = std::is_integral<T>::value && !std::is_same<T, bool>::value &&
            !std::is_same<T, char>::value && !std::is_same<T, wchar_t>::value
  };
};

template <typename ErrorHandler>
class width_checker {
 public:
  explicit constexpr width_checker(ErrorHandler &eh) : handler_(eh) {}

  template <typename T>
  constexpr typename std::enable_if<
      is_integer<T>::value, unsigned long long>::type operator()(T value) {
    if (is_negative(value))
      handler_.on_error("negative width");
    return value;
  }

  template <typename T>
  constexpr typename std::enable_if<
      !is_integer<T>::value, unsigned long long>::type operator()(T) {
    handler_.on_error("width is not integer");
    return 0;
  }

 private:
  ErrorHandler &handler_;
};

template <typename ErrorHandler>
class precision_checker {
 public:
  explicit constexpr precision_checker(ErrorHandler &eh) : handler_(eh) {}

  template <typename T>
  constexpr typename std::enable_if<
      is_integer<T>::value, unsigned long long>::type operator()(T value) {
    if (is_negative(value))
      handler_.on_error("negative precision");
    return value;
  }

  template <typename T>
  constexpr typename std::enable_if<
      !is_integer<T>::value, unsigned long long>::type operator()(T) {
    handler_.on_error("precision is not integer");
    return 0;
  }

 private:
  ErrorHandler &handler_;
};

// A format specifier handler that sets fields in basic_format_specs.
template <typename Char>
class specs_setter {
 public:
  explicit constexpr specs_setter(basic_format_specs<Char> &specs):
    specs_(specs) {}

  constexpr specs_setter(const specs_setter &other) : specs_(other.specs_) {}

  constexpr void on_align(alignment align) { specs_.align_ = align; }
  constexpr void on_fill(Char fill) { specs_.fill_ = fill; }
  constexpr void on_plus() { specs_.flags_ |= SIGN_FLAG | PLUS_FLAG; }
  constexpr void on_minus() { specs_.flags_ |= MINUS_FLAG; }
  constexpr void on_space() { specs_.flags_ |= SIGN_FLAG; }
  constexpr void on_hash() { specs_.flags_ |= HASH_FLAG; }

  constexpr void on_zero() {
    specs_.align_ = ALIGN_NUMERIC;
    specs_.fill_ = '0';
  }

  constexpr void on_width(unsigned width) { specs_.width_ = width; }
  constexpr void on_precision(unsigned precision) {
    specs_.precision_ = precision;
  }
  constexpr void end_precision() {}

  constexpr void on_type(Char type) { specs_.type_ = type; }

 protected:
  basic_format_specs<Char> &specs_;
};

// A format specifier handler that checks if specifiers are consistent with the
// argument type.
template <typename Handler>
class specs_checker : public Handler {
 public:
  constexpr specs_checker(const Handler& handler, internal::type arg_type)
    : Handler(handler), arg_type_(arg_type) {}

  constexpr specs_checker(const specs_checker &other)
    : Handler(other), arg_type_(other.arg_type_) {}

  constexpr void on_align(alignment align) {
    if (align == ALIGN_NUMERIC)
      require_numeric_argument();
    Handler::on_align(align);
  }

  constexpr void on_plus() {
    check_sign();
    Handler::on_plus();
  }

  constexpr void on_minus() {
    check_sign();
    Handler::on_minus();
  }

  constexpr void on_space() {
    check_sign();
    Handler::on_space();
  }

  constexpr void on_hash() {
    require_numeric_argument();
    Handler::on_hash();
  }

  constexpr void on_zero() {
    require_numeric_argument();
    Handler::on_zero();
  }

  constexpr void end_precision() {
    if (is_integral(arg_type_) || arg_type_ == POINTER)
      this->on_error("precision not allowed for this argument type");
  }

 private:
  constexpr void require_numeric_argument() {
    if (!is_numeric(arg_type_))
      this->on_error("format specifier requires numeric argument");
  }

  constexpr void check_sign() {
    require_numeric_argument();
    if (is_integral(arg_type_) && arg_type_ != INT && arg_type_ != LONG_LONG &&
        arg_type_ != CHAR) {
      this->on_error("format specifier requires signed argument");
    }
  }

  internal::type arg_type_;
};

template <template <typename> class Handler, typename T,
          typename Context, typename ErrorHandler>
constexpr void set_dynamic_spec(
    T &value, basic_arg<Context> arg, ErrorHandler eh) {
  unsigned long long big_value = visit(Handler<ErrorHandler>(eh), arg);
  if (big_value > (std::numeric_limits<int>::max)())
    eh.on_error("number is too big");
  value = static_cast<int>(big_value);
}

struct auto_id {};

// The standard format specifier handler with checking.
template <typename Context>
class specs_handler: public specs_setter<typename Context::char_type> {
 public:
  typedef typename Context::char_type char_type;

  constexpr specs_handler(basic_format_specs<char_type> &specs, Context &ctx)
    : specs_setter<char_type>(specs), context_(ctx) {}

  template <typename Id>
  constexpr void on_dynamic_width(Id arg_id) {
    set_dynamic_spec<width_checker>(
          this->specs_.width_, get_arg(arg_id), context_.error_handler());
  }

  template <typename Id>
  constexpr void on_dynamic_precision(Id arg_id) {
    set_dynamic_spec<precision_checker>(
          this->specs_.precision_, get_arg(arg_id), context_.error_handler());
  }

  void on_error(const char *message) {
    context_.on_error(message);
  }

 private:
  constexpr basic_arg<Context> get_arg(auto_id) {
    return context_.next_arg();
  }

  template <typename Id>
  constexpr basic_arg<Context> get_arg(Id arg_id) {
    context_.check_arg_id(arg_id);
    return context_.get_arg(arg_id);
  }

  Context &context_;
};

// An argument reference.
template <typename Char>
struct arg_ref {
  enum Kind { NONE, INDEX, NAME };

  constexpr arg_ref() : kind(NONE), index(0) {}
  constexpr explicit arg_ref(unsigned index) : kind(INDEX), index(index) {}
  explicit arg_ref(basic_string_view<Char> name) : kind(NAME), name(name) {}

  constexpr arg_ref &operator=(unsigned index) {
    kind = INDEX;
    this->index = index;
    return *this;
  }

  Kind kind;
  union {
    unsigned index;
    basic_string_view<Char> name;
  };
};

// Format specifiers with width and precision resolved at formatting rather
// than parsing time to allow re-using the same parsed specifiers with
// differents sets of arguments (precompilation of format strings).
template <typename Char>
struct dynamic_format_specs : basic_format_specs<Char> {
  arg_ref<Char> width_ref;
  arg_ref<Char> precision_ref;
};

// Format spec handler that saves references to arguments representing dynamic
// width and precision to be resolved at formatting time.
template <typename ParseContext>
class dynamic_specs_handler :
    public specs_setter<typename ParseContext::char_type> {
 public:
  using char_type = typename ParseContext::char_type;

  constexpr dynamic_specs_handler(
      dynamic_format_specs<char_type> &specs, ParseContext &ctx)
    : specs_setter<char_type>(specs), specs_(specs), context_(ctx) {}

  constexpr dynamic_specs_handler(const dynamic_specs_handler &other)
    : specs_setter<char_type>(other),
      specs_(other.specs_), context_(other.context_) {}

  template <typename Id>
  constexpr void on_dynamic_width(Id arg_id) {
    specs_.width_ref = make_arg_ref(arg_id);
  }

  template <typename Id>
  constexpr void on_dynamic_precision(Id arg_id) {
    specs_.precision_ref = make_arg_ref(arg_id);
  }

  constexpr void on_error(const char *message) {
    context_.on_error(message);
  }

 private:
  using arg_ref_type = arg_ref<char_type>;

  template <typename Id>
  constexpr arg_ref_type make_arg_ref(Id arg_id) {
    context_.check_arg_id(arg_id);
    return arg_ref_type(arg_id);
  }

  constexpr arg_ref_type make_arg_ref(auto_id) {
    return arg_ref_type(context_.next_arg_index());
  }

  dynamic_format_specs<char_type> &specs_;
  ParseContext &context_;
};

template <typename Iterator, typename IDHandler>
constexpr Iterator parse_arg_id(Iterator it, IDHandler &&handler) {
  using char_type = typename std::iterator_traits<Iterator>::value_type;
  char_type c = *it;
  if (c == '}' || c == ':') {
    handler();
    return it;
  }
  if (c >= '0' && c <= '9') {
    unsigned index = parse_nonnegative_int(it, handler);
    if (*it != '}' && *it != ':') {
      handler.on_error("invalid format string");
      return it;
    }
    handler(index);
    return it;
  }
  if (!is_name_start(c)) {
    handler.on_error("invalid format string");
    return it;
  }
  auto start = it;
  do {
    c = *++it;
  } while (is_name_start(c) || ('0' <= c && c <= '9'));
  handler(basic_string_view<char_type>(pointer_from(start), it - start));
  return it;
}

// Adapts SpecHandler to IDHandler API for dynamic width.
template <typename SpecHandler, typename Char>
struct width_adapter {
  explicit constexpr width_adapter(SpecHandler &h) : handler(h) {}

  constexpr void operator()() { handler.on_dynamic_width(auto_id()); }
  constexpr void operator()(unsigned id) { handler.on_dynamic_width(id); }
  constexpr void operator()(basic_string_view<Char> id) {
    handler.on_dynamic_width(id);
  }

  constexpr void on_error(const char *message) { handler.on_error(message); }

  SpecHandler &handler;
};

// Adapts SpecHandler to IDHandler API for dynamic precision.
template <typename SpecHandler, typename Char>
struct precision_adapter {
  explicit constexpr precision_adapter(SpecHandler &h) : handler(h) {}

  constexpr void operator()() { handler.on_dynamic_precision(auto_id()); }
  constexpr void operator()(unsigned id) { handler.on_dynamic_precision(id); }
  constexpr void operator()(basic_string_view<Char> id) {
    handler.on_dynamic_precision(id);
  }

  constexpr void on_error(const char *message) { handler.on_error(message); }

  SpecHandler &handler;
};

// Parses standard format specifiers and sends notifications about parsed
// components to handler.
// it: an iterator pointing to the beginning of a null-terminated range of
//     characters, possibly emulated via null_terminating_iterator, representing
//     format specifiers.
template <typename Iterator, typename SpecHandler>
constexpr Iterator parse_format_specs(Iterator it, SpecHandler &&handler) {
  using char_type = typename std::iterator_traits<Iterator>::value_type;
  // Parse fill and alignment.
  if (char_type c = *it) {
    alignment align = ALIGN_DEFAULT;
    int i = 1;
    do {
      auto p = it + i;
      switch (*p) {
        case '<':
          align = ALIGN_LEFT;
          break;
        case '>':
          align = ALIGN_RIGHT;
          break;
        case '=':
          align = ALIGN_NUMERIC;
          break;
        case '^':
          align = ALIGN_CENTER;
          break;
      }
      if (align != ALIGN_DEFAULT) {
        handler.on_align(align);
        if (p != it) {
          if (c == '}') break;
          if (c == '{') {
            handler.on_error("invalid fill character '{'");
            return it;
          }
          it += 2;
          handler.on_fill(c);
        } else ++it;
        break;
      }
    } while (--i >= 0);
  }

  // Parse sign.
  switch (*it) {
    case '+':
      handler.on_plus();
      ++it;
      break;
    case '-':
      handler.on_minus();
      ++it;
      break;
    case ' ':
      handler.on_space();
      ++it;
      break;
  }

  if (*it == '#') {
    handler.on_hash();
    ++it;
  }

  // Parse zero flag.
  if (*it == '0') {
    handler.on_zero();
    ++it;
  }

  // Parse width.
  if ('0' <= *it && *it <= '9') {
    handler.on_width(parse_nonnegative_int(it, handler));
  } else if (*it == '{') {
    it = parse_arg_id(it + 1, width_adapter<SpecHandler, char_type>(handler));
    if (*it++ != '}') {
      handler.on_error("invalid format string");
      return it;
    }
  }

  // Parse precision.
  if (*it == '.') {
    ++it;
    if ('0' <= *it && *it <= '9') {
      handler.on_precision(parse_nonnegative_int(it, handler));
    } else if (*it == '{') {
      it = parse_arg_id(
            it + 1, precision_adapter<SpecHandler, char_type>(handler));
      if (*it++ != '}') {
        handler.on_error("invalid format string");
        return it;
      }
    } else {
      handler.on_error("missing precision specifier");
      return it;
    }
    handler.end_precision();
  }

  // Parse type.
  if (*it != '}' && *it)
    handler.on_type(*it++);
  return it;
}

template <typename Handler, typename Char>
struct id_adapter {
  constexpr explicit id_adapter(Handler &h): handler(h) {}

  constexpr void operator()() { handler.on_arg_id(); }
  constexpr void operator()(unsigned id) { handler.on_arg_id(id); }
  constexpr void operator()(basic_string_view<Char> id) {
    handler.on_arg_id(id);
  }

  constexpr void on_error(const char *message) {
    handler.on_error(message);
  }

  Handler &handler;
};

template <typename Iterator, typename Handler>
constexpr void parse_format_string(Iterator it, Handler &&handler) {
  using char_type = typename std::iterator_traits<Iterator>::value_type;
  auto start = it;
  while (*it) {
    char_type ch = *it++;
    if (ch != '{' && ch != '}') continue;
    if (*it == ch) {
      handler.on_text(start, it);
      start = ++it;
      continue;
    }
    if (ch == '}') {
      handler.on_error("unmatched '}' in format string");
      return;
    }
    handler.on_text(start, it - 1);

    it = parse_arg_id(it, id_adapter<Handler, char_type>(handler));
    if (*it == '}') {
      handler.on_replacement_field(it);
    } else if (*it == ':') {
      ++it;
      it = handler.on_format_specs(it);
      if (*it != '}') {
        handler.on_error("unknown format specifier");
        return;
      }
    } else {
      handler.on_error("missing '}' in format string");
      return;
    }

    start = ++it;
  }
  handler.on_text(start, it);
}

template <typename T, typename ParseContext>
constexpr const typename ParseContext::char_type *
    parse_format_specs(ParseContext &ctx) {
  formatter<T, typename ParseContext::char_type> f;
  return f.parse(ctx);
}

template <typename Char, typename ErrorHandler, typename... Args>
class format_string_checker {
 public:
  explicit constexpr format_string_checker(
      basic_string_view<Char> format_str, ErrorHandler eh)
    : context_(format_str, eh) {}

  constexpr void on_text(const Char *, const Char *) {}

  constexpr void on_arg_id() {
    arg_index_ = context_.next_arg_index();
    check_arg_index();
  }
  constexpr void on_arg_id(unsigned index) {
    arg_index_ = index;
    context_.check_arg_id(index);
    check_arg_index();
  }
  constexpr void on_arg_id(basic_string_view<Char>) {}

  constexpr void on_replacement_field(const Char *) {}

  constexpr const Char *on_format_specs(const Char *s) {
    context_.advance_to(s);
    return to_unsigned(arg_index_) < NUM_ARGS ?
          parse_funcs_[arg_index_](context_) : s;
  }

  constexpr void on_error(const char *message) {
    context_.on_error(message);
  }

 private:
  using parse_context_type = basic_parse_context<Char, ErrorHandler>;
  constexpr static size_t NUM_ARGS = sizeof...(Args);

  constexpr void check_arg_index() {
    if (internal::to_unsigned(arg_index_) >= NUM_ARGS)
      context_.on_error("argument index out of range");
  }

  // Format specifier parsing function.
  using parse_func = const Char *(*)(parse_context_type &);

  int arg_index_ = -1;
  parse_context_type context_;
  parse_func parse_funcs_[NUM_ARGS > 0 ? NUM_ARGS : 1] = {
      &parse_format_specs<Args, parse_context_type>...
  };
};

template <typename Char, typename ErrorHandler, typename... Args>
constexpr bool check_format_string(
    basic_string_view<Char> s, ErrorHandler eh = ErrorHandler()) {
  format_string_checker<Char, ErrorHandler, Args...> checker(s, eh);
  parse_format_string(s.begin(), checker);
  return true;
}

// Specifies whether to format T using the standard formatter.
// It is not possible to use get_type in formatter specialization directly
// because of a bug in MSVC.
template <typename T>
struct format_type : std::integral_constant<bool, get_type<T>() != CUSTOM> {};

// Specifies whether to format enums.
template <typename T, typename Enable = void>
struct format_enum : std::integral_constant<bool, std::is_enum<T>::value> {};

template <template <typename> class Handler, typename Spec, typename Char>
void handle_dynamic_spec(
    Spec &value, arg_ref<Char> ref, basic_context<Char> &ctx) {
  switch (ref.kind) {
  case arg_ref<Char>::NONE:
    break;
  case arg_ref<Char>::INDEX:
    internal::set_dynamic_spec<Handler>(
          value, ctx.get_arg(ref.index), ctx.error_handler());
    break;
  case arg_ref<Char>::NAME:
    internal::set_dynamic_spec<Handler>(
          value, ctx.get_arg(ref.name), ctx.error_handler());
    break;
  }
}
}  // namespace internal

/** The default argument formatter. */
template <typename Char>
class arg_formatter : public internal::arg_formatter_base<Char> {
 private:
  basic_context<Char> &ctx_;

  typedef internal::arg_formatter_base<Char> Base;

 public:
  typedef typename Base::format_specs format_specs;

  /**
    \rst
    Constructs an argument formatter object.
    *buffer* is a reference to the buffer to be used for output,
    *ctx* is a reference to the formatting context, *spec* contains
    format specifier information for standard argument types.
    \endrst
   */
  arg_formatter(basic_buffer<Char> &buffer, basic_context<Char> &ctx,
                format_specs &spec)
  : internal::arg_formatter_base<Char>(buffer, spec), ctx_(ctx) {}

  using internal::arg_formatter_base<Char>::operator();

  /** Formats an argument of a custom (user-defined) type. */
  void operator()(internal::custom_value<Char> c) {
    c.format(this->writer().buffer(), c.value, &ctx_);
  }
};

template <typename Char>
class basic_context :
  public internal::context_base<Char, basic_context<Char>> {
 public:
  /** The character type for the output. */
  using char_type = Char;

  template <typename T>
  using formatter_type = formatter<T, Char>;

 private:
  internal::arg_map<basic_context<Char>> map_;

  FMT_DISALLOW_COPY_AND_ASSIGN(basic_context);

  typedef internal::context_base<Char, basic_context<Char>> Base;

  typedef typename Base::format_arg format_arg;
  using Base::get_arg;

 public:
  /**
   \rst
   Constructs a ``basic_context`` object. References to the arguments are
   stored in the object so make sure they have appropriate lifetimes.
   \endrst
   */
  basic_context(
      basic_string_view<Char> format_str, basic_args<basic_context> args)
    : Base(format_str, args) {}

  format_arg next_arg() {
    return this->do_get_arg(this->next_arg_index());
  }

  format_arg get_arg(unsigned arg_index) {
    return this->do_get_arg(arg_index);
  }

  // Checks if manual indexing is used and returns the argument with
  // specified name.
  format_arg get_arg(basic_string_view<Char> name);
};

/**
 An error returned by an operating system or a language runtime,
 for example a file opening error.
*/
class system_error : public std::runtime_error {
 private:
  void init(int err_code, string_view format_str, args args);

 protected:
  int error_code_;

  system_error() : std::runtime_error("") {}

 public:
  /**
   \rst
   Constructs a :class:`fmt::system_error` object with a description
   formatted with `fmt::format_system_error`. *message* and additional
   arguments passed into the constructor are formatted similarly to
   `fmt::format`.

   **Example**::

     // This throws a system_error with the description
     //   cannot open file 'madeup': No such file or directory
     // or similar (system message may vary).
     const char *filename = "madeup";
     std::FILE *file = std::fopen(filename, "r");
     if (!file)
       throw fmt::system_error(errno, "cannot open file '{}'", filename);
   \endrst
  */
  template <typename... Args>
  system_error(int error_code, string_view message, const Args & ... args)
    : std::runtime_error("") {
    init(error_code, message, make_args(args...));
  }

  ~system_error() throw();

  int error_code() const { return error_code_; }
};

/**
  \rst
  Formats an error returned by an operating system or a language runtime,
  for example a file opening error, and writes it to *out* in the following
  form:

  .. parsed-literal::
     *<message>*: *<system-message>*

  where *<message>* is the passed message and *<system-message>* is
  the system message corresponding to the error code.
  *error_code* is a system error code as given by ``errno``.
  If *error_code* is not a valid error code such as -1, the system message
  may look like "Unknown error -1" and is platform-dependent.
  \endrst
 */
FMT_API void format_system_error(fmt::buffer &out, int error_code,
                                 fmt::string_view message) FMT_NOEXCEPT;

/**
  \rst
  This template provides operations for formatting and writing data into a
  character buffer. The output buffer is specified by a subclass such as
  :class:`fmt::BasicMemoryWriter`.

  You can use one of the following typedefs for common character types:

  +---------+-----------------------+
  | Type    | Definition            |
  +=========+=======================+
  | writer  | basic_writer<char>    |
  +---------+-----------------------+
  | wwriter | basic_writer<wchar_t> |
  +---------+-----------------------+

  \endrst
 */
template <typename Char>
class basic_writer {
 public:
  typedef basic_format_specs<Char> format_specs;

 private:
  // Output buffer.
  basic_buffer<Char> &buffer_;

  FMT_DISALLOW_COPY_AND_ASSIGN(basic_writer);

#if FMT_SECURE_SCL
  typedef stdext::checked_array_iterator<Char*> pointer_type;
  // Returns pointer value.
  static Char *get(pointer_type p) { return p.base(); }
#else
  typedef Char *pointer_type;
  static Char *get(Char *p) { return p; }
#endif

  // Fills the padding around the content and returns the pointer to the
  // content area.
  static pointer_type fill_padding(pointer_type buffer,
      unsigned total_size, std::size_t content_size, wchar_t fill);

  // Grows the buffer by n characters and returns a pointer to the newly
  // allocated area.
  pointer_type grow_buffer(std::size_t n) {
    std::size_t size = buffer_.size();
    buffer_.resize(size + n);
    return internal::make_ptr(&buffer_[size], n);
  }

  // Writes an unsigned decimal integer.
  template <typename UInt>
  Char *write_unsigned_decimal(UInt value, unsigned prefix_size = 0) {
    unsigned num_digits = internal::count_digits(value);
    Char *ptr = get(grow_buffer(prefix_size + num_digits));
    internal::format_decimal(ptr + prefix_size, value, num_digits);
    return ptr;
  }

  // Writes a decimal integer.
  template <typename Int>
  void write_decimal(Int value) {
    typedef typename internal::int_traits<Int>::main_type main_type;
    main_type abs_value = static_cast<main_type>(value);
    if (internal::is_negative(value)) {
      abs_value = 0 - abs_value;
      *write_unsigned_decimal(abs_value, 1) = '-';
    } else {
      write_unsigned_decimal(abs_value, 0);
    }
  }

  // Prepare a buffer for integer formatting.
  pointer_type prepare_int_buffer(unsigned num_digits,
      const empty_spec &, const char *prefix, unsigned prefix_size) {
    unsigned size = prefix_size + num_digits;
    pointer_type p = grow_buffer(size);
    std::uninitialized_copy(prefix, prefix + prefix_size, p);
    return p + size - 1;
  }

  template <typename Spec>
  pointer_type prepare_int_buffer(unsigned num_digits,
    const Spec &spec, const char *prefix, unsigned prefix_size);

  // Writes a formatted integer.
  template <typename T, typename Spec>
  void write_int(T value, const Spec& spec);

  // Formats a floating-point number (double or long double).
  template <typename T>
  void write_double(T value, const format_specs &spec);

  // Writes a formatted string.
  template <typename StrChar>
  pointer_type write_str(
      const StrChar *s, std::size_t size, const align_spec &spec);

  template <typename StrChar>
  void write_str(basic_string_view<StrChar> str, const format_specs &spec);

  // Appends floating-point length specifier to the format string.
  // The second argument is only used for overload resolution.
  void append_float_length(Char *&format_ptr, long double) {
    *format_ptr++ = 'L';
  }

  template<typename T>
  void append_float_length(Char *&, T) {}

  template <typename Char_>
  friend class internal::arg_formatter_base;

 public:
  /**
    Constructs a ``basic_writer`` object.
   */
  explicit basic_writer(basic_buffer<Char> &b) : buffer_(b) {}

  /**
    \rst
    Destroys the ``basic_writer`` object.
    \endrst
   */
  virtual ~basic_writer() {}

  /**
    Returns the total number of characters written.
   */
  std::size_t size() const { return buffer_.size(); }

  /**
    Returns a pointer to the output buffer content. No terminating null
    character is appended.
   */
  const Char *data() const FMT_NOEXCEPT { return &buffer_[0]; }

  /**
    Returns a pointer to the output buffer content with terminating null
    character appended.
   */
  const Char *c_str() const {
    std::size_t size = buffer_.size();
    buffer_.reserve(size + 1);
    buffer_[size] = '\0';
    return &buffer_[0];
  }

  /**
    \rst
    Returns the content of the output buffer as an `std::string`.
    \endrst
   */
  std::basic_string<Char> str() const {
    return std::basic_string<Char>(&buffer_[0], buffer_.size());
  }

  void write(int value) {
    write_decimal(value);
  }
  void write(long value) {
    write_decimal(value);
  }
  void write(long long value) {
    write_decimal(value);
  }

  /**
    \rst
    Formats *value* and writes it to the buffer.
    \endrst
   */
  template <typename T, typename... FormatSpecs>
  typename std::enable_if<std::is_integral<T>::value, void>::type
      write(T value, FormatSpecs... specs) {
    write_int(value, format_specs(specs...));
  }

  void write(double value) {
    write_double(value, format_specs());
  }

  /**
    \rst
    Formats *value* using the general format for floating-point numbers
    (``'g'``) and writes it to the buffer.
    \endrst
   */
  void write(long double value) {
    write_double(value, format_specs());
  }

  /**
    Writes a character to the buffer.
   */
  void write(char value) {
    buffer_.push_back(value);
  }

  void write(wchar_t value) {
    internal::require_wchar<Char>();
    buffer_.push_back(value);
  }

  /**
    \rst
    Writes *value* to the buffer.
    \endrst
   */
  void write(string_view value) {
    const char *str = value.data();
    buffer_.append(str, str + value.size());
  }

  void write(basic_string_view<wchar_t> value) {
    internal::require_wchar<Char>();
    const wchar_t *str = value.data();
    buffer_.append(str, str + value.size());
  }

  template <typename... FormatSpecs>
  void write(basic_string_view<Char> str, FormatSpecs... specs) {
    write_str(str, format_specs(specs...));
  }

  void clear() FMT_NOEXCEPT { buffer_.resize(0); }

  basic_buffer<Char> &buffer() FMT_NOEXCEPT { return buffer_; }
};

template <typename Char>
template <typename StrChar>
typename basic_writer<Char>::pointer_type basic_writer<Char>::write_str(
      const StrChar *s, std::size_t size, const align_spec &spec) {
  pointer_type out = pointer_type();
  if (spec.width() > size) {
    out = grow_buffer(spec.width());
    Char fill = internal::char_traits<Char>::cast(spec.fill());
    if (spec.align() == ALIGN_RIGHT) {
      std::uninitialized_fill_n(out, spec.width() - size, fill);
      out += spec.width() - size;
    } else if (spec.align() == ALIGN_CENTER) {
      out = fill_padding(out, spec.width(), size, fill);
    } else {
      std::uninitialized_fill_n(out + size, spec.width() - size, fill);
    }
  } else {
    out = grow_buffer(size);
  }
  std::uninitialized_copy(s, s + size, out);
  return out;
}

template <typename Char>
template <typename StrChar>
void basic_writer<Char>::write_str(
    basic_string_view<StrChar> s, const format_specs &spec) {
  // Check if StrChar is convertible to Char.
  internal::char_traits<Char>::convert(StrChar());
  if (spec.type_ && spec.type_ != 's')
    internal::report_unknown_type(spec.type_, "string");
  const StrChar *str_value = s.data();
  std::size_t str_size = s.size();
  if (str_size == 0 && !str_value)
    FMT_THROW(format_error("string pointer is null"));
  std::size_t precision = static_cast<std::size_t>(spec.precision_);
  if (spec.precision_ >= 0 && precision < str_size)
    str_size = precision;
  write_str(str_value, str_size, spec);
}

template <typename Char>
typename basic_writer<Char>::pointer_type basic_writer<Char>::fill_padding(
    pointer_type buffer, unsigned total_size,
    std::size_t content_size, wchar_t fill) {
  std::size_t padding = total_size - content_size;
  std::size_t left_padding = padding / 2;
  Char fill_char = internal::char_traits<Char>::cast(fill);
  std::uninitialized_fill_n(buffer, left_padding, fill_char);
  buffer += left_padding;
  pointer_type content = buffer;
  std::uninitialized_fill_n(buffer + content_size,
                            padding - left_padding, fill_char);
  return content;
}

template <typename Char>
template <typename Spec>
typename basic_writer<Char>::pointer_type
  basic_writer<Char>::prepare_int_buffer(
    unsigned num_digits, const Spec &spec,
    const char *prefix, unsigned prefix_size) {
  unsigned width = spec.width();
  alignment align = spec.align();
  Char fill = internal::char_traits<Char>::cast(spec.fill());
  if (spec.precision() > static_cast<int>(num_digits)) {
    // Octal prefix '0' is counted as a digit, so ignore it if precision
    // is specified.
    if (prefix_size > 0 && prefix[prefix_size - 1] == '0')
      --prefix_size;
    unsigned number_size =
        prefix_size + internal::to_unsigned(spec.precision());
    align_spec subspec(number_size, '0', ALIGN_NUMERIC);
    if (number_size >= width)
      return prepare_int_buffer(num_digits, subspec, prefix, prefix_size);
    buffer_.reserve(width);
    unsigned fill_size = width - number_size;
    if (align != ALIGN_LEFT) {
      pointer_type p = grow_buffer(fill_size);
      std::uninitialized_fill(p, p + fill_size, fill);
    }
    pointer_type result = prepare_int_buffer(
        num_digits, subspec, prefix, prefix_size);
    if (align == ALIGN_LEFT) {
      pointer_type p = grow_buffer(fill_size);
      std::uninitialized_fill(p, p + fill_size, fill);
    }
    return result;
  }
  unsigned size = prefix_size + num_digits;
  if (width <= size) {
    pointer_type p = grow_buffer(size);
    std::uninitialized_copy(prefix, prefix + prefix_size, p);
    return p + size - 1;
  }
  pointer_type p = grow_buffer(width);
  pointer_type end = p + width;
  if (align == ALIGN_LEFT) {
    std::uninitialized_copy(prefix, prefix + prefix_size, p);
    p += size;
    std::uninitialized_fill(p, end, fill);
  } else if (align == ALIGN_CENTER) {
    p = fill_padding(p, width, size, fill);
    std::uninitialized_copy(prefix, prefix + prefix_size, p);
    p += size;
  } else {
    if (align == ALIGN_NUMERIC) {
      if (prefix_size != 0) {
        p = std::uninitialized_copy(prefix, prefix + prefix_size, p);
        size -= prefix_size;
      }
    } else {
      std::uninitialized_copy(prefix, prefix + prefix_size, end - size);
    }
    std::uninitialized_fill(p, end - size, fill);
    p = end;
  }
  return p - 1;
}

template <typename Char>
template <typename T, typename Spec>
void basic_writer<Char>::write_int(T value, const Spec& spec) {
  using unsigned_type = typename internal::int_traits<T>::main_type;
  struct spec_handler {
    basic_writer<Char> &writer;
    const Spec& spec;
    unsigned prefix_size = 0;
    unsigned_type abs_value;
    char prefix[4] = "";

    spec_handler(basic_writer<Char> &w, T val, const Spec& s)
      : writer(w), spec(s), abs_value(static_cast<unsigned_type>(val)) {
      if (internal::is_negative(val)) {
        prefix[0] = '-';
        ++prefix_size;
        abs_value = 0 - abs_value;
      } else if (spec.flag(SIGN_FLAG)) {
        prefix[0] = spec.flag(PLUS_FLAG) ? '+' : ' ';
        ++prefix_size;
      }
    }

    void on_dec() {
      unsigned num_digits = internal::count_digits(abs_value);
      pointer_type p =
          writer.prepare_int_buffer(num_digits, spec, prefix, prefix_size) + 1;
      internal::format_decimal(get(p), abs_value, 0);
    }

    void on_hex() {
      unsigned_type n = abs_value;
      if (spec.flag(HASH_FLAG)) {
        prefix[prefix_size++] = '0';
        prefix[prefix_size++] = spec.type();
      }
      unsigned num_digits = 0;
      do {
        ++num_digits;
      } while ((n >>= 4) != 0);
      Char *p =
          get(writer.prepare_int_buffer(num_digits, spec, prefix, prefix_size));
      n = abs_value;
      const char *digits = spec.type() == 'x' ?
          "0123456789abcdef" : "0123456789ABCDEF";
      do {
        *p-- = digits[n & 0xf];
      } while ((n >>= 4) != 0);
    }

    void on_bin() {
      unsigned_type n = abs_value;
      if (spec.flag(HASH_FLAG)) {
        prefix[prefix_size++] = '0';
        prefix[prefix_size++] = spec.type();
      }
      unsigned num_digits = 0;
      do {
        ++num_digits;
      } while ((n >>= 1) != 0);
      Char *p =
          get(writer.prepare_int_buffer(num_digits, spec, prefix, prefix_size));
      n = abs_value;
      do {
        *p-- = static_cast<Char>('0' + (n & 1));
      } while ((n >>= 1) != 0);
    }

    void on_oct() {
      unsigned_type n = abs_value;
      if (spec.flag(HASH_FLAG))
        prefix[prefix_size++] = '0';
      unsigned num_digits = 0;
      do {
        ++num_digits;
      } while ((n >>= 3) != 0);
      Char *p =
          get(writer.prepare_int_buffer(num_digits, spec, prefix, prefix_size));
      n = abs_value;
      do {
        *p-- = static_cast<Char>('0' + (n & 7));
      } while ((n >>= 3) != 0);
    }

    void on_num() {
      unsigned num_digits = internal::count_digits(abs_value);
      std::locale loc = writer.buffer_.locale();
      Char thousands_sep =
          std::use_facet<std::numpunct<Char>>(loc).thousands_sep();
      fmt::basic_string_view<Char> sep(&thousands_sep, 1);
      unsigned size = static_cast<unsigned>(
            num_digits + sep.size() * ((num_digits - 1) / 3));
      pointer_type p =
          writer.prepare_int_buffer(size, spec, prefix, prefix_size) + 1;
      internal::format_decimal(get(p), abs_value, 0,
                               internal::add_thousands_sep<Char>(sep));
    }

    void on_error() {
      internal::report_unknown_type(
        spec.type(), spec.flag(CHAR_FLAG) ? "char" : "integer");
    }
  };
  internal::handle_integral_type_spec(
        spec.type(), spec_handler(*this, value, spec));
}

template <typename Char>
template <typename T>
void basic_writer<Char>::write_double(T value, const format_specs &spec) {
  // Check type.
  char type = spec.type();
  bool upper = false;
  switch (type) {
  case 0:
    type = 'g';
    break;
  case 'e': case 'f': case 'g': case 'a':
    break;
  case 'F':
#if FMT_MSC_VER
    // MSVC's printf doesn't support 'F'.
    type = 'f';
#endif
    // Fall through.
  case 'E': case 'G': case 'A':
    upper = true;
    break;
  default:
    internal::report_unknown_type(type, "double");
    break;
  }

  char sign = 0;
  // Use isnegative instead of value < 0 because the latter is always
  // false for NaN.
  if (internal::fputil::isnegative(static_cast<double>(value))) {
    sign = '-';
    value = -value;
  } else if (spec.flag(SIGN_FLAG)) {
    sign = spec.flag(PLUS_FLAG) ? '+' : ' ';
  }

  if (internal::fputil::isnotanumber(value)) {
    // Format NaN ourselves because sprintf's output is not consistent
    // across platforms.
    std::size_t nan_size = 4;
    const char *nan = upper ? " NAN" : " nan";
    if (!sign) {
      --nan_size;
      ++nan;
    }
    pointer_type out = write_str(nan, nan_size, spec);
    if (sign)
      *out = sign;
    return;
  }

  if (internal::fputil::isinfinity(value)) {
    // Format infinity ourselves because sprintf's output is not consistent
    // across platforms.
    std::size_t inf_size = 4;
    const char *inf = upper ? " INF" : " inf";
    if (!sign) {
      --inf_size;
      ++inf;
    }
    pointer_type out = write_str(inf, inf_size, spec);
    if (sign)
      *out = sign;
    return;
  }

  std::size_t offset = buffer_.size();
  unsigned width = spec.width();
  if (sign) {
    buffer_.reserve(buffer_.size() + (width > 1u ? width : 1u));
    if (width > 0)
      --width;
    ++offset;
  }

  // Build format string.
  enum { MAX_FORMAT_SIZE = 10}; // longest format: %#-*.*Lg
  Char format[MAX_FORMAT_SIZE];
  Char *format_ptr = format;
  *format_ptr++ = '%';
  unsigned width_for_sprintf = width;
  if (spec.flag(HASH_FLAG))
    *format_ptr++ = '#';
  if (spec.align() == ALIGN_CENTER) {
    width_for_sprintf = 0;
  } else {
    if (spec.align() == ALIGN_LEFT)
      *format_ptr++ = '-';
    if (width != 0)
      *format_ptr++ = '*';
  }
  if (spec.precision() >= 0) {
    *format_ptr++ = '.';
    *format_ptr++ = '*';
  }

  append_float_length(format_ptr, value);
  *format_ptr++ = type;
  *format_ptr = '\0';

  // Format using snprintf.
  Char fill = internal::char_traits<Char>::cast(spec.fill());
  unsigned n = 0;
  Char *start = 0;
  for (;;) {
    std::size_t buffer_size = buffer_.capacity() - offset;
#if FMT_MSC_VER
    // MSVC's vsnprintf_s doesn't work with zero size, so reserve
    // space for at least one extra character to make the size non-zero.
    // Note that the buffer's capacity will increase by more than 1.
    if (buffer_size == 0) {
      buffer_.reserve(offset + 1);
      buffer_size = buffer_.capacity() - offset;
    }
#endif
    start = &buffer_[offset];
    int result = internal::char_traits<Char>::format_float(
        start, buffer_size, format, width_for_sprintf, spec.precision(), value);
    if (result >= 0) {
      n = internal::to_unsigned(result);
      if (offset + n < buffer_.capacity())
        break;  // The buffer is large enough - continue with formatting.
      buffer_.reserve(offset + n + 1);
    } else {
      // If result is negative we ask to increase the capacity by at least 1,
      // but as std::vector, the buffer grows exponentially.
      buffer_.reserve(buffer_.capacity() + 1);
    }
  }
  if (sign) {
    if ((spec.align() != ALIGN_RIGHT && spec.align() != ALIGN_DEFAULT) ||
        *start != ' ') {
      *(start - 1) = sign;
      sign = 0;
    } else {
      *(start - 1) = fill;
    }
    ++n;
  }
  if (spec.align() == ALIGN_CENTER && spec.width() > n) {
    width = spec.width();
    pointer_type p = grow_buffer(width);
    std::memmove(get(p) + (width - n) / 2, get(p), n * sizeof(Char));
    fill_padding(p, spec.width(), n, fill);
    return;
  }
  if (spec.fill() != ' ' || sign) {
    while (*start == ' ')
      *start++ = fill;
    if (sign)
      *(start - 1) = sign;
  }
  grow_buffer(n);
}

// Reports a system error without throwing an exception.
// Can be used to report errors from destructors.
FMT_API void report_system_error(int error_code,
                                 string_view message) FMT_NOEXCEPT;

#if FMT_USE_WINDOWS_H

/** A Windows error. */
class windows_error : public system_error {
 private:
  FMT_API void init(int error_code, string_view format_str, args args);

 public:
  /**
   \rst
   Constructs a :class:`fmt::windows_error` object with the description
   of the form

   .. parsed-literal::
     *<message>*: *<system-message>*

   where *<message>* is the formatted message and *<system-message>* is the
   system message corresponding to the error code.
   *error_code* is a Windows error code as given by ``GetLastError``.
   If *error_code* is not a valid error code such as -1, the system message
   will look like "error -1".

   **Example**::

     // This throws a windows_error with the description
     //   cannot open file 'madeup': The system cannot find the file specified.
     // or similar (system message may vary).
     const char *filename = "madeup";
     LPOFSTRUCT of = LPOFSTRUCT();
     HFILE file = OpenFile(filename, &of, OF_READ);
     if (file == HFILE_ERROR) {
       throw fmt::windows_error(GetLastError(),
                                "cannot open file '{}'", filename);
     }
   \endrst
  */
  template <typename... Args>
  windows_error(int error_code, string_view message, const Args & ... args) {
    init(error_code, message, make_args(args...));
  }
};

// Reports a Windows error without throwing an exception.
// Can be used to report errors from destructors.
FMT_API void report_windows_error(int error_code,
                                  string_view message) FMT_NOEXCEPT;

#endif

enum Color { BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE };

FMT_API void vprint_colored(Color c, string_view format, args args);

/**
  Formats a string and prints it to stdout using ANSI escape sequences
  to specify color (experimental).
  Example:
    print_colored(fmt::RED, "Elapsed time: {0:.2f} seconds", 1.23);
 */
template <typename... Args>
inline void print_colored(Color c, string_view format_str,
                          const Args & ... args) {
  vprint_colored(c, format_str, make_args(args...));
}

template <typename ArgFormatter, typename Char, typename Context>
void vformat_to(basic_buffer<Char> &buffer, basic_string_view<Char> format_str,
                basic_args<Context> args);

inline void vformat_to(buffer &buf, string_view format_str, args args) {
  vformat_to<arg_formatter<char>>(buf, format_str, args);
}

inline void vformat_to(wbuffer &buf, wstring_view format_str, wargs args) {
  vformat_to<arg_formatter<wchar_t>>(buf, format_str, args);
}

template <typename... Args>
inline void format_to(buffer &buf, string_view format_str,
                      const Args & ... args) {
  vformat_to(buf, format_str, make_args(args...));
}

template <typename... Args>
inline void format_to(wbuffer &buf, wstring_view format_str,
                      const Args & ... args) {
  vformat_to(buf, format_str, make_args<wcontext>(args...));
}

inline std::string vformat(string_view format_str, args args) {
  memory_buffer buffer;
  vformat_to(buffer, format_str, args);
  return to_string(buffer);
}

/**
  \rst
  Formats arguments and returns the result as a string.

  **Example**::

    std::string message = format("The answer is {}", 42);
  \endrst
*/
template <typename... Args>
inline std::string format(string_view format_str, const Args & ... args) {
  return vformat(format_str, make_args(args...));
}

template <typename String, typename... Args>
inline typename std::enable_if<
  std::is_base_of<internal::format_string, String>::value, std::string>::type
    format(String format_str, const Args & ... args) {
  constexpr bool invalid_format =
      internal::check_format_string<char, internal::error_handler, Args...>(
        string_view(format_str.value(), format_str.size()));
  (void)invalid_format;
  return vformat(format_str.value(), make_args(args...));
}

inline std::wstring vformat(wstring_view format_str, wargs args) {
  wmemory_buffer buffer;
  vformat_to(buffer, format_str, args);
  return to_string(buffer);
}

template <typename... Args>
inline std::wstring format(wstring_view format_str, const Args & ... args) {
  return vformat(format_str, make_args<wcontext>(args...));
}

FMT_API void vprint(std::FILE *f, string_view format_str, args args);

/**
  \rst
  Prints formatted data to the file *f*.

  **Example**::

    print(stderr, "Don't {}!", "panic");
  \endrst
 */
template <typename... Args>
inline void print(std::FILE *f, string_view format_str,
                  const Args & ... args) {
  vprint(f, format_str, make_args(args...));
}

FMT_API void vprint(string_view format_str, args args);

/**
  \rst
  Prints formatted data to ``stdout``.

  **Example**::

    print("Elapsed time: {0:.2f} seconds", 1.23);
  \endrst
 */
template <typename... Args>
inline void print(string_view format_str, const Args & ... args) {
  vprint(format_str, make_args(args...));
}

/**
  Fast integer formatter.
 */
class FormatInt {
 private:
  // Buffer should be large enough to hold all digits (digits10 + 1),
  // a sign and a null character.
  enum {BUFFER_SIZE = std::numeric_limits<unsigned long long>::digits10 + 3};
  mutable char buffer_[BUFFER_SIZE];
  char *str_;

  // Formats value in reverse and returns the number of digits.
  char *format_decimal(unsigned long long value) {
    char *buffer_end = buffer_ + BUFFER_SIZE - 1;
    while (value >= 100) {
      // Integer division is slow so do it for a group of two digits instead
      // of for every digit. The idea comes from the talk by Alexandrescu
      // "Three Optimization Tips for C++". See speed-test for a comparison.
      unsigned index = static_cast<unsigned>((value % 100) * 2);
      value /= 100;
      *--buffer_end = internal::data::DIGITS[index + 1];
      *--buffer_end = internal::data::DIGITS[index];
    }
    if (value < 10) {
      *--buffer_end = static_cast<char>('0' + value);
      return buffer_end;
    }
    unsigned index = static_cast<unsigned>(value * 2);
    *--buffer_end = internal::data::DIGITS[index + 1];
    *--buffer_end = internal::data::DIGITS[index];
    return buffer_end;
  }

  void FormatSigned(long long value) {
    unsigned long long abs_value = static_cast<unsigned long long>(value);
    bool negative = value < 0;
    if (negative)
      abs_value = 0 - abs_value;
    str_ = format_decimal(abs_value);
    if (negative)
      *--str_ = '-';
  }

 public:
  explicit FormatInt(int value) { FormatSigned(value); }
  explicit FormatInt(long value) { FormatSigned(value); }
  explicit FormatInt(long long value) { FormatSigned(value); }
  explicit FormatInt(unsigned value) : str_(format_decimal(value)) {}
  explicit FormatInt(unsigned long value) : str_(format_decimal(value)) {}
  explicit FormatInt(unsigned long long value) : str_(format_decimal(value)) {}

  /** Returns the number of characters written to the output buffer. */
  std::size_t size() const {
    return internal::to_unsigned(buffer_ - str_ + BUFFER_SIZE - 1);
  }

  /**
    Returns a pointer to the output buffer content. No terminating null
    character is appended.
   */
  const char *data() const { return str_; }

  /**
    Returns a pointer to the output buffer content with terminating null
    character appended.
   */
  const char *c_str() const {
    buffer_[BUFFER_SIZE - 1] = '\0';
    return str_;
  }

  /**
    \rst
    Returns the content of the output buffer as an ``std::string``.
    \endrst
   */
  std::string str() const { return std::string(str_, size()); }
};

// Formats a decimal integer value writing into buffer and returns
// a pointer to the end of the formatted string. This function doesn't
// write a terminating null character.
template <typename T>
inline void format_decimal(char *&buffer, T value) {
  typedef typename internal::int_traits<T>::main_type main_type;
  main_type abs_value = static_cast<main_type>(value);
  if (internal::is_negative(value)) {
    *buffer++ = '-';
    abs_value = 0 - abs_value;
  }
  if (abs_value < 100) {
    if (abs_value < 10) {
      *buffer++ = static_cast<char>('0' + abs_value);
      return;
    }
    unsigned index = static_cast<unsigned>(abs_value * 2);
    *buffer++ = internal::data::DIGITS[index];
    *buffer++ = internal::data::DIGITS[index + 1];
    return;
  }
  unsigned num_digits = internal::count_digits(abs_value);
  internal::format_decimal(buffer, abs_value, num_digits);
  buffer += num_digits;
}

/**
  \rst
  Returns a named argument for formatting functions.

  **Example**::

    print("Elapsed time: {s:.2f} seconds", arg("s", 1.23));

  \endrst
 */
template <typename T>
inline internal::named_arg<context> arg(string_view name, const T &arg) {
  return internal::named_arg<context>(name, arg);
}

template <typename T>
inline internal::named_arg<wcontext> arg(wstring_view name, const T &arg) {
  return internal::named_arg<wcontext>(name, arg);
}

// The following two functions are deleted intentionally to disable
// nested named arguments as in ``format("{}", arg("a", arg("b", 42)))``.
template <typename Context>
void arg(string_view, const internal::named_arg<Context>&)
  FMT_DELETED_OR_UNDEFINED;
template <typename Context>
void arg(wstring_view, const internal::named_arg<Context>&)
  FMT_DELETED_OR_UNDEFINED;

// Formatter of objects of type T.
template <typename T, typename Char>
struct formatter<
    T, Char, typename std::enable_if<internal::format_type<T>::value>::type> {

  // Parses format specifiers stopping either at the end of the range or at the
  // terminating '}'.
  template <typename ParseContext>
  constexpr typename ParseContext::iterator parse(ParseContext &ctx) {
    auto it = internal::null_terminating_iterator<Char>(ctx);
    using handler_type = internal::dynamic_specs_handler<ParseContext>;
    internal::specs_checker<handler_type>
        handler(handler_type(specs_, ctx), internal::get_type<T>());
    it = parse_format_specs(it, handler);
    if (std::is_integral<T>::value) {
      auto eh = ctx.error_handler();
      handle_integral_type_spec(
            specs_.type(), internal::int_type_checker<decltype(eh)>(eh));
    }
    return pointer_from(it);
  }

  void format(basic_buffer<Char> &buf, const T &val, basic_context<Char> &ctx) {
    internal::handle_dynamic_spec<internal::width_checker>(
      specs_.width_, specs_.width_ref, ctx);
    internal::handle_dynamic_spec<internal::precision_checker>(
      specs_.precision_, specs_.precision_ref, ctx);
    visit(arg_formatter<Char>(buf, ctx, specs_),
          internal::make_arg<basic_context<Char>>(val));
  }

 private:
  internal::dynamic_format_specs<Char> specs_;
};

template <typename T, typename Char>
struct formatter<T, Char,
    typename std::enable_if<internal::format_enum<T>::value>::type>
    : public formatter<int, Char> {
  template <typename ParseContext>
  auto parse(ParseContext &ctx) -> decltype(ctx.begin()) {
    return ctx.begin();
  }
};

// A formatter for types known only at run time such as variant alternatives.
//
// Usage:
//   using variant = std::variant<int, std::string>;
//   template <>
//   struct formatter<variant>: dynamic_formatter<> {
//     void format(buffer &buf, const variant &v, context &ctx) {
//       visit([&](const auto &val) { format(buf, val, ctx); }, v);
//     }
//   };
template <typename Char = char>
struct dynamic_formatter {
  template <typename ParseContext>
  auto parse(ParseContext &ctx) -> decltype(ctx.begin()) {
    auto it = internal::null_terminating_iterator<Char>(ctx);
    // Checks are deferred to formatting time when the argument type is known.
    internal::dynamic_specs_handler<ParseContext> handler(specs_, ctx);
    it = parse_format_specs(it, handler);
    return pointer_from(it);
  }

  template <typename T>
  void format(basic_buffer<Char> &buf, const T &val, basic_context<Char> &ctx) {
    handle_specs(ctx);
    struct null_handler : internal::error_handler {
      void on_align(alignment) {}
      void on_plus() {}
      void on_minus() {}
      void on_space() {}
      void on_hash() {}
    };
    internal::specs_checker<null_handler>
        checker(null_handler(), internal::get_type<T>());
    checker.on_align(specs_.align());
    if (specs_.flags_ == 0) {
      // Do nothing.
    } else if (specs_.flag(SIGN_FLAG)) {
      if (specs_.flag(PLUS_FLAG))
        checker.on_plus();
      else
        checker.on_space();
    } else if (specs_.flag(MINUS_FLAG)) {
      checker.on_minus();
    } else if (specs_.flag(HASH_FLAG)) {
      checker.on_hash();
    }
    if (specs_.precision_ != -1)
      checker.end_precision();
    visit(arg_formatter<Char>(buf, ctx, specs_),
          internal::make_arg<basic_context<Char>>(val));
  }

 private:
  void handle_specs(basic_context<Char> &ctx) {
    internal::handle_dynamic_spec<internal::width_checker>(
      specs_.width_, specs_.width_ref, ctx);
    internal::handle_dynamic_spec<internal::precision_checker>(
      specs_.precision_, specs_.precision_ref, ctx);
  }

  internal::dynamic_format_specs<Char> specs_;
};

template <typename Char>
inline typename basic_context<Char>::format_arg
  basic_context<Char>::get_arg(basic_string_view<Char> name) {
  if (this->check_no_auto_index()) {
    map_.init(this->args());
    if (const format_arg *arg = map_.find(name))
      return *arg;
    this->on_error("argument not found");
  }
  return format_arg();
}

/** Formats arguments and writes the output to the buffer. */
template <typename ArgFormatter, typename Char, typename Context>
void vformat_to(basic_buffer<Char> &buffer, basic_string_view<Char> format_str,
                basic_args<Context> args) {
  using iterator = internal::null_terminating_iterator<Char>;

  struct handler : internal::error_handler {
    handler(basic_buffer<Char> &b, basic_string_view<Char> str,
            basic_args<Context> format_args)
      : buffer(b), context(str, format_args) {}

    void on_text(iterator begin, iterator end) {
      buffer.append(pointer_from(begin), pointer_from(end));
    }

    void on_arg_id() { arg = context.next_arg(); }
    void on_arg_id(unsigned id) {
      context.check_arg_id(id);
      arg = context.get_arg(id);
    }
    void on_arg_id(basic_string_view<Char> id) {
      arg = context.get_arg(id);
    }

    void on_replacement_field(iterator it) {
      context.advance_to(pointer_from(it));
      using internal::custom_formatter;
      if (visit(custom_formatter<Char, Context>(buffer, context), arg))
        return;
      basic_format_specs<Char> specs;
      visit(ArgFormatter(buffer, context, specs), arg);
    }

    iterator on_format_specs(iterator it) {
      context.advance_to(pointer_from(it));
      using internal::custom_formatter;
      if (visit(custom_formatter<Char, Context>(buffer, context), arg))
        return iterator(context);
      basic_format_specs<Char> specs;
      using internal::specs_handler;
      internal::specs_checker<specs_handler<Context>>
          handler(specs_handler<Context>(specs, context), arg.type());
      it = parse_format_specs(it, handler);
      if (*it != '}')
        on_error("missing '}' in format string");
      context.advance_to(pointer_from(it));
      visit(ArgFormatter(buffer, context, specs), arg);
      return it;
    }

    basic_buffer<Char> &buffer;
    basic_context<Char> context;
    basic_arg<Context> arg;
  };
  parse_format_string(iterator(format_str.begin(), format_str.end()),
                      handler(buffer, format_str, args));
}

// Casts ``p`` to ``const void*`` for pointer formatting.
// Example:
//   auto s = format("{}", ptr(p));
template <typename T>
inline const void *ptr(const T *p) { return p; }
}  // namespace fmt

#if FMT_USE_USER_DEFINED_LITERALS
namespace fmt {
namespace internal {

# if FMT_UDL_TEMPLATE
template <typename Char, Char... CHARS>
class udl_formatter {
 public:
  template <typename... Args>
  std::basic_string<Char> operator()(const Args &... args) const {
    constexpr Char s[] = {CHARS..., '\0'};
    constexpr bool invalid_format =
        check_format_string<Char, error_handler, Args...>(
          basic_string_view<Char>(s, sizeof...(CHARS)));
    (void)invalid_format;
    return format(s, args...);
  }
};
# else
template <typename Char>
struct udl_formatter {
  const Char *str;

  template <typename... Args>
  auto operator()(Args && ... args) const
                  -> decltype(format(str, std::forward<Args>(args)...)) {
    return format(str, std::forward<Args>(args)...);
  }
};
# endif // FMT_UDL_TEMPLATE

template <typename Char>
struct udl_arg {
  const Char *str;

  template <typename T>
  named_arg<basic_context<Char>> operator=(T &&value) const {
    return {str, std::forward<T>(value)};
  }
};

} // namespace internal

inline namespace literals {

# if FMT_UDL_TEMPLATE
template <typename Char, Char... CHARS>
constexpr internal::udl_formatter<Char, CHARS...> operator""_format() {
  return {};
}
# else
/**
  \rst
  C++11 literal equivalent of :func:`fmt::format`.

  **Example**::

    using namespace fmt::literals;
    std::string message = "The answer is {}"_format(42);
  \endrst
 */
inline internal::udl_formatter<char>
operator"" _format(const char *s, std::size_t) { return {s}; }
inline internal::udl_formatter<wchar_t>
operator"" _format(const wchar_t *s, std::size_t) { return {s}; }
# endif // FMT_UDL_TEMPLATE

/**
  \rst
  C++11 literal equivalent of :func:`fmt::arg`.

  **Example**::

    using namespace fmt::literals;
    print("Elapsed time: {s:.2f} seconds", "s"_a=1.23);
  \endrst
 */
inline internal::udl_arg<char>
operator"" _a(const char *s, std::size_t) { return {s}; }
inline internal::udl_arg<wchar_t>
operator"" _a(const wchar_t *s, std::size_t) { return {s}; }
} // inline namespace literals
} // namespace fmt
#endif // FMT_USE_USER_DEFINED_LITERALS

#define FMT_STRING(s) [] { \
    struct S : fmt::internal::format_string { \
      static constexpr auto value() { return s; } \
      static constexpr size_t size() { return sizeof(s); } \
    }; \
    return S{}; \
  }()

#ifdef FMT_HEADER_ONLY
# define FMT_FUNC inline
# include "format.cc"
#else
# define FMT_FUNC
#endif

// Restore warnings.
#if FMT_GCC_VERSION >= 406
# pragma GCC diagnostic pop
#endif

#if defined(__clang__) && !defined(FMT_ICC_VERSION)
# pragma clang diagnostic pop
#endif

#endif  // FMT_FORMAT_H_
