#include <malloc.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>

#include "cmocka.h"

#include "buffer.h"


buffer_t* buffer;

static void test_bip32_length_limits( void** state )
{
  (void) state;

  uint8_t data[MAX_BIP32_PATH*4];
  uint32_t bip32Path[MAX_BIP32_PATH];
  const buffer_t buffer = { data, MAX_BIP32_PATH*4, 0 };

  // test bip32 length 0
  data[0] = 0;
  uint8_t bip32PathLength = buffer_get_bip32_path( &buffer, bip32Path );
  assert_int_equal( bip32PathLength, 0);

  // test bip32 length > MAX_BIP32_PATH
  data[0] = MAX_BIP32_PATH+1;
  bip32PathLength = buffer_get_bip32_path( &buffer, bip32Path );
  assert_int_equal( bip32PathLength, 0);
}


static void test_bip32_conversion( void** state )
{
  (void) state;

  uint8_t data[1 + MAX_BIP32_PATH*4] = { 5, 1,0,0,0, 2,0,0,0, 3,0,0,0, 4,0,0,0, 5,0,0,0 };
  uint32_t bip32Path[MAX_BIP32_PATH];
  const buffer_t buffer = { data, MAX_BIP32_PATH*4, 0 };

  // test bip32 conversion
  uint8_t bip32PathLength = buffer_get_bip32_path( &buffer, bip32Path );
  assert_int_equal( bip32PathLength, 5);
  assert_int_equal( bip32Path[0], 0x1000000 );
  assert_int_equal( bip32Path[1], 0x2000000 );
  assert_int_equal( bip32Path[2], 0x3000000 );
  assert_int_equal( bip32Path[3], 0x4000000 );
  assert_int_equal( bip32Path[4], 0x5000000 );
}


int main(void)
{
  const struct CMUnitTest tests[] =
    {
      cmocka_unit_test(test_bip32_length_limits),
      cmocka_unit_test(test_bip32_conversion)
    };
  
  return cmocka_run_group_tests(tests, NULL, NULL);
}
