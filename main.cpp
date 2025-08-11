#include <cstddef>
#include <cstdint>
#include <iostream>
#include <thread>
#include <variant>
#include <type_traits>

namespace my {
template<typename... Ts>
struct over : Ts... { using Ts::operator()...; };

template<typename T>
struct Ok {
  Ok(T v) : val(std::forward<decltype(v)>(v)) {}

  const T& operator*() const {
    return val;
  }

  T& operator*() {
    return val;
  }

  T val;
};

template<typename T>
Ok(T&&) -> Ok<T&&>;


template<typename T>
struct Err {
  Err(T v) : val(std::forward<decltype(v)>(v)) {}

  const T& operator*() const {
    return val;
  }

  T& operator*() {
    return val;
  }

  T val;
};

template<typename T>
Err(T&&) -> Err<T&&>;

template<typename OkT, typename ErrT>
struct Result {
public:
  template<typename T>
  Result(Ok<T> &&val) : value_(std::in_place_type<ok_type>, *val) {}
  template<typename T>
  Result(Err<T> &&val) : value_(std::in_place_type<err_type>, *val) {}


  template<typename RetT, typename VarT, typename FuncT>
  friend RetT visit_impl(VarT &&var, FuncT &&func);

  template<typename RetT, typename FuncT>
  RetT visit(FuncT &&func) & {
    return visit_impl<RetT>(*this, std::forward<FuncT>(func));
  }

  template<typename RetT, typename FuncT>
  RetT visit(FuncT &&func) const & {
    return visit_impl<RetT>(*this, std::forward<FuncT>(func));
  }

  template<typename RetT, typename FuncT>
  RetT visit(FuncT &&func) && {
    return visit_impl<RetT>(std::move(*this), std::forward<FuncT>(func));
  }

  template<typename RetT, typename FuncT>
  RetT visit(FuncT &&func) const && {
    return visit_impl<RetT>(*this, std::forward<FuncT>(func));
  }

private:
  using ok_type = Ok<OkT>;
  using err_type = Err<ErrT>;

  std::variant<ok_type, err_type> value_;
};



template<typename RetT, typename VarT, typename FuncT>
RetT visit_impl(VarT &&var, FuncT &&func) {
  if (std::holds_alternative<typename VarT::ok_type>(var.value_)) {
    return func(std::get<typename VarT::ok_type>(std::forward<decltype(var)>(var).value_));
  } else {
    return func(std::get<typename VarT::err_type>(std::forward<decltype(var)>(var).value_));
  }
}
}

auto OkIfCond(bool cond) -> my::Result<int, int> {
    if (cond) {
      return my::Ok(1);
    } else {
      return my::Err(2.);
    }
}

int main() {
  std::cout << OkIfCond(true).visit<float>(my::over{
    [] (my::Ok<int> ok) { return *ok; },
    [] (my::Err<int> err) { return 0ULL; }
  });
  std::cout << OkIfCond(false).visit<int>(my::over{
    [] (my::Ok<int> ok) { return 1.; },
    [] (my::Err<int> err) { return 0ULL; }
  });
}
