/* -*- C++ -*- ------------------------------------------------------------
 @@COPYRIGHT@@
 *-----------------------------------------------------------------------*/
/** @file
 */

#ifndef __CML_MATHLIB_RANDOM_UNIT_TPP
#error "mathlib/random_unit.tpp not included correctly"
#endif

#include <cml/scalar/functions.h>
#include <cml/vector/detail/check_or_resize.h>
#include <cml/vector/writable_vector.h>
#include <cml/vector/dot.h>

namespace cml {
namespace detail {

/** Generate a random 2D unit vector in a cone with direction @c d and
 * half-angle @c a, given in radians.
 */
template<class Sub1, class Sub2, class Scalar> void
random_unit(writable_vector<Sub1>& n,
  const readable_vector<Sub2>& d, const Scalar& a, cml::int_c<2>
  )
{
  typedef value_type_trait_of_t<Sub1>			value_type;
  typedef scalar_traits<value_type>			value_traits;

  /* Use the default generator: */
  std::default_random_engine gen(std::rand());

  /* Generate a uniformly random angle in [-a,a]: */
  auto theta = (std::uniform_real_distribution<value_type>(-a, a))(gen);

  /* sin(theta) and cos(theta): */
  auto st = value_traits::sin(theta);
  auto ct = value_traits::cos(theta);

  /* Compute n by rotating d by theta: */
  n[0] = ct * d[0] - st * d[1];
  n[1] = st * d[0] + ct * d[1];

  /* Normalize: */
  n.normalize();
}

/** Generate a random n-D unit vector in a cone with direction @c d and
 * half-angle @c a, given in radians.  The vector is generated by using two
 * slerps to linearly map a random vector on the unit hemisphere to the
 * spherical cone cap.  This seems to produce nice (uniform) 3D
 * distributions, at least visually.
 */
template<class Sub1, class Sub2, class Scalar, int N> void
random_unit(writable_vector<Sub1>& n,
  const readable_vector<Sub2>& d, const Scalar& a, cml::int_c<N>
  )
{
  typedef value_type_trait_of_t<Sub1>			value_type;
  typedef scalar_traits<value_type>			value_traits;

  /* Generate a uniformly random vector on the unit sphere: */
  cml::random_unit(n);

  /* Reorient n to be within 90 degrees of d: */
  auto cos_O = dot(n,d);	// [-1,1]
  if(cos_O < 0) {
    n = -n;
    cos_O = - cos_O;
  }

  /* Compute the angle between d and n: */
  auto O = acos_safe(cos_O);

  /* Needed below: */
  auto sin = &value_traits::sin;

  /* Use slerp between d (t=0) and n (t=1) to find the unit vector n_a
   * (t=a/O) lying on the cone between d and n:
   */
  auto n_a = (sin(O - a)*d + sin(a)*n) / sin(O);
  /* Note: n_a is normalized by dividing by sin(O). */

  /* Use a second slerp to "scale" the cone with half-angle O to the cone
   * with half-angle a, taking the random vector n along with it:
   */
  auto t = O / constants<value_type>::pi_over_2();
  n = (sin((1 - t)*a)*d + sin(t*a)*n_a) / sin(a);
  /* Note: n is normalized by dividing by sin(a). */
}

} // namespace detail


template<class Sub, class RNG> inline void
random_unit(writable_vector<Sub>& n, RNG& gen)
{
  typedef value_type_trait_of_t<Sub>			value_type;
  static_assert(std::is_floating_point<value_type>::value,
    "floating-point coordinates required");
  cml::check_minimum_size(n, cml::int_c<1>());

  /* Generate coordinates using a normal distribution having mean of 0 and
   * standard deviation of 1:
   */
  std::normal_distribution<value_type> d(value_type(0), value_type(1));

  /* Generate coordinates, avoiding the (improbable) case of 0 length: */
  value_type length(0);
  do {
    for(int i = 0; i < n.size(); ++ i) n[i] = d(gen);
    length = n.length_squared();
  } while(length == value_type(0));

  /* Normalize the vector: */
  n /= scalar_traits<value_type>::sqrt(length);
}

template<class Sub> inline void
random_unit(writable_vector<Sub>& n)
{
  /* Use the default generator: */
  std::default_random_engine gen(std::rand());

  /* Generate n: */
  random_unit(n, gen);
}

template<class Sub1, class Sub2, class Scalar>
void random_unit(
  writable_vector<Sub1>& n, const readable_vector<Sub2>& d, const Scalar& a
  )
{
  typedef value_type_trait_of_t<Sub1>			value_type;
  static_assert(std::is_floating_point<value_type>::value,
    "floating-point coordinates required");
  cml_require(a > 0 && a <= constants<value_type>::pi_over_2(),
    std::invalid_argument, "a must be in (0,90] deg");

  cml::detail::check_or_resize(n, d);
  detail::random_unit(n, d, a, cml::int_c<array_size_of_c<Sub2>::value>());
}

} // namespace cml

// -------------------------------------------------------------------------
// vim:ft=cpp:sw=2
