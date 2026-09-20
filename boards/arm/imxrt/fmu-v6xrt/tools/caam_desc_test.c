/****************************************************************************
 * boards/arm/imxrt/fmu-v6xrt/tools/caam_desc_test.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/* Checks the CAAM job descriptors the i.MX RT driver builds.
 *
 * The RT1170 reference manual describes CAAM in one page of features and
 * carries no register detail, so the encodings in imxrt_caam_desc.h were
 * taken from NXP's own drivers. This rebuilds every word from the SEC
 * command field definitions instead, so a typo in either side shows up as
 * a mismatch rather than as a board that does not boot.
 *
 * Host only, and not part of any firmware build. See run.sh.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "imxrt_caam_desc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* SEC command encoding. Command is bits 31:27. */

#define CMD_SHIFT             27
#define CMD_DESC_HDR          (0x16u << CMD_SHIFT)
#define CMD_OPERATION         (0x10u << CMD_SHIFT)
#define CMD_FIFO_STORE        (0x0cu << CMD_SHIFT)
#define CMD_JUMP              (0x14u << CMD_SHIFT)
#define CMD_LOAD              (0x02u << CMD_SHIFT)

#define HDR_ONE               0x00800000u

#define OP_TYPE_SHIFT         24
#define OP_TYPE_CLASS1_ALG    (0x02u << OP_TYPE_SHIFT)
#define OP_ALG_ALGSEL_SHIFT   16
#define OP_ALG_ALGSEL_RNG     (0x50u << OP_ALG_ALGSEL_SHIFT)
#define OP_ALG_AS_SHIFT       2
#define OP_ALG_AS_INIT        (1u << OP_ALG_AS_SHIFT)
#define OP_ALG_PR_ON          0x02u
#define OP_ALG_RNG4_SHIFT     4
#define OP_ALG_RNG4_SK        (0x100u << OP_ALG_RNG4_SHIFT)

#define FIFOST_TYPE_SHIFT     16
#define FIFOST_TYPE_RNGSTORE  (0x34u << FIFOST_TYPE_SHIFT)

#define JUMP_CLASS_SHIFT      25
#define JUMP_CLASS_CLASS1     (1u << JUMP_CLASS_SHIFT)

#define LDST_IMM_SHIFT        23
#define LDST_IMM              (1u << LDST_IMM_SHIFT)
#define LDST_SRCDST_SHIFT     16
#define LDST_SRCDST_WORD_CLRW (0x08u << LDST_SRCDST_SHIFT)

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int g_fail;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void eq(const char *what, uint32_t got, uint32_t want)
{
  if (got != want)
    {
      printf("FAIL %-26s got 0x%08x want 0x%08x\n", what, got, want);
      g_fail++;
    }
  else
    {
      printf("ok   %-26s 0x%08x\n", what, got);
    }
}

static void ok(const char *what, int cond)
{
  if (!cond)
    {
      printf("FAIL %s\n", what);
      g_fail++;
    }
  else
    {
      printf("ok   %s\n", what);
    }
}

/* The generate descriptor, as imxrt_caam_get_random builds it. */

static int rng_desc(uint32_t *d, uint32_t addr, uint32_t size)
{
  d[0] = CAAM_DESC_HDR(4);
  d[1] = CAAM_OP_RNG_GENERATE;
  d[2] = CAAM_FIFO_STORE_RNG | size;
  d[3] = addr;
  return 4;
}

/* The instantiation descriptor, as imxrt_caam_instantiate builds it. */

static int init_desc(uint32_t *d, int gen_sk)
{
  int words = 2;

  d[1] = CAAM_OP_RNG_INIT_SH0;

  if (gen_sk)
    {
      d[2] = CAAM_JUMP_WAIT_CLASS1;
      d[3] = CAAM_LOAD_CLRW;
      d[4] = 1;
      d[5] = CAAM_OP_RNG_GEN_SK;
      words = 6;
    }

  d[0] = CAAM_DESC_HDR(words);
  return words;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(void)
{
  uint32_t d[8];
  int n;

  printf("-- words, against the SEC command encoding\n");

  eq("header, 4 words", CAAM_DESC_HDR(4), CMD_DESC_HDR | HDR_ONE | 4u);
  eq("header, 6 words", CAAM_DESC_HDR(6), CMD_DESC_HDR | HDR_ONE | 6u);

  eq("rng generate", CAAM_OP_RNG_GENERATE,
     CMD_OPERATION | OP_TYPE_CLASS1_ALG | OP_ALG_ALGSEL_RNG | OP_ALG_PR_ON);

  /* State handle zero, so the AAI field contributes nothing. */

  eq("rng instantiate sh0", CAAM_OP_RNG_INIT_SH0,
     CMD_OPERATION | OP_TYPE_CLASS1_ALG | OP_ALG_ALGSEL_RNG |
     OP_ALG_AS_INIT | OP_ALG_PR_ON);

  eq("rng secure keys", CAAM_OP_RNG_GEN_SK,
     CMD_OPERATION | OP_TYPE_CLASS1_ALG | OP_ALG_ALGSEL_RNG |
     OP_ALG_RNG4_SK);

  eq("jump, wait class 1", CAAM_JUMP_WAIT_CLASS1,
     CMD_JUMP | JUMP_CLASS_CLASS1 | 1u);

  eq("load immediate clrw", CAAM_LOAD_CLRW,
     CMD_LOAD | LDST_IMM | LDST_SRCDST_WORD_CLRW | 4u);

  eq("fifo store, rng", CAAM_FIFO_STORE_RNG,
     CMD_FIFO_STORE | FIFOST_TYPE_RNGSTORE);

  printf("-- the generate descriptor\n");

  memset(d, 0xaa, sizeof(d));
  n = rng_desc(d, 0x20240000, 32);

  ok("length field matches the word count",
     (d[0] & CAAM_DESC_LEN_MASK) == (uint32_t)n);
  ok("length fits the field", (uint32_t)n <= CAAM_DESC_LEN_MASK);
  eq("store carries the size", d[2] & 0xffffu, 32);
  eq("store keeps its type", d[2] & ~0xffffu, CAAM_FIFO_STORE_RNG);
  eq("address is the last word", d[3], 0x20240000);
  ok("fits the driver's buffer", n <= 8);

  printf("-- the instantiation descriptor\n");

  memset(d, 0xaa, sizeof(d));
  n = init_desc(d, 0);
  ok("without secure keys it is 2 words", n == 2);
  ok("length field matches", (d[0] & CAAM_DESC_LEN_MASK) == (uint32_t)n);

  memset(d, 0xaa, sizeof(d));
  n = init_desc(d, 1);
  ok("with secure keys it is 6 words", n == 6);
  ok("length field matches", (d[0] & CAAM_DESC_LEN_MASK) == (uint32_t)n);
  ok("the jump precedes the load", d[2] == CAAM_JUMP_WAIT_CLASS1 &&
                                   d[3] == CAAM_LOAD_CLRW);
  eq("the loaded immediate is one", d[4], 1);
  ok("the secure key op is last", d[5] == CAAM_OP_RNG_GEN_SK);
  ok("fits the driver's buffer", n <= 8);

  if (g_fail)
    {
      printf("\n%d check(s) failed\n", g_fail);
      return 1;
    }

  printf("\nall descriptor checks passed\n");
  return 0;
}
