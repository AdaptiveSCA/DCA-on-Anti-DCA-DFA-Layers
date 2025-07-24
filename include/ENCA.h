#include "WBMatrix/WBMatrix.h"
#include "math.h"
#include "string.h"
static const uint8_t Sbox[16] = {0x00, 0x03, 0x07, 0x0e, 0x0d, 0x04, 0x0a, 0x09, 0x0c, 0x0f, 0x01, 0x08, 0x0b, 0x02, 0x06, 0x05};
static const uint8_t real_key = 6;
void generate_map(uint8_t map[16]);

void DCA(uint8_t map[16]);
void IDCA(uint8_t map[16]);
void CPA(uint8_t map[16]);
void CA(uint8_t map[16]);
void SA(uint8_t map[16]);