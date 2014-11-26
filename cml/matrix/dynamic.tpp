/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATRIX_DYNAMIC_TPP
#error "matrix/dynamic.tpp not included correctly"
#endif

#include <cml/common/exception.h>

namespace cml {

/* dynamic 'structors: */

template<class E, class A, typename BO, typename L>
matrix<E, dynamic<A>, BO, L>::matrix()
: m_data(0), m_rows(0), m_cols(0)
{
}

template<class E, class A, typename BO, typename L>
matrix<E, dynamic<A>, BO, L>::matrix(int rows, int cols)
: m_data(0), m_rows(0), m_cols(0)
{
  this->resize_fast(rows,cols);
}

template<class E, class A, typename BO, typename L>
matrix<E, dynamic<A>, BO, L>::matrix(const matrix_type& other)
: m_data(0), m_rows(0), m_cols(0)
{
  this->assign(other);
}

template<class E, class A, typename BO, typename L>
matrix<E, dynamic<A>, BO, L>::matrix(matrix_type&& other)
: m_data(0), m_rows(0), m_cols(0)
{
  this->operator=(std::move(other));
}

template<class E, class A, typename BO, typename L> template<class Sub>
matrix<E, dynamic<A>, BO, L>::matrix(const readable_matrix<Sub>& sub)
: m_data(0), m_rows(0), m_cols(0)
{
  this->assign(sub);
}

#if 0
template<class E, class A, typename BO, typename L>
template<class E0, class... Es,
  typename cml::enable_if_convertible<
  typename scalar_traits<E>::value_type, E0, Es...>::type*
  >
matrix<E, dynamic<A>, BO, L>::matrix(
  int rows, int cols, const E0& e0, const Es&... eN
  )
: m_data(0), m_rows(0), m_cols(0)
{
  this->resize_fast(rows,cols);
  this->assign_elements(e0, eN...);
}
#endif

template<class E, class A, typename BO, typename L> template<class Array>
matrix<E, dynamic<A>, BO, L>::matrix(
  int rows, int cols, const Array& array, cml::enable_if_array_t<Array>*
  )
: m_data(0), m_rows(0), m_cols(0)
{
  this->resize_fast(rows,cols);
  this->assign(array);
}

template<class E, class A, typename BO, typename L>
template<class Other, int R, int C>
matrix<E, dynamic<A>, BO, L>::matrix(Other const (&array)[R][C])
: m_data(0), m_rows(0), m_cols(0)
{
 this->assign(array);
}

template<class E, class A, typename BO, typename L>
matrix<E, dynamic<A>, BO, L>::~matrix()
{
  typedef typename allocator_type::size_type size_type;
  int n = this->m_rows*this->m_cols;
  this->destruct(this->m_data, n,
    typename std::is_trivially_destructible<E>::type());
  allocator_type().deallocate(this->m_data, size_type(n));
}



/* Public methods: */

template<class E, class A, typename BO, typename L> int
matrix<E, dynamic<A>, BO, L>::rows() const
{
  return this->m_rows;
}

template<class E, class A, typename BO, typename L> int
matrix<E, dynamic<A>, BO, L>::cols() const
{
  return this->m_cols;
}

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::get(int i, int j) -> mutable_value
{
  return this->m_data[
    (L::value == row_major_c) ? (i*this->m_cols + j) : (j*this->m_rows + i)
    ];
}

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::get(int i, int j) const -> immutable_value
{
  return this->m_data[
    (L::value == row_major_c) ? (i*this->m_cols + j) : (j*this->m_rows + i)
    ];
}

template<class E, class A, typename BO, typename L>
template<class Other> auto matrix<E, dynamic<A>, BO, L>::set(
  int i, int j, const Other& v
  ) __CML_REF -> matrix_type&
{
  this->get(i,j) = v;
  return *this;
}

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
template<class E, class A, typename BO, typename L>
template<class Other> auto matrix<E, dynamic<A>, BO, L>::set(
  int i, int j, const Other& v
  ) && -> matrix_type&&
{
  this->set(i,v);
  return (matrix_type&&) *this;
}
#endif

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::data() -> pointer
{
  return this->m_data;
}

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::data() const -> const_pointer
{
  return this->m_data;
}

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::begin() const -> const_pointer
{
  return this->m_data;
}

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::end() const -> const_pointer
{
  return this->m_data + this->m_rows*this->m_cols;
}

template<class E, class A, typename BO, typename L> void
matrix<E, dynamic<A>, BO, L>::resize(int rows, int cols)
{
  cml_require(rows >= 0, std::invalid_argument, "rows < 0");
  cml_require(cols >= 0, std::invalid_argument, "cols < 0");

  int n_old = this->m_rows*this->m_cols;
  int n_new = rows*cols;

  /* Short-circuit same size: */
  if(n_new == n_old) return;

  /* Allocator to use: */
  auto allocator = allocator_type();

  /* Allocate the new array: */
  pointer data = this->m_data;
  pointer copy = allocator.allocate(n_new);
  try {

    /* Destruct elements if necessary: */
    this->destruct(data, n_old,
      typename std::is_trivially_destructible<E>::type());

    /* Copy elements to the new array if necessary: */
    if(data) {
      int to = std::min(n_old, n_new);
      for(pointer src = data, dst = copy; src < data + to; ++ src, ++ dst) {
	allocator.construct(dst, *src);
      }

      /* Deallocate the old array: */
      allocator.deallocate(data, n_old);
    }
  } catch(...) {
    allocator_type().deallocate(copy, n_new);
    throw;
  }

  /* Save the new array: */
  this->m_data = copy;
  this->m_rows = rows;
  this->m_cols = cols;
}

template<class E, class A, typename BO, typename L> void
matrix<E, dynamic<A>, BO, L>::resize_fast(int rows, int cols)
{
  cml_require(rows >= 0, std::invalid_argument, "rows < 0");
  cml_require(cols >= 0, std::invalid_argument, "cols < 0");

  int n_old = this->m_rows*this->m_cols;
  int n_new = rows*cols;

  /* Short-circuit same size: */
  if(n_new == n_old) return;

  /* Allocator to use: */
  auto allocator = allocator_type();

  /* Allocate the new array: */
  pointer data = this->m_data;
  pointer copy = allocator.allocate(n_new);
  try {

    /* Destruct elements if necessary: */
    this->destruct(data, n_old,
      typename std::is_trivially_destructible<E>::type());

    /* Deallocate the old array: */
    allocator.deallocate(data, n_old);
  } catch(...) {
    allocator_type().deallocate(copy, n_new);
    throw;
  }

  /* Save the new array: */
  this->m_data = copy;
  this->m_rows = rows;
  this->m_cols = cols;
}


template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::operator=(const matrix_type& other)
-> matrix_type&
{
  return this->assign(other);
}

template<class E, class A, typename BO, typename L> auto
matrix<E, dynamic<A>, BO, L>::operator=(matrix_type&& other)
-> matrix_type&
{
  /* Ensure deletion of the current array, if any: */
  std::swap(this->m_data, other.m_data);
  std::swap(this->m_rows, other.m_rows);
  std::swap(this->m_cols, other.m_cols);
  /* Note: swap() can't throw here, so this is exception-safe. */

  return *this;
}



/* Internal methods: */

template<class E, class A, typename BO, typename L> void
matrix<E, dynamic<A>, BO, L>::destruct(pointer, int, std::true_type)
{
  /* Nothing to do. */
}

template<class E, class A, typename BO, typename L> void
matrix<E, dynamic<A>, BO, L>::destruct(pointer data, int n, std::false_type)
{
  /* Short-circuit null: */
  if(data == nullptr) return;

  /* Destruct each element: */
  else for(pointer e = data; e < data + n; ++ e) allocator_type().destroy(e);
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
