#include <immintrin.h> // portable to all x86 compilers
#include <stdio.h>

int show(__m256i *vector, char * note) {
  printf("%s\n", note);
  size_t n = sizeof(__m256i);
  printf("%d\n", n);
  char buffer[n] __attribute__ ((__aligned__(32)));
  _mm256_store_si256((__m256i*)buffer, *vector);
  for (int i = 0; i < n; i++) {
    printf("%d ", buffer[i]);
  }
  printf("\n");
  return 0;
}

int main() {
  // __m128 vector1 = _mm_set_ps(
  //     4.0, 3.0, 2.0,
  //     1.0); // high element first, opposite of C array order.  Use
  //     _mm_setr_ps
  //           // if you want "little endian" element order in the source.
  // __m128 vector2 = _mm_set_ps(7.0, 8.0, 9.0, 0.0);

  __m256i vector5 = _mm256_set_epi8(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
                                    25, 26, 27, 28, 29, 30, 31, 32);
  show(&vector5, "data vector");

  __m256i cmpvector = _mm256_set_epi8(
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16);
  
  show(&cmpvector, "CMP vector");
  __m256i cmpvector2 = _mm256_set_epi8(8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8);
  
  
  __m256i result = _mm256_cmpeq_epi8(vector5, cmpvector);
  show(&result, "CMP == 16 vector");

    __m256i result2 = _mm256_cmpeq_epi8(vector5, cmpvector2);
  show(&result2, "CMP == 8 vector");

  result = result << 8;
  show(&result, "Rotated left");
  result2 = result2 << 16;

  __m256i result3 = result | result2;
  show(&result3, "ORed");
  
  
  // _mm256_cmpgt_epi64
  // __m256 vector4 = _mm256_cmpgt_epi8(a, b, c);

  // __m128 sum = _mm_add_ps(vector1, vector2); // result = vector1 + vector 2

  // vector1 = _mm_shuffle_ps(vector1, vector1, _MM_SHUFFLE(0, 1, 2, 3));
  // vector1 is now (1, 2, 3, 4) (above shuffle reversed it)
  return 0;
}