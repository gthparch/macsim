#include <set>

#include "cache.h"

static int int_log2(int value) {
  int bits = 0;
  while (value >>= 1) {
    ++bits;
  }
  return bits;
}

static Addr n_bit_mask(int bits) {
  return bits ? ((static_cast<Addr>(1) << bits) - 1) : 0;
}

int main() {
  const int line_size = 64;
  const int line_shift = int_log2(line_size);

  {
    const int num_sets = 6;
    const int set_bits = int_log2(num_sets);
    const Addr set_mask = n_bit_mask(set_bits);
    const Addr tag_mask = ~set_mask;
    std::set<int> visited_sets;

    for (Addr addr = 0; addr < static_cast<Addr>(num_sets * 3 * line_size);
         addr += line_size) {
      Addr line = addr >> line_shift;
      Addr tag = 0;
      int set = -1;
      cache_line_to_set_and_tag(line, num_sets, set_bits, set_mask, tag_mask,
                                &tag, &set);
      visited_sets.insert(set);
    }

    if (visited_sets.size() != static_cast<size_t>(num_sets)) {
      return 1;
    }

    Addr tag0 = 0;
    int set0 = -1;
    cache_line_to_set_and_tag(0 >> line_shift, num_sets, set_bits, set_mask,
                              tag_mask, &tag0, &set0);
    Addr tag1 = 0;
    int set1 = -1;
    cache_line_to_set_and_tag((num_sets * line_size) >> line_shift, num_sets,
                              set_bits, set_mask, tag_mask, &tag1, &set1);
    if (set0 != set1 || tag0 == tag1) {
      return 1;
    }
  }

  {
    const int num_sets = 8;
    const int set_bits = int_log2(num_sets);
    const Addr set_mask = n_bit_mask(set_bits);
    const Addr tag_mask = ~set_mask;

    for (Addr addr = 0; addr < 64 * line_size; addr += line_size) {
      Addr line = addr >> line_shift;
      Addr tag = 0;
      int set = -1;
      cache_line_to_set_and_tag(line, num_sets, set_bits, set_mask, tag_mask,
                                &tag, &set);

      if (set != static_cast<int>(line & set_mask)) {
        return 1;
      }
      if (tag != (line & tag_mask)) {
        return 1;
      }
    }
  }

  return 0;
}
