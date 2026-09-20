/*****************************************************************************
 * arch/arm/src/imxrt/imxrt_caam_desc.h
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
 *****************************************************************************/

#ifndef __ARCH_ARM_SRC_IMXRT_IMXRT_CAAM_DESC_H
#define __ARCH_ARM_SRC_IMXRT_IMXRT_CAAM_DESC_H

/*****************************************************************************
 * Pre-processor Definitions
 *****************************************************************************/

/* Job descriptor words, in the SEC command encoding. A descriptor is a
 * little program CAAM's DMA engine fetches and runs, so these are opcodes
 * rather than register values, and they are kept here alone so a host test
 * can check them without the rest of the driver.
 *
 * Command field is bits 31:27, so a header is 0x16, an operation 0x10, a
 * FIFO store 0x0c, a jump 0x14 and a load 0x02.
 */

/* HEADER, with the one-descriptor bit and the length in words. */

#define CAAM_DESC_HDR(len)    (0xb0800000u | (unsigned int)(len))
#define CAAM_DESC_LEN_MASK    0x7f

/* OPERATION, class 1 algorithm, RNG, prediction resistance on. */

#define CAAM_OP_RNG_GENERATE  0x82500002

/* The same, plus AS=INIT, which instantiates state handle zero. */

#define CAAM_OP_RNG_INIT_SH0  0x82500006

/* OPERATION, RNG, generate the secure keys. */

#define CAAM_OP_RNG_GEN_SK    0x82501000

/* JUMP, class 1, to the next descriptor word: waits for the operation. */

#define CAAM_JUMP_WAIT_CLASS1 0xa2000001

/* LOAD immediate, one word, into CLRW, which returns the RNG to idle. */

#define CAAM_LOAD_CLRW        0x10880004

/* FIFO STORE of RNG output; the byte count goes in the low half. */

#define CAAM_FIFO_STORE_RNG   0x60340000

#endif /* __ARCH_ARM_SRC_IMXRT_IMXRT_CAAM_DESC_H */
