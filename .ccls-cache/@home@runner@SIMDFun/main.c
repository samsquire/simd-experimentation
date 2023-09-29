#include <ctype.h>
#include <immintrin.h> // portable to all x86 compilers
#include <stdio.h>
#include <string.h>

struct SimdDataParse {
  int pos;
  int end;
  char *data;
  int datalength;
  char *last_char;
};
struct Token {
  char *string;
  int value;
  int length;
};

int show(__m256i *vector, char *note) {
  printf("%s\n", note);
  size_t n = sizeof(__m256i);
  printf("%d\n", n);
  char buffer[n] __attribute__((__aligned__(32)));
  _mm256_store_si256((__m256i *)buffer, *vector);
  for (int i = 0; i < n; i++) {
    printf("%d ", buffer[i]);
  }
  printf("\n");
  return 0;
}

char *get_char(struct SimdDataParse *parse) {
  char *this_char = malloc(sizeof(char) * 2);
  this_char[1] = '\0';
  if (parse->pos + 1 >= parse->datalength) {
    parse->end = 1;
    this_char[0] = parse->data[parse->pos];
    parse->last_char = this_char;

    return parse->last_char;
  }

  // printf("pos%d\n", parse->pos);

  this_char[0] = parse->data[parse->pos];
  // printf("%s\n", this_char);

  parse->last_char = this_char;
  parse->pos = parse->pos + 1;
  return parse->last_char;
}

struct Token *parse_token(struct SimdDataParse *parse) {
  while (parse->end == 0 && ((strcmp(parse->last_char, "\n") == 0) ||
                             (strcmp(parse->last_char, " ") == 0))) {
    // printf("cmpnewline %d\n", strcmp(parse->last_char, "\n"));
    // printf("cmpspace %d\n", strcmp(parse->last_char, " "));
    get_char(parse);
    // printf("consuming empty\n");
  }
  if (isdigit(parse->last_char[0])) {
    // printf("%s", "was a number\n");
    char *num __attribute__((__aligned__(8))) = malloc(sizeof(char) * 100);
    memset(num, 0, 100);

    int pos = 0;
    struct Token *token = malloc(sizeof(struct Token));
    token->string = num;
    token->length = 0;
    token->value = 0;
    while (pos < 99 && parse->end == 0 && isdigit(parse->last_char[0])) {
      // printf("%s", "found another number\n");
      num[pos] = parse->last_char[0];

      get_char(parse);
      pos++;
    }
    token->value = atoi(num);
    token->length = pos;
    return token;
  }
  printf("Not a number\n");
  return 0;
}

int main(int argc, char **argv) {
  printf("%d\n", argc);
  if (argc < 3) {
    printf("usage: example.simd example.data\n");
    exit(0);
  }
  char *simdprogram = 0;
  long length;
  FILE *f = fopen(argv[0], "r");

  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    simdprogram = malloc(length);
    if (simdprogram) {
      fread(simdprogram, 1, length, f);
    }
    fclose(f);
  }

  char *simddata = 0;
  long data_length;
  FILE *d = fopen(argv[2], "r");

  if (d) {
    fseek(d, 0, SEEK_END);
    data_length = ftell(d);
    fseek(d, 0, SEEK_SET);
    simddata = malloc(data_length);
    if (simddata) {
      fread(simddata, 1, data_length, d);
    }
    fclose(d);
  }

  if (simddata && simdprogram) {
    printf("got both simd data and program\n");
    printf("%s\n", simddata);
    printf("data_length %ld\n", data_length);
    __m256i lines[100];
    __m256i comparisons[100];

    int linescount = 0;
    int datacount = 0;
    struct SimdDataParse *dataparse = malloc(sizeof(struct SimdDataParse));
    dataparse->data = simddata;
    dataparse->end = 0;
    dataparse->pos = 0;
    dataparse->last_char = " ";
    dataparse->datalength = data_length;
    char numbers[32];
    while (dataparse->end == 0) {
      int items = 0;

      while (dataparse->end == 0 && items < 32) {

        struct Token *token = parse_token(dataparse);

        

        numbers[items] = (char)token->value;
        items++;

        // printf("got token [%s]\n", token->string);
      }
      __m256i line = _mm256_loadu_si256(numbers);
      printf("linescount %d\n", linescount);
      lines[datacount] = line;
      // __m256i added = _mm256_add_epi8(line, line);

      // show(&added, "Added");

      __m256i cmpvector2 =
          _mm256_set_epi8(8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                          8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8);
      show(&cmpvector2, "Compare vector2");

      __m256i cmpvector3 =
          _mm256_set_epi8(15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
                          15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15);
      show(&cmpvector3, "Compare vector3");

      int mask = 8;
      int mask2 = 16;

      comparisons[linescount++] = _mm256_cmpeq_epi8(line, cmpvector2);
      comparisons[linescount++] = _mm256_cmpeq_epi8(line, cmpvector3);
      
      datacount++;
    }

    for (int x = 0; x < linescount; x++) {
      printf("Comparison for %d\n", x);
      show(&comparisons[x], "Comparison");
      unsigned char buffer[sizeof(__m256i) / sizeof(unsigned char)]
          __attribute__((__aligned__(32)));
      _mm256_store_si256((__m256i *)buffer, comparisons[x]);
  
printf("Mask response\n");
      for (int y = 0; y < 32; y++) {
        char result = 0;
         if (buffer[y] == 255) {
           result = 1 << x;
         }
         printf("%d ", result);
      }
    
    printf("\n");
      printf("\n");
  }
  }
  __m256i offset = _mm256_set_epi64x(8, 8, 8, 8);
  __m256i memresult = _mm256_add_epi64(offset, offset);

  // accessing an object key in assembly
  /*
  hashmap->key[hsh].len = key_length;
  276f: 48 8b 4d e8           mov    -0x18(%rbp),%rcx
  2773: 48 8b 55 f8           mov    -0x8(%rbp),%rdx
  2777: 48 89 d0              mov    %rdx,%rax
  277a: 48 c1 e0 08           shl    $0x8,%rax
  277e: 48 01 d0              add    %rdx,%rax
  2781: 48 c1 e0 02           shl    $0x2,%rax
  2785: 48 01 c8              add    %rcx,%rax
  2788: 48 8d 90 04 04 00 00  lea    0x404(%rax),%rdx
  278f: 8b 45 d4              mov    -0x2c(%rbp),%eax
  2792: 89 02                 mov    %eax,(%rdx)
    */

  // start to process your data / extract strings here...

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
  __m256i cmpvector2 =
      _mm256_set_epi8(8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                      8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8);

  __m256i result = _mm256_cmpeq_epi8(vector5, cmpvector);
  show(&result, "CMP == 16 vector");

  __m256i result2 = _mm256_cmpeq_epi8(vector5, cmpvector2);
  show(&result2, "CMP == 8 vector");

  __m256i shufflevector =
      _mm256_set_epi8(8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                      8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8);

  //     result = result << 8;
  // show(&result, "Rotated left");
  // result2 = result2 << 16;

  // __m256i result3 = result | result2;
  // show(&result3, "ORed");

  // _mm256_cmpgt_epi64
  // __m256 vector4 = _mm256_cmpgt_epi8(a, b, c);

  // __m128 sum = _mm_add_ps(vector1, vector2); // result = vector1 + vector 2

  // vector1 = _mm_shuffle_ps(vector1, vector1, _MM_SHUFFLE(0, 1, 2, 3));
  // vector1 is now (1, 2, 3, 4) (above shuffle reversed it)
  return 0;
}