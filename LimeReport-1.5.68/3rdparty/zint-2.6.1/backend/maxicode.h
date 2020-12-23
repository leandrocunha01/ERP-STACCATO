/* maxicode.h - Handles Maxicode */

/*
    libzint - the open source barcode library
    Copyright (C) 2008-2017 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 */

static const unsigned short int MaxiGrid[] = {
    /* ISO/IEC 16023 Figure 5 - MaxiCode Module Sequence */ /* 30 x 33 data grid */
    122,
    121,
    128,
    127,
    134,
    133,
    140,
    139,
    146,
    145,
    152,
    151,
    158,
    157,
    164,
    163,
    170,
    169,
    176,
    175,
    182,
    181,
    188,
    187,
    194,
    193,
    200,
    199,
    0,
    0,
    124,
    123,
    130,
    129,
    136,
    135,
    142,
    141,
    148,
    147,
    154,
    153,
    160,
    159,
    166,
    165,
    172,
    171,
    178,
    177,
    184,
    183,
    190,
    189,
    196,
    195,
    202,
    201,
    817,
    0,
    126,
    125,
    132,
    131,
    138,
    137,
    144,
    143,
    150,
    149,
    156,
    155,
    162,
    161,
    168,
    167,
    174,
    173,
    180,
    179,
    186,
    185,
    192,
    191,
    198,
    197,
    204,
    203,
    819,
    818,
    284,
    283,
    278,
    277,
    272,
    271,
    266,
    265,
    260,
    259,
    254,
    253,
    248,
    247,
    242,
    241,
    236,
    235,
    230,
    229,
    224,
    223,
    218,
    217,
    212,
    211,
    206,
    205,
    820,
    0,
    286,
    285,
    280,
    279,
    274,
    273,
    268,
    267,
    262,
    261,
    256,
    255,
    250,
    249,
    244,
    243,
    238,
    237,
    232,
    231,
    226,
    225,
    220,
    219,
    214,
    213,
    208,
    207,
    822,
    821,
    288,
    287,
    282,
    281,
    276,
    275,
    270,
    269,
    264,
    263,
    258,
    257,
    252,
    251,
    246,
    245,
    240,
    239,
    234,
    233,
    228,
    227,
    222,
    221,
    216,
    215,
    210,
    209,
    823,
    0,
    290,
    289,
    296,
    295,
    302,
    301,
    308,
    307,
    314,
    313,
    320,
    319,
    326,
    325,
    332,
    331,
    338,
    337,
    344,
    343,
    350,
    349,
    356,
    355,
    362,
    361,
    368,
    367,
    825,
    824,
    292,
    291,
    298,
    297,
    304,
    303,
    310,
    309,
    316,
    315,
    322,
    321,
    328,
    327,
    334,
    333,
    340,
    339,
    346,
    345,
    352,
    351,
    358,
    357,
    364,
    363,
    370,
    369,
    826,
    0,
    294,
    293,
    300,
    299,
    306,
    305,
    312,
    311,
    318,
    317,
    324,
    323,
    330,
    329,
    336,
    335,
    342,
    341,
    348,
    347,
    354,
    353,
    360,
    359,
    366,
    365,
    372,
    371,
    828,
    827,
    410,
    409,
    404,
    403,
    398,
    397,
    392,
    391,
    80,
    79,
    0,
    0,
    14,
    13,
    38,
    37,
    3,
    0,
    45,
    44,
    110,
    109,
    386,
    385,
    380,
    379,
    374,
    373,
    829,
    0,
    412,
    411,
    406,
    405,
    400,
    399,
    394,
    393,
    82,
    81,
    41,
    0,
    16,
    15,
    40,
    39,
    4,
    0,
    0,
    46,
    112,
    111,
    388,
    387,
    382,
    381,
    376,
    375,
    831,
    830,
    414,
    413,
    408,
    407,
    402,
    401,
    396,
    395,
    84,
    83,
    42,
    0,
    0,
    0,
    0,
    0,
    6,
    5,
    48,
    47,
    114,
    113,
    390,
    389,
    384,
    383,
    378,
    377,
    832,
    0,
    416,
    415,
    422,
    421,
    428,
    427,
    104,
    103,
    56,
    55,
    17,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    21,
    20,
    86,
    85,
    434,
    433,
    440,
    439,
    446,
    445,
    834,
    833,
    418,
    417,
    424,
    423,
    430,
    429,
    106,
    105,
    58,
    57,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    23,
    22,
    88,
    87,
    436,
    435,
    442,
    441,
    448,
    447,
    835,
    0,
    420,
    419,
    426,
    425,
    432,
    431,
    108,
    107,
    60,
    59,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    24,
    90,
    89,
    438,
    437,
    444,
    443,
    450,
    449,
    837,
    836,
    482,
    481,
    476,
    475,
    470,
    469,
    49,
    0,
    31,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    54,
    53,
    464,
    463,
    458,
    457,
    452,
    451,
    838,
    0,
    484,
    483,
    478,
    477,
    472,
    471,
    50,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    466,
    465,
    460,
    459,
    454,
    453,
    840,
    839,
    486,
    485,
    480,
    479,
    474,
    473,
    52,
    51,
    32,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    0,
    43,
    468,
    467,
    462,
    461,
    456,
    455,
    841,
    0,
    488,
    487,
    494,
    493,
    500,
    499,
    98,
    97,
    62,
    61,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    27,
    92,
    91,
    506,
    505,
    512,
    511,
    518,
    517,
    843,
    842,
    490,
    489,
    496,
    495,
    502,
    501,
    100,
    99,
    64,
    63,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    29,
    28,
    94,
    93,
    508,
    507,
    514,
    513,
    520,
    519,
    844,
    0,
    492,
    491,
    498,
    497,
    504,
    503,
    102,
    101,
    66,
    65,
    18,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    19,
    30,
    96,
    95,
    510,
    509,
    516,
    515,
    522,
    521,
    846,
    845,
    560,
    559,
    554,
    553,
    548,
    547,
    542,
    541,
    74,
    73,
    33,
    0,
    0,
    0,
    0,
    0,
    0,
    11,
    68,
    67,
    116,
    115,
    536,
    535,
    530,
    529,
    524,
    523,
    847,
    0,
    562,
    561,
    556,
    555,
    550,
    549,
    544,
    543,
    76,
    75,
    0,
    0,
    8,
    7,
    36,
    35,
    12,
    0,
    70,
    69,
    118,
    117,
    538,
    537,
    532,
    531,
    526,
    525,
    849,
    848,
    564,
    563,
    558,
    557,
    552,
    551,
    546,
    545,
    78,
    77,
    0,
    34,
    10,
    9,
    26,
    25,
    0,
    0,
    72,
    71,
    120,
    119,
    540,
    539,
    534,
    533,
    528,
    527,
    850,
    0,
    566,
    565,
    572,
    571,
    578,
    577,
    584,
    583,
    590,
    589,
    596,
    595,
    602,
    601,
    608,
    607,
    614,
    613,
    620,
    619,
    626,
    625,
    632,
    631,
    638,
    637,
    644,
    643,
    852,
    851,
    568,
    567,
    574,
    573,
    580,
    579,
    586,
    585,
    592,
    591,
    598,
    597,
    604,
    603,
    610,
    609,
    616,
    615,
    622,
    621,
    628,
    627,
    634,
    633,
    640,
    639,
    646,
    645,
    853,
    0,
    570,
    569,
    576,
    575,
    582,
    581,
    588,
    587,
    594,
    593,
    600,
    599,
    606,
    605,
    612,
    611,
    618,
    617,
    624,
    623,
    630,
    629,
    636,
    635,
    642,
    641,
    648,
    647,
    855,
    854,
    728,
    727,
    722,
    721,
    716,
    715,
    710,
    709,
    704,
    703,
    698,
    697,
    692,
    691,
    686,
    685,
    680,
    679,
    674,
    673,
    668,
    667,
    662,
    661,
    656,
    655,
    650,
    649,
    856,
    0,
    730,
    729,
    724,
    723,
    718,
    717,
    712,
    711,
    706,
    705,
    700,
    699,
    694,
    693,
    688,
    687,
    682,
    681,
    676,
    675,
    670,
    669,
    664,
    663,
    658,
    657,
    652,
    651,
    858,
    857,
    732,
    731,
    726,
    725,
    720,
    719,
    714,
    713,
    708,
    707,
    702,
    701,
    696,
    695,
    690,
    689,
    684,
    683,
    678,
    677,
    672,
    671,
    666,
    665,
    660,
    659,
    654,
    653,
    859,
    0,
    734,
    733,
    740,
    739,
    746,
    745,
    752,
    751,
    758,
    757,
    764,
    763,
    770,
    769,
    776,
    775,
    782,
    781,
    788,
    787,
    794,
    793,
    800,
    799,
    806,
    805,
    812,
    811,
    861,
    860,
    736,
    735,
    742,
    741,
    748,
    747,
    754,
    753,
    760,
    759,
    766,
    765,
    772,
    771,
    778,
    777,
    784,
    783,
    790,
    789,
    796,
    795,
    802,
    801,
    808,
    807,
    814,
    813,
    862,
    0,
    738,
    737,
    744,
    743,
    750,
    749,
    756,
    755,
    762,
    761,
    768,
    767,
    774,
    773,
    780,
    779,
    786,
    785,
    792,
    791,
    798,
    797,
    804,
    803,
    810,
    809,
    816,
    815,
    864,
    863};

static const char maxiCodeSet[256] = {
    /* from Appendix A - ASCII character to Code Set (e.g. 2 = Set B) */
    /* set 0 refers to special characters that fit into more than one set (e.g. GS) */
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 5, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2, 2, 2, 2, 2,
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 5, 5, 5, 5, 5, 5, 4, 5, 3, 4, 3, 5, 5, 4, 4, 3, 3, 3, 4, 3, 5, 4, 4, 3, 3, 4, 3, 3, 3, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};

static const char maxiSymbolChar[256] = {
    /* from Appendix A - ASCII character to symbol value */
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 30, 28, 29, 30, 35, 32, 53, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 37, 38, 39, 40, 41, 52, 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 42, 43, 44, 45, 46, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 32, 54, 34, 35, 36, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 36, 37, 37, 38, 39, 40, 41, 42, 43, 38, 44, 37, 39,
    38, 45, 46, 40, 41, 39, 40, 41, 42, 42, 47, 43, 44, 43, 44, 45, 45, 46, 47, 46, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 32, 33, 34, 35, 36, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 32, 33, 34, 35, 36};
