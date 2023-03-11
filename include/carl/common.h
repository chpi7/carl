#ifndef carl_common_h
#define carl_common_h

#include <cassert>
#include <cstddef>
#include <utility>

#define CARL_VERSION "0.1"
#define DEBUG

namespace carl {
// Cheap version of std::expected from c++23 :)
template <typename T, typename E>
class Result {
   private:
    bool is_error;
    E error;
    T result;

   public:
    Result(Result<T, E>&&) = default;
    Result<T,E>& operator=(Result<T,E>&&) = default;

    Result(const Result<T, E> &other) = delete;
    Result<T,E>& operator=(const Result<T, E> &other) = delete;

    Result() = default;
    ~Result() = default;

    E& get_error() { return error; };

    static Result make_error(E error);
    static Result make_result(T result);

    explicit operator bool() const noexcept { return !is_error; }

    const T& operator*() { return result; }
};

template<typename T, typename E>
Result<T, E> Result<T,E>::make_result(T result) {
    Result<T,E> r;
    r.result = result;
    r.is_error = false;
    return r;
}

template<typename T, typename E>
Result<T, E> Result<T,E>::make_error(E error) {
    Result<T,E> r;
    r.error = error;
    r.is_error = true;
    return r;
}

}  // namespace carl
#endif