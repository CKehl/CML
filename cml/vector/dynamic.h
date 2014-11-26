/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#pragma once

#ifndef	cml_vector_dynamic_h
#define	cml_vector_dynamic_h

#include <cml/common/mpl/enable_if_convertible.h>
#include <cml/common/dynamic_selector.h>
#include <cml/common/scalar_traits.h>
#include <cml/vector/writable_vector.h>
#include <cml/vector/vector.h>

namespace cml {

template<class Element, class Allocator>
struct vector_traits< vector<Element, dynamic<Allocator>> >
{
  typedef scalar_traits<Element>			element_traits;
  typedef typename element_traits::value_type		value_type;
  typedef typename element_traits::pointer		pointer;
  typedef typename element_traits::reference		reference;
  typedef typename element_traits::const_pointer	const_pointer;
  typedef typename element_traits::const_reference	const_reference;
  typedef typename element_traits::mutable_value	mutable_value;
  typedef typename element_traits::immutable_value	immutable_value;
  typedef dynamic_size_tag				size_tag;
  typedef vector<Element, dynamic<Allocator>>		temporary_type;
};

/** Resizable vector. */
template<class Element, class Allocator>
class vector<Element, dynamic<Allocator>>
: public writable_vector< vector<Element, dynamic<Allocator>> >
{
  protected:

    /** The real allocator type. */
    typedef typename Allocator::template
      rebind<Element>::other				allocator_type;

    /** Require a stateless allocator. */
    static_assert(std::is_empty<allocator_type>::value,
      "cannot use a stateful allocator for dynamic<> vectors");


  public:

    typedef vector<Element, dynamic<Allocator>>		vector_type;
    typedef writable_vector<vector_type>		writable_type;
    typedef vector_traits<vector_type>			traits_type;
    typedef typename traits_type::element_traits	element_traits;
    typedef typename traits_type::value_type		value_type;
    typedef typename traits_type::pointer		pointer;
    typedef typename traits_type::reference		reference;
    typedef typename traits_type::const_pointer		const_pointer;
    typedef typename traits_type::const_reference	const_reference;
    typedef typename traits_type::mutable_value		mutable_value;
    typedef typename traits_type::immutable_value	immutable_value;
    typedef typename traits_type::size_tag		size_tag;
    typedef typename traits_type::temporary_type	temporary_type;


  public:

#ifndef CML_HAS_MSVC_BRAIN_DEAD_ASSIGNMENT_OVERLOADS
    /* Include methods from writable_type: */
    using writable_type::operator=;
#endif


  public:

    /** Constant containing the array size. */
    static const int array_size = -1;


  public:

    /** Default constructor.
     *
     * @note The vector has no elements.
     */
    vector();

    /** Construct given a size.
     *
     * @throws std::invalid_argument if @c size < 0.
     */
    explicit vector(int size);

    /** Copy constructor. */
    vector(const vector_type& other);

    /** Move constructor. */
    vector(vector_type&& other);

    /** Construct from a readable_vector. */
    template<class Sub> vector(const readable_vector<Sub>& sub);

    /** Construct from at least 1 value.  The vector is resized to
     * accomodate the number of elements passed.
     *
     * @note This overload is enabled only if all of the arguments are
     * convertible to value_type.
     */
    template<class E0, class... Elements,
      typename cml::enable_if_convertible<
	value_type, E0, Elements...>::type* = nullptr>
	vector(const E0& e0, const Elements&... eN)
	// XXX Should be in vector/dynamic.tpp, but VC++12 has brain-dead
	// out-of-line template argument matching...
	: m_data(0), m_size(0)
	{
	  this->assign_elements(e0, eN...);
	}

    /** Construct from an array type. */
    template<class Array> vector(
      const Array& array, cml::enable_if_array_t<Array>* = 0);

    /** Construct from std::initializer_list. */
    template<class Other> vector(std::initializer_list<Other> l);

    /** Destructor. */
    ~vector();


  public:

    /** Return the length of the vector. */
    int size() const;

    /** Return vector element @c i. */
    mutable_value get(int i);

    /** Return vector const element @c i. */
    immutable_value get(int i) const;

    /** Set element @c i. */
    template<class Other> vector_type& set(int i, const Other& v) __CML_REF;

#ifdef CML_HAS_RVALUE_REFERENCE_FROM_THIS
    /** Set element @c i on a temporary. */
    template<class Other> vector_type&& set(int i, const Other& v) &&;
#endif

    /** Return access to the vector data as a raw pointer. */
    pointer data();

    /** Return const access to the vector data as a raw pointer. */
    const_pointer data() const;

    /** Read-only iterator. */
    const_pointer begin() const;

    /** Read-only iterator. */
    const_pointer end() const;

    /** Resize the vector to the specified size.
     *
     * @note This will reallocate the array and copy existing elements, if
     * any.
     *
     * @throws std::invalid_argument if @c n is negative.
     */
    void resize(int n);

    /** Resize the vector to the specified size without copying the old
     * elements.
     *
     * @throws std::invalid_argument if @c n is negative.
     */
    void resize_fast(int n);


  public:

    /** Copy assignment. */
    vector_type& operator=(const vector_type& other);

    /** Move assignment. */
    vector_type& operator=(vector_type&& other);

#ifdef CML_HAS_MSVC_BRAIN_DEAD_ASSIGNMENT_OVERLOADS
    template<class Other>
      inline vector_type& operator=(const readable_vector<Other>& other) {
	return this->assign(other);
      }

    template<class Array, typename cml::enable_if_array_t<Array>* = nullptr>
      inline vector_type& operator=(const Array& array) {
	return this->assign(array);
      }

    template<class Other>
      inline vector_type& operator=(std::initializer_list<Other> l) {
	return this->assign(l);
      }
#endif


  protected:

    /** No-op for trivially destructible elements
     * (is_trivially_destructible).
     */
    void destruct(pointer, int, std::true_type);

    /** Invoke non-trivial destructors for @c n elements starting at @c
     * data.
     */
    void destruct(pointer data, int n, std::false_type);


  protected:

    /** Dynamic storage. */
    pointer			m_data;

    /** Size of the vector. */
    int				m_size;
};

} // namespace cml

#define __CML_VECTOR_DYNAMIC_TPP
#include <cml/vector/dynamic.tpp>
#undef __CML_VECTOR_DYNAMIC_TPP

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
