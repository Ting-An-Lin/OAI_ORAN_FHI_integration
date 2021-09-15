/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */


#include <stdio.h>
#include "common_lib.h"
#include "ethernet_lib.h"
#include "shared_buffers.h"
#include "low_oran.h"

typedef struct {
  eth_state_t           e;
  shared_buffers        buffers;
  rru_config_msg_type_t last_msg;
  int                   capabilities_sent;
  void                  *oran_priv;
} oran_eth_state_t;


/*This function reads the IQ samples from OAI, symbol by symbol.
 It also handles the shared buffers.                          */

void oran_fh_if4p5_south_in(RU_t *ru,
                               int *frame,
                               int *subframe)
{
  oran_eth_state_t *s = (oran_eth_state_t *) ru->ifdevice.priv;
  PHY_VARS_eNB **eNB_list = ru->eNB_list, *eNB;
  LTE_DL_FRAME_PARMS *fp;
  int symbol;
  int32_t *rxdata;
  int antenna = 0;
printf("Ann: oran_fh_if4p5_south_in\n");
  lock_buffers(&s->buffers, *subframe);
next:
  while (!(s->buffers.ul_busy[*subframe] == 0x3fff ||
           s->buffers.prach_busy[*subframe] == 1))
    wait_buffers(&s->buffers, *subframe);
  if (s->buffers.prach_busy[*subframe] == 1) {
    int i;
    int antenna = 0;
    uint16_t *in;
    uint16_t *out;
    in = (uint16_t *)s->buffers.prach[*subframe];
    out = (uint16_t *)ru->prach_rxsigF[antenna];
    for (i = 0; i < 849; i++)
      out[i] = ntohs(in[i]);
    s->buffers.prach_busy[*subframe] = 0;
    ru->wakeup_prach_eNB(ru->eNB_list[0], ru, *frame, *subframe);
    goto next;
  }
 eNB = eNB_list[0];
  fp  = &eNB->frame_parms;
  for (symbol = 0; symbol < 14; symbol++) {
    int i;
    uint16_t *p = (uint16_t *)(&s->buffers.ul[*subframe][symbol*1200*4]);
    for (i = 0; i < 1200*2; i++) {
      p[i] = htons(p[i]);
    }
    rxdata = &ru->common.rxdataF[antenna][symbol * fp->ofdm_symbol_size];
#if 1
    memcpy(rxdata + 2048 - 600,
           &s->buffers.ul[*subframe][symbol*1200*4],
           600 * 4);
    memcpy(rxdata,
           &s->buffers.ul[*subframe][symbol*1200*4] + 600*4,
           600 * 4);
#endif
  }

  s->buffers.ul_busy[*subframe] = 0;
  signal_buffers(&s->buffers, *subframe);
  unlock_buffers(&s->buffers, *subframe);

  //printf("BENETEL: %s (f.sf %d.%d)\n", __FUNCTION__, *frame, *subframe);

  RU_proc_t *proc = &ru->proc;
  extern uint16_t sf_ahead;
  int f = *frame;
  int sf = *subframe;

  //calculate timestamp_rx, timestamp_tx based on frame and subframe
  proc->tti_rx       = sf;
  proc->frame_rx     = f;
  proc->timestamp_rx = ((proc->frame_rx * 10)  + proc->tti_rx ) * fp->samples_per_tti ;

  if (get_nprocs()<=4) {
    proc->tti_tx   = (sf+sf_ahead)%10;
    proc->frame_tx = (sf>(9-sf_ahead)) ? (f+1)&1023 : f;
  }
}


void oran_fh_if4p5_south_out(RU_t *ru,
                                int frame,
                                int subframe,
                                uint64_t timestamp)
{
  oran_eth_state_t *s = (oran_eth_state_t *) ru->ifdevice.priv;
  PHY_VARS_eNB **eNB_list = ru->eNB_list, *eNB;
  LTE_DL_FRAME_PARMS *fp;
  int symbol;
  int32_t *txdata;
  int aa = 0;
printf("Ann: oran_fh_if4p5_south_out\n");
  //printf("BENETEL: %s (f.sf %d.%d ts %ld)\n", __FUNCTION__, frame, subframe, timestamp);

  lock_buffers(&s->buffers, subframe);
  if (s->buffers.dl_busy[subframe]) {
    printf("%s: fatal: DL buffer busy for subframe %d\n", __FUNCTION__, subframe);
    exit(1);
  }

  eNB = eNB_list[0];
  fp  = &eNB->frame_parms;
  if (ru->num_eNB != 1 || ru->nb_tx != 1 || fp->ofdm_symbol_size != 2048 ||
      fp->Ncp != NORMAL || fp->symbols_per_tti != 14) {
    printf("%s:%d:%s: unsupported configuration\n",
           __FILE__, __LINE__, __FUNCTION__);
    exit(1);
  }

  for (symbol = 0; symbol < 14; symbol++) {
    txdata = &ru->common.txdataF_BF[aa][symbol * fp->ofdm_symbol_size];
#if 1
    memcpy(&s->buffers.dl[subframe][symbol*1200*4],
           txdata + 2048 - 600,
           600 * 4);
    memcpy(&s->buffers.dl[subframe][symbol*1200*4] + 600*4,
           txdata + 1,
           600 * 4);
#endif
    int i;
    uint16_t *p = (uint16_t *)(&s->buffers.dl[subframe][symbol*1200*4]);
    for (i = 0; i < 1200*2; i++) {
      p[i] = htons(p[i]);
    }
  }

  s->buffers.dl_busy[subframe] = 0x3fff;
  unlock_buffers(&s->buffers, subframe);
}

void *get_internal_parameter(char *name)
{
  printf("BENETEL: %s\n", __FUNCTION__);

  if (!strcmp(name, "fh_if4p5_south_in"))
    return (void *) oran_fh_if4p5_south_in;
  if (!strcmp(name, "fh_if4p5_south_out"))
    return (void *) oran_fh_if4p5_south_out;

  return NULL;
}

void init_south(openair0_device *device,
                openair0_config_t *openair0_cfg,
                eth_params_t * eth_params ){
    oran_eth_state_t *eth;

    device->get_internal_parameter = get_internal_parameter;

    eth = (oran_eth_state_t *)calloc(1, sizeof(oran_eth_state_t));
    if (eth == NULL) {
      AssertFatal(0==1, "out of memory\n");
    }

    eth->e.flags = ETH_RAW_IF4p5_MODE;
    eth->e.compression = NO_COMPRESS;
    eth->e.if_name = eth_params->local_if_name;
    device->priv = eth;
    device->openair0_cfg=&openair0_cfg[0];

    eth->last_msg = (rru_config_msg_type_t)-1;

    init_buffers(&eth->buffers);
}