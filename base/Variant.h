//
// Created by v4kst1z
//

#ifndef CPPNET_VARIANT_H
#define CPPNET_VARIANT_H

#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <functional>

template<typename T, typename ...TS>
struct TypeExist : std::true_type {};

template<typename T, typename H, typename ...R>
struct TypeExist<T, H, R...> : std::conditional<std::is_same<T, H>::value, std::true_type, TypeExist<T, R...>>::type {};

template<typename T>
struct TypeExist<T> : std::false_type {};

template<typename T>
struct function_traits;

template<typename T>
struct function_traits
    : public function_traits<decltype(&T::operator())> {
};

template<typename CT, typename R, typename ...Args>
struct function_traits<R(CT::*)(Args...) const> {
 public:
  static const size_t nargs = sizeof...(Args);

  using result_type = R;

  template<size_t i>
  struct arg {
   public:
    typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
  };
};

template<typename R, typename ...Args>
struct function_traits<R(Args...) const> {
 public:
  static const size_t nargs = sizeof...(Args);

  using result_type = R;

  template<size_t i>
  struct arg {
   public:
    typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
  };
};

template<typename ... T>
struct VariantHelper;

template<typename F, typename ...R>
struct VariantHelper<F, R...> {
  static void Destory(const std::type_index type_id, void *data) {
    if (type_id == std::type_index(typeid(F)))
      reinterpret_cast<F *>(data)->~F();
    else
      VariantHelper<R...>::Destory(type_id, data);
  }

  static void Move(const std::type_index old_type_id, void *old_addr, void *new_addr) {
    if (old_type_id == std::type_index(typeid(F)))
      new(new_addr) F(std::move(*reinterpret_cast<F *>(old_addr)));
    else
      VariantHelper<R...>::Move(old_type_id, old_addr, new_addr);
  }

  static void Copy(const std::type_index old_type_id, void *old_addr, void *new_addr) {
    if (old_type_id == std::type_index(typeid(F)))
      new(new_addr) F(*reinterpret_cast<F *>(old_addr));
    else
      VariantHelper<R...>::Copy(old_type_id, old_addr, new_addr);
  }
};

template<>
struct VariantHelper<> {
  static void Destory(const std::type_index type_id, void *data) {}
  static void Move(const std::type_index old_type_id, void *old_addr, void *new_addr) {}
  static void Copy(const std::type_index old_type_id, void *old_addr, void *new_addr) {}
};

template<typename ...T>
class Variant {
 public:
  Variant() : mem_type_id_(typeid(void)) {}

  Variant(Variant<T...> &v) : mem_type_id_(v.mem_type_id_) {
    VariantHelper<T...>::Copy(v.mem_type_id_, &v.data_, &data_);
  }

  Variant(Variant<T...> &&v) : mem_type_id_(v.mem_type_id_) {
    VariantHelper<T...>::Move(v.mem_type_id_, &v.data_, &data_);
  }

  Variant &operator=(const Variant<T...> &v) {
    mem_type_id_ = v.mem_type_id_;
    VariantHelper<T...>::Copy(v.mem_type_id_, &v.data_, &data_);
    return *this;
  }

  template<typename V,
      typename = typename std::enable_if<TypeExist<typename std::remove_reference<V>::type, T...>::value>::type>
  Variant(V &&value) : mem_type_id_(typeid(void)) {
    if (mem_type_id_ != std::type_index(typeid(void)))
      VariantHelper<T...>::Destory(mem_type_id_, &data_);
    using U = typename std::remove_reference<V>::type;
    new(&data_) U(std::forward<V>(value));
    mem_type_id_ = std::type_index(typeid(V));
  }

  ~Variant() {
    VariantHelper<T...>::Destory(mem_type_id_, &data_);
  }

  template<typename V,
      typename = typename std::enable_if<TypeExist<typename std::remove_reference<V>::type, T...>::value>::type>
  void Set(V &&value) {
    if (mem_type_id_ != std::type_index(typeid(void)))
      VariantHelper<T...>::Destory(mem_type_id_, &data_);
    using U = typename std::remove_reference<V>::type;
    new(&data_) U(std::forward<V>(value));
    mem_type_id_ = std::type_index(typeid(V));
  }

  template<typename V>
  bool Is() const {
    return mem_type_id_ == std::type_index(typeid(typename std::remove_reference<V>::type));
  }

  template<typename V>
  V &Get() {
    if (Is<V>())
      return *reinterpret_cast<typename std::remove_reference<V>::type *>(&data_);
    else
      throw std::bad_cast();
  }

  template<typename F>
  void Visit(F &&f) {
#ifdef __clang__
    using V = typename function_traits<F>::template arg<0>::type;
#elif _MSC_VER
    using V = typename function_traits<F>::arg<0>::type;
#endif
    if (Is<V>())
      f(Get<V>());
  }

  template<typename F, typename... R>
  void Visit(F &&f, R &&... rest) {
#ifdef __clang__
    using V = typename function_traits<F>::template arg<0>::type;
#elif _MSC_VER
    using V = typename function_traits<F>::arg<0>::type;
#endif
    if (Is<V>())
      Visit(std::forward<F>(f));
    else
      Visit(std::forward<R>(rest)...);
  }

  bool IsValid() const {
    return mem_type_id_ != std::type_index(typeid(void));
  }

 private:
  using data_t = typename std::aligned_union<1, T...>::type;

  data_t data_;
  std::type_index mem_type_id_;
};
#endif //CPPNET_VARIANT_H
