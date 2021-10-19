#ifndef __UTIL_H__
#define __UTIL_H__

#include <unordered_map>

/* Convenience method for grabbing a value from a map if its key exists,
 * or using a default value if no such key is in the map.
 */
template <typename K, typename V>
V GetWithDefault(const  std::unordered_map<K, V>& m, const K& key, const V& defval) {
  auto it = m.find(key);
  if (it == m.end()) {
    return defval;
  }
  else {
    return it->second;
  }
}

/* Convenience method for hashing a std::vector, which allows it
 * to be used as a key in a hashmap.
 */
template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
  seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
namespace std
{
  template<typename T>
  struct hash<vector<T>>
  {
    typedef vector<T> argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& in) const
    {
      size_t size = in.size();
      size_t seed = 0;
      for (size_t i = 0; i < size; i++)
        //Combine the hash of the current vector with the hashes of the previous ones
        hash_combine(seed, in[i]);
      return seed;
    }
  };
}


/* Check is vector a is a subsequence inside of vector b.  Useful
 * for debugging incoming packets to see if they contain a particular
 * sequence.
 */ 
template <typename T>
bool is_subsequence(const std::vector<T>& a, const std::vector<T> b) {
  int a_pos = 0;
  for (int i = 0; i < b.size(); i++) {
    if (b[i] == a[a_pos]) {
      a_pos++;
      if (a_pos == a.size())
        return true;
    }
    else {
      a_pos = 0;
    }
  }
  return false;
}


#endif __UTIL_H__